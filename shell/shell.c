#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_ARGS 64
#define PROMPT_CWD 0
#define PROMPT_CUSTOM 1

typedef struct {
    char *prompt_string;
    int prompt_style;
    char *path_variable;
} shell_state;

typedef struct {
    char **arguments;
    int arg_count;
    char *input_file;
    char *output_file;
    int append_output;
} command;

void initialize_shell(shell_state *state);
void cleanup_shell(shell_state *state);
void run_main_loop(shell_state *state);
void show_prompt(const shell_state *state);
char *get_user_input();
int process_line(char *line, shell_state *state);
command* create_command(char *line);
void destroy_command(command *cmd);
void execute_command(command *cmd, const shell_state *state);
int execute_builtin(const command *cmd);
void execute_external(const command *cmd, const shell_state *state);
int handle_prompt_assignment(char *line, shell_state *state);
int handle_path_assignment(char *line, shell_state *state);
void parse_redirections(command *cmd);
char *locate_executable(const char *cmd_name, const shell_state *state);
void setup_redirections(const command *cmd);

int main() {
    shell_state state;
    initialize_shell(&state);
    run_main_loop(&state);
    cleanup_shell(&state);
    return 0;
}

void initialize_shell(shell_state *state) {
    state->prompt_string = NULL;
    state->prompt_style = PROMPT_CWD;
    char *initial_path = getenv("PATH");
    state->path_variable = strdup(initial_path ? initial_path : "/bin:/usr/bin");
}

void cleanup_shell(shell_state *state) {
    free(state->prompt_string);
    free(state->path_variable);
}

void run_main_loop(shell_state *state) {
    char *line;
    int running = 1;
    while (running) {
        show_prompt(state);
        line = get_user_input();
        if (!line) {
            printf("exit\n");
            break;
        }
        if (*line) {
            add_history(line);
        }
        running = process_line(line, state);
        free(line);
    }
}

void show_prompt(const shell_state *state) {
    if (state->prompt_style == PROMPT_CUSTOM) {
        printf("%s", state->prompt_string);
    } else {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd))) {
            printf("[%s]$ ", cwd);
        } else {
            perror("getcwd");
            printf("$ ");
        }
    }
    fflush(stdout);
}

char *get_user_input() {
    return readline("");
}

int process_line(char *line, shell_state *state) {
    if (handle_prompt_assignment(line, state) || handle_path_assignment(line, state)) {
        return 1;
    }

    command *cmd = create_command(line);
    if (cmd->arg_count > 0) {
        if (strcmp(cmd->arguments[0], "exit") == 0) {
            destroy_command(cmd);
            return 0;
        }
        execute_command(cmd, state);
    }
    destroy_command(cmd);
    return 1;
}

command* create_command(char *line) {
    command *cmd = malloc(sizeof(command));
    cmd->arguments = malloc(MAX_ARGS * sizeof(char*));
    cmd->input_file = NULL;
    cmd->output_file = NULL;
    cmd->append_output = 0;

    int i = 0;
    char *token = strtok(line, " \t\r\n\a");
    while (token && i < MAX_ARGS - 1) {
        cmd->arguments[i++] = token;
        token = strtok(NULL, " \t\r\n\a");
    }
    cmd->arguments[i] = NULL;
    cmd->arg_count = i;

    parse_redirections(cmd);
    return cmd;
}

void destroy_command(command *cmd) {
    free(cmd->arguments);
    free(cmd);
}

void execute_command(command *cmd, const shell_state *state) {
    if (!execute_builtin(cmd)) {
        execute_external(cmd, state);
    }
}

int execute_builtin(const command *cmd) {
    if (strcmp(cmd->arguments[0], "cd") == 0) {
        char *directory_to_change_to;

        if (cmd->arg_count < 2) {
            directory_to_change_to = getenv("HOME");
            if (directory_to_change_to == NULL) {
                fprintf(stderr, "shell: cd: HOME not set\n");
                return 1;
            }
        } else {
            directory_to_change_to = cmd->arguments[1];
        }

        if (chdir(directory_to_change_to) != 0) {
            perror("shell");
        }
        return 1;
    }
    return 0;
}

void execute_external(const command *cmd, const shell_state *state) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("shell: fork failed");
        return;
    }

    if (pid == 0) {
        setup_redirections(cmd);
        char *exec_path = locate_executable(cmd->arguments[0], state);
        if (exec_path) {
            execv(exec_path, cmd->arguments);
            perror("shell: execv failed");
            free(exec_path);
        } else {
            fprintf(stderr, "shell: command not found: %s\n", cmd->arguments[0]);
        }
        exit(EXIT_FAILURE);
    } else {
        int status;
        waitpid(pid, &status, WUNTRACED);
    }
}

int handle_prompt_assignment(char *line, shell_state *state) {
    if (strncmp(line, "PS1=", 4) != 0) return 0;

    char *val = line + 4;
    free(state->prompt_string);
    state->prompt_string = NULL;

    if (strcmp(val, "\\w$") == 0) {
        state->prompt_style = PROMPT_CWD;
    } else {
        state->prompt_style = PROMPT_CUSTOM;
        if (val[0] == '"' && val[strlen(val) - 1] == '"') {
            val[strlen(val) - 1] = '\0';
            state->prompt_string = strdup(val + 1);
        } else {
            state->prompt_string = strdup(val);
        }
    }
    return 1;
}

int handle_path_assignment(char *line, shell_state *state) {
    if (strncmp(line, "PATH=", 5) != 0) return 0;
    free(state->path_variable);
    state->path_variable = strdup(line + 5);
    return 1;
}

void parse_redirections(command *cmd) {
    for (int i = 0; i < cmd->arg_count; ++i) {
        if (!cmd->arguments[i]) continue;
        int is_redirect = 0;
        if (strcmp(cmd->arguments[i], "<") == 0) {
            is_redirect = 1;
            cmd->input_file = cmd->arguments[i + 1];
        } else if (strcmp(cmd->arguments[i], ">") == 0) {
            is_redirect = 1;
            cmd->output_file = cmd->arguments[i + 1];
            cmd->append_output = 0;
        } else if (strcmp(cmd->arguments[i], ">>") == 0) {
            is_redirect = 1;
            cmd->output_file = cmd->arguments[i + 1];
            cmd->append_output = 1;
        }

        if (is_redirect) {
            if (i + 1 >= cmd->arg_count) {
                fprintf(stderr, "shell: syntax error near unexpected token `newline'\n");
                return;
            }
            cmd->arguments[i] = NULL;
            cmd->arguments[i + 1] = NULL;
        }
    }
}

char *locate_executable(const char *cmd_name, const shell_state *state) {
    if (strchr(cmd_name, '/')) {
        return (access(cmd_name, X_OK) == 0) ? strdup(cmd_name) : NULL;
    }

    char *path_copy = strdup(state->path_variable);
    if (!path_copy) return NULL;

    char full_path[1024];
    char *path = strtok(path_copy, ":");
    char *found_path = NULL;
    while (path) {
        snprintf(full_path, sizeof(full_path), "%s/%s", path, cmd_name);
        if (access(full_path, X_OK) == 0) {
            found_path = strdup(full_path);
            break;
        }
        path = strtok(NULL, ":");
    }

    free(path_copy);
    return found_path;
}

void setup_redirections(const command *cmd) {
    if (cmd->input_file) {
        int fd_in = open(cmd->input_file, O_RDONLY);
        if (fd_in == -1 || dup2(fd_in, STDIN_FILENO) == -1) {
            perror("shell: input redirection failed");
            exit(EXIT_FAILURE);
        }
        close(fd_in);
    }
    if (cmd->output_file) {
        int flags = O_WRONLY | O_CREAT;
        flags |= cmd->append_output ? O_APPEND : O_TRUNC;
        int fd_out = open(cmd->output_file, flags, 0644);
        if (fd_out == -1 || dup2(fd_out, STDOUT_FILENO) == -1) {
            perror("shell: output redirection failed");
            exit(EXIT_FAILURE);
        }
        close(fd_out);
    }
}
