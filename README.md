# Tiny C Compiler
[![Dev tests](https://github.com/rodchenkov-sn/TinyC/actions/workflows/dev-tests.yml/badge.svg?branch=dev)](https://github.com/rodchenkov-sn/TinyC/actions/workflows/dev-tests.yml) [![Codacy Badge](https://app.codacy.com/project/badge/Grade/4e9b286842784bf49a4767c29faa774e)](https://www.codacy.com/gh/rodchenkov-sn/TinyC/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=rodchenkov-sn/TinyC&amp;utm_campaign=Badge_Grade)
## Build
Requirements:
  * Java for ANTLR generator
  * pkg-config, curl, zip, unzip, tar for Debian/Ubuntu

:warning: **vcpkg install step in highly space-time-consuming**

 :exclamation: **use triplet with static CRT and lib linkage**

### Linux
```sh
git submodule update --init
vcpkg/bootstrap-vcpkg.sh
mkdir build
cmake -S . -B ./build "-DCMAKE_TOOLCHAIN_FILE=$(pwd)/vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build ./build
```
### Windows
```powershell
git submodule update --init
vcpkg\bootstrap-vcpkg.bat
mkdir build
cmake -S . -B .\build "-DCMAKE_TOOLCHAIN_FILE=$(pwd)/vcpkg/scripts/buildsystems/vcpkg.cmake" "-DVCPKG_TARGET_TRIPLET=x64-windows-static"
cmake --build .\build
```
