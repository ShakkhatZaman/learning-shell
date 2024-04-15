#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <stdbool.h>

#define BUFFER_SIZE 512
#define SHELL_TOKENS 128
#define SHELL_COMMANDS 2
#define TOKEN_DELIMITERS " \n\t\a\r"

#define SKIP_WHITESPACE(p_c) while (*p_c == ' ' || *p_c == '\t' || *p_c == '\n') p_c++

bool run_shell = true;
char *__current_directory = NULL;
char *__temp_token_pos = NULL;

static void shell_init(void);
static void shell_loop(void);
static bool run_command(char **token);
static bool launch_process(char *args[]);
static char **parse_tokens(char *line, int *p_num_tokens);
static char *_shell_read_line(unsigned int *chars);
static char *_get_token(char *buffer);
int _shell_exit(char **args);
int _shell_pwd(char **args);

char *shell_commands[SHELL_COMMANDS] = {
    "exit", "pwd"
};

int (*shell_functions[SHELL_COMMANDS])(char **) = {
    _shell_exit, _shell_pwd
};


int main(int argc, char *argv[]) {
    shell_loop();
    return 0;
}

static void shell_loop(void) {
    int num_tokens;
    unsigned int num_chars = 0;
    bool ran_command = false, ran_process = false;
    shell_init();

    while(run_shell) {
        printf(">> ");
        char *line = _shell_read_line(&num_chars);
        char *striped_line = line;
        SKIP_WHITESPACE(striped_line);
        if (strlen(striped_line) < 1) {
            free(line);
            continue;
        }

        char buffer[num_chars + 1];
        memset(buffer, 0, num_chars + 1);
        strncpy(buffer, line, num_chars);

        char **tokens = parse_tokens(buffer, &num_tokens);
        ran_command = run_command(tokens);

        if (!ran_command) {
            ran_process = launch_process(&striped_line);
            if (!ran_process) fprintf(stderr, "Unknown file or cmd \"%s\"\n", tokens[0]);
        } 
        free(tokens);
        free(line);
    }
}

static bool run_command(char **tokens) {
    for (int i = 0; i < SHELL_COMMANDS; i++) {
        if (!strcmp(tokens[0], shell_commands[i])) {
            bool status = shell_functions[i](tokens);
            return status;
        }
    }
    return false;
}

static bool launch_process(char *args[]) {
    STARTUPINFO startup_info;
    PROCESS_INFORMATION process_info;

    ZeroMemory(&startup_info, sizeof(startup_info));
    ZeroMemory(&process_info, sizeof(process_info));
    startup_info.cb = sizeof(startup_info);

    BOOL status = CreateProcess(NULL, *args,
                                NULL, NULL, FALSE,
                                0, NULL, NULL,
                                &startup_info, &process_info);
    if (!status) {
        printf("Process creation failed. Error: (%d)\n", GetLastError());
        return 0;
    }

    WaitForSingleObject(process_info.hProcess, INFINITE);

    CloseHandle(process_info.hProcess);
    CloseHandle(process_info.hThread);
    return 1;
}

static void shell_init(void) {
    int num_chars = GetCurrentDirectory(0, NULL);
    __current_directory = calloc(num_chars, 1);
    GetCurrentDirectory(num_chars, __current_directory);
}

// Parse a line feed and return an array of tokens (char **)
// Tokens are recieved from _get_token function
static char **parse_tokens(char *line, int *p_num_tokens) {
    unsigned int token_index = 0, token_buffer_size = SHELL_TOKENS;
    char **tokens = calloc(token_buffer_size, sizeof(char *));
    if (!tokens) {
        fprintf(stderr, "Error Allocating space for tokens");
        exit(1);
    }
    tokens[token_index] = _get_token(line);
    while (tokens[token_index]) {
        token_index++;
        if (!(token_index < token_buffer_size)) {
            tokens = realloc(tokens, (token_buffer_size + SHELL_TOKENS) * sizeof(char *));
            if (!tokens) {
                fprintf(stderr, "Error reallocating space for tokens");
                exit(1);
            }
            token_buffer_size += SHELL_TOKENS;
        }
        tokens[token_index] = _get_token(NULL);
    }
    *p_num_tokens = token_index;
    return tokens;
}

// Reads in a line feed from user and returns it as char *
static char *_shell_read_line(unsigned int *chars) {
    int c;
    unsigned int num_c = 0, buffer_size = BUFFER_SIZE;
    char *buffer = calloc(1, BUFFER_SIZE);
    if (!buffer) {
        fprintf(stderr, "Error unable to allocate space for buffer");
        exit(1);
    }

    while((c = getchar()) != EOF && c != '\n') {
        buffer[num_c++] = c;
        if (!(num_c < buffer_size)) {
            buffer = realloc(buffer, buffer_size + BUFFER_SIZE);
            if (!buffer) {
                fprintf(stderr, "Error unable to reallocate space for buffer");
                exit(1);
            }
            buffer_size += BUFFER_SIZE;
        }
    }
    buffer[num_c] = '\0';
    *chars = num_c;
    return buffer;
}

// Modifies the original line buffer and returns a token as char *
// puts null bytes where the tokens are separated
// each call returns a token from the buffer provided
// returns another token from previous buffer if buffer == NULL
static char *_get_token(char *buffer) {
    if (!buffer && !__temp_token_pos) return NULL;
    if (buffer) __temp_token_pos = buffer;
    SKIP_WHITESPACE(__temp_token_pos);

    char *p_left = __temp_token_pos;
    int c = *__temp_token_pos;
    int num_double_quotes = 0, num_single_quotes = 0;

    while (c != EOF) {
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            if (!(num_single_quotes || num_double_quotes)) break;
        }
        else if (c == '\'') num_single_quotes += (num_single_quotes) ? -1 : 1;
        else if (c == '"') num_double_quotes += (num_double_quotes) ? -1 : 1;
        else if (c == '\0') {
            __temp_token_pos = NULL;
            return (*p_left) ? p_left : NULL;
        }
        c = *(++__temp_token_pos);
    }
    *__temp_token_pos = '\0';
    __temp_token_pos++;
    return p_left;
}

int _shell_exit(char **args) {
    run_shell = false;
    free(__current_directory);
    return 1;
}

int _shell_pwd(char **args) {
    printf("%s\n", __current_directory);
    return 1;
}
