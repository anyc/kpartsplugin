find_package(
    KDE4
    4.3
    REQUIRED
)
include(
    KDE4Defaults
)

add_definitions(
    ${QT_DEFINITIONS}
    ${KDE4_DEFINITIONS}
)
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${KDE4_INCLUDES}
)

set(
    kpartsplugin_LIB_SRCS
    kpartsplugin.cpp
    mimetypestreeview.cpp
    mimetypehelper.cpp
    qtbrowserplugin.cpp
    qtbrowserplugin_x11.cpp
)

kde4_add_library(
    kpartsplugin
    SHARED
    ${kpartsplugin_LIB_SRCS}
)

target_link_libraries(
    kpartsplugin
    ${QT_QTGUI_LIBRARY}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KPARTS_LIBS}
)

install(
    TARGETS
    kpartsplugin
    DESTINATION
    ${NSPLUGIN_INSTALL_DIR}
)

########## KCM Module ###############

set(
    kcm_kpartsplugin_SRCS
    kcm_kpartsplugin.cpp
    mimetypestreeview.cpp
    mimetypehelper.cpp
)

kde4_add_plugin(
    kcm_kpartsplugin
    ${kcm_kpartsplugin_SRCS}
)

target_link_libraries(
    kcm_kpartsplugin
    ${KDE4_KDEUI_LIBS}
)

install(
    TARGETS
    kcm_kpartsplugin
    DESTINATION
    ${PLUGIN_INSTALL_DIR}
)

install(
    FILES
    kcm_kpartsplugin.desktop
    DESTINATION
    ${SERVICES_INSTALL_DIR}
)
