#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Utils.h"
#include "Ast.h"
#include "Utils.h"
#include "Value.h"
#include "SymbolTable.h"
#include "LLVMJit.h"

class COMPUTE_DUCK_API LLVMCompilerImpl
{
public:
    LLVMCompilerImpl();
    ~LLVMCompilerImpl();

    llvm::Function *Compile(const std::vector<Stmt *> &stmts);

    void Run(llvm::Function *fn);
private:
    void ResetStatus();

    void CompileStmt(Stmt *stmt);
    void CompileExprStmt(ExprStmt *stmt);
    void CompileIfStmt(IfStmt *stmt);
    void CompileScopeStmt(ScopeStmt *stmt);
    void CompileWhileStmt(WhileStmt *stmt);
    void CompileReturnStmt(ReturnStmt *stmt);
    void CompileStructStmt(StructStmt *stmt);

    void CompileExpr(Expr *expr, const RWState &state = RWState::READ);
    void CompileInfixExpr(InfixExpr *expr);
    void CompileNumExpr(NumExpr *expr);
    void CompileBoolExpr(BoolExpr *expr);
    void CompilePrefixExpr(PrefixExpr *expr);
    void CompileStrExpr(StrExpr *expr);
    void CompileNilExpr(NilExpr *expr);
    void CompileGroupExpr(GroupExpr *expr);
    void CompileArrayExpr(ArrayExpr *expr);
    void CompileIndexExpr(IndexExpr *expr);
    void CompileIdentifierExpr(IdentifierExpr *expr, const RWState &state);
    void CompileFunctionExpr(FunctionExpr *expr);
    void CompileFunctionCallExpr(FunctionCallExpr *expr);
    void CompileStructCallExpr(StructCallExpr *expr, const RWState &state = RWState::READ);
    void CompileRefExpr(RefExpr *expr);
    void CompileStructExpr(StructExpr *expr);
    void CompileDllImportExpr(DllImportExpr *expr);

	template <typename T>
		requires(std::is_same_v<T,llvm::CallInst> || std::is_same_v<T,llvm::Function>)
    T* AddBuiltinFnOrCallParamAttributes(T* fnOrCallInst)
    {
		fnOrCallInst->addRetAttr(llvm::Attribute::ZExt);
		fnOrCallInst->addParamAttr(0, llvm::Attribute::NoUndef);
		fnOrCallInst->addParamAttr(1, llvm::Attribute::NoUndef);
		fnOrCallInst->addParamAttr(1, llvm::Attribute::ZExt);
		fnOrCallInst->addParamAttr(2, llvm::Attribute::NoUndef);
		fnOrCallInst->addParamAttr(2, llvm::Attribute::NonNull);

        return fnOrCallInst;
    }

	template <typename T>
		requires(std::is_same_v<T, llvm::CallInst> || std::is_same_v<T, llvm::Function>)
	T* AddValueFnParamAttributes(T* fnOrCallInst)
	{
		fnOrCallInst->addParamAttr(0, llvm::Attribute::NoUndef);
		fnOrCallInst->addParamAttr(0, llvm::Attribute::NonNull);
		fnOrCallInst->addParamAttr(1, llvm::Attribute::NoUndef);
		fnOrCallInst->addParamAttr(1, llvm::Attribute::NonNull);
		fnOrCallInst->addParamAttr(2, llvm::Attribute::NoUndef);
		fnOrCallInst->addParamAttr(2, llvm::Attribute::NonNull);

		return fnOrCallInst;
	}

    void Push(llvm::Value *v);
    llvm::Value *Peek(int32_t distance);
    llvm::Value *Pop();

    llvm::Function *GetCurFunction();
    void PushFunction(llvm::Function *fn);
    llvm::Function *PeekFunction(int32_t distance);
    llvm::Function *PopFunction();

    void EnterScope();
    void ExitScope();

    void InitModuleAndPassManager();

    void RegisterLlvmFn(std::string_view name, llvm::Function *fn);
    llvm::Function *GetLlvmFn(std::string_view name);

    std::unordered_map<std::string_view, llvm::Function *> m_LlvmBuiltins;

    llvm::StructType *m_ValueType;
    llvm::PointerType *m_ValuePtrType;

    llvm::StructType *mUnionType;

    llvm::StructType *m_ObjectType;
    llvm::PointerType *m_ObjectPtrType;
    llvm::StructType *m_StrObjectType;
    llvm::PointerType *m_StrObjectPtrType;

    llvm::FunctionType *m_BuiltinFunctionType;
    llvm::FunctionType *m_ValueFunctionType;

    llvm::Type *m_Int8Type;
    llvm::Type *m_BoolType;
    llvm::Type *m_DoubleType;
    llvm::Type *m_Int64Type;
    llvm::Type *m_Int32Type;
    llvm::Type *m_VoidType;
    llvm::Type *m_Int64PtrType;
    llvm::Type *m_Int32PtrType;
    llvm::Type *m_Int8PtrType;
    llvm::Type *m_BoolPtrType;
    llvm::Type *m_DoublePtrType;

    std::vector<llvm::Value *> m_ValueStack;
    std::vector<llvm::Function *> m_FunctionStack;

    SymbolTable *m_SymbolTable;

    std::unique_ptr<llvm::LLVMContext> m_Context;
    std::unique_ptr<llvm::Module> m_Module;
    std::unique_ptr<llvm::IRBuilder<>> m_Builder;

    llvm::ExitOnError m_ExitOnErr;

    std::unique_ptr<LLVMJit> m_Jit;
    std::unique_ptr<llvm::legacy::FunctionPassManager> m_FPM;
};