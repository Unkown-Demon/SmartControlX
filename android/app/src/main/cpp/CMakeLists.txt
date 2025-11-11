cmake_minimum_required(VERSION 3.22.1)

project("smartcontrolx")

add_library(
    smartcontrolx
    SHARED
    native-lib.cpp
    video_encoder.cpp
    network_manager.cpp
)

# Link against Android NDK libraries for logging, media, and native window
target_link_libraries(
    smartcontrolx
    android
    log
    mediandk
    jnigraphics
    # Link against system libraries for networking (sockets)
    # The system library for sockets is usually just 'log' and 'android' on modern NDK,
    # but we explicitly add 'c' and 'm' for standard C/math functions.
    c
    m
)
