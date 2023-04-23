#pragma once

#include <string>
#include <unordered_map>

namespace backend::gitlab {
std::unordered_map<std::string, size_t> request_stats(const std::string& url);
}  // namespace backend::gitlab