/**
 * @file       printer_cpuanalyzer.c
 * @defgroup   PRINTER_CPUANALYZER printer cpuanalyzer
 *
 * @brief      This file implements printer cpuanalyzer.
 *
 * @author     A.Voznesenskyi
 * @date       06.12.2023
 */

#include "printer_cpuanalyzer.h"

/**
 * @brief      Printer thread function to print CPU statistics to the console
 *
 * @param      seq   The sequence
 *
 * @return     void
 * 
 * This function runs in a separate thread and continuously prints CPU statistics to the console.
 * It retrieves the data from the print buffer and prints it using the printf function.
 * The thread runs until a termination signal is received.
 * The function uses the slots_filled_sem_printer semaphore and the print_bufferMutex mutex to synchronize access to the print buffer.
 * After printing the data, the thread sleeps for 1 second before processing the next set of data.
 *
 * @note The CPU statistics are printed in the format: "<processor_number>. <usage_percentage>"
 * @note The function does not perform error handling for the printf function.
 */
void *printer_proc_stat_thread(void *seq)
{
	U_L *averages = NULL;
	while(1)
	{

		watchdog_notifier(printer_thread);
		if (term_signal)
        {
        	return NULL;
        }
		sem_wait(&slots_filled_sem_printer);

		pthread_mutex_lock(&print_bufferMutex);


		averages = insert_to_print_buffer();

		for(int i = 0; i < available_proc; i++)
		{
			printf("%d. %lu\n", i,averages[i]);
		}

		pthread_mutex_unlock(&print_bufferMutex);

		sem_post(&slots_empty_sem_printer);

		sleep(1);
	}
	return seq;
}