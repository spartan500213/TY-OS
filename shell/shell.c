#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

int main() {
    char *line = NULL;
    size_t len = 0;
    char cwd[1024];
    char *custom_prompt = NULL;
    char *path_env = strdup("/bin:/usr/bin");
    while (1) {
        //if custom prompt null then print cwd
        if (custom_prompt) {
            printf("%s", custom_prompt);
        } else {
            getcwd(cwd, sizeof(cwd));
            printf("%s$ ", cwd);
        }
        fflush(stdout);
        if (getline(&line, &len, stdin) == -1) {
            printf("\n");
            break;
        }
        line[strcspn(line, "\n")] = 0;
        if (strlen(line) == 0) {
            continue;
        }
        if (strcmp(line, "exit") == 0) {
            break;
        }
        //for changing prompt
        if (strncmp(line, "PS1=", 4) == 0) {
            if (strcmp(line + 4, "\\w$") == 0) {
                free(custom_prompt);
                custom_prompt = NULL;
            } else {
                free(custom_prompt);
                custom_prompt = strdup(line + 4);
            }
            continue;
        }
        //cd
        if (strncmp(line, "cd", 2) == 0) {
            char *dir = strtok(line + 2, " ");
            if (dir == NULL) {
                dir = getenv("HOME");
            }
            if (chdir(dir) != 0) {
                perror("cd");
            }
            continue;
        }
        if (strncmp(line, "PATH=", 5) == 0) {
            free(path_env);
            path_env = strdup(line + 5);
            continue;
        }
        char *args[64];
        int arg_count = 0;
        char *infile = NULL;
        char *outfile = NULL;
        int append = 0;
        char *token = strtok(line, " ");
        while (token != NULL) {
            if (strcmp(token, "<") == 0) {
                token = strtok(NULL, " ");
                infile = token;
            } else if (strcmp(token, ">") == 0) {
                token = strtok(NULL, " ");
                outfile = token;
                append = 0;
            } else if (strcmp(token, ">>") == 0) {
                token = strtok(NULL, " ");
                outfile = token;
                append = 1;
            } else {
                args[arg_count] = token;
                arg_count++;
            }
            token = strtok(NULL, " ");
        }
        args[arg_count] = NULL;
        if (arg_count == 0) {
            continue;
        }
        pid_t pid = fork();
        if (pid == 0) {
            if (infile) {
                int fd = open(infile, O_RDONLY);
                if (fd < 0) {
                    perror("open");
                    exit(1);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }
            if (outfile) {
                int fd;
                if (append == 1) {
                    fd = open(outfile, O_WRONLY | O_CREAT | O_APPEND, 0644);
                } else {
                    fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                }
                if (fd < 0) {
                    perror("open");
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }
            char *path_copy = strdup(path_env);
            char *dir = strtok(path_copy, ":");
            struct stat sb;
            int found = 0;
            while (dir != NULL) {
                char full_path[1024];
                snprintf(full_path, sizeof(full_path), "%s/%s", dir, args[0]);
                if (stat(full_path, &sb) == 0) {
                    if (sb.st_mode & S_IXUSR) {
                        execv(full_path, args);
                        perror("execv");
                        found = 1;
                        break;
                    }
                }
                dir = strtok(NULL, ":");
            }
            free(path_copy);
            if (found == 0) {
                fprintf(stderr, "%s: command not found\n", args[0]);
            }
            exit(1);
        } else {
            if (pid > 0) {
                wait(NULL);
            } else {
                perror("fork");
            }
        }
    }
    free(line);
    free(custom_prompt);
    free(path_env);
    return 0;
}
