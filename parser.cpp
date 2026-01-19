#include "ast.h"
#include "lexer.h"
#include <cstdio>
#include <memory>
#include <vector>
#include <string>

using namespace ASTNode;

static int CurrentToken;

static int getNextToken() {
    return CurrentToken = gettok();
};

std::unique_ptr<ExpressionASTNode> LogError(const char *str) {
    fprintf(stderr, "Error: %s\n", str);
    return nullptr;
}

std::unique_ptr<SignatureASTNode> LogErrorS(const char *str) {
    LogError(str);
    return nullptr;
}

static std::unique_ptr<ExpressionASTNode> ParseExpression();

static std::unique_ptr<ExpressionASTNode> ParseNumberExpression() {
    auto Result = std::make_unique<NumberExpressionASTNode>(NumVal);
    getNextToken();
    return std::move(Result);
}

static std::unique_ptr<ExpressionASTNode> ParseParenthesisExpression() {
    getNextToken();
    auto ASTNodePtr = ParseExpression();
    if (ASTNodePtr == nullptr) {
        return nullptr;
    }
    if (CurrentToken != ')') {
        return LogError("expected ')'");
    }
    getNextToken();
    return ASTNodePtr;
}

static std::unique_ptr<ExpressionASTNode> ParseIdentifierExpression() {
    std::string IdName = IdentifierStr;
    getNextToken();
    if (CurrentToken != '(') {
        // if it is a variable
        return std::make_unique<VariableExpressionASTNode>(IdName);
    }
    getNextToken();
    std::vector<std::unique_ptr<ExpressionASTNode> > Arguments;
    if (CurrentToken != ')') {
        while (true) {
            auto Argument = ParseExpression();
            if (Argument) {
                Arguments.push_back(std::move(Argument));
            } else {
                return nullptr;
            }
            if (CurrentToken == ')')
                break;
            if (CurrentToken != ',')
                return LogError("Expected ')' or ',' in argument list");
            getNextToken();
        }
    }
    getNextToken();
    return std::make_unique<FunctionCallExpressionASTNode>(IdName, std::move(Arguments));
}

static std::unique_ptr<ExpressionASTNode> ParsePrimaryExpression() {
    switch (CurrentToken) {
        case tok_identifier:
            return ParseIdentifierExpression();
        case tok_number:
            return ParseNumberExpression();
        case '(':
            return ParseParenthesisExpression();
        default:
            return LogError("unexpected token");
    }
}