from presets import *

WIN_COMBINATIONS = AllowedCombination(
    [
        Triplet("x64-win", "x64-windows-static"),
        Triplet("x64-win-min", "x64-windows-static-rel")
    ],
    [
        Generator("vs2022", "Visual Studio 17")
    ],
    [
        BuildPreset("dbg", "Debug"),
        BuildPreset("rel", "Release")
    ],
    [
        Variable(
            "CMAKE_MSVC_RUNTIME_LIBRARY",
            "MultiThreadedDebug",
            "x64-win",
            ".*",
            "dbg"
        ),
        Variable(
            "CMAKE_MSVC_RUNTIME_LIBRARY",
            "MultiThreaded",
            "x64-win",
            ".*",
            "rel"
        ),
        Variable(
            "CMAKE_MSVC_RUNTIME_LIBRARY",
            "MultiThreaded",
            "x64-win-min",
            ".*",
            ".*"
        )
    ]
)
