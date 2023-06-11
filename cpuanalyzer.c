// CPU usage analizer 
#include "cpuanalyzer.h"
#include "reader_cpuanalyzer.h"
#include "printer_cpuanalyzer.h"
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
#include "reader_cpuanalyzer.h"
#include "analyzer_cpuanalyzer.h"
#include "watchdog_analyzer.h"
#include "logger_cpuanalyzer.h"




int available_proc = 0;
sem_t slots_filled_sem;
sem_t slots_empty_sem;
pthread_mutex_t bufferMutex = PTHREAD_MUTEX_INITIALIZER;
struct kernel_proc_stat *array_stat[BUFFER_SIZE];

sem_t slots_filled_sem_printer;
sem_t slots_empty_sem_printer;
pthread_mutex_t print_bufferMutex = PTHREAD_MUTEX_INITIALIZER;
U_L *print_buffer[BUFFER_SIZE];

pthread_t analyzer_thread_id;
pthread_t reader_thread_id;
pthread_t printer_thread_id;
pthread_t logger_thread_id;

pthread_mutex_t watchdog_bufferMutex = PTHREAD_MUTEX_INITIALIZER;

int threads_to_watchdog[THREADS_NUMBER];

void usage(char *name)
{
    fprintf(stderr, "Usage: %s (no arguments) ...\n", name);
}

// Global variable to store the last received signal
volatile sig_atomic_t term_signal = 0;


int set_handler(void (*f)(int), int sigNo)
{
    struct sigaction act;
    memset(&act, 0, sizeof(struct sigaction));
    act.sa_handler = f;
    if (-1 == sigaction(sigNo, &act, NULL))
    {
        fprintf(stderr, "Error setting signal handler for signal %d: %s\n", sigNo, strerror(errno));
        return -1;
    }
    else
    {
        fprintf(stdout, "Signal handler for signal %d set successfully.\n", sigNo);
    }
    return 0;
}
// Function for setting signal handlers
void sigterm_handler()
{
    term_signal = 1;
    pthread_cancel(analyzer_thread_id);
    pthread_cancel(printer_thread_id);
    pthread_cancel(reader_thread_id);
}


// get the number fot the processors currently availabele _SC_NPROCESSORS_ONLN

int get_available_proc(int *available_proc_p)
{
    *available_proc_p = sysconf(_SC_NPROCESSORS_ONLN);
    if(*available_proc_p == -1)
    {
       ERR("available processors");
       return -1;
   }
    return 0;
}

int get_semaphore_value(sem_t *semaphore) 
{
    int value;
    sem_getvalue(semaphore, &value);
    return value;
}

/**
 * [insert_to_array_stat insertion to teh array_stat[]
 * @return [ the last element inserter on the ifree index availagble array_stay[free_index]] 
 */
struct kernel_proc_stat *insert_to_array_stat()
{
    int index = get_semaphore_value(&slots_filled_sem);

    return array_stat[index];
}


U_L *insert_to_print_buffer()
{
    int index = get_semaphore_value(&slots_filled_sem_printer);

    return print_buffer[index];
}

void join_threads() 
{
    if (pthread_join(reader_thread_id, NULL) != 0) 
    {
        fprintf(stderr, "Error joining reader thread: %s\n", strerror(errno));
        ERR("pthread_join");
    }
    if (pthread_join(analyzer_thread_id, NULL) != 0) 
    {
        fprintf(stderr, "Error joining analyzer thread: %s\n", strerror(errno));
        ERR("pthread_join");
    }
    if (pthread_join(printer_thread_id, NULL) != 0) 
    {
        fprintf(stderr, "Error joining printer thread: %s\n", strerror(errno));
        ERR("pthread_join");
    }
    if (pthread_join(logger_thread_id, NULL) != 0) 
    {
        fprintf(stderr, "Error joining logger thread: %s\n", strerror(errno));
        ERR("pthread_join");
    }

}
// Cleanup modules
// 


void cleanup_pthread_mutex_sem()
{
    if (pthread_mutex_destroy(&bufferMutex) != 0) {
        fprintf(stderr, "Error destroying buffer mutex: %s\n", strerror(errno));
        ERR("pthread_mutex_destroy");
    }
    if (sem_destroy(&slots_filled_sem) != 0) {
        fprintf(stderr, "Error destroying filled slots semaphore: %s\n", strerror(errno));
        ERR("pthread_mutex_destroy");
    }
    if (sem_destroy(&slots_empty_sem) != 0) {
        fprintf(stderr, "Error destroying empty slots semaphore: %s\n", strerror(errno));
        ERR("pthread_mutex_destroy");
    }
    if (pthread_mutex_destroy(&print_bufferMutex) != 0) {
        fprintf(stderr, "Error destroying print buffer mutex: %s\n", strerror(errno));
        ERR("pthread_mutex_destroy");
    }
    if (sem_destroy(&slots_filled_sem_printer) != 0) {
        fprintf(stderr, "Error destroying filled slots semaphore (printer): %s\n", strerror(errno));
        ERR("pthread_mutex_destroy");
    }
    if (sem_destroy(&slots_empty_sem_printer) != 0) {
        fprintf(stderr, "Error destroying empty slots semaphore (printer): %s\n", strerror(errno));
        ERR("pthread_mutex_destroy");
    }
    if (pthread_mutex_destroy(&watchdog_bufferMutex) != 0) {
        fprintf(stderr, "Error destroying watchdog buffer mutex: %s\n", strerror(errno));
        ERR("pthread_mutex_destroy");
    }
}

void cleanup() 
{
    cleanup_pthread_mutex_sem();
    for (int i = 0; i < BUFFER_SIZE; i++) 
    {
        free(array_stat[i]);
        free(print_buffer[i]);
    }
}

int main(int argc, char **argv) 
{
    if(set_handler(sigterm_handler, SIGTERM) == -1)
    {
        ERR("Setting SIGTERM handler");
        return EXIT_FAILURE;
    }

    if (argc > 1 && strcmp(argv[1], "--terminate") != 0) 
    {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    if(-1 == get_available_proc(&available_proc))
        ERR("No available processors");

    available_proc++;

    for(int i = 0; i < BUFFER_SIZE; i++)
    {
        array_stat[i] = malloc(available_proc * sizeof(struct kernel_proc_stat));
        if(array_stat[i] == NULL)
        {
            ERR("malloc");
            return EXIT_FAILURE;
        }
        print_buffer[i] = malloc(available_proc * sizeof(struct kernel_proc_stat));
        if(print_buffer[i] == NULL)
        {
            ERR("malloc");
            return EXIT_FAILURE;
        }

    }

    // initializing semaphores
    if(TEMP_FAILURE_RETRY(sem_init(&slots_empty_sem, 0, BUFFER_SIZE)) || TEMP_FAILURE_RETRY(sem_init(&slots_filled_sem, 0, 0)))
    {
         ERR("sem_init");
         return EXIT_FAILURE;
    }   
     if(TEMP_FAILURE_RETRY(sem_init(&slots_empty_sem_printer, 0, BUFFER_SIZE)) || TEMP_FAILURE_RETRY(sem_init(&slots_filled_sem_printer, 0, 0)))
    {
         ERR("sem_init");
         return EXIT_FAILURE;
    }  

    if(pthread_create(&reader_thread_id, NULL, read_proc_stat_thread, NULL))
        ERR("pthread_create");
    if(pthread_create(&analyzer_thread_id, NULL, analyzer_proc_stat_thread, NULL))
        ERR("pthread_create");
    if(pthread_create(&printer_thread_id, NULL, printer_proc_stat_thread, NULL))
        ERR("pthread_create");
    if (pthread_create(&logger_thread_id, NULL, logger_proc_stat_thread, NULL)) // Create the logger thread
        ERR("pthread_create");

    
    watchdog_proc_stat_thread();

    // waiting fot the reading thread ana analizing thread to be terminated
    join_threads();

    // Cleanup
    cleanup();

    // Run the tests
    //test_cpu_analyzer();
    return EXIT_SUCCESS;
}