#ifndef CPUANALYZER_H
#define CPUANALYZER_H

#define _GNU_SOURCE
#include <stdio.h>
#include <signal.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>


#include "watchdog_analyzer.h"



#define U_L unsigned long

#define BUFFER_SIZE 10
#define ARRAY_BUFFER_SIZE 1024
#define S_TIME_SLEEP 2
#define THREADS_NUMBER 4

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

// struct THREAD_STATE
// {
//     int reader_thread;
//     int analizer_thread;
//     int printer_thread;
// };

//
 enum THREAD_STATE {
    reader_thread = 0,
    analizer_thread = 1,
    printer_thread = 2,
    logger_thread = 3
 };
// Set the initial state
// Miscellaneous kernel statistics in /proc/stat from the  https://docs.kernel.org/filesystems/proc.html:
struct kernel_proc_stat
{
    char name[BUFFER_SIZE];
    U_L user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
};

extern volatile sig_atomic_t term_signal ;

extern int available_proc;
extern sem_t slots_filled_sem;
extern sem_t slots_empty_sem;

extern pthread_mutex_t bufferMutex;

extern struct kernel_proc_stat *array_stat[BUFFER_SIZE];


extern pthread_mutex_t data_mutex;

extern sem_t slots_filled_sem_printer;
extern sem_t slots_empty_sem_printer;
extern pthread_mutex_t print_bufferMutex;
extern U_L * print_buffer[BUFFER_SIZE];


extern pthread_mutex_t watchdog_bufferMutex;
extern int threads_to_watchdog[THREADS_NUMBER];

void usage(char *name);
int set_handler(void (*f)(int), int sigNo);
void sigterm_handler();
void readProcStat(FILE* file_to_read, struct kernel_proc_stat** stat, int* count_thread, int* read_cap);
void parseProcStatLine(char* line, struct kernel_proc_stat* stat);
void printProcStat(struct kernel_proc_stat* stat, int count_thread);
struct kernel_proc_stat *insert_to_array_stat();
U_L *insert_to_print_buffer();
void join_threads();
void cleanup_pthread_mutex_sem();
void cleanup();



#endif /* CPUANALYZER_H */
