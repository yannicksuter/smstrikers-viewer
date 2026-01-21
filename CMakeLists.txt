cmake_minimum_required(VERSION 3.15)
project(SMStrikersViewer VERSION 0.1.0 LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IDEs
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Platform detection
if(WIN32)
    set(PLATFORM_WINDOWS TRUE)
elseif(APPLE)
    set(PLATFORM_MACOS TRUE)
elseif(UNIX)
    set(PLATFORM_LINUX TRUE)
endif()

# Compiler-specific options
if(MSVC)
    add_compile_options(/W4)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")

# Find required packages
find_package(OpenGL REQUIRED)

# Option to build with system libraries or fetch them
option(USE_SYSTEM_LIBS "Use system libraries instead of fetching" OFF)

# =============================================================================
# External Libraries
# =============================================================================

# GLFW - Window and input management
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glfw)

# GLAD - OpenGL loader (using cmake subdirectory)
add_subdirectory(external/glad-source/cmake)
glad_add_library(glad_gl_core_33 REPRODUCIBLE API gl:core=3.3)

# GLM - Mathematics library
add_subdirectory(external/glm)

# ImGui - UI library
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui)
file(GLOB IMGUI_SOURCES 
    ${IMGUI_DIR}/*.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC 
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)
target_link_libraries(imgui 
    PRIVATE glfw OpenGL::GL
    INTERFACE glfw  # Need GLFW headers for imgui_impl_glfw.h
)
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)

# ImGuizmo - 3D manipulation gizmos
set(IMGUIZMO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/ImGuizmo)
add_library(imguizmo STATIC 
    ${IMGUIZMO_DIR}/ImGuizmo.cpp
    ${IMGUIZMO_DIR}/GraphEditor.cpp
    ${IMGUIZMO_DIR}/ImCurveEdit.cpp
    ${IMGUIZMO_DIR}/ImGradient.cpp
    ${IMGUIZMO_DIR}/ImSequencer.cpp
)
target_include_directories(imguizmo PUBLIC ${IMGUIZMO_DIR})
target_link_libraries(imguizmo 
    PRIVATE imgui
    INTERFACE imgui  # Need imgui headers for ImGuizmo.h
)

# nlohmann/json - JSON parsing library (header-only)
add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}/external/json/include
)

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/external
    ${OPENGL_INCLUDE_DIRS}
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${IMGUIZMO_DIR}
)

# Source files
set(VIEWER_SOURCES
    src/main.cpp
    src/viewer.cpp
    src/camera.cpp
    src/mesh.cpp
    src/config.cpp
)

set(VIEWER_HEADERS
    include/viewer.h
    include/camera.h
    include/mesh.h
    include/config.h
)

# Create executable
add_executable(${PROJECT_NAME} ${VIEWER_SOURCES} ${VIEWER_HEADERS})

# Link libraries
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        OpenGL::GL
        glad_gl_core_33
        glm::glm
        imgui
        imguizmo
        nlohmann_json
)

# Platform-specific linking
if(PLATFORM_MACOS)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
elseif(PLATFORM_LINUX)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${CMAKE_DL_LIBS}
        pthread
    )
endif()

# Set target properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "smstrikers-viewer"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Install targets (optional)
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Print summary
message(STATUS "=================================")
message(STATUS "Super Mario Strikers Viewer")
message(STATUS "=================================")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Install Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "=================================")
