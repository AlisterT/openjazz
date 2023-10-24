
# Platforms

_This is currently unfinished._

Use a cross-compiler, enable specific platform code with option:

## Embedded Linux systems

  * Pandora: `-DPANDORA=ON`
  * GP2X Canoo: `-DCAANOO=ON`
  * GP2X Wiz: `-DWIZ=ON`
  * GP2X: `-DGP2X=ON`
  * Dingoo: `-DDINGOO=ON`
  * GameShell: `-DGAMESHELL=ON`

## RISC OS

`-DRISCOS=ON`

## Homebrew toolchains

Use the provided CMake Toolchain file or wrapper script:

### Wii

`cmake -DCMAKE_TOOLCHAIN_FILE=${DEVKITPRO}/cmake/Wii.cmake`

### 3DS

`cmake -DCMAKE_TOOLCHAIN_FILE=${DEVKITPRO}/cmake/3DS.cmake`

#### Additional options:

  * Embed a directory in the executable: `-DROMFS=ON`

### PSP

Use `psp-cmake`.

## Haiku

Works natively.

## Windows

Works natively.
