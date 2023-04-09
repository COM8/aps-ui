#include "Token.hpp"
#include "cpr/api.h"
#include "cpr/body.h"
#include "cpr/cprtypes.h"
#include "cpr/payload.h"
#include "cpr/response.h"
#include "logger/Logger.hpp"
#include "spdlog/spdlog.h"
#include <chrono>
#include <nlohmann/json.hpp>
#include <string>
#include <utility>

namespace backend::teams {
Token::Token(std::string&& tenant, std::string&& clientId, std::string&& clientSecret) : tenant(std::move(tenant)),
                                                                                         clientId(std::move(clientId)), clientSecret(std::move(clientSecret)) {}

bool Token::is_expired() const {
    // Reduce the valid until timeout by one minute to prevent issues further down the line:
    return std::chrono::system_clock::now() + std::chrono::minutes(1) >= validUntil;
}

bool Token::is_valid() const {
    return !is_expired() && !token.empty();
}

bool Token::renew() {
    token = "";

    const std::string url = "https://login.microsoftonline.com/" + tenant + "/oauth2/v2.0/token";
    const cpr::Payload body = {
        {"grant_type", "client_credentials"},
        {"client_id", clientId},
        {"client_secret", clientSecret},
        {"scope", "https://graph.microsoft.com/.default"}};

    const cpr::Header header{
        {"Content-Type", "application/x-www-form-urlencoded"}};

    SPDLOG_INFO("Requesting teams authentication token for tenant: {}", tenant);
    cpr::Response response = cpr::Post(cpr::Url{url}, body, header);
    if (response.status_code != 200) {
        if (response.error.code == cpr::ErrorCode::OK) {
            SPDLOG_ERROR("Requesting teams authentication token failed. Status code: {}\nResponse: {}", response.status_code, response.text);
        } else {
            SPDLOG_ERROR("Requesting teams authentication token failed. Status code: {}\nError: {}", response.status_code, response.error.message);
        }
        return false;
    }

    parse_response(response.text);

    if (is_valid()) {
        SPDLOG_INFO("Successfully requested new teams authentication token.");
        SPDLOG_DEBUG("Token: {}", token);
        return true;
    }
    SPDLOG_ERROR("Requesting teams authentication token failed.");
    return false;
}

Token Token::request_new_token(std::string&& tenant, std::string&& clientId, std::string&& clientSecret) {
    Token token(std::move(tenant), std::move(clientId), std::move(clientSecret));
    token.renew();
    return token;
}

void Token::parse_response(const std::string& response) {
    try {
        nlohmann::json j = nlohmann::json::parse(response);

        if (!j.contains("token_type")) {
            SPDLOG_ERROR("Failed to parse teams token request. 'token_type' missing.");
            return;
        }
        std::string type = j["token_type"].get<std::string>();

        if (!j.contains("access_token")) {
            SPDLOG_ERROR("Failed to parse teams token request. 'access_token' missing.");
            return;
        }
        std::string token = j["access_token"].get<std::string>();

        if (!j.contains("expires_in")) {
            SPDLOG_ERROR("Failed to parse teams token request. 'expires_in' missing.");
            return;
        }
        std::chrono::minutes expiresIn(j["expires_in"].get<int>());

        this->type = type;
        this->token = token;
        this->validUntil = std::chrono::system_clock::now() + expiresIn;
    } catch (nlohmann::json::parse_error& e) {
        SPDLOG_ERROR("Error parsing teams token request from '{}' with: {}", response, e.what());
    }
}
}  // namespace backend::teams
