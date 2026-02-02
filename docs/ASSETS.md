# Asset System Documentation

## Overview

The viewer builds its asset tree directly from the filesystem. The root directory is configured via the `assetsRoot` setting in the viewer config file. There is no asset manifest JSON.

## Config

The config file is stored at:

```
~/.smstrikers-viewer.conf
```

Relevant setting:

```
assetsRoot=game_assets
```

You can also edit this value from the Settings panel inside the viewer and rescan.

## Directory Structure

When you extract game files from your legitimate copy of Super Mario Strikers, organize them as follows:

```
game_assets/
├── models/
│   ├── characters/
│   │   ├── mario.glg
│   │   ├── luigi.glg
│   │   └── ...
│   ├── environments/
│   │   ├── stadium.glg
│   │   └── ...
│   └── effects/
│       └── ...
├── textures/
│   ├── world_01.glt
│   └── ...
└── animations/
    └── ...
```

## Tree Behavior

- The tree reflects the actual contents of `assetsRoot`.
- Folders are displayed as directories.
- Files are displayed with their filenames.
- Files with `.glt` or `.glg` extensions are flagged as **loadable**.
- Selecting a loadable file shows a placeholder cube (until parsing is implemented).

## File Types

- **`.glt`**: Texture bundle (loadable)
- **`.glg`**: Model bundle (loadable)

## Example Workflow

1. Extract game files using Dolphin Emulator.
2. Place them under your configured `assetsRoot` (default `game_assets/`).
3. Launch the viewer.
4. Browse assets in the left panel.
5. Click on `.glt` or `.glg` files to load the placeholder cube.

## Current Limitations

- ✅ Filesystem-based tree
- ✅ Loadable flags for `.glt` / `.glg`
- ✅ Dummy cube preview
- ❌ Binary parsing not implemented yet
- ❌ Texture/animation loading not implemented yet

## Legal Reminder

This viewer does NOT include any game assets. You must own a legitimate copy of Super Mario Strikers. Do not distribute extracted game files.
