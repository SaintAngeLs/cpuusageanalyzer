#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

void send_sigterm() {
    printf("Sending SIGTERM...\n");
    pid_t pid = fork();
    if (pid == 0) {
        execl("./cpuanalyzer", "--terminate", NULL);
        perror("Error executing cpuanalyzer with --terminate option");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            printf("cpuanalyzer exited with status: %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("cpuanalyzer terminated by signal: %d\n", WTERMSIG(status));
        }
    } else {
        perror("Error forking process");
        exit(EXIT_FAILURE);
    }
}

void sigterm_handler(int sig) {
    printf("SIGTERM received. Exiting...\n");
    exit(EXIT_SUCCESS);
}

int main() {
    // Set SIGTERM signal handler
    if (signal(SIGTERM, sigterm_handler) == SIG_ERR) {
        perror("Error setting SIGTERM handler");
        return EXIT_FAILURE;
    }

    printf("Testing SIGTERM handling...\n");
    printf("Press Ctrl+C to send SIGTERM...\n");

        // Run your testing program logic here
        // ...

        // Send SIGTERM signal
        send_sigterm();

        sleep(1);


    return EXIT_SUCCESS;
}
