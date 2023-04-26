#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

namespace backend::gitlab {
struct GitLabStats {
    std::unordered_map<std::string, size_t> status;
    size_t runnerCount;
} __attribute__((aligned(64)));

GitLabStats request_stats(const std::string& url);
}  // namespace backend::gitlab