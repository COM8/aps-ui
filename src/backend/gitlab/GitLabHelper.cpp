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
#include <re2/re2.h>

namespace backend::gitlab {
std::string parse_state(std::string_view part) {
    const re2::RE2 stateRegex = "state=\"([a-z]+)\"";
    re2::StringPiece result;

    if (RE2::PartialMatch(part, stateRegex, &result)) {
        return std::string{result};
    }
    SPDLOG_ERROR("Error parsing GitLab runner state from '{}'.", part);
    return "";
}

GitLabStats parse_response(const std::string& response) {
    std::unordered_map<std::string, size_t> stateResult;
    size_t runnerCount = 0;
    std::string_view responseSV(response);
    size_t notIdle = 0;

    while (!responseSV.empty()) {
        const std::string::size_type pos = responseSV.find('\n');
        const std::string_view part = responseSV.substr(0, pos);
        static const std::string STATUS_PREFIX = "gitlab_runner_jobs{";
        static const std::string RUNNER_COUNT_PREFIX = "gitlab_runner_concurrent ";
        if (part.find(STATUS_PREFIX) == 0) {
            std::string::size_type indexStart = part.find('}');
            if (indexStart != std::string::npos) {
                std::string_view indexSV = part.substr(indexStart + 1, part.size() - indexStart - 1);
                size_t count = std::strtoul(std::string{indexSV}.c_str(), nullptr, 10);

                const std::string state = parse_state(part.substr(STATUS_PREFIX.size() - 1, indexStart + 2 - STATUS_PREFIX.size()));
                if (!state.empty()) {
                    if (state != "idle") {
                        notIdle += count;
                    }
                    stateResult[state] += count;
                }
            }
        } else if (part.find(RUNNER_COUNT_PREFIX) == 0) {
            const std::string runnerCountStr = std::string(part.substr(RUNNER_COUNT_PREFIX.size(), part.size() - RUNNER_COUNT_PREFIX.size()));
            runnerCount = std::strtoul(runnerCountStr.c_str(), nullptr, 10);
        }
        responseSV.remove_prefix(pos != std::string_view::npos ? pos + 1 : responseSV.size());
    }

    // Fix the idle count:
    stateResult["idle"] = runnerCount - notIdle;

    return GitLabStats{stateResult, runnerCount};
}

GitLabStats request_stats(const std::string& url) {
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