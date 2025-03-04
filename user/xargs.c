#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

#define MAX_LINE 512
#define MAX_ARGS MAXARG

int string_compare(char* s1, char* s2) {
    while (*s1 && *s1 == *s2) {
        s1++;
        s2++;
    }
    return *s1 - *s2;
}

int string_to_int(char* s) {
    int num = 0;
    while (*s >= '0' && *s <= '9') {
        num = num * 10 + (*s - '0');
        s++;
    }
    return num;
}

int main(int argc, char* argv[]) {
    char* command_args[MAX_ARGS];
    int command_arg_count = 0;
    int batch_size = 0;
    int command_start_index = 1;

    if (argc < 2) {
        printf("Usage: xargs command [args...]\n");
        exit(1);
    }

    if (argc >= 3 && string_compare(argv[1], "-n") == 0) {
        batch_size = string_to_int(argv[2]);
        if (batch_size <= 0) {
            printf("xargs: invalid number: %s\n", argv[2]);
            exit(1);
        }
        command_start_index = 3;
    }

    for (int i = 0; i < argc - command_start_index; i++) {
        command_args[i] = argv[i + command_start_index];
        command_arg_count++;
    }

    char input_line[MAX_LINE];
    char character;
    int input_length = 0;

    while (1) {
        input_length = 0;
        int bytes_read;
        while ((bytes_read = read(0, &character, 1)) > 0) {
            if (character == '\n') break;
            if (input_length < MAX_LINE - 1) {
                input_line[input_length++] = character;
            }
        }
        input_line[input_length] = '\0';

        if (bytes_read == 0 && input_length == 0) {
            break;
        }
        if (input_length == 0) continue;

        char* token_start = input_line;
        int token_index = 0;
        char* tokens[MAX_ARGS];
        int token_count = 0;

        while (token_index < input_length) {
            while (token_index < input_length && (input_line[token_index] == ' ' || input_line[token_index] == '\t')) {
                input_line[token_index] = '\0';
                token_index++;
            }
            if (token_index >= input_length) break;
            token_start = input_line + token_index;
            tokens[token_count++] = token_start;
            while (token_index < input_length && input_line[token_index] != ' ' && input_line[token_index] != '\t') {
                token_index++;
            }
        }

        if (token_count == 0) continue;

        if (batch_size > 0) {
            for (int i = 0; i < token_count; i += batch_size) {
                int pid = fork();
                if (pid < 0) {
                    printf("xargs: fork failed\n");
                    exit(1);
                }
                if (pid == 0) {
                    for (int j = 0; j < command_arg_count; j++) {
                        command_args[j] = argv[j + command_start_index];
                    }
                    int tokens_to_add = (i + batch_size < token_count) ? batch_size : (token_count - i);
                    for (int j = 0; j < tokens_to_add; j++) {
                        command_args[command_arg_count + j] = tokens[i + j];
                    }
                    command_args[command_arg_count + tokens_to_add] = 0;
                    exec(command_args[0], command_args);
                    printf("xargs: exec %s failed\n", command_args[0]);
                    exit(1);
                } else {
                    wait(0);
                }
            }
        } else {
            for (int i = 0; i < token_count; i++) {
                if (command_arg_count + i < MAX_ARGS - 1) {
                    command_args[command_arg_count + i] = tokens[i];
                }
            }
            command_args[command_arg_count + token_count] = 0;
            int pid = fork();
            if (pid < 0) {
                printf("xargs: fork failed\n");
                exit(1);
            }
            if (pid == 0) {
                exec(command_args[0], command_args);
                printf("xargs: exec %s failed\n", command_args[0]);
                exit(1);
            } else {
                wait(0);
            }
        }
    }

    exit(0);
}
