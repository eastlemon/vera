# Vera — Source Layout

After the legacy cleanup the source tree is organised by responsibility:

```
src/
├── app/           Entry point + main view
│   ├── main.cpp                SDL bootstrap, event loop
│   ├── VeraView.{h,cpp}        The main "view" class — declaration + skeleton:
│   │                            ctor/dtor, OnCreate, InitializeOpenGL, OnDraw, lifecycle
│   ├── VeraView_Menu.cpp       Menu rendering & menu input handlers
│   │                            (OnDrawCurrentMenu, DrawMenu, ReturnPressedOnCurrentMenu,
│   │                             CancelPressedOnCurrentMenu, OnKeyDownCurrentMenu, ...)
│   ├── VeraView_Game.cpp       In-game frame: OnDrawGame + all Draw* HUD methods
│   │                            (DrawDashboard, DrawNavSat, DrawServiceWnd,
│   │                             DrawPickupInfo, DrawDeliveryInfo, DrawPanel, ...)
│   ├── VeraView_Input.cpp      Keyboard/mouse/touch dispatch + dialog helpers
│   │                            (OnKeyDown, OnKeyUp, OnChar, OnMouseMove,
│   │                             OnLButtonDown, OnFingerDown, HandleBUITouch, ...)
│   └── SDLHelpers.h            Small SDL helpers (font loading, rects, colors)
│
├── game/          Game-state layer
│   ├── BGame.{h,cpp}       Static "control center": singletons, menus, weather
│   ├── BSimulation.{h,cpp} Per-frame physics tick + Paint() (3D world render)
│   ├── BPlayer.{h,cpp}     Cash, fuel, owned vehicles, save/load Player.state
│   └── BMessages.{h,cpp}   On-screen timed text messages
│
├── world/         The 3D world (terrain, scene, objects, procedural generators)
│   ├── BScene.{h,cpp}            .scene loader (INI), holds up to 100 objects
│   ├── BSceneEditor.{h,cpp}      In-game scene editor (state machine)
│   ├── BGround.{h,cpp}           Legacy 96x48 ground field (mostly empty now)
│   ├── BTerrain.{h,cpp}          LOD block terrain system, 64x64 tiles
│   ├── BObject.{h,cpp}           Scene props (Stonehenge, base, gas station…)
│   ├── BaseClasses.{h,cpp}       Shared data structures (BVector, BPart, etc.)
│   ├── CoreClasses.h             Vector / matrix primitives
│   ├── HeightMap.{h,cpp}         Procedural terrain generator
│   └── PerlinNoise.{h,cpp}       2D Perlin noise
│
├── vehicle/       Player vehicle
│   └── BVehicle.{h,cpp}    Loads .vehicle INI, wraps PPEC_Vehicle
│
├── physics/       Rigid-body physics engine (Vera's own)
│   └── VeraPhysicsEngine.{h,cpp}  Body points + springs + wheels + aero
│
├── render/        Rendering layer (OpenGL 1.x fixed-function)
│   ├── OpenGLHelpers.{h,cpp}     GL state, multitexture, primitives, display lists
│   ├── OpenGLExtFunctions.h      ARB multitexture function-pointer typedefs
│   ├── BTextures.{h,cpp}         TGA loader + texture manager
│   ├── BCamera.{h,cpp}           Camera location / orientation / mode
│   └── BTextRenderer.{h,cpp}     In-game text via glyph atlas textures
│
├── ui/            In-game UI
│   ├── BMenu.{h,cpp}             Menu data model
│   ├── BUI.{h,cpp}               BUIEdit + BUISelectionList primitives
│   ├── BCmdModule.{h,cpp}        Console-style command dispatcher
│   ├── BServiceWnd.{h,cpp}       41x17 service terminal
│   ├── BNavSatWnd.{h,cpp}        Mini-map (worker-thread rendered)
│   └── dialogs/                  SDL2-surface based dialogs
│       ├── Dialog.{h,cpp}        Base dialog container
│       ├── Item.{h,cpp}          Base widget
│       ├── Button.{h,cpp}        TTF-rendered push button
│       ├── KbdButton.{h,cpp}     Button that emits an SDL keycode
│       └── DlgOnScreenKbd.{h,cpp}  Full-screen on-screen keyboard
│
├── audio/         Audio mixer
│   └── SoundModule.{h,cpp}  SDL_audio callback mixer + libvorbisfile .ogg loader
│
└── platform/      Platform abstractions
    ├── Settings.{h,cpp}        Reads/writes settings.dat
    ├── ControllerModule.{h,cpp} Keyboard map (joystick support stubbed out)
    ├── FileIOHelpers.{h,cpp}   INI file reader/writer
    └── StringTools.{h,cpp}     makeUpper / compareNoCase helpers
```

## Build

### macOS
```bash
brew install sdl2 sdl2_ttf sdl2_image libvorbis pkg-config
make -f Makefile.macos
./Vera
```

### Linux
```bash
sudo apt install libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev \
                 libglu1-mesa-dev libvorbis-dev
make
./Vera
```

## Run

The game loads all assets (scenes, models, textures, sounds) relative to the
**current working directory**, so always `cd` into the project folder before
running `./Vera`. On first launch on macOS, strip the quarantine attribute:

```bash
xattr -dr com.apple.quarantine ./Vera
```

## What was removed (legacy cleanup)

- **VeraDoc / CVeraDoc** — empty MFC `CDocument` shell, never used.
- **BSceneModule** — superseded by `BScene`, no callers.
- **GLCompat.h** — never `#include`d anywhere.
- **BMenuItemOBSOLETE** class and `m_miMenu[7]` member — dead.
- All `// CRect`, `// CDC*`, `// afx_msg`, `// DECLARE_DYNCREATE`,
  `// BEGIN_MESSAGE_MAP`, `//{{AFX_*`, `// Microsoft Visual C++…` boilerplate.
- All `#ifdef __EMSCRIPTEN__` blocks and the `EMXX*` section of the Makefile.
- Commented-out MFC/Win32 type references (`CWinThread*`, `HANDLE`, `HGLRC`,
  `CString`, `DEVMODE`, `LPDIRECTINPUT*`, `FSOUND_SAMPLE*`, `IMediaControl*`,
  `CRITICAL_SECTION`, etc.).
- The `em++` / `Vera.html` / `.em.o` rules from the Makefile.

## What's still on the TODO list

1. Replace the hand-rolled SDL_audio mixer in `src/audio/SoundModule.cpp`
   with SDL_mixer (~50 lines instead of 376).
2. Modernise the OpenGL renderer from fixed-function (`glBegin/glEnd`,
   display lists) to GLSL 3.3 core profile — needed for Metal compatibility
   on modern macOS.
3. Re-implement joystick support via SDL2's game-controller API
   (`ControllerModule::SwitchToController` / `GetValueFromRaw` are stubs).
4. The dead AVI video subsystem (`flipIt`/`GrabAVIFrame`/`CloseAVI`/`InitVideo`
   in `src/app/VeraView.cpp`) is still compiled in but never called — should
   be deleted in a future cleanup pass.
