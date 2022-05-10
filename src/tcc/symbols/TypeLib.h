#ifndef TINYC_TCTYPELIB_H
#define TINYC_TCTYPELIB_H

#include <string>
#include <unordered_map>

#include "Type.h"

class TypeLibrary {
public:
    static TypeLibrary& inst();

    Type::Id get(const std::string& name) const;

private:
    TypeLibrary();

    std::unordered_map<std::string, Type::Id> named_types_;
};

#endif
