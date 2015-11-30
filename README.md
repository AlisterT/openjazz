
# OpenJazz - http://www.alister.eu/jazz/oj/

## About

### OpenJazz

is a free, open-source version of the classic Jazz Jackrabbit™ games.

OpenJazz can be compiled on a wide range of operating systems, including
Windows 98/Me/XP, Mac OS X and Linux.

To play, you will need the files from one of the original games.

With the demise of DOS-based operating systems, it has become necessary to use
emulators to play old DOS games. Jazz Jackrabbit™ deserves more - and would
benefit greatly from new features.

### Jazz Jackrabbit™

is a PC platform game. Produced by Epic Games (then Epic MegaGames), it was
first released in 1994. The fast-paced, colourful gameplay proved popular,
and the game won PC Format's Arcade Game of the Year award.
Many people still fondly recall the shareware versions.

## History

OpenJazz was started on the 23rd of August, 2005, by Alister Thomson.
Academic pressures put the project on hold until late December 2005.
The source code was released on the 25th, and the first version with a degree
of playability was released on the 15th of January.
Since then, a variety of ports have been released by other people.

More academic pressures meant there were few updates over the following few
years, but in 2009 a multiplayer version was released.

## License

OpenJazz is available under the GNU General Public License version 2 or later,
see `licenses.txt` for additional information.

## Controls

`Enter` to choose a menu option, `Escape` to go back to the previous menu.
`F9` to view in-game statistics, `P` to pause.
`Alt` + `Enter` switches between full-screen and windowed mode.

The other controls are configurable via the "setup options" menu.
By default, the controls are as follows:

- Left and right arrow keys to move left and right.
- Under Windows, `Alt Gr (right Alt)` to jump and the `Space bar` to shoot.
- Under Linux, the `Space bar` to jump and `left Alt` to shoot.
- `Right Ctrl` to change weapon.

## Building

You will need the SDL 1.2.x library (https://libsdl.org/).

For network play, you need a platform which provides sockets or use the SDL_net
library (https://www.libsdl.org/projects/SDL_net/), then either define
`USE_SOCKETS` or `USE_SDL_NET` in the Makefile.

For music support, you need to define `USE_MODPLUG` in the Makefile and the
Modplug library (http://modplug-xmms.sourceforge.net/). The Modplug library
needs to be patched to support looping tracks, otherwise half of the level will
be silent.

Further instructions are available at: http://www.alister.eu/jazz/oj/build.php

### other options

- `DATAPATH` - use a fixed path for data files
- `HOMEDIR` - use the user's home directory for data files
- `SCALE` - enable scaling of the video output (i.e. Scale2X...)
- `FULLSCREEN_ONLY` - disable window mode, useful for console ports

Please see the provided Makefiles for reference.

## Running

Execute `OpenJazz`, depending on the platform and compile time options, the
data files are expected to be under different paths (see above).

## Author

Alister Thomson (alister_j_t at yahoo dot com)
