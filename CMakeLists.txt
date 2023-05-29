cmake_minimum_required(VERSION 3.5)

project(TreeModel VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql)

#set(PROJECT_SOURCES
#        main.cc
#        mainwindow.cc
#        mainwindow.h
#        mainwindow.ui
#        treemodel.h
#        treemodel.cc
#        SqlTable.md
#        tablemodel.h tablemodel.cc
#        comboboxdelegate.cc
#        comboboxdelegate.h
#)

# Automatic file addition
file(GLOB PROJECT_SOURCES
    "*.h"
    "*.cc"
    "*.ui"
    "*.md"
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(TreeModel
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET TreeModel APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(TreeModel SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(TreeModel
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(TreeModel PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt6::Sql)

set_target_properties(TreeModel PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS TreeModel
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(TreeModel)
endif()
