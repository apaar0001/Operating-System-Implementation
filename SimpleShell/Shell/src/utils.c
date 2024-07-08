#include "shell.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

// Implementing the function to read user input from the command prompt
char *read_user_input() {
    char *line = NULL;
    size_t bufsize = 0;

    // Using getline to read a line from the command prompt
    if (getline(&line, &bufsize, stdin) == -1) {
        if (feof(stdin)) {
            printf("Reached the End of file\n");
            exit(EXIT_SUCCESS);
        } else {
            perror("Error in reading from the command prompt\n");
            exit(EXIT_FAILURE);
        }
    }
    return line;
}

// Implementing the function to calculate the time difference between two timespec structs in milliseconds
long long diff_in_ms(struct timespec *start, struct timespec *end) {
    return (end->tv_sec - start->tv_sec) * 1000LL + (end->tv_nsec - start->tv_nsec) / 1000000LL;
}

// Implementing the function to split a line into individual tokens based on whitespace characters
char **split_on_whiteSpaces(char *line) {
    int bufsz = 64;
    int index = 0;
    char **tokens = malloc(bufsz * sizeof(char *));
    char *token;

    if (tokens == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    
    // Using strtok to split the line into tokens based on space and newline characters
    token = strtok(line, " \n");
    while (token != NULL) {
        tokens[index++] = strdup(token);
        token = strtok(NULL, " \n");
    }
    tokens[index] = NULL;

    return tokens;
}

// Implementing the function to split a line into individual tokens based on the pipe symbol (|)
char **split_on_pipeSymbol(char *line) {
    int bufsz = 1024;
    int index = 0;
    char **tokens = malloc(bufsz * sizeof(char *));
    char *token;

    if (tokens == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    
    // Using the strtok to split the line into tokens based on the pipe symbol (|)
    token = strtok(line, "|");
    while (token != NULL) {
        tokens[index++] = strdup(token);
        token = strtok(NULL, "|");
    }
    tokens[index] = NULL;

    return tokens;
}

// Implementing the function to count the number of commands in an array of tokens
int numCommands(char **commands) {
    if (commands == NULL) {
        printf("There are no commands to execute, please enter a valid command");
        return 0;
    }
    int num_commands = 0;

    for (int i = 0; commands[i] != NULL; i++) {
        num_commands++;
    }
    return num_commands;
}

// Implementing the signal handler function for handling Ctrl+C (SIGINT) signals
void my_handler(int signum) {
    if (signum == SIGINT) {
        printf("\nTerminating the shell...\n");
        print_command_history();
        exit(EXIT_SUCCESS);
    }
}

void print_command_history() {
    printf("\nCommand History:\n");
    for (int i = 0; i < count; i++) {
        printf("[%d]:\n", i + 1);
        printf("Command: %s\n", command_history[i].wholeCommand);
        printf("PID: %d\n", command_history[i].pid);

        // Format the start time in a human-readable format
        char start_time_formatted[20]; // Assumes a buffer of sufficient size
        strftime(start_time_formatted, sizeof(start_time_formatted), "%Y-%m-%d %H:%M:%S", localtime(&command_history[i].start_time.tv_sec));
        printf("Start Time: %s.%09ld\n", start_time_formatted, command_history[i].start_time.tv_nsec);

        // Format the total duration in milliseconds
        printf("Total Duration: %lld milliseconds\n\n", command_history[i].total_duration_ms);
    }
}
