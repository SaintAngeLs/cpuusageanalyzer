#include "reader_cpuanalyzer.h"


/**
 * @brief Function to open the /proc/stat file
 * @return FILE* Pointer to the opened file, or NULL if an error occurs
 *
 * This function opens the /proc/stat file in read mode and returns a pointer to the opened file.
 * If the file cannot be opened, an error message is printed and NULL is returned.
 * The caller is responsible for closing the file when no longer needed.
 */
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

/**
 * @brief Function to handle error checking for a token
 * @param token The token to check
 * @return char* The input token if it is not NULL, otherwise NULL
 *
 * This function checks if the input token is NULL.
 * If the token is NULL, a "Parsing error" message is printed, and NULL is returned.
 * Otherwise, the input token is returned.
 */char* check_token(char *token) 
{
    if (token == NULL) 
    {
        perror("Parsing error");
        return NULL;
    }
    return token;
}

/**
 * @brief Function to set values in a kernel_proc_stat structure
 * @param stat Pointer to the kernel_proc_stat structure to set values in
 * @param values Array of unsigned long values to set in the structure
 * @param name Name string to set in the structure
 *
 * This function sets the values in a kernel_proc_stat structure.
 * The name is copied into the structure using strncpy.
 * The values are assigned to the corresponding members of the structure.
 */
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

/**
 * @brief Function to parse a line from /proc/stat and store the values in a kernel_proc_stat structure
 * @param line The line from /proc/stat to parse
 * @param stat Pointer to the kernel_proc_stat structure to store the parsed values
 * @return int 0 on success, -1 on failure
 *
 * This function parses a line from the /proc/stat file.
 * It uses sscanf to extract the values from the line and store them in the kernel_proc_stat structure.
 * If the parsing fails or the number of parsed values is incorrect, an error message is printed, and -1 is returned.
 * On success, 0 is returned.
 */
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

/**
 * @brief Function to read CPU statistics from /proc/stat file
 * @param stats Pointer to the kernel_proc_stat array to store the read statistics
 * @return int 0 on success, -1 on failure
 *
 * This function reads CPU statistics from the /proc/stat file.
 * It opens the file using the open_proc_stat_file function and reads each line.
 * The lines are parsed using the parse_proc_line function to extract the relevant statistics.
 * The parsed statistics are stored in the kernel_proc_stat array.
 * If any error occurs during the file reading or parsing, -1 is returned.
 * On success, 0 is returned.
 *
 * @note The function reads one line for each CPU thread, excluding the overall CPU statistics.
 * @note The /proc/stat file is closed after reading.
 */
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




/**
 * @brief Reader thread function to read CPU statistics from /proc/stat file
 * @param seq [unused parameter]
 * @return void* [unused return value]
 *
 * This function runs in a separate thread and continuously reads CPU statistics from the /proc/stat file.
 * It opens the /proc/stat file using the open_proc_stat_file function and reads each line.
 * The lines are parsed using the parse_proc_line function to extract the relevant statistics.
 * The parsed statistics are then stored in the kernel_proc_stat structure.
 * The thread runs until a termination signal is received.
 * The function uses the slots_empty_sem semaphore and the bufferMutex mutex to synchronize access to the array_stat buffer.
 * After reading the statistics, the thread suspends execution for a specified amount of time using the usleep function.
 *
 * @note The /proc/stat file is read line by line, and each line represents the statistics for a specific CPU thread.
 * @note The function handles error checking for file opening, parsing, and file closing.
 */
void *read_proc_stat_thread(void *seq)
{ 
    struct kernel_proc_stat *stat = NULL;

    while(1)
    {
        watchdog_notifier(reader_thread);
        if (term_signal)
        {   
            return NULL;
        }
        sem_wait(&slots_empty_sem);
        pthread_mutex_lock(&bufferMutex);
        stat = insert_to_array_stat();
        if(get_proc_stat(stat) == -1)
        {
            pthread_mutex_unlock(&bufferMutex);
            continue;
        }
            
        if(stat == NULL)
        {
            continue;
        }

        pthread_mutex_unlock(&bufferMutex);
        sem_post(&slots_filled_sem);

        // suspen exec
        usleep(TIME_SUSPEND);
    }
    return seq;
        
}
