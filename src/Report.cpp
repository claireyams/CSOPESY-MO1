#include "Report.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace {

// Pads `s` with spaces on the right up to `width` characters.
std::string padRight(const std::string& s, std::size_t width) {
    return s.size() >= width ? s : s + std::string(width - s.size(), ' ');
}

}  // namespace

// Layout (matches the spec mockup, lines 38-54):
//
//   CPU utilization: 100%
//   Cores used: 4
//   Cores available: 0
//
//   --------------------------------------
//   Running processes:
//   p05   (01/18/2024 09:15:22AM)   Core: 0   1235 / 5876
//
//   Finished processes:
//   p01   (01/18/2024 09:00:21AM)   Finished   5876 / 5876
//   --------------------------------------
std::string buildUtilizationReport(const SchedulerSnapshot& snap) {
    std::ostringstream out;
    out << "CPU utilization: " << std::fixed << std::setprecision(0)
        << snap.utilizationPercent << "%\n";
    out << "Cores used: " << snap.coresUsed << "\n";
    out << "Cores available: " << snap.coresAvailable << "\n\n";
    out << "--------------------------------------\n";

    // Column widths are computed from the data so names of any length stay aligned.
    std::size_t nameW = 0;
    std::size_t stampW = 0;
    for (const auto& r : snap.running) {
        nameW = std::max(nameW, r.process->name().size());
        stampW = std::max(stampW, r.process->createdAt().size());
    }
    for (const auto& p : snap.finished) {
        nameW = std::max(nameW, p->name().size());
        stampW = std::max(stampW, p->createdAt().size());
    }

    // Status column ("Core: 3" / "Finished") is padded so the progress column lines up too.
    std::size_t statusW = snap.finished.empty() ? 0 : std::string("Finished").size();
    for (const auto& r : snap.running)
        statusW = std::max(statusW, ("Core: " + std::to_string(r.core)).size());

    out << "Running processes:\n";
    for (const auto& r : snap.running) {
        const auto& p = r.process;
        out << padRight(p->name(), nameW) << "   "
            << padRight("(" + p->createdAt() + ")", stampW + 2) << "   "
            << padRight("Core: " + std::to_string(r.core), statusW) << "   "
            << p->currentLine() << " / " << p->totalLines() << "\n";
    }

    out << "\nFinished processes:\n";
    for (const auto& p : snap.finished) {
        out << padRight(p->name(), nameW) << "   "
            << padRight("(" + p->createdAt() + ")", stampW + 2) << "   "
            << padRight("Finished", statusW) << "   "
            << p->totalLines() << " / " << p->totalLines() << "\n";
    }

    out << "--------------------------------------\n";
    return out.str();
}

bool saveReport(const std::string& path, const std::string& content) {
    std::ofstream f(path);
    if (!f) return false;
    f << content;
    return static_cast<bool>(f);
}
