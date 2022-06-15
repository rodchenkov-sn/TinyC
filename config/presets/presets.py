from dataclasses import dataclass
from typing import Any, List, Tuple, Dict


@dataclass
class Triplet:
    name: str
    triplet: str


@dataclass
class Generator:
    name: str
    generator: str


@dataclass
class Variable:
    name: str
    value: Any
    filter_triplet: str
    filter_generator: str
    filter_build_preset: str


@dataclass
class BuildPreset:
    name: str
    configuration: str


@dataclass
class AllowedCombination:
    triplets: List[Triplet]
    genarators: List[Generator]
    build_presets: List[BuildPreset]
    variables: List[Variable]


@dataclass
class Configuration:
    name: str

    # configurePresets entity
    triplet: str
    generator: str
    variables: List[Variable]

    # buildPresets entity
    build_config: str

    def serialize(self) -> Tuple[Dict, Dict]:
        cp = {
            "name": self.name,
            "toolchainFile": "${sourceDir}/vcpkg/scripts/buildsystems/vcpkg.cmake",
            "binaryDir": "${sourceDir}/build",
            "generator": self.generator,
            "cacheVariables": {
                "VCPKG_TARGET_TRIPLET": self.triplet,
                "VCPKG_OVERLAY_TRIPLETS": "${sourceDir}/config/triplets"
            }
        }
        for variable in self.variables:
            cp["cacheVariables"][variable.name] = variable.value
        bc = {
            "name": self.name,
            "configuration": self.build_config,
            "configurePreset": self.name
        }
        return cp, bc
