cmake_minimum_required(VERSION 3.21)
project(heretic)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)

# Imgui
file(GLOB_RECURSE IMGUI_SOURCES "lib/cimgui/*.cpp")
add_library(cimgui STATIC ${IMGUI_SOURCES})
target_include_directories(cimgui PRIVATE lib/cimgui)

# Sokol
file(GLOB_RECURSE SOKOL_SOURCES "lib/sokol/*.c")
add_library(sokol STATIC ${SOKOL_SOURCES})
target_include_directories(sokol PRIVATE lib/sokol lib/cimgui)
target_link_libraries(sokol PRIVATE X11 Xi Xcursor GL dl m)

# Heretic
file(GLOB_RECURSE HERETIC_SOURCES "src/*.c" "src/*.h")
add_executable(heretic ${HERETIC_SOURCES})
target_include_directories(heretic SYSTEM PRIVATE lib/sokol lib/cimgui lib/stb)
target_link_libraries(heretic sokol cimgui)

set_source_files_properties(
  ${HERETIC_SOURCES}
  PROPERTIES
  COMPILE_FLAGS "-Wall -Wextra -Wpedantic -Werror -Werror=vla"
)
