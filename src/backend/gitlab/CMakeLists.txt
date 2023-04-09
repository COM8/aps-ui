cmake_minimum_required(VERSION 3.16)

add_library(backend_gitlab GitLabHelper.hpp
                           GitLabHelper.cpp)

target_link_libraries(backend_gitlab PRIVATE logger cpr::cpr
                                     PUBLIC nlohmann_json::nlohmann_json)
