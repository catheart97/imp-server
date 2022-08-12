# IMP-Server

## Configuration

Before building the application you might want to modify some of its configuration. These can be found in `source/imp/Settings.hpp`.

## Build

Get code using '--recurse-submodules'.

Requirements:
* CMake
* C++-20 Compiler with OpenMP support (Apple Clang is not supported)
* fcl (depends on libccd and octomap)
* oat++ (via submodule)
* eigen (via submodule)

### Windows 

After installing the upper requirements (e.g. using `cmake --build . --config Release --target INSTALL`) you can run the following commands to build the application:

```
mkdir build 
cd build
cmake .. 
cmake --build . --config Release
```

### macOS

As this application uses OpenMP for parallelization Apple Clang is not supported. You can use the binaries from homebrew and the provided `macos.sh` script to set the correct environment variables.
