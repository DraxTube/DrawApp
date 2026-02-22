# DrawApp - PS Vita Drawing Application

A homebrew drawing application for PlayStation Vita with touch screen support.

## Features

- **8 Drawing Tools**: Pencil, Eraser, Line, Rectangle, Circle, Filled Rectangle, Filled Circle, Spray
- **20 Color Palette**: Tap directly on the color bar or use L/R triggers
- **Touch Drawing**: Full front touchscreen support with smooth line interpolation
- **Shape Tools**: Tap-drag-release for lines, rectangles and circles with live preview
- **Adjustable Brush Size**: From 1px to 30px
- **Undo Support**: Single-level undo
- **Clean UI**: Toggleable toolbar and palette

## Controls

| Input | Action |
|-------|--------|
| **Touch Screen** | Draw on canvas |
| **D-Pad Up/Down** | Increase/Decrease brush size |
| **L Trigger** | Previous color |
| **R Trigger** | Next color |
| **△ Triangle** | Cycle through tools |
| **□ Square** | Clear canvas |
| **○ Circle** | Undo |
| **✕ Cross** | Toggle UI visibility |
| **SELECT** | Show/Hide help |
| **START** | Exit application |

## Building

### Via GitHub Actions (Recommended)

1. Fork/clone this repository
2. Push to `main` branch
3. Go to **Actions** tab
4. Download the VPK artifact from the latest build

### Locally (requires VitaSDK)

```bash
export VITASDK=/path/to/vitasdk
mkdir build && cd build
cmake ..
make
