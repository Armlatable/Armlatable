
cmake_minimum_required(VERSION 3.14)
project(LocomoHost)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(include)
include_directories(../module/src)

# --- Dear ImGui (FetchContent) ---
include(FetchContent)

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui
  GIT_TAG        v1.90.1
)
FetchContent_MakeAvailable(imgui)

# --- GLFW (Required for ImGui) ---
find_package(glfw3 3.3 QUIET)
if (NOT glfw3_FOUND)
  FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw
    GIT_TAG        3.3.8
  )
  FetchContent_MakeAvailable(glfw)
endif()

# Define sources for ImGui
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

# --- Targets ---

# 1. CLI Main App
add_executable(locomo_cli
    apps/main.cpp
    src/algorithm/algorithm_teleop.cpp
)

# 2. GUI Viewer App
add_executable(locomo_viewer
    apps/viewer.cpp
    src/algorithm/algorithm_teleop.cpp
    ${IMGUI_SOURCES}
)

# Include directories for ImGui
target_include_directories(locomo_viewer PRIVATE
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

# Link libraries
# macOS OpenGL framework
if (APPLE)
    find_library(OPENGL_LIB OpenGL)
    target_link_libraries(locomo_viewer PRIVATE glfw ${OPENGL_LIB})
else()
    target_link_libraries(locomo_viewer PRIVATE glfw OpenGL::GL)
endif()

# Link shared module (currently header only mainly, but structure allows src)
# If protocol.cpp had content, we would add it to add_executable sources.
