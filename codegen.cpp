#include <map>
#include "ast.h"
#include "parser.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Value.h"

static std::unique_ptr<llvm::LLVMContext> TheContext; // Tool set
static std::unique_ptr<llvm::IRBuilder<>> Builder; // Generate IR
static std::unique_ptr<llvm::Module> TheModule; // IR code container
static std::map<std::string, llvm::Value *> NamedValues; // Symbol table

llvm::Value *LogErrorV(const char *str) {
    LogError(str);
    return nullptr;
}

