project(
    kpartsplugin
)

cmake_minimum_required(
    VERSION
    2.6
)

find_package(
    Qt4
    REQUIRED
)
find_package(
    KDE4
    REQUIRED
)

if(
    NOT
    NSPLUGIN_INSTALL_DIR
)
    set(
        NSPLUGIN_INSTALL_DIR
        "/usr/lib${LIB_SUFFIX}/nsbrowser/plugins/"
        CACHE
        PATH
        "NS/Mozilla Browser plugin path"
    )
endif(
    NOT
    NSPLUGIN_INSTALL_DIR
)

# translations
## FIXME translations do not work (yet)
##add_subdirectory( po )

# source code
add_subdirectory(
    src
)
