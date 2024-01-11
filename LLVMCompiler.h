#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include "llvm/IR/Value.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APFixedPoint.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "Config.h"
#include "Ast.h"
#include "Config.h"
#include "Utils.h"

class COMPUTE_DUCK_API LLVMCompiler
{
public:
    LLVMCompiler();
    ~LLVMCompiler();
    llvm::Function *Compile(const std::vector<Stmt *> &stmts);

    void ResetStatus();

private:
    llvm::Value *CompileStmt(Stmt *stmt);
    llvm::Value *CompileExprStmt(ExprStmt *stmt);
    llvm::Value *CompileIfStmt(IfStmt *stmt);
    llvm::Value *CompileScopeStmt(ScopeStmt *stmt);
    llvm::Value *CompileWhileStmt(WhileStmt *stmt);
    llvm::Value *CompileReturnStmt(ReturnStmt *stmt);
    llvm::Value *CompileStructStmt(StructStmt *stmt);

    llvm::Value *CompileExpr(Expr *expr);
    llvm::Value *CompileInfixExpr(InfixExpr *expr);
    llvm::Value *CompileNumExpr(NumExpr *expr);
    llvm::Value *CompileBoolExpr(BoolExpr *expr);
    llvm::Value *CompilePrefixExpr(PrefixExpr *expr);
    llvm::Value *CompileStrExpr(StrExpr *expr);
    llvm::Value *CompileNilExpr(NilExpr *expr);
    llvm::Value *CompileGroupExpr(GroupExpr *expr);
    llvm::Value *CompileArrayExpr(ArrayExpr *expr);
    llvm::Value *CompileIndexExpr(IndexExpr *expr);
    llvm::Value *CompileIdentifierExpr(IdentifierExpr *expr);
    llvm::Value *CompileFunctionExpr(FunctionExpr *expr);
    llvm::Value *CompileFunctionCallExpr(FunctionCallExpr *expr);
    llvm::Value *CompileStructCallExpr(StructCallExpr *expr);
    llvm::Value *CompileRefExpr(RefExpr *expr);
    llvm::Value *CompileAnonyStructExpr(AnonyStructExpr *expr);
    llvm::Value *CompileDllImportExpr(DllImportExpr *expr);

    llvm::Function* GetFunction(const std::string& name);

    std::unique_ptr<llvm::LLVMContext> m_Context;
    std::unique_ptr<llvm::Module> m_Module;
    std::unique_ptr<llvm::IRBuilder<>> m_Builder;
    std::unordered_map<std::string, llvm::Value *> m_NamedValues;
};