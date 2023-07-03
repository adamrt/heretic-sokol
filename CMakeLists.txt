cmake_minimum_required(VERSION 3.21)
project(heretic)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_library(cimgui STATIC
    lib/cimgui/cimgui.cpp
    lib/cimgui/cimgui.h
    lib/cimgui/imgui/imgui.cpp
    lib/cimgui/imgui/imgui.h
    lib/cimgui/imgui/imgui_widgets.cpp
    lib/cimgui/imgui/imgui_draw.cpp
    lib/cimgui/imgui/imgui_tables.cpp
    lib/cimgui/imgui/imgui_demo.cpp)
target_include_directories(cimgui INTERFACE lib/cimgui)

set(SOKOL_HEADERS
    lib/sokol/sokol_gfx.h
    lib/sokol/sokol_app.h
    lib/sokol/sokol_imgui.h
    lib/sokol/sokol_glue.h)

add_library(sokol STATIC lib/sokol/sokol.c ${SOKOL_HEADERS})
target_link_libraries(sokol INTERFACE X11 Xi Xcursor GL dl m)
target_link_libraries(sokol PUBLIC Threads::Threads)

target_link_libraries(sokol PUBLIC cimgui)
target_include_directories(sokol INTERFACE lib/sokol lib )

add_executable(heretic src/main.c src/mesh.c)
target_link_libraries(heretic sokol)

if (CMAKE_C_COMPILER_ID MATCHES "Clang")
    target_link_options(demo PRIVATE LINKER:-dead_strip)
else()
    target_link_options(heretic PRIVATE LINKER:--gc-sections)
endif()
