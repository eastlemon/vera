# Vera - Makefile (Linux / *nix)
#
# Build:   make
# Run:     ./Vera
# Clean:   make clean
#
# Dependencies (Debian/Ubuntu): libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev
#                               libglu1-mesa-dev libvorbis-dev
# Dependencies (macOS):         use Makefile.macos instead
# Dependencies (Windows/MSYS2): use Makefile.macos as a template, swap
#                               -framework OpenGL -> -lopengl32 -lglu32

CXX      ?= g++
CXXFLAGS ?= -std=c++11 -O2 -g \
            -Wall -Wno-unused -Wno-write-strings \
            -Wno-deprecated-declarations -Wno-sign-compare
LDFLAGS  ?= -lSDL2 -lSDL2_ttf -lSDL2_image -lGL -lGLU -lvorbisfile

# Source layout - see src/README.md for the rationale.
SUBDIRS := app game world vehicle render ui ui/dialogs audio platform physics
SRC_DIRS := $(addprefix src/,$(SUBDIRS))

# All .cpp files in src/, recursively.
SOURCES := $(shell find src -name '*.cpp')
OBJECTS := $(patsubst src/%.cpp,build/%.o,$(SOURCES))

# Include paths: every src/ subdir + the macOS GL headers location.
INCLUDES := $(addprefix -I,$(SRC_DIRS)) -Isrc

EXECUTABLE := Vera

.PHONY: all clean run

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@

# Build rule: src/foo/bar.cpp -> build/foo/bar.o
build/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

run: $(EXECUTABLE)
	./$(EXECUTABLE)

clean:
	rm -rf build $(EXECUTABLE)
