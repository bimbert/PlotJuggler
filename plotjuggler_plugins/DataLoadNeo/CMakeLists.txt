
include_directories(  ../ )

add_definitions(${QT_DEFINITIONS})
add_definitions(-DQT_PLUGIN)

QT5_WRAP_UI ( UI_SRC
    neo_data_dialog.ui
    )

SET( SRC
    dataload_neo.cpp
    neo_parser.cpp
    neo_data_dialog.cpp
    )

add_library(DataLoadNeo SHARED ${SRC} ${UI_SRC})

target_link_libraries(DataLoadNeo
    ${Qt5Widgets_LIBRARIES}
    ${Qt5Xml_LIBRARIES}
    plotjuggler_base)

install(TARGETS DataLoadNeo DESTINATION ${PJ_PLUGIN_INSTALL_DIRECTORY})
