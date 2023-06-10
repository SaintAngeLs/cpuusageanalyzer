#include "reader_cpuanalyzer.h"


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
    char line_to_read[ARRAY_BUFFER_SIZE];
    for (int thread = 0; thread < available_proc; thread++) 
    {
        fgets(line_to_read, sizeof(line_to_read), file_to_read);

        if (strncmp(line_to_read, "cpu", 3) != 0) 
        {
            perror("Reading thread info failed");

            if (fclose(file_to_read) == EOF) 
            {
                ERR("Error closing file");
                return -1;
            }
            
            return -1;
        }
        int result = parse_proc_line(line_to_read, &stats[thread]);
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




void *read_proc_stat_thread()
{ 
    struct kernel_proc_stat *stat = NULL;

    while(1)
    {
        sem_wait(&slots_empty_sem);
        pthread_mutex_lock(&bufferMutex);
        stat = insert_to_array_stat();
        if(get_proc_stat(stat) == -1)
        {
            pthread_mutex_unlock(&bufferMutex);
            continue;
        }
            
        if(stat == NULL)
            continue;

        pthread_mutex_unlock(&bufferMutex);
        sem_post(&slots_filled_sem);

        // suspen exec
        usleep(TIME_SUSPEND);
    }
        
}
