#include "watchdog_analyzer.h"

void watchdog_notifier(int thread_id)
{
    pthread_mutex_lock(&watchdog_bufferMutex);
    threads_to_watchdog[thread_id] = 1;
    pthread_mutex_unlock(&watchdog_bufferMutex);
}

void watchdog_proc_stat_thread()
{
	while(1)
	{
		// The time for the watchdog to wait for the threads sync is 2 s.
		sleep(S_TIME_SLEEP + 10);
		pthread_mutex_lock(&watchdog_bufferMutex);
		for(int i = 0; i < THREADS_NUMBER; i++)
		{
			if(threads_to_watchdog[i] == 0)
			{
				pthread_mutex_unlock(&watchdog_bufferMutex);
        		fprintf(stderr, "Theread is waiting for too long ... %d: %s\n", threads_to_watchdog[i], strerror(errno));
        		sigterm_handler();
        		return;
			}
			threads_to_watchdog[i] = 0;
		}
		pthread_mutex_unlock(&watchdog_bufferMutex);
	}
}