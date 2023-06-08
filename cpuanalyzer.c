// CPU usage analizer 
#include "cpuanalyzer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>


// 
/**
 * function to dislay the informatio nabout the usage of the main function
 * @param name first usage argument 
 */
void usage(char *name)
{
    fprintf(stderr, "Usage: %s <argument1> <argument2> ...\n", name);
}

// Global variable to store the last received signal
volatile sig_atomic_t last_signal = 0;

/**
 * @brief Signal handler for SIGALRM
 * @param sig signal number
 */
void sigalrm_handler(int sig)
{
    last_signal = sig;
}
// Function for setting signal handlers
int sethandler(void (*f)(int), int sigNo)
{
    struct sigaction act;
    memset(&act, 0, sizeof(struct sigaction));
    act.sa_handler = f;
    if (-1 == sigaction(sigNo, &act, NULL))
        return -1;
    return 0;
}

void readProcStat(FILE* file_to_read, struct kernel_proc_stat** stat, int* count_thread, int* read_cap)
{
    char read_line[BUFFER_SIZE];
    while (fgets(read_line, sizeof(read_line), file_to_read))
    {
        if (strncmp(read_line, "cpu", 3) == 0)
        {
            if (*count_thread == *read_cap)
            {
                *read_cap = (*read_cap == 0) ? 1 : (*read_cap) * 2;
                struct kernel_proc_stat* temp = realloc(*stat, (*read_cap) * sizeof(struct kernel_proc_stat));
                if (NULL == temp)
                {
                    ERR("realloc");
                }
                *stat = temp;
            }
            
            parseProcStatLine(read_line, &(*stat)[*count_thread]);
            (*count_thread)++;
        }
    }
}

void parseProcStatLine(char* line, struct kernel_proc_stat* stat)
{
    char* token = strtok(line, " ");
    strncpy(stat->name, token, sizeof(stat->name));
    
    for (int i = 0; i < 10; i++)
    {
        token = strtok(NULL, " ");
        if (NULL == token)
        {
            ERR("Parsing error");
        }
        
        switch (i)
        {
            case 0:
                stat->user = strtoul(token, NULL, 10);
                break;
            case 1:
                stat->nice = strtoul(token, NULL, 10);
                break;
            case 2:
                stat->system = strtoul(token, NULL, 10);
                break;
            case 3:
                stat->idle = strtoul(token, NULL, 10);
                break;
            case 4:
                stat->iowait = strtoul(token, NULL, 10);
                break;
            case 5:
                stat->irq = strtoul(token, NULL, 10);
                break;
            case 6:
                stat->softirq = strtoul(token, NULL, 10);
                break;
            case 7:
                stat->steal = strtoul(token, NULL, 10);
                break;
            case 8:
                stat->guest = strtoul(token, NULL, 10);
                break;
            case 9:
                stat->guest_nice = strtoul(token, NULL, 10);
                break;
        }
    }
}


void printProcStat(struct kernel_proc_stat* stat, int count_thread)
{
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

    for (int i = 0; i < count_thread; i++)
    {
        printf("%-10s %-10lu %-10lu %-10lu %-10lu %-10lu %-10lu %-10lu %-10lu %-10lu %-10lu\n",
               stat[i].name, stat[i].user, stat[i].nice, stat[i].system,
               stat[i].idle, stat[i].iowait, stat[i].irq, stat[i].softirq,
               stat[i].steal, stat[i].guest, stat[i].guest_nice);
    }
}


void testCPUsageAnalyzer()
{
    FILE *file_to_read = fopen("/proc/stat", "r");
    if (file_to_read == NULL)
        ERR("fopen");

    struct kernel_proc_stat *stat = NULL;
    int read_cap = 0;
    int count_thread = 0;

    readProcStat(file_to_read, &stat, &count_thread, &read_cap);
    printProcStat(stat, count_thread);

    if (EOF == fclose(file_to_read))
        ERR("fclose");

    free(stat);
}

 /**
  * [main description]
  * @param  argc [description]
  * @param  argv [description]
  * @return      [description]
  */
int main(int argc, char **argv)
{
	testCPUsageAnalyzer();
    return EXIT_SUCCESS;
}
