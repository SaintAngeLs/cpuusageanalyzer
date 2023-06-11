#ifndef READER_CPUANALIZER_H
#define READER_CPUANALIZER_H
#include "cpuanalyzer.h"


#define TIME_SUSPEND 500

struct kernel_proc_stat;
FILE* open_proc_stat_file();
char* check_token(char *token);
void set_kernel_proc_stat_values(struct kernel_proc_stat* stat, unsigned long values[], char* name);
int parse_proc_line(const char* line, struct kernel_proc_stat* stat);
int get_proc_stat(struct kernel_proc_stat *stats);
void *read_proc_stat_thread(void *seq);

#endif /* READER_CPUANALIZER_H */
