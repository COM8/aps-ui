#include "Websocket.hpp"
#include "spdlog/spdlog.h"
#include <cassert>
#include <chrono>
#include <cstddef>
#include <logger/Logger.hpp>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <curl/curl.h>

namespace backend::ws {
Websocket::Websocket(std::string&& url) : url(std::move(url)) {}

Websocket::Websocket(const std::string& url) : Websocket(std::string{url}) {}

Websocket::~Websocket() {
    stop();
}

void Websocket::start() {
    std::unique_lock lock(wsMutex);
    if (shouldRun) { return; }
    assert(!wsThread);

    SPDLOG_INFO("Starting websocket for '{}'...", url);
    shouldRun = true;
    wsThread = std::make_unique<std::thread>(&Websocket::thread_run, this);
}

void Websocket::stop() {
    std::unique_lock lock(wsMutex);
    if (!shouldRun) { return; }
    assert(wsThread);

    SPDLOG_INFO("Stopping websocket for '{}'...", url);
    shouldRun = false;
    wsThread->join();
    wsThread = nullptr;
    SPDLOG_INFO("Websocket for '{}' stopped.", url);
}

void Websocket::thread_run() {
    SPDLOG_INFO("Websocket for '{}' started.", url);

    while (shouldRun) {
        // Connect:
        connected = false;
        while (!connected && shouldRun) {
            connected = curl_connect();
        }

        // Send/Receive:
        while (shouldRun && connected) {
            if (!recv_any()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(250));
            }
        }
    }
    curl_disconnect();
}

bool Websocket::recv_any() {
    constexpr size_t CHUNK_SIZE{512};
    std::string result{};
    std::array<char, CHUNK_SIZE> buffer{};

    size_t recvLen{0};
    const curl_ws_frame* meta{nullptr};

    while (true) {
        CURLcode ret = curl_ws_recv(curl, buffer.data(), buffer.size(), &recvLen, &meta);

        // Nothing to read
        if (ret == CURLE_AGAIN) {
            if (result.empty()) {
                return false;
            }

            // Wait for more to arrive
            continue;
        }

        // Should run?
        if (!shouldRun) {
            SPDLOG_DEBUG("Websocket canceling receive.");
            return true;
        }

        // Success?
        if (ret != CURLE_OK) {
            SPDLOG_ERROR("curl_ws_recv() for '{}' failed with: {}", url, curl_easy_strerror(ret));
            curl_disconnect();
            return false;
        }

        // Websocket closed:
        if (meta->flags & CURLWS_CLOSE) {
            SPDLOG_INFO("Receive websocket close for '{}'.", url);
            curl_disconnect();
            return false;
        }

        // Ping:
        if (meta->flags & CURLWS_PING) {
            SPDLOG_DEBUG("Receive websocket ping for '{}'.", url);
            const std::string pong{"pong"};
            size_t sentCount{0};
            ret = curl_ws_send(curl, pong.c_str(), pong.length(), &sentCount, 0, CURLWS_PONG);

            if (ret != CURLE_OK) {
                SPDLOG_ERROR("curl_ws_send(PONG) for '{}' failed with: {}", url, curl_easy_strerror(ret));
                return false;
            }

            if (sentCount != pong.length()) {
                SPDLOG_ERROR("curl_ws_send(PONG) for '{}' failed with: did only send {} out of {} chars.", url, sentCount, pong.length());
                return false;
            }
            return true;
        }

        // Append the read buffer to the result:
        result.append(buffer.begin(), buffer.begin() + recvLen);

        // There is more coming:
        if (meta->flags & CURLWS_CONT) {
            continue;
        }

        // No more data available since we did not receive CHUNK_SIZE chars:
        if (recvLen < CHUNK_SIZE) {
            SPDLOG_DEBUG("Websocket received {} chars.", result.length());
            break;
        }
    }

    // Invoke the event handler:
    if (onRcv) {
        onRcv(result);
    }

    return true;
}

void Websocket::send(const std::string& msg) {
    assert(connected);

    size_t sentCount{0};
    CURLcode ret = curl_ws_send(curl, msg.c_str(), msg.size(), &sentCount, 0, CURLWS_TEXT);
    if (ret != CURLE_OK) {
        SPDLOG_ERROR("curl_ws_send(TEXT) for '{}' failed with: {}", url, curl_easy_strerror(ret));
        return;
    }

    if (sentCount != msg.size()) {
        SPDLOG_ERROR("curl_ws_send(TEXT) for '{}' failed with: did only send {} out of {} chars.", url, sentCount, msg.size());
        return;
    }
}

bool Websocket::curl_connect() {
    SPDLOG_DEBUG("Setting up curl websocket for '{}'...", url);

    assert(!curl);
    curl = curl_easy_init();
    if (!curl) {
        SPDLOG_ERROR("Failed to perform curl_easy_init() for '{}'.", url);
        return false;
    }

    CURLcode ret = CURLE_OK;

    ret = curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    if (ret != CURLE_OK) {
        SPDLOG_ERROR("Failed to setup curl connection for '{}'. Setting the CURLOPT_URL failed with: {}", url, curl_easy_strerror(ret));
        curl_disconnect();
        return false;
    }

    ret = curl_easy_setopt(curl, CURLOPT_CONNECT_ONLY, 2L);
    if (ret != CURLE_OK) {
        SPDLOG_ERROR("Failed to setup curl connection for '{}'. Setting the CURLOPT_CONNECT_ONLY failed with: {}", url, curl_easy_strerror(ret));
        curl_disconnect();
        return false;
    }

    ret = curl_easy_perform(curl);
    if (ret != CURLE_OK) {
        SPDLOG_ERROR("Failed to setup curl connection for '{}'. curl_easy_perform() failed with: {}", url, curl_easy_strerror(ret));
        curl_disconnect();
        return false;
    }

    SPDLOG_DEBUG("Setup curl websocket done.", url);
    connected = true;
    if (onConnected) {
        onConnected();
    }
    return true;
}

void Websocket::curl_disconnect() {
    SPDLOG_DEBUG("Tearing down curl websocket for '{}'...", url);
    curl_easy_cleanup(curl);
    curl = nullptr;
    connected = false;
    SPDLOG_DEBUG("Teardown curl websocket complete.", url);
}

bool Websocket::is_connected() const {
    return connected;
}

}  // namespace backend::ws