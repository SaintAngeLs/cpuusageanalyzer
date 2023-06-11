#ifndef ANALYZER_CPUANALYZER_H
#define ANALYZER_CPUANALYZER_H

#include "cpuanalyzer.h"
#include "reader_cpuanalyzer.h"

struct kernel_proc_stat;
void *analyzer_proc_stat_thread(void *seq);
U_L calculate_avarage_cpu_usage(struct kernel_proc_stat current, struct kernel_proc_stat previouis);

#endif /* ANALYZER_CPUANALYZER_H */ 