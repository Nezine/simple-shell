#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    /* Flush after every printf */
    setbuf(stdout, NULL);

    /* TODO: Uncomment the code below to pass the first stage */
    while (1) {
        printf("$ ");
        char command[1024];

        if (fgets(command, sizeof(command), stdin)) {
            command[strcspn(command, "\n")] = '\0';

            if (strcmp(command, "exit") == 0) {
                break;
            } else if (strncmp(command, "echo ", 5) == 0) {
                printf("%s\n", command + 5);
            } else if (strncmp(command, "type ", 5) == 0) {
                char *name = command + 5;

                if (strcmp(name, "type") == 0 ||
                    strcmp(name, "echo") == 0 ||
                    strcmp(name, "exit") == 0) {
                    printf("%s is a shell builtin\n", name);
                    continue;
                }

                char *path_env = getenv("PATH");
                if (!path_env) {
                    printf("%s not found\n", name);
                    continue;
                }

                char pathcopy[4096];
                strncpy(pathcopy, path_env, sizeof(pathcopy) - 1);
                pathcopy[sizeof(pathcopy) - 1] = '\0';

                int found = 0;
                char *dir = strtok(pathcopy, ":");
                while (dir != NULL) {
                    char candidate[1024];
                    snprintf(candidate, sizeof candidate, "%s/%s", dir, name);
                    if (access(candidate, F_OK) == 0) {
                        if (access(candidate, X_OK) == 0) {
                            printf("%s is %s\n", name, candidate);
                            found = 1;
                            break;
                        }
                    }
                    dir = strtok(NULL, ":");
                }

                if (!found) {
                    printf("%s: not found\n", name);
                }
            } else {
                printf("%s: command not found\n", command);
            }
        }
    }

    return 0;
}
