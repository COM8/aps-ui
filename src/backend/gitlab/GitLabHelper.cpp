#include "GitLabHelper.hpp"
#include "logger/Logger.hpp"
#include <cstddef>
#include <cstdlib>
#include <string>
#include <string_view>
#include <vector>
#include <cpr/cpr.h>

namespace backend::gitlab {
void parse_response(const std::string& response) {
    std::vector<std::string_view> parts;
    std::vector<size_t> counts;
    std::string_view responseSV(response);

    while (!responseSV.empty()) {
        const std::string::size_type pos = responseSV.find('\n');
        const std::string_view part = responseSV.substr(0, pos);
        static const std::string PREFIX = "gitlab_runner_jobs{";
        if (part.find(PREFIX) == 0) {
            std::string::size_type indexStart = part.find('}');
            if (indexStart != std::string::npos) {
                std::string_view indexSV = part.substr(indexStart + 1, part.size() - indexStart - 1);
                size_t count = std::strtoul(std::string{indexSV}.c_str(), nullptr, 10);
                parts.push_back(part.substr(PREFIX.size() - 1, indexStart + 2 - PREFIX.size()));
                counts.push_back(count);
            }
        }
        responseSV.remove_prefix(pos != std::string_view::npos ? pos + 1 : responseSV.size());
    }
}

void request_stats(const std::string& url) {
    cpr::Session session;
    session.SetUrl(url);

    SPDLOG_DEBUG("Requesting GitLab metrics from '{}'...", url);
    cpr::Response response = session.Get();
    if (response.status_code != 200) {
        if (response.error.code == cpr::ErrorCode::OK) {
            SPDLOG_ERROR("Requesting GitLab metrics failed. Status code: {}\nResponse: {}", response.status_code, response.text);
        } else {
            SPDLOG_ERROR("Requesting GitLab metrics failed. Status code: {}\nError: {}", response.status_code, response.error.message);
        }
        return;
    }
    SPDLOG_DEBUG("GitLab metrics requested successfully. Parsing...");
    parse_response(response.text);
}
}  // namespace backend::gitlab