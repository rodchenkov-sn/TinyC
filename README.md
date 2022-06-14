# Tiny C Compiler
[![Dev tests](https://github.com/rodchenkov-sn/TinyC/actions/workflows/dev-tests.yml/badge.svg?branch=dev)](https://github.com/rodchenkov-sn/TinyC/actions/workflows/dev-tests.yml) [![CodeFactor](https://www.codefactor.io/repository/github/rodchenkov-sn/tinyc/badge)](https://www.codefactor.io/repository/github/rodchenkov-sn/tinyc)
## Build
Requirements:
* Java for ANTLR generator
* pkg-config, curl, zip, unzip, tar for Debian/Ubuntu

**Warning: vcpkg install step in highly space-time-consuming**

**Note: use triplet with static CRT and lib linkage**

### Linux
```sh
$ git submodule update --init
$ vcpkg/bootstrap-vcpkg.sh
$ mkdir build
$ cmake -S . -B ./build "-DCMAKE_TOOLCHAIN_FILE=$(pwd)/vcpkg/scripts/buildsystems/vcpkg.cmake"
$ cmake --build ./build
```
### Windows
```powershell
$ git submodule update --init
$ vcpkg\bootstrap-vcpkg.bat
$ mkdir build
$ cmake -S . -B .\build "-DCMAKE_TOOLCHAIN_FILE=$(pwd)/vcpkg/scripts/buildsystems/vcpkg.cmake" "-DVCPKG_TARGET_TRIPLET=x64-windows-static"
$ cmake --build .\build
```
