# Asset System Documentation

## Overview

The viewer uses a JSON-based asset manifest system (`assets/assets.json`) to define the hierarchy and structure of game assets. This provides flexibility while the game's binary format is being reverse-engineered.

## Directory Structure

### Proposed Asset Organization

When you extract game files from your legitimate copy of Super Mario Strikers, organize them as follows:

```
assets/
├── assets.json          # Manifest file (edit this to define your asset tree)
├── models/
│   ├── characters/      # Character models
│   │   ├── mario.bin
│   │   ├── luigi.bin
│   │   ├── peach.bin
│   │   └── ...
│   ├── environments/    # Environment/stage models
│   │   ├── stadium.bin
│   │   ├── field.bin
│   │   └── ...
│   └── effects/         # Effect models
│       └── ...
├── textures/            # Texture files
│   └── ...
└── animations/          # Animation data
    └── ...
```

## Asset Manifest Format

The `assets.json` file defines the asset tree displayed in the viewer. It follows this structure:

```json
{
  "version": "1.0",
  "description": "Super Mario Strikers Asset Manifest",
  "comment": "Paths are relative to the assets/ directory",
  "assets": [
    {
      "name": "Characters",
      "type": "folder",
      "children": [
        {
          "name": "Mario",
          "type": "model",
          "path": "models/characters/mario.bin",
          "children": [
            {
              "name": "Body",
              "type": "mesh"
            },
            {
              "name": "Head",
              "type": "mesh"
            }
          ]
        }
      ]
    }
  ]
}
```

### Asset Types

- **`folder`**: Container for organizing assets (does not trigger 3D preview)
- **`model`**: Complete 3D model (triggers preview, may have sub-components)
- **`mesh`**: Individual mesh geometry
- **`bones`**: Skeleton/armature data
- **`texture`**: Texture image
- **`animation`**: Animation clip

### Fields

- **`name`** (required): Display name in the asset tree
- **`type`** (required): Asset type (see above)
- **`path`** (optional): Relative file path from `assets/` directory
- **`children`** (optional): Array of child assets

## Extracting Game Assets

### Recommended Method: Dolphin Emulator

1. **Load your game ISO** in Dolphin Emulator
2. **Right-click the game** in the game list
3. Select **Properties → Filesystem**
4. Browse the virtual filesystem and export files
5. Organize extracted files according to the directory structure above

### Future: Automated Extraction Tool

The decomp project may eventually provide an extraction tool:

```bash
# Example (not yet implemented):
./extract_smstrikers /path/to/game.iso assets/
```

## Current Limitations

As of version 0.1.0, the viewer:
- ✅ Loads and displays the asset tree from JSON
- ✅ Shows a dummy cube when models are selected
- ❌ Cannot yet parse actual game model files
- ❌ Does not load textures or animations

## Viewer Behavior

### Selection Rules

- **Selecting folders**: Expands/collapses the folder, does NOT show preview
- **Selecting models/meshes**: Shows 3D preview in viewport (currently dummy cube)
- **Multiple selection**: Not yet supported

### File Loading

When you select an asset with a `path` field:
1. The viewer will check if the file exists at `assets/<path>`
2. Future versions will parse the binary format
3. For now, a colored cube is displayed for all models

## Example Workflow

1. **Extract game files** using Dolphin Emulator
2. **Organize files** into `assets/models/`, `assets/textures/`, etc.
3. **Edit `assets/assets.json`** to define your asset hierarchy
4. **Launch viewer**: `./build/bin/smstrikers-viewer`
5. **Browse assets** in the left panel
6. **Click on models** to preview them (once parsing is implemented)

## Future Enhancements

Planned features:
- [ ] Binary format parser for model files
- [ ] Texture loading and display
- [ ] Material system
- [ ] Animation playback
- [ ] Mesh inspector (vertex count, polygon count, etc.)
- [ ] Export to common formats (glTF, OBJ, etc.)

## Legal Reminder

⚠️ **IMPORTANT**: 
- This viewer does NOT include any game assets
- You MUST own a legitimate copy of Super Mario Strikers
- Extracted assets are for personal use and preservation only
- Do NOT distribute copyrighted game files

## Contributing

If you discover information about the game's file formats:
- Document them in the [decomp project](https://github.com/your-decomp-repo)
- Submit PRs to add parsing support
- Share findings with the community (respecting copyright)

## Questions?

For questions about:
- **Asset extraction**: See Dolphin Emulator documentation
- **File formats**: Check the decomp project documentation
- **Viewer features**: Open an issue on GitHub
- **Legal concerns**: Consult the LICENSE and CONTRIBUTING.md files
