cmake_minimum_required(VERSION 3.16)
project(MarvelTowerDefense)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SFML
set(SFML_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/SFML/include")
set(SFML_LIB_DIR "${CMAKE_SOURCE_DIR}/SFML/lib")

# OpenGL
find_package(OpenGL REQUIRED)

# ImGui sources
set(IMGUI_SOURCES
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_demo.cpp
    imgui-sfml/imgui-SFML.cpp
)

# Main executable
add_executable(${PROJECT_NAME}
    src/main.cpp
    ${IMGUI_SOURCES}
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${SFML_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/imgui
    ${CMAKE_SOURCE_DIR}/imgui-sfml
)

# Link directories
target_link_directories(${PROJECT_NAME} PRIVATE
    ${SFML_LIB_DIR}
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    sfml-graphics
    sfml-window
    sfml-audio
    sfml-system
    OpenGL::GL
    winmm
    gdi32
)

# Copy SFML DLLs to output directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/SFML/bin"
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

# Copy assets directory to output directory (so game can find assets)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/assets"
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
)

message(STATUS "SFML Include: ${SFML_INCLUDE_DIR}")
message(STATUS "SFML Lib: ${SFML_LIB_DIR}")
