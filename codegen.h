#ifndef CODEGEN_H
#define CODEGEN_H

#include <map>
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Constant.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/IR/Verifier.h"

extern std::unique_ptr<llvm::LLVMContext> TheContext; // Tool set
extern std::unique_ptr<llvm::IRBuilder<> > Builder; // Generate IR
extern std::unique_ptr<llvm::Module> TheModule; // IR code container
extern std::map<std::string, llvm::Value *> NamedValues; // Symbol table

void InitializeModule();

llvm::Value *LogErrorV(const char *str);


#endif
