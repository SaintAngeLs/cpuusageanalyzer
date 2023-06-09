#include "analyzer_cpuanalyzer.h"

void *analyzer_proc_stat_thread()
{
    // states definition (cfarther calculartion)
    struct kernel_proc_stat *previous = malloc(available_proc * sizeof(struct kernel_proc_stat));
    struct kernel_proc_stat *stat = NULL; 

    while (1)
    {
        sem_wait(&slots_filled_sem);
        pthread_mutex_lock(&bufferMutex);
        stat = insert_to_array_stat();
        for(int i = 0; i < available_proc; i ++)
        {
            printf("%lu ", calculate_avarage_cpu_usage(stat[i], previous[i]));
            previous[i] = stat[i];
        }
        printf("\n");
        pthread_mutex_unlock(&bufferMutex);
        // //struct kernel_proc_stat *stat = insert_to_array_stat();
        // if (get_proc_stat(stat) == -1)
        // {
        //     pthread_mutex_unlock(&bufferMutex);
        //     continue;
        // }
        // // if (stat == NULL)
        // //     continue;

        
        

        // // Print the stats
        // int numColumns = 11; // Number of columns in the table
        // int lineWidth = (numColumns * 10) + (numColumns - 1); // Calculate the line width dynamically

        // printf("%-10s %-10s %-10s %-10s %-10s %-10s %-10s %-10s %-10s %-10s %-10s\n",
        //        "Name", "User", "Nice", "System", "Idle", "IOWait", "IRQ", "SoftIRQ",
        //        "Steal", "Guest", "GuestNice");

        // for (int i = 0; i < lineWidth; i++)
        // {
        //     printf("=");
        // }
        // printf("\n");

        // for (int i = 0; i < available_proc; i++)
        // {
        //     printf("%-10s %-10lu %-10lu %-10lu %-10lu %-10lu %-10lu %-10lu %-10lu %-10lu %-10lu\n",
        //            stat[i].name, stat[i].user, stat[i].nice, stat[i].system,
        //            stat[i].idle, stat[i].iowait, stat[i].irq, stat[i].softirq,
        //            stat[i].steal, stat[i].guest, stat[i].guest_nice);
        // }
        // pthread_mutex_unlock(&bufferMutex);

        sem_post(&slots_filled_sem);
    }
    free(previous);

}

// cpu usage analizer calculation formula

U_L calculate_avarage_cpu_usage(struct kernel_proc_stat current, struct kernel_proc_stat previous)
{
   
    U_L previousIdle = previous.idle + previous.iowait;
    U_L currentIdle = current.idle + current.iowait;
    U_L previousNonIdle = previous.user + previous.nice + previous.system + previous.irq + previous.softirq + previous.steal;
    U_L currentNonIdle = current.user + current.nice + current.system + current.irq + current.softirq + current.steal;

    U_L previousTotal = previousNonIdle + previousIdle;
    U_L currentTotal = currentNonIdle + currentIdle;

    printf("previousTotal: %lu, currentTotal: %lu\n", previousTotal, currentTotal);  // Debug print statement

    U_L totalDiff = currentTotal - previousTotal;
    U_L idleDiff = currentIdle - previousIdle;

    printf("currentTotal: %lu, currentIdle: %lu\n", currentTotal, currentIdle);  // Debug print statement

    if (totalDiff == 0) {
        printf("Total difference is zero, CPU usage calculation is not possible.\n");
        return 0;  // or any appropriate value indicating failure
    }

    // Calculate CPU usage as a percentage
    U_L usage = (totalDiff - idleDiff) * 100 / totalDiff;

    return usage;
}
