#include "logger_cpuanalyzer.h"

void *logger_proc_stat_thread(void *seq)
{
    FILE *log_file = fopen("log_cpuanalyzer.txt", "w"); // Open the log file in write mode
    if (log_file == NULL) {
        perror("Error opening log file");
        return NULL;
    }

    while (1) {
        watchdog_notifier(logger_thread);
        if (term_signal) {
            fclose(log_file); // Close the log file before exiting
            return NULL;
        }

        sem_wait(&slots_filled_sem_printer);
        pthread_mutex_lock(&print_bufferMutex);

        U_L *averages = insert_to_print_buffer();

        for (int i = 0; i < available_proc; i++) {
            fprintf(log_file, "%d. %lu\n", i, averages[i]); // Write the data to the log file
        }

        pthread_mutex_unlock(&print_bufferMutex);
        sem_post(&slots_empty_sem_printer);

        fflush(log_file); // Flush the buffer to ensure data is written to the file immediately
    }

     if (fclose(log_file) == EOF) 
    {
        ERR("Error closing file");
    }
    return seq;
}
