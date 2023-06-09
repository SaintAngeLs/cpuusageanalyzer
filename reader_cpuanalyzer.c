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
