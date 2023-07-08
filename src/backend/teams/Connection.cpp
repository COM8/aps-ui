#include "Connection.hpp"
#include "backend/ws/Websocket.hpp"
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

std::optional<CallEndEvent> CallEndEvent::from_json(const nlohmann::json& j) {
    std::string reason;
    if (!j.contains("reason")) {
        return std::nullopt;
    }
    if (!j["reason"].is_null()) {
        j["reason"].get_to(reason);
    }

    std::string phrase;
    if (!j.contains("phrase")) {
        return std::nullopt;
    }
    if (!j["phrase"].is_null()) {
        j["phrase"].get_to(phrase);
    }

    if (j.contains("sender")) {
        const nlohmann::json& jSender = j["sender"];
        std::string from;
        if (!jSender.contains("displayName")) {
            return std::nullopt;
        }
        if (!jSender["displayName"].is_null()) {
            jSender["displayName"].get_to(from);
        }

        std::string id;
        if (!jSender.contains("id")) {
            return std::nullopt;
        }
        if (!jSender["id"].is_null()) {
            jSender["id"].get_to(id);
        }
        return std::make_optional<CallEndEvent>(std::move(id), std::move(from), std::move(reason), std::move(phrase));
    }
    return std::nullopt;
}

std::optional<ActivityEvent> ActivityEvent::from_json(const nlohmann::json& j) {
    int count = 0;
    if (!j.contains("count")) {
        return std::nullopt;
    }
    j["count"].get_to(count);
    return std::make_optional<ActivityEvent>(count);
}

Connection::Connection(std::string&& endPoint, std::string&& authentication) : webSocket(std::move(endPoint)), authentication(std::move(authentication)) {}

void Connection::start() {
    webSocket.onRcv.append([this](const std::string& msg) { this->on_message(msg); });
    webSocket.onConnected.append([this]() { authenticate(); });
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

void Connection::on_message(const std::string& msg) {
    SPDLOG_DEBUG("[TEAMS_WEBSOCKET]: {}", msg);
    parse(msg);
}

void Connection::parse(const std::string& s) {
    // Parse as JSON:
    nlohmann::json j;
    try {
        j = nlohmann::json::parse(s);

    } catch (nlohmann::json::parse_error& e) {
        SPDLOG_ERROR("Error parsing teams websocket message JSON from '{}' with: {}", s, e.what());
        return;
    }

    if (j.contains("activity")) {
        std::optional<ActivityEvent> activityEvent = ActivityEvent::from_json(j["activity"]);
        if (!activityEvent) {
            SPDLOG_ERROR("Malformed activity event received!");
            return;
        }
        SPDLOG_INFO("Received teams activity event.");

        // Invoke the event handler:
        if (onActivityEvent) {
            onActivityEvent(*activityEvent);
        }
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

    if (j.contains("callEnd")) {
        std::optional<CallEndEvent> callEndEvent = CallEndEvent::from_json(j["callEnd"]);
        if (!callEndEvent) {
            SPDLOG_ERROR("Malformed call end event received!");
            return;
        }
        SPDLOG_INFO("Received teams call ended event from '{}' with id '{}' with reason '{}' ({}).", callEndEvent->from, callEndEvent->id, callEndEvent->phrase, callEndEvent->reason);

        // Invoke the event handler:
        if (onCallEndEvent) {
            onCallEndEvent(*callEndEvent);
        }
        return;
    }

    // Check if the message is an authentication message:
    if (j.contains("auth")) {
        j["auth"].get_to(authenticated);
        SPDLOG_INFO("Teams websocket authentication changed to: {}", authenticated);
        return;
    }

    SPDLOG_WARN("Unknown teams event received. Ignoring...");
}
}  // namespace backend::teams
