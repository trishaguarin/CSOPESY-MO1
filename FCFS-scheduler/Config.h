#pragma once
#include <string>

namespace settings {

constexpr int CORE_COUNT = 4;
constexpr int PROCESS_COUNT = 10;
constexpr int PRINTS_PER_PROCESS = 100;
constexpr int PRINT_DELAY_MS = 25;
inline const std::string LOG_DIR = "process_logs";

} 
