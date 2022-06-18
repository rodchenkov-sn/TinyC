# Tiny C Compiler
[![Dev tests](https://github.com/rodchenkov-sn/TinyC/actions/workflows/dev-tests.yml/badge.svg?branch=dev)](https://github.com/rodchenkov-sn/TinyC/actions/workflows/dev-tests.yml) [![Codacy Badge](https://app.codacy.com/project/badge/Grade/4e9b286842784bf49a4767c29faa774e)](https://www.codacy.com/gh/rodchenkov-sn/TinyC/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=rodchenkov-sn/TinyC&amp;utm_campaign=Badge_Grade)
## Build
Requirements:
*  Java for ANTLR generator
*  pkg-config, curl, zip, unzip, tar for Debian/Ubuntu

:warning: **vcpkg install step in highly space-time-consuming**

```sh
# download vcpkg submodule
git submodule update --init
# init vcpkg
vcpkg/bootstrap-vcpkg
# generate CMakePresets.json
python config/presets/generate.py
# cmake & build
cmake --preset x64-win-vs2022-rel
cmake --build --preset x64-win-vs2022-rel
```
