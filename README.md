# Super Mario Strikers - Asset Viewer

A cross-platform model and content viewer for Super Mario Strikers (Nintendo GameCube, 2005).

## ⚠️ Important Legal Disclaimer

**This project does NOT contain any assets from the original game.**

To use this viewer, you **MUST** own a legitimate copy of Super Mario Strikers and extract the assets yourself. This project is strictly for educational and archival purposes, utilizing knowledge gained from the [smstrikers-decomp](https://github.com/yannicksuter/smstrikers-decomp) project.

**No game files, textures, models, or any copyrighted content from Super Mario Strikers will ever be included in this repository.**

## About

This project is a companion tool to the Super Mario Strikers decompilation project. It uses the knowledge gained during the decompilation process to read and interpret game assets for viewing and analysis purposes.

### Current Features
- 🚧 Project in early development

### Planned Features
- 📦 Model viewing (initial focus)
- 🎬 Animation playback
- 📜 Script analysis
- 🔍 Asset inspection tools

### Not in Scope
- This is **NOT** a recompilation project
- This is **NOT** a port to other platforms
- This is **NOT** a game engine

## Technical Stack

- **Language**: C++
- **Graphics**: OpenGL
- **Build System**: CMake (generates Ninja files)
- **Platform Support**: macOS, Linux, Windows

## Building

### Prerequisites

- CMake 3.15 or higher
- C++17 compatible compiler
  - GCC 7+ (Linux)
  - Clang 5+ (macOS)
  - MSVC 2017+ (Windows)
- Ninja build system (optional, but recommended)
- OpenGL development libraries

### Build Instructions

```bash
# Clone the repository
git clone https://github.com/yannicksuter/smstrikers-viewer.git
cd smstrikers-viewer

# Initialize submodules (if any)
git submodule update --init --recursive

# Create build directory
mkdir build && cd build

# Configure with CMake (using Ninja generator)
cmake -G Ninja ..

# Build
ninja

# Or use CMake's generic build command
cmake --build .
```

### Platform-Specific Notes

#### macOS
```bash
# Install dependencies via Homebrew
brew install cmake ninja
```

#### Linux (Debian/Ubuntu)
```bash
# Install dependencies
sudo apt-get install cmake ninja-build build-essential libgl1-mesa-dev
```

#### Windows
- Install Visual Studio 2019 or later with C++ support
- Install CMake from https://cmake.org
- Optionally install Ninja from https://ninja-build.org

## Project Structure

```
smstrikers-viewer/
├── src/           # Source files
├── include/       # Header files
├── external/      # Third-party libraries
├── docs/          # Documentation
├── assets/        # Example/test assets (NOT game assets)
├── build/         # Build output (gitignored)
└── CMakeLists.txt # Build configuration
```

## Usage

### Running the Viewer

```bash
# From the project root directory
./build/bin/smstrikers-viewer

# Or with command-line options
./build/bin/smstrikers-viewer --no_gui          # Headless mode (for testing)
./build/bin/smstrikers-viewer --object Mario    # Preselect an object
```

### Setting Up Assets

1. **Extract game assets** from your legitimate copy of Super Mario Strikers:
   - Use Dolphin Emulator's filesystem browser (Right-click game → Properties → Filesystem)
   - Export files to the `assets/` directory

2. **Organize files** following the recommended structure:
   ```
   assets/
   ├── assets.json         # Edit this to define your asset tree
   ├── models/
   │   ├── characters/
   │   └── environments/
   └── textures/
   ```

3. **Edit `assets/assets.json`** to define the asset hierarchy shown in the viewer

4. **Launch the viewer** and browse your assets!

See [docs/ASSETS.md](docs/ASSETS.md) for detailed documentation on the asset system.

### Controls

- **Right Mouse Button**: Rotate camera around object
- **Middle Mouse Button**: Pan camera
- **Mouse Wheel**: Zoom in/out
- **Home Key**: Reset camera to default position
- **ESC**: Exit application

### Configuration

Settings are saved automatically to `~/.smstrikers-viewer.conf`. Access them via:
- **Menu**: View → Settings

Available settings:
- Camera controls (pan inversion)
- Default render mode (Wireframe/Opaque/Shaded)
- UI visibility (gizmo, camera info, controls)

## Current Features

✅ Asset browser with JSON-based manifest  
✅ Orbital camera controls  
✅ Multiple render modes (Wireframe, Opaque, Shaded)  
✅ Configurable UI and controls  
✅ Command-line options  

🚧 Game asset format parsing (in development)  
🚧 Texture loading  
🚧 Animation playback  

## Contributing

This is currently a personal research project. Contributions, suggestions, and feedback are welcome!

Please ensure:
- No copyrighted game assets are included in any pull requests
- Code follows the existing style and conventions
- Cross-platform compatibility is maintained

## Related Projects

- [smstrikers-decomp](https://github.com/yannicksuter/smstrikers-decomp) - Super Mario Strikers decompilation project

## License

This project is licensed under the MIT License - see the LICENSE file for details.

**Nintendo, Super Mario Strikers, and all related trademarks are property of Nintendo Co., Ltd.**

This project is not affiliated with, endorsed by, or connected to Nintendo in any way.

## Acknowledgments

- The Super Mario Strikers decompilation community
- Contributors to reverse engineering documentation
- Open source library maintainers
