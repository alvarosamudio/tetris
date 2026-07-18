cmake_minimum_required(VERSION 3.16)
project(tetris VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia)
find_package(Dtk6Widget REQUIRED)

add_executable(tetris
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/gameboard.cpp
    src/gameboard.h
    src/tetrisgame.cpp
    src/tetrisgame.h
    src/nextpiecewidget.cpp
    src/nextpiecewidget.h
    src/soundmanager.cpp
    src/soundmanager.h
    src/settingsdialog.cpp
    src/settingsdialog.h
    src/widget.cpp
    src/widget.h
    src/tetriscolors.h
    src/resources.qrc
)

target_link_libraries(tetris PRIVATE
    Qt6::Widgets
    Qt6::Multimedia
    Dtk6::Widget
)

install(TARGETS tetris DESTINATION bin)
