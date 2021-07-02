cmake_minimum_required(VERSION 3.10)

project(h264decpp VERSION 0.1)

option(H264DECPP_BUILD_DEMO "Build the h264decpp-demo binary" ON)

include(cmake/build_type.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)

aux_source_directory(upstream/ffmpeg-h264-dec/ffmpeg-src/libavcodec/ upstream-ffmpeg-libavcodec-src)
aux_source_directory(upstream/ffmpeg-h264-dec/ffmpeg-src/libavutil/ upstream-ffmpeg-libavutil-src)

set(upstream-ffmpeg-src ${upstream-ffmpeg-libavutil-src} ${upstream-ffmpeg-libavcodec-src})

list(FILTER upstream-ffmpeg-src EXCLUDE REGEX "_template\.c$")

add_library(upstream-ffmpeg-h264
    ${upstream-ffmpeg-src}
)

target_compile_options(upstream-ffmpeg-h264 PRIVATE "-w")

target_include_directories(upstream-ffmpeg-h264 PUBLIC
    upstream/ffmpeg-h264-dec/ffmpeg-src/
)

add_library(h264decpp
    src/decoder.cpp
)

target_compile_options(h264decpp PRIVATE "-w")

target_include_directories(h264decpp PUBLIC
    upstream/ffmpeg-h264-dec/ffmpeg-src/
    include/
)

target_link_libraries(h264decpp PUBLIC
    upstream-ffmpeg-h264
)

add_dependencies(h264decpp
    upstream-ffmpeg-h264
)

if(H264DECPP_BUILD_DEMO)
    add_executable(h264decpp-demo
        demo/main.cpp
        demo/readers/file.cpp
        demo/readers/h264.cpp
    )

    target_include_directories(h264decpp-demo PUBLIC
        include/
        demo/
    )

    target_link_libraries(h264decpp-demo PUBLIC
        h264decpp
    )

    add_dependencies(h264decpp
        h264decpp
    )
endif()