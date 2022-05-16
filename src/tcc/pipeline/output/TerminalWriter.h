#ifndef TINYC_TERMINALWRITER_H
#define TINYC_TERMINALWRITER_H

#include "pipeline/PipelineStage.h"

class TerminalWriter : public PipeOutputBase {
public:
    bool consume(std::any data) override;
};

#endif
