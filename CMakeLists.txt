cmake_minimum_required(VERSION 3.16)
project(midisniff VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
    rtmidi
    GIT_REPOSITORY https://github.com/thestk/rtmidi.git
    GIT_TAG 6.0.0
)

set(RTMIDI_BUILD_TESTING OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(rtmidi)

add_executable(midisniff
    src/main.cpp
    src/Logger.cpp
    src/MidiFilter.cpp
    src/MidiMessageFormatter.cpp
)

target_link_libraries(midisniff PRIVATE rtmidi)
target_include_directories(midisniff PRIVATE src)
