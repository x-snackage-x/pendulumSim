# PendulumSim (WIP)
A kinematics based pendulum simulation built with raylib.

https://github.com/user-attachments/assets/22576ce8-0da3-446a-8de0-3054a0e0cfb8

The project was built with using Jeffery Myers's [Raylib-Quickstart](https://github.com/raylib-extras/raylib-quickstart). Original *README.md* provided under *README_RAYLIBQS.md*.

## Build
For more detailed info on building the project see *README_RAYLIBQS.md*.

### Linux 

To generate the makefiles:

* `cd` into the build folder
* run `./premake5 gmake`

To build:

* At the top of the generated `Makefile` set the desired `config`:

```Makefile
ifndef config
  config=debug_x86
endif
```

* Run `make`.

### Windows using Visual Studio

To generate Visual Studio project solution:
* Run `build-VisualStudio202*.bat`
* Open project using Visual Studio and build

### MacOS

Analogous to Linux but run `./premake5.osx gmake` instead

## Dependencies

raylib and its dependencies: 
- https://www.raylib.com/
- https://github.com/raysan5/raylib/wiki


## Todo

1. Add 2nd Mass/Double Pendulum
1. Anchor Responsive to Screen size?
1. Add Resistance force?
1. Icon
