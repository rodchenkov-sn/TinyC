#ifndef TINYC_TCTYPELIB_H
#define TINYC_TCTYPELIB_H

#include "Type.h"

class TypeLibrary {
public:
    static TypeLibrary& inst();

    Type::Id get(const std::string& name) const;
    bool add(const std::string& name, const Type::Id& tid);

private:
    TypeLibrary();

    std::unordered_map<std::string, Type::Id> named_types_;
};

#endif
