cmake_minimum_required(VERSION 3.16)
project(retrocenter VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2)

qt_add_executable(retrocenter
    src/main.cpp
    src/library.cpp
    src/library.h
    resources.qrc
)

target_link_libraries(retrocenter PRIVATE Qt6::Quick Qt6::QuickControls2)

if(UNIX AND NOT APPLE)
    get_filename_component(_qt_lib_dir "${Qt6_DIR}/../.." ABSOLUTE)
    set_target_properties(retrocenter PROPERTIES
        BUILD_RPATH "\$ORIGIN/../lib;${_qt_lib_dir}"
        INSTALL_RPATH "\$ORIGIN/../lib;${_qt_lib_dir}"
        LINK_FLAGS "-Wl,--disable-new-dtags"
    )
endif()
