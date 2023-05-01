#include "Connection.hpp"
#include "ixwebsocket/IXSocketTLSOptions.h"
#include "logger/Logger.hpp"
#include "nlohmann/json.hpp"
#include "spdlog/spdlog.h"
#include <optional>
#include <utility>

namespace backend::teams {
std::optional<CallEvent> CallEvent::from_json(const nlohmann::json& j) {
    if (j.contains("from")) {
        const nlohmann::json& jFrom = j["from"];
        std::string from;
        if (!jFrom.contains("displayName")) {
            return std::nullopt;
        }
        if (!jFrom["displayName"].is_null()) {
            jFrom["displayName"].get_to(from);
        }

        std::string id;
        if (!jFrom.contains("id")) {
            return std::nullopt;
        }
        if (!jFrom["id"].is_null()) {
            jFrom["id"].get_to(id);
        }
        return std::make_optional<CallEvent>(std::move(id), std::move(from));
    }
    return std::nullopt;
}

Connection::Connection(std::string&& endPoint, std::string&& authentication) : endPoint(std::move(endPoint)), authentication(std::move(authentication)) {}

void Connection::start() {
    webSocket.setUrl(endPoint);
    webSocket.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) { this->on_message(msg); });
    webSocket.start();
    SPDLOG_INFO("Teams websocket started.");
}

void Connection::stop() {
    webSocket.stop();
    SPDLOG_INFO("Teams websocket stopped.");
}

void Connection::authenticate() {
    webSocket.send(authentication);
    SPDLOG_INFO("Teams websocket authentication sent.");
}

void Connection::on_message(const ix::WebSocketMessagePtr& msg) {
    if (msg->type == ix::WebSocketMessageType::Message) {
        SPDLOG_DEBUG("[TEAMS_WEBSOCKET]: {}", msg->str);
        parse(msg->str);
    } else if (msg->type == ix::WebSocketMessageType::Open) {
        SPDLOG_INFO("[TEAMS_WEBSOCKET]: Connection established.");
        authenticate();
    } else if (msg->type == ix::WebSocketMessageType::Error) {
        SPDLOG_ERROR("[TEAMS_WEBSOCKET]: Error: {}", msg->errorInfo.reason);
    }
}

void Connection::parse(const std::string& s) {
    // Parse as JSON:
    nlohmann::json j;
    try {
        j = nlohmann::json::parse(s);

    } catch (nlohmann::json::parse_error& e) {
        SPDLOG_ERROR("Error parsing teams websocket message JSON from '{}' with: {}", s, e.what());
    }

    // Check if the message is an authentication message:
    if (j.contains("auth")) {
        j["auth"].get_to(authenticated);
        SPDLOG_INFO("Teams websocket authentication changed to: {}", authenticated);
        return;
    }

    if (j.contains("callNotification")) {
        std::optional<CallEvent> callEvent = CallEvent::from_json(j["callNotification"]);
        if (!callEvent) {
            SPDLOG_ERROR("Malformed call event received!");
            return;
        }
        SPDLOG_INFO("Received teams call event from '{}' with id '{}'.", callEvent->from, callEvent->id);

        // Invoke the event handler:
        if (onCallEvent) {
            onCallEvent(*callEvent);
        }
        return;
    }

    SPDLOG_WARN("Unknown teams event received. Ignoring...");
}
}  // namespace backend::teams
