# Development Documentation

This directory contains development documentation for the Super Mario Strikers Viewer project.

## Documentation Structure

- **API.md** - API documentation for the viewer classes and functions
- **ARCHITECTURE.md** - Overall architecture and design decisions
- **FILE_FORMATS.md** - Documentation of Super Mario Strikers file formats
- **BUILDING.md** - Detailed build instructions for all platforms
- **CONTRIBUTING.md** - Contribution guidelines

## File Format Documentation

As the decompilation project progresses, document the discovered file formats here:

- Model formats (.mdl, .mesh, etc.)
- Texture formats (.tex, .tpl, etc.)
- Animation formats (.anim, etc.)
- Script formats
- Archive formats

## External Resources

Reference materials and links to external documentation should be placed here.

### Related Links
- [smstrikers-decomp](https://github.com/yannicksuter/smstrikers-decomp) - Decompilation project
- GameCube development documentation
- OpenGL documentation
- C++ best practices

## Generating Documentation

If using Doxygen for API documentation:

```bash
# Install Doxygen
brew install doxygen  # macOS
sudo apt install doxygen  # Linux

# Generate docs (if Doxyfile is configured)
doxygen Doxyfile
```

Documentation will be generated in `docs/html/` (not committed to git).
