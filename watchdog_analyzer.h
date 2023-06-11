#ifndef WATCHDOG_CPUANALYZER_H
#define WATCHDOG_CPUANALYZER_H

#include "cpuanalyzer.h"

void watchdog_notifier(int thread_id);
void watchdog_proc_stat_thread();

#endif /* WATCHDOG_CPUANALYZER_H */ 