#pragma once
#include <vector>
#include "Chunk.h"
#include "Ast.h"
#include "Value.h"
#include "SymbolTable.h"

class COMPUTEDUCK_API Compiler
{
public:
    Compiler() = default;
    ~Compiler();

    const Chunk &Compile(const std::vector<Stmt *> &stmts);

    // Only for debugging
    size_t GetSymbolDefCount() { return m_SymbolTable->GetDefinitionCount(); }

private:
    enum class RWState
    {
        READ,
        WRITE,
    };

    void ResetStatus();

    void CompileStmt(Stmt *stmt);
    void CompileExprStmt(ExprStmt *stmt);

    void CompileExpr(Expr *expr, const RWState &state = RWState::READ);
    void CompileBinaryExpr(BinaryExpr *expr);
    void CompileNumExpr(NumExpr *expr);
    void CompileBoolExpr(BoolExpr *expr);
    void CompileUnaryExpr(UnaryExpr *expr);
    void CompileStrExpr(StrExpr *expr);
    void CompileIdentifierExpr(IdentifierExpr *expr, const RWState &state);
    void CompileNilExpr(NilExpr *expr);
    void CompileGroupExpr(GroupExpr *expr);
    void CompileArrayExpr(ArrayExpr *expr);
    void CompileIndexExpr(IndexExpr *expr, const RWState &state);

    Chunk &CurChunk();

    uint32_t Emit(int16_t opcode);
    uint32_t EmitConstant(const Value &value);

    void DefineSymbol(const Symbol &symbol);
    void LoadSymbol(const Symbol &symbol);
    void StoreSymbol(const Symbol &symbol);

    Chunk m_Chunk;

    SymbolTable *m_SymbolTable{nullptr};
};