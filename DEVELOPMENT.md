# Vera — Development Guide

This document describes the development process, architecture, and database
migration system for the Vera game.

---

## 1. Project Overview

Vera is a 3D pizza-delivery driving game. The player drives a vehicle
between a base and delivery points across procedurally generated terrain,
managing fuel and money.

### Game mechanics

- **2 menu types**: main menu (START GAME / SETTINGS / EXIT) and in-game
  pause menu (quit to main menu / help / settings / cancel order / resume)
- **2 vehicle types**: Bogian (ground vehicle) and Spirit (airplane/heli/jet)
- **6 scenes** (maps): Coastal, Desert, IceWorld, CraterIsland, Haven, TheWall
  — terrain is **procedurally generated** via Perlin noise (HeightMap.cpp)
- **Environmental effects**: earthquakes (random, timed) and wind (affects
  vehicle handling, visible particle effects)
- **Resources**: fuel (depletes while driving), kerosine (for jet/heli),
  cash (earned from deliveries, spent on fuel/vehicles)
- **Cargo**: pizza (single type, delivered from base to client)
- **World objects**: 1 base (delivery origin), gas stations (refuel),
  client locations (delivery targets) — placed per scene definition
- **6 terrain detail levels**: Minimum → Low → Medium → High → Maximum → Ultra

### Architecture

```
src/
├── app/           Entry point, main view, input dispatch
├── game/          Game state machine, simulation, player, messages
├── world/         Scene, terrain, objects, procedural generation
├── vehicle/       Vehicle class
├── physics/       Vera's rigid-body physics engine
├── render/        OpenGL helpers, textures, camera, text, shaders
├── ui/            Menus, dialogs, service window, NavSat
├── audio/         Sound mixer (SDL_mixer)
├── platform/      Settings, file I/O, keyboard bindings, database
└── physics/       Physics engine
```

### Rendering pipeline

The renderer is mid-migration from fixed-function OpenGL (glBegin/glEnd)
to modern GLSL shaders:

- **Legacy (fixed-function)**: 3D world, terrain, objects, vehicle, sky, water
- **Modern (GLSL 1.20 shaders + VAO/VBO)**: 2D UI — menu background, text
  rendering, quick help overlay, preview images

Both paths coexist via OpenGL compatibility profile. See
`src/render/README.md` for the migration plan.

---

## 2. Build & Run

### macOS

```bash
brew install sdl2 sdl2_ttf sdl2_image sdl2_mixer glew libvorbis pkg-config sqlite3
make -f Makefile.macos
./Vera
```

### Linux

```bash
sudo apt install libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev \
                 libsdl2-mixer-dev libglew-dev libglu1-mesa-dev \
                 libvorbis-dev libsqlite3-dev
make
./Vera
```

### Clean build

```bash
rm -rf build Vera
make -f Makefile.macos   # or just: make
```

---

## 3. Database & Migrations

### Overview

Player settings and game progress are stored in a local SQLite database
(`vera.db`), created automatically on first launch. The database uses a
**versioned migration system**: each schema change is a numbered SQL script
that runs on startup if the database version is older.

### Database file

```
vera.db          — SQLite database (settings + player progress)
```

Legacy files (`settings.dat`, `Player.state`) are still read on first
launch to migrate existing data into the database. After successful
migration, the legacy files are renamed to `.bak`.

### Schema versioning

The `schema_version` table tracks the current database version:

```sql
CREATE TABLE schema_version (
    version INTEGER PRIMARY KEY,
    applied_at TEXT NOT NULL DEFAULT (datetime('now'))
);
```

On startup, the `Database` class checks the current version and applies
pending migrations in order.

### Migration system

Migrations are numbered SQL scripts embedded in the C++ source
(`src/platform/Database.cpp`). Each migration:

1. Has a unique version number (1, 2, 3, ...)
2. Contains SQL statements to create/alter tables
3. Is applied inside a transaction (atomic — all or nothing)
4. Updates `schema_version` on success

To add a new migration:

```cpp
// In Database.cpp, add to the migrations array:
static const Migration kMigrations[] = {
  {1, kMigrationV1},
  {2, kMigrationV2},  // <-- new migration
  {0, nullptr}         // sentinel
};
```

### Current schema (v1)

```
schema_version      — tracks DB version
settings            — key-value game settings (graphics, audio, controls)
player_state        — player progress (cash, fuel, kerosine, vehicles)
scene_progress      — per-scene delivery progress + vehicle location
```

### Migration from legacy files

On first launch with an existing `settings.dat` and/or `Player.state`:

1. Database v1 schema is created
2. `settings.dat` is parsed line-by-line, each `key = value` inserted into
   the `settings` table
3. `Player.state` is parsed, cash/fuel/kerosine/vehicles inserted into
   `player_state`, per-scene info into `scene_progress`
4. Legacy files are renamed to `.bak`
5. All future reads/writes go through the database

If legacy files don't exist, the database is populated with defaults.

---

## 4. Development Workflow

### Making changes

1. **Clone**: `git clone https://github.com/eastlemon/vera.git`
2. **Branch**: `git checkout -b feature/your-feature`
3. **Build**: `make -f Makefile.macos` (macOS) or `make` (Linux)
4. **Test**: `./Vera` — the game must run from the project root directory
   (assets are loaded relative to CWD)
5. **Commit**: use descriptive commit messages
6. **Push**: `git push origin feature/your-feature`

### Adding a new setting

1. Add a migration in `Database.cpp` (e.g., `ALTER TABLE settings ADD COLUMN ...`)
2. Update `Settings.cpp` to read/write via `Database::getSetting()` / `Database::setSetting()`
3. Update the settings menu in `BGame.cpp` if the setting is user-facing

### Adding a new scene

1. Create `MyScene.scene` (INI format with `[Properties]`, `[Geometry]`, `[Delivery]`)
2. Add `MyScene` to the `scenes` manifest file
3. Add a preview texture `Textures/MyScene.tga` (256×256 TGA)
4. The scene appears in the "Choose Scene" menu automatically

### Adding a new vehicle

1. Create `MyVehicle.vehicle` (INI format with `[Properties]`, `[Body]`, `[Engine]`, `[Wheels]`)
2. Add `MyVehicle` to the `vehicles` manifest file
3. Add a preview texture `Textures/MyVehicle.tga` (256×256 TGA)
4. Add the vehicle model (`Models/MyVehicle*.obj`)
5. The vehicle appears in the "Choose Vehicle" menu automatically

---

## 5. Dependencies

| Dependency | Purpose | macOS install | Linux install |
|-----------|---------|---------------|---------------|
| SDL2 | Window, input, audio, GL context | `brew install sdl2` | `apt install libsdl2-dev` |
| SDL2_ttf | TrueType font rendering | `brew install sdl2_ttf` | `apt install libsdl2-ttf-dev` |
| SDL2_image | Image loading (TGA) | `brew install sdl2_image` | `apt install libsdl2-image-dev` |
| SDL2_mixer | Audio playback (OGG) | `brew install sdl2_mixer` | `apt install libsdl2-mixer-dev` |
| GLEW | Modern GL function loading | `brew install glew` | `apt install libglew-dev` |
| libvorbis | Ogg Vorbis decoding | `brew install libvorbis` | `apt install libvorbis-dev` |
| SQLite3 | Local database | `brew install sqlite3` | `apt install libsqlite3-dev` |
| GLM | Math library (vendored) | included in `vendor/glm/` | included in `vendor/glm/` |

---

## 6. File structure

```
vera/
├── Makefile               — Linux build
├── Makefile.macos         — macOS build
├── DEVELOPMENT.md         — this file
├── BUILD_MACOS.md         — macOS build & run guide
├── src/                   — C++ source (see Architecture above)
├── vendor/glm/            — GLM math library (header-only)
├── shaders/               — GLSL shader sources (reference copies)
├── Models/                — .obj meshes
├── Textures/              — .tga textures
├── Sounds/                — .ogg audio files
├── *.scene                — scene definitions
├── *.vehicle              — vehicle definitions
├── vehicles, scenes       — text manifests (one filename per line)
├── FreeSans.ttf           — font for on-screen keyboard
└── gpl.txt                — GPL license
```

### Runtime files (generated)

```
vera.db              — SQLite database (settings + progress)
Vera.log             — game log
*_TerrainCache.dat   — terrain cache (binary, per scene)
```
