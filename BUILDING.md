
# Building OpenJazz

Needed:
- SDL 1.2.x library (https://libsdl.org/).

Optional:
- SDL_net 1.2.x library (https://www.libsdl.org/projects/SDL_net/)
- CMake (https://cmake.org/)

OpenJazz ships a basic Makefile that may be used and adapted to the specific
needs of the user or platform where it shall run. It is usually enough to simply
run `make` on UNIX-like platforms.

For more sophisticated building, a CMake script is also provided:

    cmake -B OJ -S .
    cmake --build OJ

The optional installation step (`cmake --install OJ`) is not needed for all
platforms, but usually for UNIX-like. For Homebrew targets a packaging step is
also provided (`cmake --build OJ --target package`), which automatically creates
ZIP archives.

For network play, you need a platform which natively provides sockets or use
`SDL_net`, this is detected by CMake. You can always use `-DNETWORK=OFF` to
disable it.
On the Windows platform, the socket library (`-lws2_32`) is needed for linking
the executable, others might need also additional libraries.

Further, currently outdated instructions are available at:
http://www.alister.eu/jazz/oj/build.php

## additional build options

- `DATAPATH` - use a fixed path for data files
- `SCALE` - enable scaling of the video output (i.e. Scale2X...)

Some ports have their own options, see (Platforms)[PLATFORMS.md] for details.

### advanced options

~~- `HOMEDIR` - use the user's home directory for data files~~ default on unix
- `FULLSCREEN_ONLY` - disable window mode, useful for console ports
