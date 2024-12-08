#pragma once

#include <chrono>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace backend::weather {
struct Weather {
    int id;
    std::string main;
    std::string description;
    std::string icon;

    static Weather from_json(const nlohmann::json& j);
} __attribute__((aligned(128)));

struct Forecast {
    Weather weather;
    std::chrono::system_clock::time_point time;
    double temp{};
    double feelsLike{};

    static Forecast from_json(const nlohmann::json& j);
};

struct ThreeHourForecast {
    std::vector<Forecast> forecast;

    static ThreeHourForecast from_json(const nlohmann::json& j);
} __attribute__((aligned(32)));

}  // namespace backend::weather