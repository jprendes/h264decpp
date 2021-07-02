cmake_minimum_required(VERSION 3.10)

project(aacpp VERSION 0.1)

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

add_executable(h264decpp
    src/main.cpp
)

target_include_directories(h264decpp PUBLIC
    upstream/ffmpeg-h264-dec/ffmpeg-src/
)

target_link_libraries(h264decpp PUBLIC
    upstream-ffmpeg-h264
)

add_dependencies(h264decpp
    upstream-ffmpeg-h264
)