#ifndef TINYC_PIPELINE_H
#define TINYC_PIPELINE_H

#include <memory>
#include <vector>

#include "PipelineStage.h"

class Pipeline {
public:
    Pipeline& add(std::unique_ptr<PipeInputBase> input);
    Pipeline& add(std::unique_ptr<PipeModifierBase> modifier);
    void add(std::unique_ptr<PipeOutputBase> output);

    bool run();

private:
    std::unique_ptr<PipeInputBase> input_;
    std::vector<std::unique_ptr<PipeModifierBase>> modifiers_;
    std::unique_ptr<PipeOutputBase> output_;
};

#endif
