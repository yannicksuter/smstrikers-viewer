# Asset Format Documentation

This document describes the various asset file formats used in Super Mario Strikers.

## File Formats

### .glt - Texture Bundle

**Type:** Texture Pack  
**Loading Function:** `glLoadTextureBundle()`

The `.glt` file format contains bundled texture data for a world/level. These files are loaded before the corresponding geometry to ensure textures are available when rendering models.

**Usage Pattern:**
```cpp
char buffer[256];
nlSNPrintf(buffer, 0xFF, "%s.glt", szWorldName);
glLoadTextureBundle(buffer);
```

**Example Files:**
- `{worldname}.glt` - Texture bundle for a specific world

---

### .glg - Geometry/Model Bundle

**Type:** Model Pack  
**Loading Function:** `glLoadModel()`

The `.glg` file format contains geometry and model data for a world/level. This includes meshes, vertex data, and potentially other geometric information needed to render the environment.

**Usage Pattern:**
```cpp
char buffer[256];
nlSNPrintf(buffer, 0xFF, "%s.glg", szWorldName);
m_pModels = glLoadModel(buffer, &m_uNumModels);
```

**Example Files:**
- `{worldname}.glg` - Geometry bundle for a specific world

---

## Loading Sequence

Based on the `World::LoadGeometry()` method, assets are loaded in the following order:

1. **Texture Bundle (.glt)** - Loaded first to ensure textures are available
2. **Geometry Bundle (.glg)** - Loaded second with references to the textures

### Code Reference

From `World::LoadGeometry()` in the SMS-Decomp repository:

```cpp
bool World::LoadGeometry(const char* szWorldName, bool bMakeDrawables, 
                         bool keepTransform, unsigned long* pDrawableObjectHashes, 
                         int* pNumObjectsLoaded)
{
    char buffer[256];

    // Load texture bundle
    nlSNPrintf(buffer, 0xFF, "%s.glt", szWorldName);
    tDebugPrintManager::Print(DC_RENDER, "Loading world texture file: %s\n", buffer);
    glLoadTextureBundle(buffer);

    // Load geometry bundle
    nlSNPrintf(buffer, 0xFF, "%s.glg", szWorldName);
    tDebugPrintManager::Print(DC_RENDER, "Loading world geometry file: %s\n", buffer);
    m_pModels = glLoadModel(buffer, &m_uNumModels);

    return LoadGeometry(m_pModels, m_uNumModels, bMakeDrawables, keepTransform, 
                       pDrawableObjectHashes, pNumObjectsLoaded, false);
}
```

---

## Notes

- Both formats appear to use a custom binary format specific to the game engine
- The `gl` prefix suggests these may be related to the game's graphics library layer
- Files are loaded per-world, with the world name serving as the base filename
- Format specifications and binary layouts are still being reverse-engineered

---

## TODO

- [ ] Document binary structure of .glt files
- [ ] Document binary structure of .glg files
- [ ] Identify header formats and magic numbers
- [ ] Document texture formats and compression methods
- [ ] Document vertex formats and mesh structures
- [ ] Create tools for extracting and converting these formats
