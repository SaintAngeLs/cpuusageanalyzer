#include "printer_cpuanalyzer.h"

void *printer_proc_stat_thread()
{
	U_L *averages = NULL;
	while(1)
	{
		sem_wait(&slots_filled_sem_printer);

		pthread_mutex_lock(&print_bufferMutex);

		system("clear");

		averages = insert_to_print_buffer();

		for(int i = 0; i < available_proc; i++)
		{
			printf("%d. %lu\n", i,averages[i]);
		}

		pthread_mutex_unlock(&print_bufferMutex);

		sem_post(&slots_empty_sem_printer);

		sleep(1);
	}
}