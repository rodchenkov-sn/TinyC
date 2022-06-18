from presets import AllowedCombination, Triplet, Generator, BuildPreset, Variable

DEFAULT_BUILD_PRESETS = [
    BuildPreset("dbg", "Debug", "tcc"),
    BuildPreset("rel", "Release", "tcc")
]

WIN_COMBINATIONS = AllowedCombination(
    [
        Triplet("x64-win", "x64-windows-static"),
        Triplet("x64-win-min", "x64-windows-static-rel")
    ],
    [
        Generator("vs2022", "Visual Studio 17")
    ],
    DEFAULT_BUILD_PRESETS,
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


LINUX_COMBINATIONS = AllowedCombination(
    [
        Triplet("x64-linux", "x64-linux"),
        Triplet("x64-linux-min", "x64-linux-rel")
    ],
    [
        Generator("gcc", "Unix Makefiles")
    ],
    DEFAULT_BUILD_PRESETS,
    []
)


ALL_COMBINATIONS = [WIN_COMBINATIONS, LINUX_COMBINATIONS]
