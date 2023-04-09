cmake_minimum_required(VERSION 3.16)

add_library(ui_widgets_gitlab PlotWidget.cpp
                              PlotWidget.hpp)
target_link_libraries(ui_widgets_gitlab PUBLIC PkgConfig::GTKMM 
                                        PRIVATE logger backend_storage ui_utils backend_gitlab backend_storage)
