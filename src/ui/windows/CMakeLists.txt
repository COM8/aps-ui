cmake_minimum_required(VERSION 3.16)

add_library(ui_windows MainWindow.hpp
                       MainWindow.cpp)

target_link_libraries(ui_windows PUBLIC PkgConfig::GTKMM ui_widgets ui_widgets_gitlab ui_widgets_teams
                                 PRIVATE logger ui_utils backend_systemUtils)
