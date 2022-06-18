#ifndef TINYC_PCH_H
#define TINYC_PCH_H

#pragma warning(disable : 4624)// llvm warnings

// std
#include <algorithm>
#include <any>
#include <cstdlib>
#include <deque>
#include <functional>
#include <iostream>
#include <memory>
#include <numeric>
#include <optional>
#include <regex>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

// llvm
#include <llvm/Analysis/CGSCCPassManager.h>
#include <llvm/Analysis/LoopAnalysisManager.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Passes/OptimizationLevel.h>
#include <llvm/Passes/PassBuilder.h>

// argparse
#include <argparse/argparse.hpp>

// commonly used proj
#include "log/Logging.h"
#include "os/Platform.h"
#include "utils/Defs.h"

#endif
