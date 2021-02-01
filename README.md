# LuaParser plugin for CoppeliaSim

Parser for Lua code.

Requires CoppeliaSim version 3.5.1 or greater.

### Compiling

1. Install required packages for [libPlugin](https://github.com/CoppeliaRobotics/libPlugin): see libPlugin's [README](external/libPlugin/README.md)
2. Copy the [`luacheck`](https://github.com/mpeterv/luacheck/tree/master/src/luacheck) directory into `$COPPELIASIM_ROOT_DIR/lua`
3. Checkout and compile
```
$ git clone --recursive https://github.com/CoppeliaRobotics/simExtLuaParser.git
$ cmake .
$ cmake --build .
```
