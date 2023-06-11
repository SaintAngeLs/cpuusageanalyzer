#include "analyzer_cpuanalyzer.h"

/**
 * [analyzer_proc_stat_thread analizer wthread]
 * @return [ ]
 */
void *analyzer_proc_stat_thread(void *seq)
{
    // states definition (cfarther calculartion)
    // decl;arations
    struct kernel_proc_stat *stat = NULL; 
    U_L *averages = malloc(available_proc * sizeof(struct kernel_proc_stat));
    if(averages == NULL)
    {
        ERR("malloc");
        return NULL;
    }
    struct kernel_proc_stat *previous = malloc(available_proc * sizeof(struct kernel_proc_stat));
    if(averages == NULL)
    {
        free(averages);
        ERR("malloc");
        return NULL;
    }
    
    pthread_cleanup_push(free, averages);
    pthread_cleanup_push(free, previous);
    while (1)
    {
       if (term_signal)
        {
            return NULL;
        }
        // udpdating working threads
        watchdog_notifier(analizer_thread);
        sem_wait(&slots_filled_sem);
        pthread_mutex_lock(&bufferMutex);
        stat = insert_to_array_stat();

        for(int i = 0; i < available_proc; i++)
        {
            averages[i] = calculate_avarage_cpu_usage(stat[i], previous[i]);
            previous[i] = stat[i];
        }

        pthread_mutex_unlock(&bufferMutex);
        // Copy averages to print buffer
        sem_wait(&slots_empty_sem_printer);
        pthread_mutex_lock(&print_bufferMutex);
        U_L *buffer_averages = insert_to_print_buffer();
        memcpy(buffer_averages, averages, available_proc * sizeof(U_L));
        pthread_mutex_unlock(&print_bufferMutex);
        sem_post(&slots_filled_sem_printer);

        sem_post(&slots_empty_sem);
    }
    pthread_cleanup_pop(1);
    pthread_cleanup_pop(1);
    free(averages);
    free(previous);
    return seq;
}

// cpu usage analizer calculation formula
/**
 * [calculate_avarage_cpu_usage to calculate the cpu ussage]
 * @param  current  [current tate of the struct]
 * @param  previous [the state before]
 * @return          [percent of cpu ussage]
 */
U_L calculate_avarage_cpu_usage(struct kernel_proc_stat current, struct kernel_proc_stat previous)
{
   
    U_L previousIdle = previous.idle + previous.iowait;
    U_L currentIdle = current.idle + current.iowait;
    U_L previousNonIdle = previous.user + previous.nice + previous.system + previous.irq + previous.softirq + previous.steal;
    U_L currentNonIdle = current.user + current.nice + current.system + current.irq + current.softirq + current.steal;

    U_L previousTotal = previousNonIdle + previousIdle;
    U_L currentTotal = currentNonIdle + currentIdle;

    //printf("previousTotal: %lu, currentTotal: %lu\n", previousTotal, currentTotal);  // Debug print statement

    U_L totalDiff = currentTotal - previousTotal;
    U_L idleDiff = currentIdle - previousIdle;

    //printf("currentTotal: %lu, currentIdle: %lu\n", currentTotal, currentIdle);  // Debug print statement

    if (totalDiff == 0) 
    {
        printf("Total difference is zero, CPU usage calculation is not possible.\n");
        return 0;  // or any appropriate value indicating failure
    }

    // Calculate CPU usage as a percentage
    U_L usage = (totalDiff - idleDiff) * 100 / totalDiff;

    return usage;
}
