#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <signal.h>

#define MAX_HISTORY_SIZE 10000
#define MAX_LINE_LENGTH 2048

struct cmd {
    char *name;
    char *wholeCommand;
    pid_t pid;
    struct timespec start_time;
    struct timespec end_time;
    long long total_duration_ms; // Total duration in milliseconds
};

extern struct cmd command_history[MAX_HISTORY_SIZE];
extern int count;

long long diff_in_ms(struct timespec *start, struct timespec *end);

char *read_user_input();
char **split_on_whiteSpaces(char *line);
char **split_on_pipeSymbol(char *line);

int numCommands(char **commands);
pid_t execute_without_pipes(char **commands, int background);
pid_t launch_without_pipes(char **commands);
pid_t execute_with_pipes(int argc, char **argv);
pid_t launch_with_pipes(char **commands);
void shell_loop();
void print_command_history();
void my_handler(int signum);
void clean_up(char *line, char **commands);
void shell_execute_script(const char *script_filename);
#endif
