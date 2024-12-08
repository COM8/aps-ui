#include "Forecast.hpp"
#include <nlohmann/json.hpp>

namespace backend::weather {
ThreeHourForecast ThreeHourForecast::from_json(const nlohmann::json& j) {
    nlohmann::json::array_t listJson;
    j.at("list").get_to(listJson);

    std::vector<Forecast> forecastList{};
    for (const nlohmann::json& forecastJson : listJson) {
        forecastList.push_back(Forecast::from_json(forecastJson));
    }

    return ThreeHourForecast{std::move(forecastList)};
}

Weather Weather::from_json(const nlohmann::json& j) {
    int id = -1;
    j.at("id").get_to(id);
    std::string main;
    j.at("main").get_to(main);
    std::string description;
    j.at("description").get_to(description);
    std::string icon;
    j.at("icon").get_to(icon);

    return Weather{
        id,
        main,
        description,
        icon};
}

Forecast Forecast::from_json(const nlohmann::json& j) {
    nlohmann::json::array_t weatherListJson;
    j.at("weather").get_to(weatherListJson);
    Weather weather{};
    if (!weatherListJson.empty()) {
        weather = Weather::from_json(weatherListJson[0]);
    }

    size_t time = 0;
    j.at("dt").get_to(time);
    std::chrono::system_clock::time_point timeTp = std::chrono::system_clock::from_time_t(static_cast<time_t>(time));

    const nlohmann::json& mainJson = j["main"];
    double temp = 0;
    mainJson.at("temp").get_to(temp);
    double feelsLike = 0;
    mainJson.at("feels_like").get_to(feelsLike);

    return Forecast{
        std::move(weather),
        timeTp,
        temp,
        feelsLike};
}
}  // namespace backend::weather