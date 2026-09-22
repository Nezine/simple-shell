#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#ifndef _WIN32
#include <sys/wait.h>
#endif
#include <errno.h>


#define LINE_BUF 1024
#define PATH_COPY 4096
#define MAX_ARGS 64

/* Check if `name` is an exact builtin */
static int is_builtin(const char *name) {
    return (strcmp(name, "exit") == 0 || strcmp(name, "echo") == 0 || strcmp(name, "type") == 0);
}

/* Search PATH for an executable named `name`.
 * If found, copy full path into `out` and return 1. Otherwise return 0.
 */
static int find_in_path(const char *name, char *out, size_t out_size) {
    if (strchr(name, '/')) {
        // name contains a slash; treat as path
        if (access(name, F_OK) == 0 && access(name, X_OK) == 0) {
            strncpy(out, name, out_size - 1);
            out[out_size - 1] = '\0';
            return 1;
        }
        return 0;
    }

    char *path_env = getenv("PATH");
    if (!path_env) return 0;

    char pathcopy[PATH_COPY];
    strncpy(pathcopy, path_env, sizeof(pathcopy) - 1);
    pathcopy[sizeof(pathcopy) - 1] = '\0';

    char *dir = strtok(pathcopy, ":");
    while (dir) {
        char candidate[LINE_BUF];
        snprintf(candidate, sizeof(candidate), "%s/%s", dir, name);
        if (access(candidate, F_OK) == 0 && access(candidate, X_OK) == 0) {
            strncpy(out, candidate, out_size - 1);
            out[out_size - 1] = '\0';
            return 1;
        }
        dir = strtok(NULL, ":");
    }

    return 0;
}

/* Split `line` into argv-style array. This function modifies `line`.
 * Returns argc (number of args) and fills `argv` with pointers into `line`.
 */
static int split_args(char *line, char **argv, int max_args) {
    int argc = 0;
    char *tok = strtok(line, " ");
    while (tok && argc < max_args - 1) {
        argv[argc++] = tok;
        tok = strtok(NULL, " ");
    }
    argv[argc] = NULL;
    return argc;
}

int main(void) {
    setbuf(stdout, NULL);
    char line[LINE_BUF];

    while (1) {
        printf("$ ");
        if (!fgets(line, sizeof(line), stdin)) break; // EOF
        line[strcspn(line, "\n")] = '\0'; // remove newline

        // skip empty lines
        if (line[0] == '\0') continue;

        // builtins: exit and echo are simple
        if (strcmp(line, "exit") == 0) break;

        if (strncmp(line, "echo ", 5) == 0) {
            printf("%s\n", line + 5);
            continue;
        }

        if (strncmp(line, "type ", 5) == 0) {
            char *name = line + 5;
            while (*name == ' ') name++; // skip leading spaces
            if (*name == '\0') continue;

            if (is_builtin(name)) {
                printf("%s is a shell builtin\n", name);
                continue;
            }

            char full[LINE_BUF];
            if (find_in_path(name, full, sizeof(full))) {
                printf("%s is %s\n", name, full);
            } else {
                printf("%s: not found\n", name);
            }
            continue;
        }

        /* Not a builtin: try to run external program */
        {
            // make a writable copy because split_args uses strtok
            char work[LINE_BUF];
            strncpy(work, line, sizeof(work) - 1);
            work[sizeof(work) - 1] = '\0';

            char *argv[MAX_ARGS];
            int argc = split_args(work, argv, MAX_ARGS);
            if (argc == 0) continue; // nothing to run

            char fullpath[LINE_BUF] = "";
            if (!find_in_path(argv[0], fullpath, sizeof(fullpath))) {
                printf("%s: not found\n", argv[0]);
                continue;
            }

            pid_t pid = fork();
            if (pid < 0) {
                perror("fork");
                continue;
            }

            if (pid == 0) {
                // child: execute
                execv(fullpath, argv);
                // if execv returns, it failed
                fprintf(stderr, "exec failed for %s: %s\n", fullpath, strerror(errno));
                exit(1);
            } else {
                // parent: wait for child to finish
                int status = 0;
                if (waitpid(pid, &status, 0) == -1) {
                    perror("waitpid");
                }
            }
        }
    }

    return 0;
}
