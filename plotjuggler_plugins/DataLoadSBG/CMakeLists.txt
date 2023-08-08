
include_directories(  ../ )

add_definitions(${QT_DEFINITIONS})
add_definitions(-DQT_PLUGIN)

QT5_WRAP_UI ( UI_SRC
    dataload_sbg.ui
    sbg_data_dialog.ui
    )

file(GLOB_RECURSE SBG_SRC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "sbg/*.c")
if (NOT WIN32)
    list(REMOVE_ITEM SBG_SRC "sbg/common/interfaces/sbgInterfaceSerialWin.c")
else ()
    list(REMOVE_ITEM SBG_SRC "sbg/common/interfaces/sbgInterfaceSerialUnix.c")
    set(DataLoadSBG_LIBRARIES ws2_32)
endif()

SET( SRC
    dataload_sbg.cpp
    sbg_parser.cpp
    sbg_data_dialog.cpp
    )

add_library(DataLoadSBG SHARED ${SRC} ${UI_SRC} ${SBG_SRC} )

target_link_libraries(DataLoadSBG
    ${DataLoadSBG_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    ${Qt5Xml_LIBRARIES}
    plotjuggler_base)

target_include_directories(DataLoadSBG AFTER
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/sbg/common
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/sbg/src)

install(TARGETS DataLoadSBG DESTINATION ${PJ_PLUGIN_INSTALL_DIRECTORY}  )

