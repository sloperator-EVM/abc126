cmake_minimum_required(VERSION 3.16)
project(winrun C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(winrun
    src/main.c
    src/pe_parser.c
    src/memory_mapper.c
    src/relocations.c
    src/import_resolver.c
    src/runtime.c
    src/signals.c
)

target_include_directories(winrun PRIVATE include)
target_link_libraries(winrun PRIVATE dl)

if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(winrun PRIVATE -Wall -Wextra -Wpedantic)
endif()

file(GLOB WG_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/.wg/*.c")
if (WG_SOURCES)
    set(WG_PROTOCOL_SOURCES
        "${CMAKE_SOURCE_DIR}/.wg/tmp/single-pixel-buffer-v1.c"
        "${CMAKE_SOURCE_DIR}/.wg/tmp/viewporter.c"
        "${CMAKE_SOURCE_DIR}/.wg/tmp/wlr-layer-shell-unstable-v1.c"
        "${CMAKE_SOURCE_DIR}/.wg/tmp/wlr-virtual-pointer-unstable-v1.c"
        "${CMAKE_SOURCE_DIR}/.wg/tmp/xdg-shell.c"
    )

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(WAYLAND_CLIENT QUIET wayland-client)
    pkg_check_modules(LIBINPUT QUIET libinput)
    pkg_check_modules(LIBUDEV QUIET libudev)
    if (NOT WAYLAND_CLIENT_FOUND OR NOT LIBINPUT_FOUND OR NOT LIBUDEV_FOUND)
        message(FATAL_ERROR "Detected .wg WinAPI sources but could not find wayland-client, libinput, and libudev via pkg-config. Install Wayland + input development packages and retry.")
    endif()

    add_library(winapi SHARED ${WG_SOURCES} ${WG_PROTOCOL_SOURCES})
    set_target_properties(winapi PROPERTIES
        OUTPUT_NAME winapi
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    )
    target_include_directories(winapi PRIVATE
        ${CMAKE_SOURCE_DIR}/.wg
        ${CMAKE_SOURCE_DIR}/.wg/tmp
        ${WAYLAND_CLIENT_INCLUDE_DIRS}
        ${LIBINPUT_INCLUDE_DIRS}
        ${LIBUDEV_INCLUDE_DIRS}
    )
    target_link_libraries(winapi PRIVATE ${WAYLAND_CLIENT_LIBRARIES} ${LIBINPUT_LIBRARIES} ${LIBUDEV_LIBRARIES} pthread)
    target_compile_options(winapi PRIVATE ${WAYLAND_CLIENT_CFLAGS_OTHER} ${LIBINPUT_CFLAGS_OTHER} ${LIBUDEV_CFLAGS_OTHER})
    if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(winapi PRIVATE -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-sign-compare)
    endif()
    message(STATUS "Detected .wg WinAPI sources; will build libwinapi.so")
endif()
