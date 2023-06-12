#include "watchdog_analyzer.h"

/**
 * @brief Function to notify the watchdog about a thread's activity
 * @param thread_id The ID of the thread to notify
 *
 * This function notifies the watchdog about a thread's activity by setting the corresponding flag in the threads_to_watchdog array.
 * It acquires the watchdog_bufferMutex mutex before updating the flag to ensure thread safety.
 */
void watchdog_notifier(int thread_id)
{
    pthread_mutex_lock(&watchdog_bufferMutex);
    threads_to_watchdog[thread_id] = 1;
    pthread_mutex_unlock(&watchdog_bufferMutex);
}

/**
 * @brief Watchdog thread function to monitor thread activity
 *
 * This function runs in a separate thread and continuously monitors the activity of the CPU analyzer threads.
 * It sleeps for a specific time using the sleep function to allow the threads to perform their work.
 * After the sleep period, it checks the threads_to_watchdog array to ensure that all threads have notified their activity.
 * If any thread fails to notify its activity, an error message is printed, the sigterm_handler function is called to initiate program termination, and the thread exits.
 * The watchdog_bufferMutex mutex is used for thread safety during the activity check.
 */
void watchdog_proc_stat_thread()
{
	while(1)
	{
		// The time for the watchdog to wait for the threads sync is 2 s.
		sleep(S_TIME_SLEEP);
		pthread_mutex_lock(&watchdog_bufferMutex);
		for(int i = 0; i < THREADS_NUMBER; i++)
		{
			if(threads_to_watchdog[i] == 0)
			{
				pthread_mutex_unlock(&watchdog_bufferMutex);
        		fprintf(stderr, "Theread is waiting for too long ... %d: %s\n", threads_to_watchdog[i], strerror(errno));
        		sigterm_handler();
        		exit(0);
			}
			threads_to_watchdog[i] = 0;
		}
		pthread_mutex_unlock(&watchdog_bufferMutex);
	}
}