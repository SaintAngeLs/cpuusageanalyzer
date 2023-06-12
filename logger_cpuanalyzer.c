/**
 * @file       logger_cpuanalyzer.c
 * @defgroup   LOGGER_CPUANALYZER logger cpuanalyzer
 *
 * @brief      This file implements logger cpuanalyzer.
 *
 * @author     A.Voznesenskyi
 * @date       06.12.2023
 */

#include "logger_cpuanalyzer.h"

/**
 * @brief      Logger thread function to log CPU statistics to a file
 *
 * @param      seq   additional paramteter for synchronization
 *
 * @return     void
 * 
 * This function runs in a separate thread and continuously logs CPU statistics to a log file.
 * It opens the log file in write mode and writes the data received from the print buffer.
 * The thread runs until a termination signal is received.
 * The function uses the slots_filled_sem_printer semaphore and the print_bufferMutex mutex to synchronize access to the print buffer.
 * The log file is flushed after writing each data set to ensure immediate persistence of the logged data.
 *
 * @note        The log file is created as "log_cpuanalyzer.txt" in the current directory.
 * @note        If the log file cannot be opened or closed successfully, an error message is printed.
 */
void *logger_proc_stat_thread(void *seq)
{
    FILE *log_file = fopen("log_cpuanalyzer.txt", "w"); // Open the log file in write mode
    if (log_file == NULL) 
    {
        perror("Error opening log file");
        return NULL;
    }

    while (1) {
        watchdog_notifier(logger_thread);
        if (term_signal) 
        {
            if (fclose(log_file) == EOF) 
            {
                fprintf(log_file, "Terminating logger thread. Reason: Signal received.\n");
                ERR("Error closing file");
            }
            return NULL;
        }

        sem_wait(&slots_filled_sem_printer);
        pthread_mutex_lock(&print_bufferMutex);

        U_L *averages = insert_to_print_buffer();

        for (int i = 0; i < available_proc; i++) 
        {
            fprintf(log_file, "%d. %lu\n", i, averages[i]); // Write the data to the log file
        }

        pthread_mutex_unlock(&print_bufferMutex);
        sem_post(&slots_empty_sem_printer);

        fflush(log_file); // Flush the buffer to ensure data is written to the file immediately
        fprintf(log_file, "Logger thread: Data logged successfully.\n");
    }

    if (fclose(log_file) == EOF) 
    {
        ERR("Error closing file");
    }
    return seq;
}
