#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 512
#define SHELL_TOKENS 128
#define TOKEN_DELIMITERS " \n\t\a\r"

#define SKIP_WHITESPACE(p_c) while (*p_c == ' ' || *p_c == '\t' || *p_c == '\n') p_c++

char *__temp_token_pos = NULL;

static void shell_loop(void);
static char **parse_tokens(char *line, int *p_num_tokens);
static char *_shell_read_line(void);
static char *_get_token(char *buffer);

int main(int argc, char *argv[]) {
    shell_loop();
    return 0;
}

static void shell_loop(void) {
    int num_tokens;
    printf(">> ");
    char *line = _shell_read_line();
    char **tokens = parse_tokens(line, &num_tokens);

    free(line);
    free(tokens);
}

static char **parse_tokens(char *line, int *p_num_tokens) {
    unsigned int token_index = 0, token_buffer_size = SHELL_TOKENS;
    char **tokens = calloc(token_buffer_size, sizeof(char *));
    if (!tokens) {
        fprintf(stderr, "Error Allocating space for tokens");
        exit(1);
    }
    // tokens[token_index] = strtok(line, TOKEN_DELIMITERS);
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
        // tokens[token_index] = strtok(NULL, TOKEN_DELIMITERS);
        tokens[token_index] = _get_token(NULL);
    }
    *p_num_tokens = token_index;
    for (int i = 0; i < token_index; i++) printf("[\"%s\"] ", tokens[i]);
    printf("\n");
    printf("%d\n", token_index);
    return tokens;
}

static char *_shell_read_line(void) {
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
    return buffer;
}

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
