#pragma once

#include "Lightning.hpp"
#include <backend/ws/Websocket.hpp>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <eventpp/callbacklist.h>

namespace backend::lightning {
class LightningHelper {
 private:
    ws::Websocket webSocket{"wss://live.lightningmaps.org"};

    bool coordinatesChanged{false};
    double latCenter{0};
    double longCenter{0};
    double zoomFactor{0};
    double latMax{0};
    double longMax{0};
    double latMin{0};
    double longMin{0};

 public:
    // Event handler:
    eventpp::CallbackList<void(const std::vector<Lightning>&)> newLightningsEventHandler;

    void start();
    void stop();
    void set_coordinates(double latCenter, double longCenter, double zoomFactor, double latMax, double longMax, double latMin, double longMin);
    void set_coordinates();

 private:
    void on_message(const std::string& msg);
    void parse(const std::string& s);
};

LightningHelper* get_instance();
}  // namespace backend::lightning