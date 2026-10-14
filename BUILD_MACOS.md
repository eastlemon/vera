# Building and Running Vera on macOS

Vera is a 3D pizza-delivery driving game written in C++11 / SDL2 / OpenGL.

This guide covers installing dependencies, building, and running the game on
macOS (Intel or Apple Silicon).

---

## 1. Prerequisites

Install the **Xcode Command Line Tools** and **Homebrew**:

```bash
xcode-select --install
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

Install the game's dependencies via Homebrew:

```bash
brew install sdl2 sdl2_ttf sdl2_image libvorbis pkg-config
```

| Dependency   | What it provides                                |
|--------------|-------------------------------------------------|
| sdl2         | Window, input, audio, OpenGL context            |
| sdl2_ttf     | TrueType font rendering (for in-game UI text)   |
| sdl2_image   | Image loading (TGA textures used by the game)   |
| libvorbis    | Ogg Vorbis decoding for the `Sounds/*.ogg` files|
| pkg-config   | Lets the Makefile find the above                |

---

## 2. Build

The source tree lives under `src/` (see `src/README.md` for the layout).

```bash
make -f Makefile.macos
```

A successful build produces `./Vera` in the project root. Typical build
time: 5–15 seconds.

For a debug build:

```bash
make -f Makefile.macos CXXFLAGS="-std=c++11 -O0 -g -Wall \
    $(pkg-config --cflags sdl2 sdl2_ttf sdl2_image vorbisfile)"
```

To clean:

```bash
make -f Makefile.macos clean
```

---

## 3. Run

The game loads all assets relative to the current working directory,
so `cd` into the project folder before running it:

```bash
cd /path/to/vera
./Vera
```

On first launch you may see two macOS prompts:

1. **"Vera cannot be opened because the developer cannot be verified."**
   Right-click `Vera` in Finder → *Open* → *Open* in the dialog. You only
   need to do this once. Alternatively, from the terminal:

   ```bash
   xattr -dr com.apple.quarantine ./Vera
   ```

2. **macOS may ask for audio input permission.** Deny it — the game only
   needs audio output.

To exit, close the window or press ⌘Q.

---

## 4. Common issues

### `fatal error: 'SDL2/SDL.h' file not found`
Re-run `brew install sdl2 sdl2_ttf sdl2_image`. If `pkg-config --cflags sdl2`
returns nothing, set:

```bash
export PKG_CONFIG_PATH="$(brew --prefix)/lib/pkgconfig:$PKG_CONFIG_PATH"
```

### `ld: framework not found OpenGL`
Run `xcode-select --install` to install the Command Line Tools.

### `'OpenGL/gl.h' file not found`
Install Mesa via Homebrew (`brew install mesa`) — though on a normal macOS
install this should not happen, the OpenGL framework is still shipped
through at least macOS 15.

### Game window opens but is black / crashes immediately
Make sure you launched the game from the project directory — the game reads
`*.scene`, `*.vehicle`, `Models/*.obj`, `Textures/*.tga`, `Sounds/*.ogg`
and `FreeSans.ttf` from the current working directory.

### "Cannot open ground file!"
This means `World/ground.raw` is missing. The game generates it on the fly
from the scene you select — pick a scene from the in-game menu (Coastal,
Desert, IceWorld, CraterIsland, Haven, TheWall) and the ground will be
built procedurally.

### Audio is silent
Check that `Sounds/MixDown3.ogg`, `Sounds/EngineJuliet2.ogg` and
`Sounds/CRASH.ogg` are present. If you have multiple audio devices, set the
default output device in System Settings → Sound before launching the game.

### Apple Silicon: "Verifying Vera…" hangs
This is Gatekeeper scanning a freshly-built unsigned binary. Wait a few
seconds; if it persists, run `xattr -dr com.apple.quarantine ./Vera`.

---

## 5. Project layout

See `src/README.md` for the full source-tree breakdown. Top-level
directory contains:

```
vera/
├── Makefile               # Linux build
├── Makefile.macos         # macOS build (this file's target)
├── BUILD_MACOS.md         # this file
├── src/                   # all C++ sources (see src/README.md)
├── Models/                # .obj meshes
├── Textures/              # .tga textures
├── Sounds/                # .ogg audio
├── *.scene                # scene definitions
├── *.vehicle              # vehicle definitions
├── FreeSans.ttf           # font used by the in-game UI
├── vehicles, scenes       # text manifests (one filename per line)
└── gpl.txt                # GPL licence
```
