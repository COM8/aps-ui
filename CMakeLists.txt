cmake_minimum_required(VERSION 3.16)

set(PROJECT_DESCRIPTION "GTK4 Based UI for Stats all Around AP Sensing in Böblingen using a Raspberry Pi.")

project("aps_ui"  VERSION 1.0.0
                  DESCRIPTION "${PROJECT_DESCRIPTION}"
                  HOMEPAGE_URL "https://github.com/COM8/aps-ui")
set(VERSION_NAME "dev")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP in CMake 3.24:
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
    cmake_policy(SET CMP0135 NEW)
endif()

macro(aps_ui_option OPTION_NAME OPTION_TEXT OPTION_DEFAULT)
    option(${OPTION_NAME} ${OPTION_TEXT} ${OPTION_DEFAULT})

    if(DEFINED ENV{${OPTION_NAME}})
        # Allow setting the option through an environment variable
        set(${OPTION_NAME} $ENV{${OPTION_NAME}})
    endif()

    if(${OPTION_NAME})
        add_definitions(-D${OPTION_NAME})
    endif()

    message(STATUS "  ${OPTION_NAME}: ${${OPTION_NAME}}")
endmacro()

message(STATUS "C++ Aps UI Options")
message(STATUS "=======================================================")
aps_ui_option(APS_UI_STATIC_ANALYZE "Set to ON to enable the GCC 10 static analysis. If enabled, APS_UI_ENABLE_LINTING has to be disabled." OFF)
aps_ui_option(APS_UI_ENABLE_LINTING "Set to ON to enable clang linting. If enabled, APS_UI_STATIC_ANALYZE has to be disabled." OFF)
aps_ui_option(APS_UI_FLATPAK_BUILD "Set to ON in case it is built for flatpak." OFF)
message(STATUS "=======================================================")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

include(sanitizer)
include(gcc_analyze)
include(glib-resource-compiler)
include(clear_variable)
include(FetchContent)

if(${APS_UI_ENABLE_LINTING})
    message(STATUS "Enabling linting")
    include(clang-tidy)
else()
    message(STATUS "Linting is disabled")
endif()

if(APS_UI_FLATPAK_BUILD)
    add_compile_definitions(APS_UI_FLATPAK_BUILD=1)
endif()

# gtk4 stuff
include(glib-resource-compiler)
find_package(PkgConfig REQUIRED)

# Additional search path since gtkmm-4.0 will usually be installed in /usr
if(APS_UI_FLATPAK_BUILD)
    set(ENV{PKG_CONFIG_PATH} "/app/lib/pkgconfig")
else()
    set(ENV{PKG_CONFIG_PATH} "/usr/lib/pkgconfig")
endif()
pkg_check_modules(GTKMM gtkmm-4.0>=4.4.0 REQUIRED IMPORTED_TARGET)
pkg_check_modules(EPOXY epoxy REQUIRED IMPORTED_TARGET)

# Lib adwaita
pkg_check_modules(ADWAITA REQUIRED libadwaita-1>=1.1.0)
include_directories(${ADWAITA_INCLUDE_DIRS})

if(APS_UI_FLATPAK_BUILD)
    pkg_check_modules(SHUMATE REQUIRED shumate-1.0>=1.0.0)
    include_directories(${SHUMATE_INCLUDE_DIRS})

    find_package(cpr CONFIG REQUIRED)
    find_package(nlohmann_json CONFIG REQUIRED)
    find_package(spdlog CONFIG REQUIRED)
    find_package(re2 CONFIG REQUIRED)
    find_package(eventpp CONFIG REQUIRED)
    find_package(date CONFIG REQUIRED)
else()
    # libshumate
    include(ExternalProject)
    find_program(MESON_PATH meson REQUIRED)

    if(MESON_PATH STREQUAL "MESON_PATH-NOTFOUND")
        message(FATAL_ERROR "meson not found. Please make sure you have meson installed on your system.")
        return()
    endif()

    ExternalProject_Add(shumate_build GIT_REPOSITORY https://gitlab.gnome.org/GNOME/libshumate.git
                                      GIT_TAG main
                                      CONFIGURE_COMMAND ${MESON_PATH} "<BINARY_DIR>" "<SOURCE_DIR>" -Dgtk_doc=false
                                      BUILD_COMMAND cd "<BINARY_DIR>" && ninja
                                      CONFIGURE_HANDLED_BY_BUILD ON
                                      INSTALL_COMMAND ""
                                      COMMENT "Building shumate"
                                      BUILD_BYPRODUCTS "<BINARY_DIR>/shumate/libshumate-1.0.so")

    add_library(libshumate SHARED IMPORTED)
    set(SHUMATE_LIBRARIES "libshumate")
    add_dependencies(libshumate shumate_build)

    ExternalProject_Get_Property(shumate_build BINARY_DIR)
    set(SHUMATE_LIB "${BINARY_DIR}/shumate/libshumate-1.0.so")
    set_target_properties(libshumate PROPERTIES IMPORTED_LOCATION ${SHUMATE_LIB})

    ExternalProject_Get_Property(shumate_build SOURCE_DIR)
    set(SHUMATE_INCLUDE_DIRS "${SOURCE_DIR}" "${BINARY_DIR}")
    target_include_directories(libshumate INTERFACE "${SHUMATE_INCLUDE_DIRS}")
    include_directories(${SHUMATE_INCLUDE_DIRS})

    # Disable linting for fetch content projects
    clear_variable(DESTINATION CMAKE_CXX_CLANG_TIDY BACKUP CMAKE_CXX_CLANG_TIDY_BKP)

    FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
                             GIT_TAG 1750af467726733b0ccd0fc84a3fe32ab5b0612d
                             PATCH_COMMAND "git" "reset" "--hard" "&&" "git" "apply" "${CMAKE_SOURCE_DIR}/cmake/cpr_patch.patch")
    set(CPR_BUILD_TESTS OFF CACHE INTERNAL " " FORCE)
    set(CURL_CA_FALLBACK ON CACHE INTERNAL " " FORCE) # Workaround for failing SSL requests
    FetchContent_MakeAvailable(cpr)

    FetchContent_Declare(json GIT_REPOSITORY https://github.com/nlohmann/json.git
                              GIT_TAG v3.10.5)
    FetchContent_MakeAvailable(json)

    set(SPDLOG_INSTALL ON CACHE INTERNAL " " FORCE)
    FetchContent_Declare(spdlog GIT_REPOSITORY https://github.com/gabime/spdlog.git
                                GIT_TAG v1.9.2)
    FetchContent_MakeAvailable(spdlog)

    FetchContent_Declare(re2 GIT_REPOSITORY https://github.com/google/re2.git
                             GIT_TAG 2023-03-01)
    FetchContent_MakeAvailable(re2)

    set(BUILD_TZ_LIB ON CACHE INTERNAL " " FORCE)
    FetchContent_Declare(date GIT_REPOSITORY https://github.com/HowardHinnant/date.git
                              GIT_TAG cc4685a21e4a4fdae707ad1233c61bbaff241f93)
    FetchContent_MakeAvailable(date)

    FetchContent_Declare(eventpp
                         GIT_REPOSITORY https://github.com/wqking/eventpp.git
                         GIT_TAG 6813ec217f8489d8105d52008c8aca11093fb749) # State on master from the 28.09.2022
    FetchContent_MakeAvailable(eventpp)

    restore_variable(DESTINATION CMAKE_CXX_CLANG_TIDY BACKUP CMAKE_CXX_CLANG_TIDY_BKP)
endif()

if(APS_UI_FLATPAK_BUILD)
    # Disable Werror flags for flatpak builds to prevent issues with outdated flatpak CIs
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
endif()

string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWER)
if(CMAKE_BUILD_TYPE_LOWER MATCHES ".*release.*")
    message(STATUS "Some sort of release build detected.")
else()
    message(STATUS "Some sort of debug build detected.")
    add_compile_definitions(DEBUG)
endif()

include_directories(${CMAKE_SOURCE_DIR}/src)
add_subdirectory(src)
