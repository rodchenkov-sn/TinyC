#include "Pipeline.h"

Pipeline& Pipeline::add(std::unique_ptr<PipeInputBase> input)
{
    if (input_) {
        throw std::runtime_error{"input was already added"};
    }
    input_ = std::move(input);
    return *this;
}

Pipeline& Pipeline::add(std::unique_ptr<PipeModifierBase> modifier)
{
    modifiers_.emplace_back(std::move(modifier));
    return *this;
}

void Pipeline::add(std::unique_ptr<PipeOutputBase> output)
{
    if (output_) {
        throw std::runtime_error{"output was already added"};
    }
    output_ = std::move(output);
}

bool Pipeline::run()
{
    auto data = input_->produce();
    if (!data.has_value()) {
        return false;
    }
    for (auto& modifier : modifiers_) {
        data = modifier->modify(data);
        if (!data.has_value()) {
            return false;
        }
    }
    return output_->consume(data);
}
