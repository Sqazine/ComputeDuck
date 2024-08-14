#pragma once
#include <vector>
#include "Chunk.h"
#include "Ast.h"
#include "Value.h"
#include "Object.h"
#include "SymbolTable.h"

class COMPUTE_DUCK_API Compiler
{

public:
    Compiler() = default;
    ~Compiler();

    Value Compile(const std::vector<Stmt *> &stmts);

private:
    void ResetStatus();

    void CompileStmt(Stmt *stmt);
    void CompileExprStmt(ExprStmt *stmt);
    void CompileIfStmt(IfStmt *stmt);
    void CompileScopeStmt(ScopeStmt *stmt);
    void CompileWhileStmt(WhileStmt *stmt);
    void CompileReturnStmt(ReturnStmt *stmt);
    void CompileStructStmt(StructStmt *stmt);

    ValueType CompileExpr(Expr *expr, const RWState &state = RWState::READ, ValueType type = ValueType::NIL);
    ValueType CompileInfixExpr(InfixExpr *expr);
    ValueType CompileNumExpr(NumExpr *expr);
    ValueType CompileBoolExpr(BoolExpr *expr);
    ValueType CompilePrefixExpr(PrefixExpr *expr);
    ValueType CompileStrExpr(StrExpr *expr);
    ValueType CompileNilExpr(NilExpr *expr);
    ValueType CompileGroupExpr(GroupExpr *expr);
    ValueType CompileArrayExpr(ArrayExpr *expr);
    ValueType CompileIndexExpr(IndexExpr *expr, const RWState& state);
    ValueType CompileIdentifierExpr(IdentifierExpr *expr, const RWState &state = RWState::READ, ValueType type = ValueType::NIL);
    ValueType CompileFunctionExpr(FunctionExpr *expr);
    ValueType CompileFunctionCallExpr(FunctionCallExpr *expr);
    ValueType CompileStructCallExpr(StructCallExpr *expr, const RWState &state);
    ValueType CompileRefExpr(RefExpr *expr);
    ValueType CompileStructExpr(StructExpr *expr);
    ValueType CompileDllImportExpr(DllImportExpr *expr);

    void EnterScope();
    void ExitScope();

    Chunk &CurChunk();

    uint32_t Emit(int16_t opcode);
    uint32_t EmitConstant(const Value &value);

    void ModifyOpCode(uint32_t pos, int16_t opcode);

    void LoadSymbol(const Symbol &symbol);
    void StoreSymbol(const Symbol &symbol);

    void PushFnObj(FunctionObject* fnObj);
    void PopFnObj();
    FunctionObject* GetCurFnObj();

    std::vector<Chunk> m_ScopeChunks;

    std::vector<FunctionObject*> m_FunctionObjectList;

    SymbolTable *m_SymbolTable{nullptr};
};