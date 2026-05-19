#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  // Flush after every printf
  setbuf(stdout, NULL);

  // TODO: Uncomment the code below to pass the first stage
  while (1){
    printf("$ ");
    char command[1024];
    if (fgets(command, sizeof(command), stdin)) {
      command[strcspn(command, "\n")] = '\0';
      if(strcmp(command, "exit") == 0){
        break;
      }
      else if(strncmp(command, "echo ", 5) == 0){
        printf("%s\n", command + 5);
      }
      else if(strncmp(command, "type ", 5) == 0){
        if(strncmp(command + 5, "type", 4) == 0 || strncmp(command + 5, "echo", 4) == 0 || strncmp(command + 5, "exit", 4) == 0){
          printf("%s is a shell builtin\n", command + 5);
        }
        else printf("%s: not found\n", command + 5);
      }
      else printf("%s: command not found\n", command);
      }
  }
  return 0;
}
