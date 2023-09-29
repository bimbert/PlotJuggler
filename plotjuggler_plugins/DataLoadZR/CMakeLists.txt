
include_directories(  ../ )

add_definitions(${QT_DEFINITIONS})
add_definitions(-DQT_PLUGIN)

QT5_WRAP_UI ( UI_SRC
    dataload_zr.ui
    zr_data_dialog.ui
    )

SET( SRC
    dataload_zr.cpp
    zr_parser.cpp
    zr_data_dialog.cpp
    )

add_library(DataLoadZR SHARED ${SRC} ${UI_SRC})

target_link_libraries(DataLoadZR
    ${Qt5Widgets_LIBRARIES}
    ${Qt5Xml_LIBRARIES}
    plotjuggler_base)

install(TARGETS DataLoadZR DESTINATION ${PJ_PLUGIN_INSTALL_DIRECTORY})
