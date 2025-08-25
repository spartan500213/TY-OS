#include "stdio.h"
#include "unistd.h"
#include "fcntl.h"
#include "sys/wait.h"
#include "string.h"
#include "stdlib.h"
#include <readline/readline.h>
#include <readline/history.h>

int main(int argc, char const *argv[])
{
    int p;
    char cmd[512];
    char *prompt = (char *)NULL; // points to either pwd or the user entered prompt
    char pwd[512];               // pwd
    char user_prompt[512];       // custom prompt instead of pwd
    char *line = (char *) NULL;
    getcwd(pwd, 512);            // initially storing pwd
    prompt = pwd;

    char *path = getenv("PATH");
    char MY_PATH[512];
    strncpy(MY_PATH, path, sizeof(path) - 1);
    MY_PATH[sizeof(path) - 1] = '\0';
    while(1){
        line = readline(prompt);
        if (line == NULL || strncmp(line, "exit", 4) == 0) {
            free(line);
            printf("Exiting gracefully ;)\n");
            return 0;
        }
        else if(strncmp(line, "PS1=", 4) == 0){
            int i;
            if(line[4] == '\\' && line[5] == 'w' && line[6] == '$'){
                prompt = pwd;
            }
            else {
                for(i = 4; line[i] != '\0'; i++){
                    user_prompt[i - 4] = line[i];
                }
                user_prompt[i-4] = '\0';
                prompt = user_prompt;
            }
        } 
        else if (strncmp(line, "cd", 2) == 0) {
            char *token = strtok(line, " "); 
            token = strtok(NULL, " ");   

            if (token == NULL) {
                printf("Bad Argument : Provide pathname");
            } else {
                if (chdir(token) == 0) {
                    getcwd(pwd, sizeof(pwd));
                    prompt = pwd;
                } else {
                    perror("cd");
                }
            }
        }
        else if (strncmp(line, "PATH=", 5) == 0){
            
        }
        else {
            p = fork();
            if (p == 0) {
                char *newargv[128];
                int i = 0;

                newargv[i] = strtok(line, " ");
                while (newargv[i] != NULL) {
                    i++;
                    newargv[i] = strtok(NULL, " ");
                }

                execv(newargv[0], newargv);
                if (path) {
                    char *path_copy = strdup(path);
                    char *dir = strtok(path_copy, ":");
                    char fullpath[512];

                    while (dir) {
                        snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, newargv[0]);
                        if (access(fullpath, X_OK) == 0) {
                            execv(fullpath, newargv);
                        }
                        dir = strtok(NULL, ":");
                    }
                    free(path_copy);
                }
                perror("Bad command : ");
                exit(EXIT_FAILURE);
            } else if (p > 0) {
                wait(NULL);
            } else {
                perror("fork");
            }
        }
    }
    return 0;
}
