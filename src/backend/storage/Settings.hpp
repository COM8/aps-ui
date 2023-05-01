#pragma once

#include "backend/storage/Serializer.hpp"

#include <filesystem>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace backend::storage {
struct SettingsData {
    /**
     * The location id, where the DB widget should fetch departures for.
     * Default value: "8003022" (Hulb)
     * Head over to [0] and get the location ID from there (Developer Tools -> XHR)
     * [0]: https://bahn.expert/
     **/
    std::string dbStationId = "8003022";
    int dbLookAheadCount = 150;
    int dbLookBehindCount = 10;
    std::string dbDestRegex{"Stuttgart"};
    bool dbDestRegexEnabled = true;
    std::string dbStopsAtRegex{"Hulb"};
    bool dbStopsAtRegexEnabled = true;
    bool dbFilterDepartedTrains = true;

    /**
     * The open weather map location for the weather forecast. 
     **/
    std::string weatherLat{"48.6777281"};
    std::string weatherLong{"8.9795143"};
    std::string openWeatherApiKey{"https://openweathermap.org/price"};

    /**
     * A list of all device IPs there status should be displayed.
     **/
    std::vector<std::string> devices;

    /**
     * Lightning map configuration.
     **/
    double lightningMapCenterLat{48.6777281};
    double lightningMapCenterLong{8.9795143};
    double lightningMapApsLat{48.6777281};
    double lightningMapApsLong{8.9795143};
    double lightningMapZoomLevel{13};

    /**
     * Microsoft Teams related options.
     **/
    std::string teamsHookEndpoint{"The endpoint for the teams hook: https://github.com/COM8/teams-hook"};
    std::string teamsHookAuthentication{"Your authentication token from the teams hook: https://github.com/COM8/teams-hook"};

    /**
     * GitLab runner metrics endpoint.
     * https://docs.gitlab.com/runner/monitoring/
     **/
    std::string gitlabRunnerUrl{"The URL to your GitLab runner metrics endpoint. For example: http://10.0.0.42:9252/metrics"};

} __attribute__((aligned(128)));

class Settings {
 public:
    explicit Settings(const std::filesystem::path& configFilePath);

    SettingsData data{};
    void write_settings();

 private:
    storage::Serializer fileHandle;
};

Settings* get_settings_instance();
}  // namespace backend::storage
