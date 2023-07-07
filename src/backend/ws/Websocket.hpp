#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <eventpp/callbacklist.h>

using CURL = void;

namespace backend::ws {
/**
 * curl based websocket implementation.
 * Based on: https://curl.se/libcurl/c/websocket.html
 **/
class Websocket {
 private:
    std::string url;

    std::mutex wsMutex;
    std::unique_ptr<std::thread> wsThread;
    std::atomic_bool shouldRun{false};
    bool connected{false};

    CURL* curl{nullptr};

 public:
    eventpp::CallbackList<void(const std::string&)> onRcv;
    eventpp::CallbackList<void(void)> onConnected;

    explicit Websocket(std::string&& url);
    explicit Websocket(const std::string& url);

    Websocket(Websocket&&) = delete;
    Websocket(const Websocket&) = delete;
    Websocket& operator=(Websocket&&) = delete;
    Websocket& operator=(const Websocket&) = delete;
    ~Websocket();

    void start();
    void stop();

    void send(const std::string& msg);
    [[nodiscard]] bool is_connected() const;

 private:
    void thread_run();
    bool recv_any();

    bool curl_connect();
    void curl_disconnect();
};
}  // namespace backend::ws