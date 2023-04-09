#pragma once

#include "Token.hpp"
#include <eventpp/callbacklist.h>
#include <ixwebsocket/IXWebSocket.h>

namespace backend::teams {
class Connection {
    Token token;
    ix::WebSocket webSocket;

 public:
    explicit Connection(Token&& token);

    // Event handler:
    eventpp::CallbackList<void(void)> newEvent;

    void start();
    void stop();

 private:
    void on_message(const ix::WebSocketMessagePtr& msg);
    void parse(const std::string& s);
};
}  // namespace backend::teams