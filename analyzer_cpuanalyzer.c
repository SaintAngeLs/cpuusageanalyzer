#include "analyzer_cpuanalyzer.h"

void *analyzer_proc_stat_thread()
{

    while (1)
    {
        sem_wait(&slots_empty_sem);
        pthread_mutex_lock(&bufferMutex);
        struct kernel_proc_stat *stat = insert_to_array_stat();
        if (get_proc_stat(stat) == -1)
            continue;
        if (stat == NULL)
            continue;

        pthread_mutex_unlock(&bufferMutex);
        sem_post(&slots_filled_sem);

        pthread_mutex_lock(&bufferMutex);
        // Print the stats
        int numColumns = 11; // Number of columns in the table
        int lineWidth = (numColumns * 10) + (numColumns - 1); // Calculate the line width dynamically

        printf("%-10s %-10s %-10s %-10s %-10s %-10s %-10s %-10s %-10s %-10s %-10s\n",
               "Name", "User", "Nice", "System", "Idle", "IOWait", "IRQ", "SoftIRQ",
               "Steal", "Guest", "GuestNice");

        for (int i = 0; i < lineWidth; i++)
        {
            printf("=");
        }
        printf("\n");

        for (int i = 0; i < available_proc; i++)
        {
            printf("%-10s %-10lu %-10lu %-10lu %-10lu %-10lu %-10lu %-10lu %-10lu %-10lu %-10lu\n",
                   stat[i].name, stat[i].user, stat[i].nice, stat[i].system,
                   stat[i].idle, stat[i].iowait, stat[i].irq, stat[i].softirq,
                   stat[i].steal, stat[i].guest, stat[i].guest_nice);
        }

        pthread_mutex_unlock(&bufferMutex);
    }
    

}

// cpu usage analizer calculation formula
// 
// static U_L calculate_avarage_cpu_usage(struct kernel_proc_stat current, struct kernel_proc_stat previouis)
// {
//     U_L previousIdle = previouis.idle + previouis.iowait;
//     U_L currentIdle = current.idle + current.iowait;
//     U_L previousNonIdle = previouis.user + previouis.nice + previouis.system + previouis.irq + previouis.softirq + previouis.steal;
//     U_L currentNonIdle = current.user + current.nice + current.system + current.irq + current.softirq + current.steal;

//     U_L previousTotal = previousNonIdle + previousIdle;
//     U_L currentTotal = currentNonIdle + currentIdle;

//     currentTotal -= previousTotal;
//     currentIdle -= previousIdle;

//     return (currentTotal - currentIdle)*100/currentTotal;
// }
