cmake_minimum_required(VERSION 3.16)

project(Artichoke LANGUAGES CXX)

# Require C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find dependencies
find_package(imgui CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLUT REQUIRED)
find_package(OpenGL REQUIRED)

# Add source files
file(GLOB SRC_FILES src/*.cpp src/*.hpp)
add_executable(Artichoke ${SRC_FILES})

# Set debug working directory
set_target_properties(Artichoke PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:Artichoke>"
)

# Link and include OpenCV
target_link_libraries(Artichoke PRIVATE ${OpenCV_LIBS})
target_include_directories(Artichoke PRIVATE ${OpenCV_INCLUDE_DIRS})

# Link and include FreeGLUT
target_link_libraries(Artichoke PRIVATE GLUT::GLUT)
target_include_directories(Artichoke PRIVATE ${GLUT_INCLUDE_DIR})

# Link and include GLEW
find_package(GLEW REQUIRED)
target_link_libraries(Artichoke PRIVATE GLEW::GLEW)

# Include glm headers
target_link_libraries(Artichoke PRIVATE glm::glm-header-only)

# Add Dear ImGui sources
target_sources(Artichoke PRIVATE
    lib/imgui/imgui.cpp
    lib/imgui/imgui_draw.cpp
    lib/imgui/imgui_tables.cpp
    lib/imgui/imgui_widgets.cpp
    lib/imgui/backends/imgui_impl_opengl3.cpp
    lib/imgui/backends/imgui_impl_glut.cpp
)

# Link Dear ImGui libraries
target_include_directories(Artichoke PRIVATE
    lib/imgui
    lib/imgui/backends
)

# Copy the entire res directory to the build output directory after build
add_custom_command(TARGET Artichoke POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/res"
    "$<TARGET_FILE_DIR:Artichoke>/res")
