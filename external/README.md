# External Dependencies

This directory contains third-party libraries used by the Super Mario Strikers Viewer.

## Adding Dependencies

You can add external dependencies using one of these methods:

### Method 1: Git Submodules (Recommended)

For libraries available on GitHub/Git:

```bash
# Add a submodule
git submodule add https://github.com/organization/library.git external/library

# Update all submodules
git submodule update --init --recursive
```

### Method 2: CMake FetchContent

Add to `CMakeLists.txt`:

```cmake
include(FetchContent)

FetchContent_Declare(
    library_name
    GIT_REPOSITORY https://github.com/organization/library.git
    GIT_TAG        v1.0.0
)
FetchContent_MakeAvailable(library_name)
```

### Method 3: System Libraries

Use your system's package manager and find_package in CMake.

## Recommended Libraries

These open-source libraries are recommended for the viewer:

### Window & Input Management
- **GLFW** (https://github.com/glfw/glfw) - Cross-platform window creation
- **SDL2** (https://github.com/libsdl-org/SDL) - Alternative to GLFW

### OpenGL Loading
- **GLAD** (https://github.com/Dav1dde/glad) - OpenGL loader
- **GLEW** (https://github.com/nigels-com/glew) - Alternative loader

### Mathematics
- **GLM** (https://github.com/g-truc/glm) - OpenGL Mathematics

### Image Loading
- **stb_image** (https://github.com/nothings/stb) - Single-header image loader

### UI (Optional)
- **Dear ImGui** (https://github.com/ocornut/imgui) - Immediate mode GUI

### File Formats
- **nlohmann/json** (https://github.com/nlohmann/json) - JSON parsing
- **pugixml** (https://github.com/zeux/pugixml) - XML parsing

## Example: Adding GLFW

```bash
cd external
git submodule add https://github.com/glfw/glfw.git glfw
```

Then in `CMakeLists.txt`:

```cmake
# Add GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glfw)

# Link to target
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
```

## License Compliance

Ensure all third-party libraries are compatible with this project's license.
Always review and comply with each library's license terms.
