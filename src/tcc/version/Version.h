#ifndef TINYC_VERSION_H
#define TINYC_VERSION_H

#define TINYC_VERSION_MAJOR 0
#define TINYC_VERSION_MINOR 1
#define TINYC_VERSION_PATCH 3

inline std::string getVersion()
{
    std::stringstream ss;
    ss << "TinyC " << TINYC_VERSION_MAJOR << '.' << TINYC_VERSION_MINOR << '.' << TINYC_VERSION_PATCH;
    return ss.str();
}

#endif
