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

include_directories(lib)

set(PROJECT_SOURCES
    main.cpp
    MainWindow.cpp
    MainWindow.h
    MainWindow.ui
    FrameMapArea.cpp
    FrameMapArea.h
    FrameMapArea.ui
    )

file(GLOB_RECURSE _lib_sources lib/*)

add_executable(qqt_map_editor_fin
    ${PROJECT_SOURCES}
    ${_lib_sources}
    )

target_include_directories(${PROJECT_NAME} PUBLIC lib)
target_precompile_headers(${PROJECT_NAME} PRIVATE stlpch.h)
target_link_libraries(qqt_map_editor_fin PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(qqt_map_editor_fin PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    )
