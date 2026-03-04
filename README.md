
# ![OJ Logo][logo] OpenJazz

## About

*OpenJazz* is a free, open-source version of the classic Jazz Jackrabbit™ games.

OpenJazz can be compiled on a wide range of operating systems, including
Windows, macOS, GNU/Linux and BSD flavors. Also ports are available for some
homebrew platforms, consoles and handhelds. See [Platforms][platforms] for
details. You can even play it [inside your web browser][web port].

To play, you will need the files from one of the original games.

With the demise of DOS-based operating systems, it has become necessary to use
emulators to play old DOS games. Jazz Jackrabbit™ deserves more - and would
benefit greatly from new features.

*Jazz Jackrabbit™* is a PC platform game. Produced by Epic Games (then Epic
MegaGames), it was first released in 1994. The fast-paced, colourful gameplay
proved popular, and the game won PC Format's Arcade Game of the Year award.
Many people still fondly recall the shareware versions.

## History

OpenJazz was started on the 23rd of August, 2005, by AJ Thomson.
Academic pressures put the project on hold until late December 2005.
The source code was released on the 25th, and the first version with a degree
of playability was released on the 15th of January, 2006.
Since then, a variety of ports have been released by other people. A lot of
them have since been merged back into the main code base.
More academic pressures meant there were few updates over the following few
years, but in 2009 a multiplayer version was released.
After that, more eye candy features have made it into the engine. These are 
features not directly involving the levels. However, without bonus levels, movie
playback and the menu plasma effect, OpenJazz would be incomplete.
In 2014, the project has been moved to GitHub to allow others to easily propose
changes, report bugs and also fix them. This also lead to more ports being made. 
Nowadays, development has slowed down. The original author is involved with
other projects and there is no clear roadmap for new features, besides striving
for a "true" and faithful Jazz Jackrabbit™ adaption on all possible platforms.
GitHub user [carstene1ns][carstene1ns] has cleaned and restructured parts of
the code base, integrated some ports and tries to resolve bug reports.

## License

OpenJazz is available under the GNU General Public License version 2 or later,
see [licenses.txt][licenses] for additional information and other included
software.

## [Controls](res/unix/OpenJazz.6.adoc#ingame-controls) (See manual)

## [Building][building]

## Running [![Play in Browser][web badge]][web port]

Execute `OpenJazz`. Depending on the platform and compile time options, the
data files are expected to be under different paths (alongside the executable
works always as a general fallback). You can also specifiy a game folder as
command line argument.

## Steam Deck / AppImage

A pre-configured AppImage for the Steam Deck is available, built with
1280×800 fullscreen, 2× integer scaling and widescreen support.

### Building the AppImage

Requires: CMake, Ninja, a C++14 compiler, SDL2 dev headers, and
[linuxdeploy](https://github.com/linuxdeploy/linuxdeploy) on your `$PATH`.

```bash
./builds/steamdeck/build_appimage.sh
```

Output: `OpenJazz-<version>-steamdeck-x86_64.AppImage`

### Running on Steam Deck

1. Copy the AppImage and your Jazz Jackrabbit™ game data to the Deck
2. Place game data either next to the AppImage or in a separate folder
3. Make executable and run:

```bash
chmod +x OpenJazz-*-steamdeck-x86_64.AppImage

# Game data next to the AppImage:
./OpenJazz-*-steamdeck-x86_64.AppImage

# Game data in a separate folder:
./OpenJazz-*-steamdeck-x86_64.AppImage /path/to/jazz1
```

First launch defaults to 1280×800 fullscreen at 2× scale.
Press **Escape → Setup Options → Video** to adjust resolution or scaling.

## Authors

Original author: AJ Thomson (alister_j_t at yahoo dot com)
Current Maintainer: Carsten Teibes (dev f4ke de)

[Additional Authors](res/unix/OpenJazz.6.adoc#authors) (See manual)

## Homepage

http://alister.eu/jazz/oj/

[logo]: res/unix/OpenJazz.png
[carstene1ns]: https://github.com/carstene1ns
[platforms]: doc/PLATFORMS.md
[licenses]: doc/licenses.txt
[building]: doc/BUILDING.md
[web badge]: https://img.shields.io/badge/Play_in-Browser-blue?style=plastic
[web port]: https://openjazz.github.io
