cmake_minimum_required(VERSION 3.16)
project(tetris VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia)
find_program(LRELEASE NAMES lrelease HINTS /usr/lib/qt6/bin /usr/lib/qt6/bin REQUIRED)

file(GLOB TS_FILES src/translations/*.ts)

set(QM_FILES "")
set(QRC_ENTRIES "")
foreach(TS_FILE ${TS_FILES})
    get_filename_component(TS_NAME ${TS_FILE} NAME_WE)
    set(QM_FILE "${CMAKE_CURRENT_BINARY_DIR}/${TS_NAME}.qm")
    add_custom_command(
        OUTPUT ${QM_FILE}
        COMMAND ${LRELEASE} ${TS_FILE} -qm ${QM_FILE}
        DEPENDS ${TS_FILE}
        COMMENT "Compiling ${TS_NAME}.ts"
    )
    list(APPEND QM_FILES ${QM_FILE})
    string(APPEND QRC_ENTRIES "    <file>${TS_NAME}.qm</file>\n")
endforeach()

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/translations.qrc"
    "<RCC>\n    <qresource prefix=\"/translations\">\n${QRC_ENTRIES}    </qresource>\n</RCC>\n"
)

find_package(dtk6widget QUIET)
if(NOT dtk6widget_FOUND)
    find_package(Dtk6Widget REQUIRED)
endif()

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
    "${CMAKE_CURRENT_BINARY_DIR}/translations.qrc"
    ${QM_FILES}
)

target_link_libraries(tetris PRIVATE
    Qt6::Widgets
    Qt6::Multimedia
    Dtk6::Widget
)

install(TARGETS tetris DESTINATION bin)
install(FILES src/org.deepinenespanol.tetris.desktop DESTINATION share/applications)
install(FILES src/icons/tetris.svg DESTINATION share/icons/hicolor/scalable/apps)
