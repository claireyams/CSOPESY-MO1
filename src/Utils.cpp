#include "Utils.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <sstream>

std::string timestampNow() {
    std::time_t t = std::time(nullptr);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char buf[40];
    std::strftime(buf, sizeof(buf), "%m/%d/%Y %I:%M:%S%p", &tm);
    return buf;
}

std::string trim(const std::string& s) {
    auto b = s.begin();
    while (b != s.end() && std::isspace(static_cast<unsigned char>(*b))) ++b;
    auto e = s.end();
    while (e != b && std::isspace(static_cast<unsigned char>(*(e - 1)))) --e;
    return std::string(b, e);
}

std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return s;
}

std::vector<std::string> splitWords(const std::string& line) {
    std::istringstream iss(line);
    std::vector<std::string> out;
    std::string w;
    while (iss >> w) out.push_back(w);
    return out;
}

void clearScreen() {
#ifdef _WIN32
    std::system("cls");
#else
    std::system("clear");
#endif
}
