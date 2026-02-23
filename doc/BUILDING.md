
# Building OpenJazz

Needed:
- SDL 3.4, 2 or SDL 1.2 (deprecated) library (https://libsdl.org/)

Optional:
- SDL_net library (https://www.libsdl.org/projects/SDL_net/)
- CMake (https://cmake.org/)

OpenJazz traditionally ships a basic Makefile that may be used. It is usually
enough to simply run `make` on UNIX-like platforms.
However, the old SDL 1.2 library will be used then.

For more sophisticated building, a CMake script is provided:

    cmake -B OJ -S .
    cmake --build OJ

The optional installation step (`cmake --install OJ`) is not needed for all
platforms, but usually for UNIX-like. For Homebrew targets a packaging step is
also provided (`cmake --build OJ --target package`), which automatically creates
ZIP archives. CMake will use the SDL library version 2 by default, but can also
build against version 3.

For network play, you need a platform which natively provides sockets or use
`SDL_net`, this is detected by CMake. You can always use `-DNETWORK=OFF` to
disable it.
On the Windows platform, the socket library (`-lws2_32`) is needed for linking
the executable, others might need also additional libraries.
If you need help porting OpenJazz to a new platform, you can always ask by email
or opening an issue (see README.md for details).

## additional build options

- `SDL_VERSION` - SDL library version to use (2=default or 3)
- `DATAPATH` - add a fixed path for game data files
- `SCALE` - enable scaling of the video output (i.e. Scale2X...)
- `PORTABLE` - Do not use external directories for configuration saving, etc.
  (This only affects Unix platforms, Windows version is always portable)

Some ports have their own options, see [Platforms](PLATFORMS.md) for details.

### advanced options

- `FULLSCREEN_ONLY` - disable window mode, useful for console ports
