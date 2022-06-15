import itertools
import json
import re

from typing import List, Iterable

from presets import AllowedCombination, Configuration
from CMakePresets import WIN_COMBINATIONS


def genarate(combinations: List[AllowedCombination]) -> Iterable[Configuration]:
    for combination in combinations:
        cs = list(itertools.product(combination.triplets, combination.genarators, combination.build_presets))
        for triplet, generator, build in cs:
            variables = []
            for v in combination.variables:
                if re.match(v.filter_triplet, triplet.name) and re.match(v.filter_generator, generator.name) \
                        and re.match(v.filter_build_preset, build.name):
                    variables.append(v)
            yield Configuration(
                f'{triplet.name}-{generator.name}-{build.name}',
                triplet.triplet,
                generator.generator,
                variables,
                build.configuration
            )

def main():
    structure = {
        "version": 3,
        "configurePresets": [],
        "buildPresets": []
    }
    print("Generated presets:")
    for conf in genarate([WIN_COMBINATIONS]):
        print(conf.name)
        cp, bp = conf.serialize()
        structure["configurePresets"].append(cp)
        structure["buildPresets"].append(bp)

    with open("CMakePresets.json", "w") as cmake_presets:
        cmake_presets.write(json.dumps(structure, indent=4))


if __name__ == '__main__':
    main()
