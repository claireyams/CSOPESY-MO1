#pragma once
#include <string>
#include <vector>

// Timestamp in the format used by the spec: "08/06/2024 09:15:22AM"
std::string timestampNow();

std::string trim(const std::string& s);
std::string toLower(std::string s);
std::vector<std::string> splitWords(const std::string& line);
void clearScreen();
