
include_directories(  ../ )

add_definitions(${QT_DEFINITIONS})
add_definitions(-DQT_PLUGIN)

QT5_WRAP_UI ( UI_SRC
    usd1_data_dialog.ui
    )

SET( SRC
    dataload_usd1.cpp
    usd1_parser.cpp
    usd1_data_dialog.cpp
    )

add_library(DataLoadUsd1 SHARED ${SRC} ${UI_SRC})

target_link_libraries(DataLoadUsd1
    ${Qt5Widgets_LIBRARIES}
    ${Qt5Xml_LIBRARIES}
    plotjuggler_base)

install(TARGETS DataLoadUsd1 DESTINATION ${PJ_PLUGIN_INSTALL_DIRECTORY})
