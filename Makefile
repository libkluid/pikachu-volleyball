# MinGW-w64 build.
#
#   make                                            # native mingw-w64 (MSYS2)
#   make CXX=i686-w64-mingw32-g++ WINDRES=i686-w64-mingw32-windres
#
# On Debian/Ubuntu the cross compiler is packaged as g++-mingw-w64-i686.

CXX      ?= i686-w64-mingw32-g++
WINDRES  ?= i686-w64-mingw32-windres
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Isrc
# -static keeps libstdc++/libgcc/libwinpthread inside the exe, so the build
# has no MinGW runtime DLL dependencies.
LDFLAGS  ?= -mwindows -static
LIBS     ?= -ldsound -lwinmm -lgdi32 -luser32

SRCS   := $(wildcard src/*.cpp)
OBJS   := $(patsubst src/%.cpp,build/%.o,$(SRCS))
RES    := build/pikachu.res.o
TARGET := build/pikachu.exe

all: $(TARGET)

build:
	@mkdir -p build

build/%.o: src/%.cpp | build
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(RES): res/pikachu.rc | build
	@test -d assets || { echo "assets/ is missing"; exit 1; }
	$(WINDRES) -c 65001 -I . -I src $< -O coff -o $@

$(TARGET): $(OBJS) $(RES)
	$(CXX) $(OBJS) $(RES) $(LDFLAGS) $(LIBS) -o $@
	@echo "BUILD OK: $@"

clean:
	rm -rf build

.PHONY: all clean
