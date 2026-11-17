cmake_minimum_required(VERSION 3.15)
project(pikachu LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT WIN32)
    message(FATAL_ERROR
        "pikachu is a Win32 program. Build it on Windows, or "
        "cross-compile with mingw-w64 (see README.md).")
endif()

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/assets")
    message(FATAL_ERROR "assets/ is missing")
endif()

file(GLOB PIKA_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")

add_executable(pikachu WIN32 ${PIKA_SOURCES} res/pikachu.rc)

target_include_directories(pikachu PRIVATE src)
target_link_libraries(pikachu PRIVATE user32 gdi32 winmm dsound)

# res/pikachu.rc refers to assets/... relative to the project root, while
# CMake runs the resource compiler inside the build directory.
if(MSVC)
    target_compile_options(pikachu PRIVATE /utf-8 /W3)
    set(CMAKE_RC_FLAGS "${CMAKE_RC_FLAGS} /I \"${CMAKE_SOURCE_DIR}\" /c 65001")
else()
    target_compile_options(pikachu PRIVATE -Wall -Wextra)
    if(MINGW)
        # Keep libstdc++/libgcc/libwinpthread inside the executable.
        target_link_options(pikachu PRIVATE -static)
    endif()
    set(CMAKE_RC_FLAGS "${CMAKE_RC_FLAGS} -I \"${CMAKE_SOURCE_DIR}\" --codepage=65001")
endif()
