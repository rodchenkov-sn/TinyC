# Tiny C Compiler
## Build
Requirements:
* Java for ANTLR generator
* pkg-config, curl, zip, unzip, tar for Debian/Ubuntu

**Warning: vcpkg install step in highly space-time-consuming**

```
$ git submodule update --init
$ vcpkg/bootstrap-vcpkg.sh # or .bat
$ mkdir build
$ cmake -S . -B ./build "-DCMAKE_TOOLCHAIN_FILE=$(pwd)/vcpkg/scripts/buildsystems/vcpkg.cmake"
$ cmake --build ./build
```
