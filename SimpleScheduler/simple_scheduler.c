#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>

#define READY 1
#define RUNNING 2
#define TERMINATED 3

struct itimerval timer;
int fd[2];

pid_t scheduler_pid;

// Define struct process with timespec and total_duration_ms
struct process
{
    char *name;
    pid_t pid;
    int priority;
    int state;
    struct timeval start_time;
    struct timeval wait_time;
    struct timeval exec_time;
    bool never_executed;
};

struct Node
{
    struct process *data;
    struct Node *next;
};

struct Queue
{
    struct Node *front;
    struct Node *rear;
};

float TSLICE;
int NCPU;

struct Queue *readyQueue;
struct Queue *runningQueue;
struct Queue *terminatedQueue;
struct Queue *newQueue;
struct Queue *suspendedQueue;

// Function to calculate the difference between two timeval values
struct timeval timeval_diff(struct timeval x, struct timeval y)
{
    struct timeval result;
    if (x.tv_usec < y.tv_usec)
    {
        int borrow = (y.tv_usec - x.tv_usec) / 1000000 + 1;
        y.tv_usec -= 1000000 * borrow;
        y.tv_sec += borrow;
    }
    result.tv_sec = x.tv_sec - y.tv_sec;
    result.tv_usec = x.tv_usec - y.tv_usec;
    return result;
}

bool isEmpty(struct Queue *queue)
{
    return queue->front == NULL;
}

void enqueue(struct Queue *queue, struct process *data)
{
    struct Node *node = (struct Node *)malloc(sizeof(struct Node));

    if (node == NULL)
    {
        printf("Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    node->data = data;
    node->next = NULL;

    if (isEmpty(queue) || data->priority > queue->front->data->priority)
    {
        // If the queue is empty or the new item has higher priority,
        // insert it at the front.
        node->next = queue->front;
        queue->front = node;
    }
    else
    {
        // Otherwise, find the appropriate position in the queue based on priority.
        struct Node *current = queue->front;
        while (current->next != NULL && current->next->data->priority >= data->priority)
        {
            current = current->next;
        }
        node->next = current->next;
        current->next = node;
    }
    if (node->next == NULL)
    {
        // If the new node is inserted at the end, update the rear pointer.
        queue->rear = node;
    }
}

struct process *dequeue(struct Queue *queue)
{
    if (isEmpty(queue))
    {
        printf("Queue is empty\n");
        return NULL;
    }

    struct Node *node = queue->front;
    struct process *data = node->data;
    queue->front = queue->front->next;
    free(node);
    return data;
}

struct process *peek(struct Queue *queue)
{
    if (isEmpty(queue))
    {
        printf("Queue is empty\n");
        return NULL;
    }

    return queue->front->data;
}

char *formatTime(long long timestamp)
{
    struct tm tm_info;
    time_t time_sec = (time_t)timestamp;
    localtime_r(&time_sec, &tm_info);
    char *time_str = (char *)malloc(9); // Allocate 9 bytes for HH:MM:SS
    strftime(time_str, 9, "%H:%M:%S", &tm_info);
    return time_str;
}

char *formatMilliseconds(long long milliseconds)
{
    char *time_str = (char *)malloc(20);
    int seconds = milliseconds / 1000;
    int ms = milliseconds % 1000;
    snprintf(time_str, 20, "%d.%03d", seconds, ms);
    return time_str;
}

void printQueue(struct Queue *queue)
{
    if (isEmpty(queue))
    {
        printf("Queue is empty\n");
        return;
    }

    printf("\nPriority Queue Contents:\n");
    printf("%-5s %-10s %-8s %-10s %-15s %-12s %-12s %-15s\n", "PID", "Priority", "State", "Name", "Start Time", "Wait Time (ms)", "Exec Time (ms)", "Never Executed");

    // Separator line
    printf("-------------------------------------------------------------------------------------------------------------------\n");

    struct Node *node = queue->front;
    while (node != NULL)
    {
        struct process *p = node->data;
        char *start_time_str = formatTime((long long)p->start_time.tv_sec);
        long long wait_time_ms = (p->wait_time.tv_sec * 1000) + (p->wait_time.tv_usec / 1000);
        long long exec_time_ms = (p->exec_time.tv_sec * 1000) + (p->exec_time.tv_usec / 1000);

        printf("%-5d %-10d %-8d %-10s %-15s %-12lld %-12lld %-15s\n",
               p->pid, p->priority, p->state, p->name,
               start_time_str, wait_time_ms, exec_time_ms,
               p->never_executed ? "Yes" : "No");

        free(start_time_str);

        node = node->next;
    }

    // End with another separator
    printf("--------------------------------------------------------------------------------------------------------------\n");
}


void make_process(char **buff)
{
    if (buff[1] == NULL)
    {
        printf("Invalid submit command format\n");
        return;
    }

    struct process *process = (struct process *)malloc(sizeof(struct process));
    if (process == NULL)
    {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    process->pid = -1;
    process->name = strdup(buff[1]);
    if (process->name == NULL)
    {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    if (buff[2] == NULL)
    {
        process->priority = 1;
    }
    else
    {
        process->priority = atoi(buff[2]);
    }
    process->state = READY;

    // Get the start time using gettimeofday
    if (gettimeofday(&process->start_time, NULL) == -1)
    {
        perror("gettimeofday");
        exit(EXIT_FAILURE);
    }

    process->never_executed = true;

    enqueue(readyQueue, process);

    printf("READY QUEUE AFTER SUBMITTING A PROCESS\n");
    printQueue(readyQueue);
    printf("RUNNING QUEUE AFTER SUBMITTING A PROCESS\n");
    printQueue(runningQueue);
    printf("TERMINATED QUEUE AFTER SUBMITTING A PROCESS\n");
    printQueue(terminatedQueue);
}

pid_t execute_without_pipes(char **commands) {
    pid_t pid;
    int status;
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
    
    return pid;
}

char **split(char *line)
{
    int bufsz = 64;
    int index = 0;
    char **tokens = malloc(bufsz * sizeof(char *));
    char *token;

    if (tokens == NULL)
    {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    // Using strtok to split the line into tokens based on space and newline characters
    token = strtok(line, " \n");
    while (token != NULL)
    {
        tokens[index++] = strdup(token);
        token = strtok(NULL, " \n");
    }
    tokens[index] = NULL;

    return tokens;
}

void set_timer()
{
    timer.it_value.tv_sec = TSLICE;
    timer.it_value.tv_usec = 0;
    timer.it_interval = timer.it_value;
    if (setitimer(ITIMER_REAL, &timer, NULL) == -1)
    {
        perror("setitimer");
        exit(EXIT_FAILURE);
    }
}

void stop_timer()
{
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;
    if (setitimer(ITIMER_REAL, &timer, NULL) == -1)
    {
        perror("setitimer");
        exit(EXIT_FAILURE);
    }
}

void execute_process(struct process *process)
{
    if (process->pid == -1)
    {
        pid_t pid = fork();

        if (pid == 0)
        {
            // Child process
            printf("Executing process: %s (PID: %d)\n", process->name, getpid());
            system(process->name);
            printf("Process execution finished (PID: %d).\n", getpid());

            if (kill(getppid(), SIGCHLD) == -1)
            {
                perror("kill");
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        }
        else
        {
            process->pid = pid;
            process->state = RUNNING;
            process->never_executed = false;
            enqueue(runningQueue, process);

            // int status;
            // pid_t child_pid = wait(&status);

            // if (child_pid == -1)
            // {
            // perror("wait");
            // exit(EXIT_FAILURE);
            // }

            // Calculate wait time in milliseconds
            struct timeval curr_time;
            if (gettimeofday(&curr_time, NULL) == -1)
            {
                perror("gettimeofday");
                exit(EXIT_FAILURE);
            }
            process -> wait_time = timeval_diff(curr_time,process->start_time);
            printf("PROCESS CURRENTLY RUNNING\n");
            printQueue(runningQueue);
            printf("PROCESS CURRENTLY READY\n");
            printQueue(readyQueue);
            printf("PROCESS CURRENTLY TERMINATED\n");
            printQueue(terminatedQueue);
        }
    }
    else
    {
        // Process has been executed before, so just continue it
        printf("Continuing process: %s\n", process->name);
        process->state = RUNNING;
        enqueue(runningQueue, process);
        if (kill(process->pid, SIGCONT) == -1)
        {
            perror("kill");
            exit(EXIT_FAILURE);
        }
        printf("RUNNING QUEUE AFTER CONTINUING A PROCESS\n");
        printQueue(runningQueue);
        printf("READY QUEUE AFTER CONTINUING A PROCESS\n");
        printQueue(readyQueue);
        printf("TERMINATED QUEUE AFTER CONTINUING A PROCESS\n");
        printQueue(terminatedQueue);
    }
}

int count_processes(struct Queue *queue);

void start_scheduling()
{
    if (count_processes(runningQueue) >= NCPU)
    {

        printf("All CPUs are busy. Waiting for a process to terminate.\n");

        printf("RUNNING QUEUE WHEN IT IS FULL\n");
        printQueue(runningQueue);
        printf("READY QUEUE WHEN RUNNING QUEUE IS FULL\n");
        printQueue(readyQueue);
        printf("TERMINATED QUEUE WHEN RUNNING QUEUE IS FULL\n");
        printQueue(terminatedQueue);
        return;
    }

    while (!isEmpty(readyQueue))
    {
        int availableCPUs = NCPU;
        struct process *process;
        while (availableCPUs > 0 && (process = dequeue(readyQueue)) != NULL)
        {
            if (runningQueue->front == NULL)
            {
                // No processes running, so we can start scheduling new ones
                printf("Number of processes running: %d\n", count_processes(runningQueue));
                printf("Scheduling process: %s\n", process->name);
                execute_process(process);
                availableCPUs--;
            }
            else if (count_processes(runningQueue) < NCPU)
            {
                // There are some processes running, but not all CPUs are busy
                printf("Number of processes running: %d\n", count_processes(runningQueue));
                printf("Scheduling process: %s\n", process->name);
                execute_process(process);
                availableCPUs--;
            }
            else
            {
                // All CPUs are busy, so we can't schedule more processes right now
                enqueue(readyQueue, process); // Put it back in the ready queue
            }
        }
        if (availableCPUs == 0)
        {
            printf("No more CPUs available. Waiting for a process to terminate.\n");
            return;
        }
    }
}

int count_processes(struct Queue *queue)
{
    int count = 0;
    struct Node *node = queue->front;
    while (node != NULL)
    {
        count++;
        node = node->next;
    }
    return count;
}

void handle_signal(int signum)
{
    int flag = 0;
    if (signum == SIGCHLD)
    {
        // Handle SIGCHLD signal (child process terminated)
        int status;
        pid_t child_pid;

        while ((child_pid = waitpid(-1, &status, WNOHANG)) > 0)
        {
            // Traverse the running queue to identify which process terminated
            struct Node *current = runningQueue->front;
            struct Node *prev = NULL; // To keep track of the previous node.

            while (current != NULL)
            {
                struct process *p = current->data;

                if (p->pid == child_pid)
                {
                    // Mark the process as terminated
                    p->state = TERMINATED;
                    printf("Process %s (PID: %d) terminated.\n", p->name, p->pid);
                    struct timeval end_time;
                    if (gettimeofday(&end_time, NULL) == -1)
                    {
                        perror("gettimeofday");
                        exit(EXIT_FAILURE);
                    }

                    p->exec_time = timeval_diff(end_time, p->start_time);

                    // Remove the terminated process from the running queue
                    if (prev != NULL)
                    {
                        prev->next = current->next;
                    }
                    else
                    {
                        runningQueue->front = current->next;
                    }
                    free(current);

                    // Enqueue the terminated process
                    enqueue(terminatedQueue, p);

                    // Update your queues accordingly
                    printf("RUNNING QUEUE AFTER TERMINATING A PROCESS\n");
                    printQueue(runningQueue);
                    printf("READY QUEUE AFTER TERMINATING A PROCESS\n");
                    printQueue(readyQueue);
                    printf("TERMINATED QUEUE AFTER TERMINATING A PROCESS\n");
                    printQueue(terminatedQueue);
                    // sleep(1);
                    break;
                }

                prev = current;
                current = current->next;
            }
        }
    }

    if (signum == SIGALRM)
    {
        stop_timer();
        printf("Time slice expired. Stopping running processes.\n");
        struct process *process;
        printf("RUNNING QUEUE BEFORE STOPPING ALL PROCESSES\n");
        printQueue(runningQueue);
        while ((process = dequeue(runningQueue)) != NULL)
        {
            process->state = READY;

            // Calculate the wait time
            // if (gettimeofday(&process->wait_time, NULL) == -1)
            // {
            // perror("gettimeofday");
            // exit(EXIT_FAILURE);
            // }

            if (kill(process->pid, SIGSTOP) == -1)
            {
                perror("kill");
                exit(EXIT_FAILURE);
            }
            enqueue(readyQueue, process);
        }
        int flags = fcntl(fd[0], F_GETFL);
        fcntl(fd[0], F_SETFL, flags | O_NONBLOCK);
        printf("RUNNING QUEUE AFTER STOPPING ALL PROCESSES\n");
        printQueue(runningQueue);
        printf("READY QUEUE AFTER STOPPING ALL PROCESSES\n");
        printQueue(readyQueue);
        start_scheduling();
        // sleep(1);
        set_timer();
    }
    else if (signum == SIGUSR1)
    {
        printf("\nRUNNING QUEUE AFTER TERMINATING THE SHELL\n");
        printQueue(runningQueue);
        printf("\nREADY QUEUE AFTER TERMINATING THE SHELL\n");
        printQueue(readyQueue);
        printf("\nTERMINATED QUEUE TERMINATING THE SHELL\n");
        printQueue(terminatedQueue);

        printf("\nTerminating the scheduler...\n");
        exit(EXIT_SUCCESS);
    }
}

char *read_user_input()
{
    char *line = NULL;
    size_t bufsize = 0;

    ssize_t read = getline(&line, &bufsize, stdin);

    if (read == -1)
    {
        if (feof(stdin))
        {
            printf("Reached the end of the file\n");
            exit(EXIT_SUCCESS);
        }
        else
        {
            perror("readline");
            exit(EXIT_FAILURE);
        }
    }
    return line;
}

pid_t fork_scheduler()
{
    if (pipe(fd) == -1)
    {
        perror("Pipe failed");
        exit(1);
    }

    pid_t pid = fork();

    if (pid == 0)
    {
        // Scheduler process
        signal(SIGALRM, handle_signal);
        signal(SIGCHLD, handle_signal);
        set_timer();

        readyQueue = (struct Queue *)malloc(sizeof(struct Queue));
        readyQueue->front = readyQueue->rear = NULL;
        runningQueue = (struct Queue *)malloc(sizeof(struct Queue));
        runningQueue->front = runningQueue->rear = NULL;
        terminatedQueue = (struct Queue *)malloc(sizeof(struct Queue));
        terminatedQueue->front = terminatedQueue->rear = NULL;

        // Set up signal handling for SIGALRM
        struct sigaction sa_usr1;
        sa_usr1.sa_handler = handle_signal;
        sa_usr1.sa_flags = SA_RESTART;
        sigaction(SIGUSR1, &sa_usr1, NULL);

        struct sigaction sa;
        sa.sa_handler = handle_signal;
        sa.sa_flags = SA_RESTART;
        sigaction(SIGALRM, &sa, NULL);
        sigaction(SIGCHLD, &sa, NULL);

        while (1)
        {
            char buffer[256];
            ssize_t bytes_read = read(fd[0], buffer, sizeof(buffer));
            if (bytes_read > 0)
            {
                buffer[bytes_read] = '\0';
                printf("Scheduler received command: %s", buffer);
                char **buff = split(buffer);
                make_process(buff);
                start_scheduling();
            }
        }
    }
    else
    {
        // Shell process
        close(fd[0]);
        char *line;
        do
        {
            printf("iiitd@possum:~$ ");
            line = read_user_input();
            if (strncmp(line, "submit", 6) == 0)
            {
                write(fd[1], line, strlen(line));
            }
            else
            {
                // system(line);

                // Execute other commands in the shell
                pid_t child_pid = fork();

                if (child_pid == -1)
                {
                    perror("Fork failed");
                    exit(1);
                }

                if (child_pid == 0)
                {
                    // Child process
                    // Execute other commands in the shell
                    char **commands = split(line);
                    execute_without_pipes(commands);
                    exit(0);
                }
                else
                {
                    // Parent process
                    int status;
                    waitpid(child_pid, &status, 0);
                    // Handle the completion status of other commands if needed
                }
            }
        } while (1);
    }
    return pid;
}

void my_handler(int signum)
{
    if (signum == SIGINT)
    {
        printf("\nTerminating the shell...\n");
        sleep(2);
        // Send a SIGUSR1 signal to the scheduler process
        if (kill(scheduler_pid, SIGUSR1) == -1)
        {
            perror("kill");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s <NCPU> <TSLICE>\n", argv[0]);
        return 1;
    }

    struct sigaction sig;
    memset(&sig, 0, sizeof(sig));
    sig.sa_handler = my_handler;

    if (sigaction(SIGINT, &sig, NULL) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    // Number of CPUs and time slice from command-line arguments
    NCPU = atoi(argv[1]);
    TSLICE = atof(argv[2]);

    // Fork the scheduler and shell processes
    scheduler_pid = fork_scheduler();

    if (scheduler_pid < 0)
    {
        perror("fork_scheduler");
        return 1;
    }
    // Parent process (main) doesn't need to do anything further
    if (scheduler_pid > 0)
    {
        // Wait for the scheduler process to finish
        int status;
        waitpid(scheduler_pid, &status, 0);
    }
    return 0;
}