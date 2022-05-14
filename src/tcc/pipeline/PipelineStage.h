#ifndef TINYC_PIPELINESTAGE_H
#define TINYC_PIPELINESTAGE_H

#include <any>

struct PipeInputBase {
    virtual ~PipeInputBase() = default;

    virtual std::any produce() = 0;
};

struct PipeModifierBase {
    virtual ~PipeModifierBase() = default;

    virtual std::any modify(std::any data) = 0;
};

struct PipeOutputBase {
    virtual ~PipeOutputBase() = default;

    virtual bool consume(std::any data) = 0;
};

#endif
