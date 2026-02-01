#include "codegen.h"
#include "ast.h"
#include "parser.h"

std::unique_ptr<llvm::LLVMContext> TheContext; // Tool set
std::unique_ptr<llvm::IRBuilder<> > Builder; // Generate IR
std::unique_ptr<llvm::Module> TheModule; // IR code container
std::map<std::string, llvm::Value *> NamedValues; // Symbol table

void InitializeModule() {
    TheContext = std::make_unique<llvm::LLVMContext>();
    Builder = std::make_unique<llvm::IRBuilder<> >(*TheContext);
    TheModule = std::make_unique<llvm::Module>("JIT", *TheContext);
}

llvm::Value *LogErrorV(const char *str) {
    LogError(str);
    return nullptr;
}

llvm::Value *ASTNode::NumberExpressionASTNode::codegen() {
    return llvm::ConstantFP::get(*TheContext, llvm::APFloat(value));
}

llvm::Value *ASTNode::VariableExpressionASTNode::codegen() {
    llvm::Value *V = NamedValues[Name];
    if (V == nullptr) {
        LogErrorV("Unknown variable name");
    }
    return V;
}

llvm::Value *ASTNode::BinaryExpressionASTNode::codegen() {
    llvm::Value *L = LHS->codegen();
    llvm::Value *R = RHS->codegen();

    if (L == nullptr or R == nullptr) {
        return nullptr;
    }

    switch (Operator) {
        case '+':
            return Builder->CreateAdd(L, R, "addtmp");
        case '-':
            return Builder->CreateSub(L, R, "subtmp");
        case '*':
            return Builder->CreateFMul(L, R, "multmp");
        case '<':
            L = Builder->CreateFCmpULT(L, R, "cmptmp");
            return Builder->CreateUIToFP(L, llvm::Type::getDoubleTy(*TheContext), "booltmp");
        default:
            return LogErrorV("invalid binary operator");
    }
}

llvm::Value *ASTNode::FunctionCallExpressionASTNode::codegen() {
    llvm::Function *CalleeFunction = TheModule->getFunction(Callee);

    if (CalleeFunction == nullptr) {
        return LogErrorV("Unknown function referenced");
    }

    if (CalleeFunction->arg_size() != Arguments.size()) {
        return LogErrorV("Incorrect number of arguments passed");
    }

    std::vector<llvm::Value *> ArgumentsVector;

    for (unsigned int i = 0, e = Arguments.size(); i != e; ++i) {
        ArgumentsVector.push_back(Arguments[i]->codegen());
        if (ArgumentsVector.back() == nullptr) {
            return nullptr;
        }
    }
    return Builder->CreateCall(CalleeFunction, ArgumentsVector, "calltmp");
}

llvm::Function *ASTNode::SignatureASTNode::codegen() {
    std::vector<llvm::Type *> Doubles(Arguments.size(), llvm::Type::getDoubleTy(*TheContext));
    llvm::FunctionType *FT = llvm::FunctionType::get(llvm::Type::getDoubleTy(*TheContext), Doubles, false);
    llvm::Function *F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, Name, TheModule.get());

    unsigned int Index = 0;
    for (auto &Argument: F->args()) {
        Argument.setName(Arguments[Index++]);
    }
    return F;
}

llvm::Function *ASTNode::FunctionASTNode::codegen() {
    llvm::Function *TheFunction = TheModule->getFunction(Signature->getName());

    if (TheFunction == nullptr) {
        TheFunction = Signature->codegen();
    }

    if (TheFunction == nullptr) {
        return nullptr;
    }

    if (TheFunction->empty() == false) {
        LogErrorV("Function cannot be redefined.");
        return nullptr;
    }

    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*TheContext, "entry", TheFunction);
    Builder->SetInsertPoint(BB);

    NamedValues.clear();
    for (auto &Argument: TheFunction->args()) {
        NamedValues[Argument.getName().str()] = &Argument;
    }

    llvm::Value *ReturnValue = Body->codegen();
    if (ReturnValue == nullptr) {
        TheFunction->eraseFromParent();
        return nullptr;
    }
    Builder->CreateRet(ReturnValue);
    llvm::verifyFunction(*TheFunction);
    return TheFunction;
}
