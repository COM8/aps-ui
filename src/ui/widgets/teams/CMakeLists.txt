cmake_minimum_required(VERSION 3.16)

add_library(ui_widgets_teams TeamsWidget.cpp
                             TeamsWidget.hpp)
target_link_libraries(ui_widgets_teams PUBLIC PkgConfig::GTKMM backend_teams
                                        PRIVATE logger backend_storage ui_utils)
