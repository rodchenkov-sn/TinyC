#ifndef TINYC_FILEWRITER_H
#define TINYC_FILEWRITER_H

#include "pipeline/PipelineStage.h"

class FileWriter : public PipeOutputBase {
public:
    explicit FileWriter(std::string fileName);

    bool consume(std::any data) override;

private:
    std::string file_name_;
};

#endif
