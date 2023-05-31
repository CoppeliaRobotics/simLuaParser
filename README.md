# LuaParser plugin for CoppeliaSim

Parser for Lua code.

Requires CoppeliaSim version 3.5.1 or greater.

### Compiling

1. Install required packages for simStubsGen: see simStubsGen's [README](https://github.com/CoppeliaRobotics/include/blob/master/simStubsGen/README.md)
2. Copy the [`luacheck`](https://github.com/mpeterv/luacheck/tree/master/src/luacheck) directory into `$COPPELIASIM_ROOT_DIR/lua`
3. Checkout, compile and install into CoppeliaSim:
```sh
$ git clone https://github.com/CoppeliaRobotics/simLuaParser.git
$ cd simLuaParser
$ git checkout coppeliasim-v4.5.0-rev0
$ mkdir -p build && cd build
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ cmake --build .
$ cmake --install .
```

NOTE: replace `coppeliasim-v4.5.0-rev0` with the actual CoppeliaSim version you have.
