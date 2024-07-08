#include "shell.h"
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>
#include <signal.h>

int main(int argc, char *argv[])
{
    struct sigaction sig;
    memset(&sig, 0, sizeof(sig));
    sig.sa_handler = my_handler;

    // Setting up a signal handler for SIGINT (invoked when Ctrl+C is pressed)

    if (sigaction(SIGINT, &sig, NULL) == -1)
    {
        perror("sigaction");
        return 1;
    }

    if (argc != 2)
    {
        // If the program is running with no arguments or more than one argument,
        // enter interactive shell mode.
        shell_loop();
        return 1;
    }

    // If the program is running with exactly one argument (a script file),
    // execute the commands from the script file.
    if(argv != NULL){
        shell_execute_script(argv[1]);
    }
    else{
        printf("Either an invalid or empty command has been entered\n");
    }

    return 0;
}
