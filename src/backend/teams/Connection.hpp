#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <eventpp/callbacklist.h>
#include <ixwebsocket/IXWebSocket.h>

namespace backend::teams {

struct CallEvent {
    std::string id;
    std::string from;

    static std::optional<CallEvent> from_json(const nlohmann::json& j);
} __attribute__((aligned(64)));

struct CallEndEvent {
    std::string id;
    std::string from;
    std::string reason;
    std::string phrase;

    static std::optional<CallEndEvent> from_json(const nlohmann::json& j);
} __attribute__((aligned(128)));

class Connection {
 private:
    ix::WebSocket webSocket;
    bool authenticated{false};
    std::string endPoint;
    std::string authentication;

 public:
    explicit Connection(std::string&& endPoint, std::string&& authentication);

    // Event handler:
    eventpp::CallbackList<void(const CallEvent&)> onCallEvent;
    eventpp::CallbackList<void(const CallEndEvent&)> onCallEndEvent;

    void start();
    void stop();

 private:
    void authenticate();
    void on_message(const ix::WebSocketMessagePtr& msg);
    void parse(const std::string& s);
};
}  // namespace backend::teams