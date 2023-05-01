cmake_minimum_required(VERSION 3.16)

add_library(backend_teams Connection.hpp
                          Connection.cpp)

target_link_libraries(backend_teams PRIVATE logger ixwebsocket
                                    PUBLIC nlohmann_json::nlohmann_json eventpp::eventpp cpr::cpr)
