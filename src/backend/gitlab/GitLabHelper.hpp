#pragma once

#include <string>

namespace backend::gitlab {
void request_stats(const std::string& url);
}  // namespace backend::gitlab