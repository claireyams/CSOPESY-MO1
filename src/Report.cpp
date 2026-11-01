#include "Report.h"

#include <fstream>
#include <iomanip>
#include <sstream>

std::string buildUtilizationReport(const SchedulerSnapshot& snap) {
    std::ostringstream out;
    out << "CPU utilization: " << std::fixed << std::setprecision(0)
        << snap.utilizationPercent << "%\n";
    out << "Cores used: " << snap.coresUsed << "\n";
    out << "Cores available: " << snap.coresAvailable << "\n\n";
    out << "--------------------------------------\n";

    out << "Running processes:\n";
    // TODO(Member 4): for each snap.running entry print, aligned like the spec mockup:
    //   <name>  (<created timestamp>)  Core: <n>  <currentLine> / <totalLines>

    out << "\nFinished processes:\n";
    // TODO(Member 4): for each snap.finished entry print:
    //   <name>  (<timestamp>)  Finished  <totalLines> / <totalLines>

    out << "--------------------------------------\n";
    return out.str();
}

bool saveReport(const std::string& path, const std::string& content) {
    std::ofstream f(path);
    if (!f) return false;
    f << content;
    return static_cast<bool>(f);
}
