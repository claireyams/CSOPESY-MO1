#pragma once
#include <string>

#include "Scheduler.h"

// Text used by BOTH "screen -ls" (printed) and "report-util" (saved to csopesy-log.txt).
std::string buildUtilizationReport(const SchedulerSnapshot& snap);

// Writes `content` to `path`. Returns false on I/O failure.
bool saveReport(const std::string& path, const std::string& content);
