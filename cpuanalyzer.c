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
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>



int array_stat_count = 0;
int available_proc = 0;
sem_t slots_filled_sem;
sem_t slots_empty_sem;
pthread_mutex_t bufferMutex = PTHREAD_MUTEX_INITIALIZER;
struct kernel_proc_stat *array_stat[BUFFER_SIZE];

void usage(char *name);
void sigalrm_handler(int sig);
int sethandler(void (*f)(int), int sigNo);
int get_available_proc();
int get_semaphore_value(sem_t *semaphore);

void parseProcStatLine(char* line, struct kernel_proc_stat* stat);
void read_proc_stat();



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
pthread_mutex_t data_mutex;
// condition variable
pthread_cond_t data_ready_cond;

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

// get the number fot the processors currently availabele _SC_NPROCESSORS_ONLN

int get_available_proc(int *available_proc_p)
{
    *available_proc_p = sysconf(_SC_NPROCESSORS_ONLN);
    if(*available_proc_p == -1)
        ERR("available processors");
    return 0;
}

int get_semaphore_value(sem_t *semaphore) 
{
    int value;
    sem_getvalue(semaphore, &value);
    return value;
}

struct kernel_proc_stat *insert_to_array_stat()
{
    int index = get_semaphore_value(&slots_filled_sem);

    if(index >= BUFFER_SIZE)
    {
        return NULL;
    }
    return array_stat[index];
}


FILE* open_proc_stat_file() 
{
    FILE *file_to_read = fopen("/proc/stat", "r");
    if (file_to_read == NULL) 
    {
        ERR("fopen");
        return NULL;
    }
    return file_to_read;
}
// Function to handle error checking for token
char* check_token(char *token) 
{
    if (token == NULL) 
    {
        perror("Parsing error");
        return NULL;
    }
    return token;
}

// Function to set kernel_proc_stat values
void set_kernel_proc_stat_values(struct kernel_proc_stat* stat, unsigned long values[], char* name) 
{
    strncpy(stat->name, name, sizeof(stat->name));

    stat->user = values[0];
    stat->nice = values[1];
    stat->system = values[2];
    stat->idle = values[3];
    stat->iowait = values[4];
    stat->irq = values[5];
    stat->softirq = values[6];
    stat->steal = values[7];
    stat->guest = values[8];
    stat->guest_nice = values[9];
}

// // Main parse_line function
// int parse_line(char* read_line, struct kernel_proc_stat* stats, int thread) 
// {
//     char name[16];
//     unsigned long values[10] = {0};

//     char *token = strtok(read_line, " ");
//     strncpy(name, token, sizeof(name));

//     for (int i = 0; i < 10; i++) 
//     {
//         token = strtok(NULL, " ");
//         if (token == NULL) 
//         {
//             break;
//         }
//         values[i] = strtoul(token, NULL, 10);
//     }

//     set_kernel_proc_stat_values(&stats[thread], values, name);

//     return 0;
// }


// int get_proc_stat(struct kernel_proc_stat *stat) 
// {
//     FILE *file_to_read = open_proc_stat_file();
//     if(stat == NULL)
//     {
//         ERR("Error allocating memory");
//         if (fclose(file_to_read) == EOF) 
//         {
//             ERR("Error closing file");
//             return -1;
//         }
//         return -1;
//     }

//     char read_line[BUFFER_SIZE];
    

//     int thread = 0;

//     while (fgets(read_line, sizeof(read_line), file_to_read)) 
//     {
//         if (strncmp(read_line, "cpu", 3) != 0) 
//         {
//             continue;
//         }

//         if (parse_line(read_line, stat, thread) == -1) 
//         {
//             if (fclose(file_to_read) == EOF) 
//             {
//                 ERR("Error closing file");
//                 return -1;
//             }
//             return -1;
//         }

//         thread++;
//         if (thread == available_proc) 
//         {
//             break;
//         }
//     }

//     if (fclose(file_to_read) == EOF) 
//     {
//         ERR("Error closing file");
//         return -1;
//     }

//     return 0;
// }
// 
// 
int parse_proc_line(const char* line, struct kernel_proc_stat* stat)
{
    if (sscanf(line, "%s %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
               stat->name, &stat->user, &stat->nice,
               &stat->system, &stat->idle, &stat->iowait,
               &stat->irq, &stat->softirq, &stat->steal,
               &stat->guest, &stat->guest_nice) != 11)
    {
        ERR("Error parsing line");
        return -1;
    }

    return 0;
}

int get_proc_stat(struct kernel_proc_stat *stats) 
{
    FILE *file_to_read = open_proc_stat_file();
    char line[1024];
    for (int thread = 0; thread < available_proc; thread++) 
    {
        fgets(line, sizeof(line), file_to_read);

        if (strncmp(line, "cpu", 3) != 0) 
        {
            perror("Reading thread info failed");

            if (fclose(file_to_read) == EOF) 
            {
                ERR("Error closing file");
                return -1;
            }
            
            return -1;
        }
        int result = parse_proc_line(line, &stats[thread]);
        if (result == -1) 
        {
            return -1;
        }
    }

    if (fclose(file_to_read) == EOF) 
    {
        ERR("Error closing file");
        return -1;
    }
    return 0;
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



void *read_proc_stat_thread()
{ 
    while(1)
    {
        sem_wait(&slots_empty_sem);
        pthread_mutex_lock(&bufferMutex);
        struct kernel_proc_stat *stat = insert_to_array_stat();
        if(get_proc_stat(stat) == -1)
        {
            pthread_mutex_unlock(&bufferMutex);
            continue;
        }
            
        if(stat == NULL)
            continue;

        pthread_mutex_unlock(&bufferMutex);
        sem_post(&slots_filled_sem);
    }
        
}


//  /**
//   * [main description]
//   * @param  argc [description]
//   * @param  argv [description]
//   * @return      [description]
//   */
// int main(int argc, char **argv)
// {
// 	if(-1 == get_available_proc())
//         ERR("No available processors");
//     // initializing semaphores
//     if(TEMP_FAILURE_RETRY(sem_init(&slots_empty_sem, 0, BUFFER_SIZE)) || TEMP_FAILURE_RETRY(sem_init(&slots_filled_sem, 0, 0)))
//         ERR("sem_init");

//     available_proc++;
//     // in ttesting function do not foreget to increast the available process stat
//     read_proc_stat();

//     return EXIT_SUCCESS;
    
// }

void test_cpu_analyzer() 
{
    // Test open_proc_stat_file function
    FILE *file = open_proc_stat_file();
    if (file == NULL) 
    {
        printf("open_proc_stat_file FAILED.\n");
    } 
    else 
    {
        printf("open_proc_stat_file PASSED.\n");
        if (fclose(file) == EOF) 
        {
            ERR("Error closing file");
        }
    }

    // Test get_available_proc function
    int result = get_available_proc(&available_proc);
    if (result == -1) 
    {
        printf("get_available_proc FAILED: theere is no available processors\n");
    } 
    else 
    {
        printf("get_available_proc PASSED.\n");
    }

    // Test get_proc_stat function
    struct kernel_proc_stat *stat = insert_to_array_stat();

    if (get_proc_stat(stat) == -1) 
    {
        printf("get_proc_stat FAILED: stats == NULL\n");
    } 
    else 
    {
        printf("get_proc_stat PASSED.\n");
    }
}

int main(int argc, char **argv) 
{
    if(-1 == get_available_proc(&available_proc))
        ERR("No available processors");
    available_proc++;
    for(int i = 0; i < BUFFER_SIZE; i++)
    {
        array_stat[i] = malloc(available_proc * sizeof(struct kernel_proc_stat));
        if(array_stat[i] == NULL)
        {
            ERR("malloc");
            return EXIT_FAILURE;
        }
    }

    // initializing semaphores
    if(TEMP_FAILURE_RETRY(sem_init(&slots_empty_sem, 0, BUFFER_SIZE)) || TEMP_FAILURE_RETRY(sem_init(&slots_filled_sem, 0, 0)))
        ERR("sem_init");


    // The ananlizer thread to add
    // add the reading and analizing in thread...
    // 
    pthread_t reader_thread_id;
    pthread_t analyzer_thread_id;

    if(pthread_create(&reader_thread_id, NULL, read_proc_stat_thread, NULL))
        ERR("pthread_create");
    if(pthread_create(&analyzer_thread_id, NULL, analyzer_proc_stat_thread, NULL))
        ERR("pthread_create");


    // waiting fot the reading thread ana analizing thread to be terminated
    
    pthread_join(reader_thread_id, NULL);
    pthread_join(analyzer_thread_id, NULL);

        // Cleanup
    pthread_mutex_destroy(&bufferMutex);
    sem_destroy(&slots_filled_sem);
    sem_destroy(&slots_empty_sem);

    for(int i = 0; i < BUFFER_SIZE; i++)
    {
        free(array_stat[i]);
    }

        // Run the tests
    //test_cpu_analyzer();
    return EXIT_SUCCESS;
}