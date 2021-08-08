cmake_minimum_required(VERSION 3.16)

project(qqt_map_editor_fin VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)

include_directories(.)
include_directories(lib)

file(GLOB_RECURSE _lib_sources lib/*)
file(GLOB_RECURSE _ui_sources ui/*)

# Set some Win32 Specific Settings
if (NOT CMAKE_BUILD_TYPE MATCHES Debug)
if (WIN32)
set(_gui WIN32)
endif (WIN32)
# Set some Apple MacOS Specific settings
if (APPLE)
set(_gui MACOSX_BUNDLE)
endif (APPLE)
endif ()

add_executable(${PROJECT_NAME}
    ${_gui}
    main.cpp res.qrc
    ${_lib_sources}
    ${_ui_sources}
    )

target_include_directories(${PROJECT_NAME} PUBLIC lib)
target_precompile_headers(${PROJECT_NAME} PRIVATE stlpch.h qtpch.h)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Widgets)

set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    )
