# 'Zorya' Native PlugIn Example
This is an example native plugin for the next generation of real-time music analysis and visualization software. The 'Zorya' (development name) platform is built on [FlightlessManicotti](https://github.com/GameClay/FlightlessManicotti) and is currently targeted at Mac OS only.

## Requirements
Building a 'Zorya' plugin requires [LuaJIT](http://luajit.org/). By default, the build script will look for LuaJIT in `/usr/local`.

## Building
Building a 'Zorya' plugin requires [SCons](http://www.scons.org/), to build the example in debug mode type: `scons CPU=x86-64`
The command for a release build is: `scons CPU=x86-64 VARIANT=release`

To specify the location of the LuaJIT, FlightlessManicotti and 'Zorya' SDKs, use the following variables:
* MANICOTTI
* ZORYA
* LUAJIT

For example: `scons CPU=x86-64 MANICOTTI=/path/to/ManicottiSDK ZORYA=/path/to/ZoryaSDK LUAJIT=/path/to/LuaJIT`
