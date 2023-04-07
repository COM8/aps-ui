#include "backend/storage/Serializer.hpp"
#include "backend/storage/Settings.hpp"

#include <chrono>
#include <cstddef>
#include <functional>
namespace backend::storage {
void to_json(nlohmann::json& j, const SettingsData& d) {
    j = nlohmann::json{
        {"dbStationId", d.dbStationId},
        {"dbLookAheadCount", d.dbLookAheadCount},
        {"dbLookBehindCount", d.dbLookBehindCount},
        {"dbDestRegex", d.dbDestRegex},
        {"dbDestRegexEnabled", d.dbDestRegexEnabled},
        {"dbStopsAtRegex", d.dbStopsAtRegex},
        {"dbStopsAtRegexEnabled", d.dbStopsAtRegexEnabled},
        {"dbFilterDepartedTrains", d.dbFilterDepartedTrains},

        {"weatherLat", d.weatherLat},
        {"weatherLong", d.weatherLong},
        {"openWeatherApiKey", d.openWeatherApiKey},

        {"devices", d.devices},

        {"lightningMapCenterLat", d.lightningMapCenterLat},
        {"lightningMapCenterLong", d.lightningMapCenterLong},
        {"lightningMapApsLat", d.lightningMapApsLat},
        {"lightningMapApsLong", d.lightningMapApsLong},
        {"lightningMapZoomLevel", d.lightningMapZoomLevel}};
}

void from_json(const nlohmann::json& j, SettingsData& d) {
    j.at("dbStationId").get_to(d.dbStationId);
    j.at("dbLookAheadCount").get_to(d.dbLookAheadCount);
    j.at("dbLookBehindCount").get_to(d.dbLookBehindCount);
    j.at("dbDestRegex").get_to(d.dbDestRegex);
    j.at("dbDestRegexEnabled").get_to(d.dbDestRegexEnabled);
    j.at("dbStopsAtRegex").get_to(d.dbStopsAtRegex);
    j.at("dbStopsAtRegexEnabled").get_to(d.dbStopsAtRegexEnabled);
    j.at("dbFilterDepartedTrains").get_to(d.dbFilterDepartedTrains);

    j.at("weatherLat").get_to(d.weatherLat);
    j.at("weatherLong").get_to(d.weatherLong);
    j.at("openWeatherApiKey").get_to(d.openWeatherApiKey);

    j.at("devices").get_to(d.devices);

    j.at("lightningMapCenterLat").get_to(d.lightningMapCenterLat);
    j.at("lightningMapCenterLong").get_to(d.lightningMapCenterLong);
    j.at("lightningMapApsLat").get_to(d.lightningMapApsLat);
    j.at("lightningMapApsLong").get_to(d.lightningMapApsLong);
    j.at("lightningMapZoomLevel").get_to(d.lightningMapZoomLevel);
}
}  // namespace backend::storage

namespace nlohmann {
void adl_serializer<std::chrono::system_clock::time_point>::to_json(json& j, const std::chrono::system_clock::time_point& tp) {
    j = std::chrono::system_clock::to_time_t(tp);
}

void adl_serializer<std::chrono::system_clock::time_point>::from_json(const json& j, std::chrono::system_clock::time_point& tp) {
    std::time_t t = 0;
    j.get_to(t);
    tp = std::chrono::system_clock::from_time_t(t);
}
}  // namespace nlohmann
