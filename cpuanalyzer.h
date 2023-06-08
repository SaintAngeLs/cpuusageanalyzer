#ifndef CPUANALYZER_H
#define CPUANALYZER_H

#define _GNU_SOURCE
#include <stdio.h>
#include <signal.h> 

#define U_L unsigned long

#define BUFFER_SIZE 1024
#define THREADS_NUMBER 1024

// Macro for handling errors with source indication:
#define ERR(source) { \
    perror(source); \
    fprintf(stderr, "%s:%d\n", __FILE__, __LINE__); \
    exit(EXIT_FAILURE); \
}

#define HERR(source) { \
    fprintf(stderr, "%s(%d) at %s:%d\n", source, h_errno, __FILE__, __LINE__); \
    exit(EXIT_FAILURE); \
}

// Miscellaneous kernel statistics in /proc/stat from the  https://docs.kernel.org/filesystems/proc.html:
struct kernel_proc_stat
{
    char name[BUFFER_SIZE];
    U_L user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
};

void usage(char *name);
void sigalrm_handler(int sig);
int sethandler(void (*f)(int), int sigNo);
void readProcStat(FILE* file_to_read, struct kernel_proc_stat** stat, int* count_thread, int* read_cap);
void parseProcStatLine(char* line, struct kernel_proc_stat* stat);
void printProcStat(struct kernel_proc_stat* stat, int count_thread);

#endif /* CPUANALYZER_H */
