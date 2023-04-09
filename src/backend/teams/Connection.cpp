#include "Connection.hpp"
#include "backend/teams/Token.hpp"
#include "logger/Logger.hpp"
#include "spdlog/spdlog.h"

namespace backend::teams {
Connection::Connection(Token&& token) : token(std::move(token)) {}

void Connection::start() {
    webSocket.setUrl("wss://graph.microsoft.com/v1.0/me/communications/callRecords?$deltaToken=null&$skipInitial=true");
    webSocket.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) { this->on_message(msg); });
    webSocket.setExtraHeaders({{"Authorization", "Bearer " + token.token}});
    webSocket.start();
    SPDLOG_INFO("Teams websocket started.");
}

void Connection::stop() {
    webSocket.stop();
    SPDLOG_INFO("Teams websocket stopped.");
}

void Connection::on_message(const ix::WebSocketMessagePtr& msg) {
    if (msg->type == ix::WebSocketMessageType::Message) {
        SPDLOG_DEBUG("[WEBSOCKET]: {}", msg->str);
        parse(msg->str);
    } else if (msg->type == ix::WebSocketMessageType::Open) {
        SPDLOG_INFO("[WEBSOCKET]: Connection established.");
    } else if (msg->type == ix::WebSocketMessageType::Error) {
        SPDLOG_ERROR("[WEBSOCKET]: Error: {}", msg->errorInfo.reason);
    }
}

void Connection::parse(const std::string& s) {
    SPDLOG_INFO("[TEAMS]: {}", s);

    // Invoke the event handler:
    if (newEvent) {
        newEvent();
    }
}
}  // namespace backend::teams
