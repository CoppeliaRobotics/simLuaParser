# LuaParser plugin for V-REP

Parser for Lua code.

Requires V-REP version 3.5.1 or greater.

### Compiling

1. Install required packages for [v_repStubsGen](https://github.com/fferri/v_repStubsGen): see v_repStubsGen's [README](external/v_repStubsGen/README.md)
2. Copy the [`luacheck`](https://github.com/mpeterv/luacheck/tree/master/src/luacheck) directory into `$VREP_ROOT/lua`
3. Checkout and compile
```
$ git clone --recursive https://github.com/CoppeliaRobotics/v_repExtLuaParser.git
$ cmake .
$ cmake --build .
```
