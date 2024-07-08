# Process Scheduler and Shell

## Overview

This C code provides a simple process scheduler and shell interface, allowing users to submit and execute processes. The code includes various functions and data structures to manage the scheduling and execution of processes.

## Functionalities and structures used 

### `struct process`

- `name`: The name of the process (command to execute).
- `pid`: The process ID of the running process.
- `priority`: The priority of the process.
- `state`: The state of the process (READY, RUNNING, or TERMINATED).
- `start_time`: A timestamp of when the process started.
- `wait_time`: The amount of time the process has spent waiting.
- `exec_time`: The total execution time of the process.
- `never_executed`: A flag indicating whether the process has been executed.

### `struct Node`

A structure used to create a linked list of processes.

### `struct Queue`

A queue data structure implemented as a linked list, used to manage processes in different states, such as the ready queue, running queue, terminated queue, and new queue.

## Functions

### `timeval_diff(struct timeval x, struct timeval y)`

Calculates the time difference between two `timeval` structures.

### `isEmpty(struct Queue *queue)`

Checks if a given queue is empty.

### `enqueue(struct Queue *queue, struct process *data)`

Adds a process to the queue while maintaining priority order.

### `dequeue(struct Queue *queue)`

Removes and returns a process from the queue.

### `peek(struct Queue *queue)`

Returns the process at the front of the queue without removing it.

### `formatTime(long long timestamp)`

Formats a timestamp as HH:MM:SS.

### `formatMilliseconds(long long milliseconds)`

Formats a time duration in milliseconds as seconds and milliseconds.

### `printQueue(struct Queue *queue)`

Prints the contents of a queue, including process details, state, and timing information.

### `make_process(char **buff)`

Creates a new process and adds it to the ready queue.

### `execute_without_pipes(char **commands)`

Executes a command without pipes and returns the child process ID.

### `split(char *line)`

Splits a string into an array of tokens based on spaces and newline characters.

### `set_timer()`

Sets a timer for the scheduler to execute processes.

### `stop_timer()`

Stops the timer, pausing the execution of processes.

### `execute_process(struct process *process)`

Executes a process, either starting it or resuming it.

### `count_processes(struct Queue *queue)`

Counts the number of processes in a given queue.

### `start_scheduling()`

Starts scheduling processes from the ready queue to the running queue.

### `handle_signal(int signum)`

Handles signals, including SIGCHLD (child process termination), SIGALRM (time slice expiration), and SIGUSR1 (termination signal).

### `read_user_input()`

Reads user input from the shell.

### `fork_scheduler()`

Forks the scheduler and shell processes, setting up communication via a pipe.

### `my_handler(int signum)`

Handles the SIGINT (Ctrl+C) signal for terminating the shell and signaling the scheduler to exit.

### `main(int argc, char *argv[])`

The main function that initializes the process scheduler and shell based on user-defined CPU count and time slice.

## Usage

To use this program, compile the code and run the resulting executable with the specified CPU count and time slice values as command-line arguments. Processes can be submitted using the "submit" command from the shell interface.

Please note that this code is a basic implementation for educational purposes and may require additional modifications and improvements for production use.

## Contribution

Angadjeet Singh(2022071): Impelmented the pipe logic for scheduler
Apaar IIITD(2022089): Worked extensively on error handling and basic structure of the scheduler
