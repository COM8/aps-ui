#include "GitLabHelper.hpp"
#include "logger/Logger.hpp"
#include <cstddef>
#include <cstdlib>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <cpr/cpr.h>

namespace backend::gitlab {
std::string parse_state(std::string_view part) {
    try {
        nlohmann::json j = nlohmann::json::parse(part);

        if (j.contains("state")) {
            std::string state;
            j.at("state").get_to(state);
            return state;
        }

    } catch (nlohmann::json::parse_error& e) {
        SPDLOG_ERROR("Error parsing GitLab runner state from '{}' with: {}", part, e.what());
    }
    return "";
}

std::unordered_map<std::string, size_t> parse_response(const std::string& response) {
    std::unordered_map<std::string, size_t> result;
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

                // const std::string state = parse_state(part.substr(PREFIX.size() - 1, indexStart + 2 - PREFIX.size()));
                const std::string state = std::string(part.substr(PREFIX.size() - 1, indexStart + 2 - PREFIX.size()));
                if (!state.empty()) {
                    result[state] += count;
                }
            }
        }
        responseSV.remove_prefix(pos != std::string_view::npos ? pos + 1 : responseSV.size());
    }

    return result;
}

std::unordered_map<std::string, size_t> request_stats(const std::string& url) {
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
        return {};
    }
    SPDLOG_DEBUG("GitLab metrics requested successfully. Parsing...");
    return parse_response(response.text);
}
}  // namespace backend::gitlab