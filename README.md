# Tiny C Compiler
## Build
Requirements:
* Java for ANTLR generator
* pkg-config, curl, zip, unzip, tar for Debian/Ubuntu

**Warning: vcpkg install step in highly space-time-consuming**
**Note: use triplet with static CRT and lib linkage**

```sh
$ git submodule update --init
$ vcpkg/bootstrap-vcpkg
$ mkdir build
$ cmake -S . -B ./build "-DCMAKE_TOOLCHAIN_FILE=$(pwd)/vcpkg/scripts/buildsystems/vcpkg.cmake"
$ cmake --build ./build
```
