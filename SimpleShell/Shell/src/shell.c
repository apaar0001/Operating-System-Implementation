#include "shell.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

//Defining a structure to store command history
struct cmd command_history[MAX_HISTORY_SIZE];
int count = 0; // Counter required for command history

//Implementing a function to clean up memory
void clean_up(char *line, char **commands) {
    if (line != NULL) {
        free(line);
    }
    if (commands != NULL) {
        for (int i = 0; commands[i] != NULL; i++) {
            free(commands[i]);
        }
        free(commands);
    }
}

// Implementing a function to execute a single command without pipes
pid_t execute_without_pipes(char **commands, int background) {
    pid_t pid;
    int status;

    if (!background) {
        // Creating fork to initialize a child process to execute the command
        pid = fork();
        if (pid == 0) {
            // Child process
            // Execute the command
            if (execvp(commands[0], commands) == -1) {
                perror("Child process execution failed");
                exit(EXIT_FAILURE);
            }
        } else if (pid < 0) {  // Condition to check fork failure
            perror("Fork failure");
            exit(0);
        } else {
            // Parent process
            // Waiting  for the child process to complete
            do {
                waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }
    } else {
        // Implementing the background execution
        // Initialising the fork for child process and redirect output to /dev/null
        pid = fork();
        if (pid == 0) {
            freopen("/dev/null", "w", stdout);
            // Executing the command
            if (execvp(commands[0], commands) == -1) {
                perror("Child process execution failed");
                exit(EXIT_FAILURE);
            }
        } else if (pid < 0) { // Condition to check fork failure
            perror("Fork failure");
            exit(0);
        } else {
            // Printing the information for background process
            printf("The process with pid %d is now running in the background...\n", pid);
        }
    }
    return pid;
}

// Implementing the function to launch a command without pipes
pid_t launch_without_pipes(char **commands) {
    pid_t child_pid;
    if (commands[0] == NULL) {
        // Handling the empty command
        printf("An empty command has been entered\n");
        return -1;
    }
    int is_background = 0;
    int num_commands = numCommands(commands);
    if (num_commands > 0) {
        // Checking the case if the command is to be run in the background
        if (strcmp(commands[num_commands - 1], "&") == 0) {
            is_background = 1;
            commands[num_commands - 1] = NULL;
        }
    }

    if (strcmp(commands[0], "cd") == 0) {
        // Implementing the 'cd' command
        if (commands[1] != NULL) {
            // Changing the directory to the specified directory
            if (chdir(commands[1]) != 0) {
                perror("cd command failed");
            }
        } else {
            // Change to the home directory(directory for the current session) if no directory specified
            char *home = getenv("PWD");
            if (home != NULL) {
                if (chdir(home) != 0) {
                    perror("cd command failed");
                }
            } else {
                printf("PWD environment variable not set\n");
            }
            child_pid = getpid();
        }
    } else {
        // Executing the command
        child_pid = execute_without_pipes(commands, is_background);
    }
    return child_pid;
}

// Implementing the function for executing multiple commands with pipes
pid_t execute_with_pipes(int argc, char **argv) {
    int status;
    int stand_in = 0;
    char ***cmd;
    cmd = malloc(argc * sizeof(char **));

    if (cmd == NULL) {
        perror("Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    pid_t pids[argc];

    for (int i = 0; i < argc; i++) {
        // Spliting each command into arguments
        cmd[i] = split_on_whiteSpaces(argv[i]);

        if (cmd[i] == NULL) {
            perror("Null command occurred which is not possible\n");
            exit(EXIT_FAILURE);
        }
    }
    for (int i = 0; i < argc; i++) {
        int fd[2];
        pipe(fd);

        pid_t child_pid = fork();

        if (child_pid == 0) {
            if (i != 0) {
                // Redirecting the input from the previous command's output
                dup2(stand_in, 0);
            }
            if (i != argc - 1) {
                // Redirecting the output to the next command's input
                dup2(fd[1], 1);
            }
            close(fd[0]);
            // Executing the command
            execvp(cmd[i][0], cmd[i]);
            perror("Command execution failed");
            exit(EXIT_FAILURE);
        } else if (child_pid < 0) {
            perror("Fork failure");
            exit(EXIT_FAILURE);
        } else {
            pids[i] = child_pid;
            close(fd[1]);
            stand_in = fd[0];
        }
    }
    for (int i = 0; i < argc; i++) {
        // Waiting for each command to be completed
        waitpid(pids[i], &status, 0);
    }
    for (int i = 0; i < argc; i++) {
        free(cmd[i]);
    }
    free(cmd);

    return pids[0];
}

// Implementing the function to launch commands with pipes
pid_t launch_with_pipes(char **commands) {
    pid_t child_pid = -1;
    if (commands[0] == NULL) {
        // Handling the case of empty command
        printf("An empty command has been entered\n");
    }
    int num_commands = numCommands(commands);

    if (num_commands == 0) {
        // Case if there is no command to execute
        printf("There are no commands to execute, please enter a valid command\n");
        return -1;
    }

    child_pid = execute_with_pipes(num_commands, commands);
    if(child_pid == 0){
        printf("Error in the launch\n");
    }

    return child_pid;
}

// Implementing the function to execute commands from a script file
void shell_execute_script(const char *script_filename) {
    FILE *scriptFile = fopen(script_filename, "r");
    if (scriptFile == NULL) {
        perror("Error opening script file");
        return;
    }
    char command[1024];
    while (fgets(command, sizeof(command), scriptFile) != NULL) {
        system(command);
    }
    fclose(scriptFile);
}

// Main shell loop function
void shell_loop() {
    char *wholeCommand; // For storing the entire user command
    char **individualCommands; // For storing individual arguments of the command

    do {
        printf("iiitd@possum:~$ ");

        wholeCommand = read_user_input(); // Reading the user input

        pid_t pid;
        char *name = wholeCommand; // Extracting the command name
        char *whcmd = strdup(wholeCommand); // Copy the whole command for the history part

        struct timespec start_time, end_time;
        clock_gettime(CLOCK_REALTIME, &start_time); // Recording the command start time

        if ((strcmp(wholeCommand, "history\n") == 0)) {
            // Checking if the user wants to see command history
            // If so, printing the command history and continue to the next iteration
            for (int i = 0; i < count; i++) {
                printf("%s", command_history[i].wholeCommand);
            }
            continue;
        } else {
            char *pipe_check = strchr(wholeCommand, '|');
            if (pipe_check == NULL) {
                // Check if the command contains pipes ('|')
                // If not, it's a single command without pipes
                // Split the command into individual arguments
                individualCommands = split_on_whiteSpaces(wholeCommand);
                // Launching a command without pipes
                pid = launch_without_pipes(individualCommands);
            } else {
                // The command contains pipes
                // Split the command into multiple commands at the pipe symbol
                individualCommands = split_on_pipeSymbol(wholeCommand);
                // Launching a command with pipes
                pid = launch_with_pipes(individualCommands);
            }

            int status;
            waitpid(pid, &status, 0); // Waiting for the command to complete
            clock_gettime(CLOCK_REALTIME, &end_time); // Recording command end time

            if (count < MAX_HISTORY_SIZE) {
                // Storing command and execution information in history
                command_history[count].wholeCommand = whcmd;
                command_history[count].name = strdup(name);
                command_history[count].pid = pid;
                command_history[count].start_time = start_time;
                command_history[count].end_time = end_time;
                command_history[count].total_duration_ms = diff_in_ms(&start_time, &end_time);
                count++;
            } else {
                printf("History is full. Cannot store more commands.\n");
            }
        }
        clean_up(wholeCommand, individualCommands); // Clean up the allocated memory
    } while (1); // Continue the loop indefinitely until ctrl+C
}

