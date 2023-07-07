cmake_minimum_required(VERSION 3.16)

add_library(backend_websocket Websocket.cpp
                              Websocket.hpp)

target_link_libraries(backend_websocket PRIVATE CURL::libcurl
                                                logger
                                        PUBLIC eventpp::eventpp)
