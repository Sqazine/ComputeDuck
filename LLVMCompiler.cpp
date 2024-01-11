#include "LLVMCompiler.h"

LLVMCompiler::LLVMCompiler()
{
    ResetStatus();
}

LLVMCompiler::~LLVMCompiler()
{
}

llvm::Function *LLVMCompiler::Compile(const std::vector<Stmt *> &stmts)
{
    llvm::FunctionType *ft = llvm::FunctionType::get(llvm::Type::getVoidTy(*m_Context), false);
    llvm::Function *topLevelFn = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "main", m_Module.get());

    llvm::BasicBlock *block = llvm::BasicBlock::Create(*m_Context, "", topLevelFn);
    m_Builder->SetInsertPoint(block);

    for (const auto &stmt : stmts)
    {
        auto v = CompileStmt(stmt);
        // v->print(llvm::errs());
        m_Builder->CreateRet(v);
    }

    topLevelFn->print(llvm::errs());

    return topLevelFn;
}

void LLVMCompiler::ResetStatus()
{
    m_Context = std::make_unique<llvm::LLVMContext>();
    m_Module = std::make_unique<llvm::Module>("ComputeDuckModule", *m_Context);
    m_Builder = std::make_unique<llvm::IRBuilder<>>(*m_Context);
}

llvm::Value *LLVMCompiler::CompileStmt(Stmt *stmt)
{
    switch (stmt->type)
    {
    case AstType::RETURN:
        return CompileReturnStmt((ReturnStmt *)stmt);

    case AstType::EXPR:
        return CompileExprStmt((ExprStmt *)stmt);
    case AstType::SCOPE:
        return CompileScopeStmt((ScopeStmt *)stmt);
    case AstType::IF:
        return CompileIfStmt((IfStmt *)stmt);
    case AstType::WHILE:
        return CompileWhileStmt((WhileStmt *)stmt);
    case AstType::STRUCT:
        return CompileStructStmt((StructStmt *)stmt);
    default:
        return nullptr;
    }
}

llvm::Value *LLVMCompiler::CompileExprStmt(ExprStmt *stmt)
{
    return CompileExpr(stmt->expr);
}

llvm::Value *LLVMCompiler::CompileIfStmt(IfStmt *stmt)
{
    auto condV = CompileExpr(stmt->condition);
    if (!condV)
        ASSERT("Failed to compile condition");

    // condV = m_Builder->CreateFCmpONE(condV, llvm::ConstantFP::get(*m_Context, llvm::APFloat(0.0)));

    llvm::Function *function = m_Builder->GetInsertBlock()->getParent();

    llvm::BasicBlock *thenBlock = llvm::BasicBlock::Create(*m_Context, "then", function);
    llvm::BasicBlock *elseBlock = llvm::BasicBlock::Create(*m_Context, "else");
    llvm::BasicBlock *mergeBlock = llvm::BasicBlock::Create(*m_Context, "ifcont");

    m_Builder->CreateCondBr(condV, thenBlock, elseBlock);

    // Emit then value.
    m_Builder->SetInsertPoint(thenBlock);

    auto thenV = CompileStmt(stmt->thenBranch);
    if (!thenV)
         ASSERT("Failed to compile then branch");

    m_Builder->CreateBr(mergeBlock);

    // Codegen of 'Then' can change the current block, update ThenBB for the PHI.
    thenBlock = m_Builder->GetInsertBlock();

     llvm::Value *elseV =nullptr;
    if (stmt->elseBranch)
    {
        // Emit else block.
        function->getBasicBlockList().push_back(elseBlock);
        m_Builder->SetInsertPoint(elseBlock);

        llvm::Value *elseV = CompileStmt(stmt->elseBranch);
        if (!elseV)
             ASSERT("Failed to compile else branch");

        m_Builder->CreateBr(mergeBlock);
        // Codegen of 'Else' can change the current block, update ElseBB for the PHI.
        elseBlock = m_Builder->GetInsertBlock();
    }

    function->getBasicBlockList().push_back(mergeBlock);
    m_Builder->SetInsertPoint(mergeBlock);
    llvm::PHINode* pn=m_Builder->CreatePHI(llvm::Type::getDoubleTy(*m_Context),2);
    pn->addIncoming(thenV,thenBlock);
    if (stmt->elseBranch)
        pn->addIncoming(elseV,elseBlock);

    return pn;
}
llvm::Value *LLVMCompiler::CompileScopeStmt(ScopeStmt *stmt)
{
    return nullptr;
}
llvm::Value *LLVMCompiler::CompileWhileStmt(WhileStmt *stmt)
{
    return nullptr;
}
llvm::Value *LLVMCompiler::CompileReturnStmt(ReturnStmt *stmt)
{
    return nullptr;
}
llvm::Value *LLVMCompiler::CompileStructStmt(StructStmt *stmt)
{
    return nullptr;
}

llvm::Value *LLVMCompiler::CompileExpr(Expr *expr)
{
    switch (expr->type)
    {
    case AstType::NUM:
        return CompileNumExpr((NumExpr *)expr);
    case AstType::STR:
        return CompileStrExpr((StrExpr *)expr);
    case AstType::BOOL:
        return CompileBoolExpr((BoolExpr *)expr);
    case AstType::NIL:
        return CompileNilExpr((NilExpr *)expr);
    case AstType::IDENTIFIER:
        return CompileIdentifierExpr((IdentifierExpr *)expr);
    case AstType::GROUP:
        return CompileGroupExpr((GroupExpr *)expr);
    case AstType::ARRAY:
        return CompileArrayExpr((ArrayExpr *)expr);
    case AstType::INDEX:
        return CompileIndexExpr((IndexExpr *)expr);
    case AstType::PREFIX:
        return CompilePrefixExpr((PrefixExpr *)expr);
    case AstType::INFIX:
        return CompileInfixExpr((InfixExpr *)expr);
    case AstType::FUNCTION_CALL:
        return CompileFunctionCallExpr((FunctionCallExpr *)expr);
    case AstType::STRUCT_CALL:
        return CompileStructCallExpr((StructCallExpr *)expr);
    case AstType::REF:
        return CompileRefExpr((RefExpr *)expr);
    case AstType::FUNCTION:
        return CompileFunctionExpr((FunctionExpr *)expr);
    case AstType::ANONY_STRUCT:
        return CompileAnonyStructExpr((AnonyStructExpr *)expr);
    case AstType::DLL_IMPORT:
        return CompileDllImportExpr((DllImportExpr *)expr);
    default:
        return nullptr;
    }
}
llvm::Value *LLVMCompiler::CompileInfixExpr(InfixExpr *expr)
{
    if (expr->op == "=")
    {
        auto lValue = CompileExpr(expr->left);
        auto rValue = CompileExpr(expr->right);
    }
    else
    {
        auto rValue = CompileExpr(expr->right);
        auto lValue = CompileExpr(expr->left);

        if (!lValue || !rValue)
            return nullptr;

        if (expr->op == "+")
            return m_Builder->CreateFAdd(lValue, rValue);
        else if (expr->op == "-")
            return m_Builder->CreateFSub(lValue, rValue);
        else if (expr->op == "*")
            return m_Builder->CreateFMul(lValue, rValue);
        else if (expr->op == "/")
            return m_Builder->CreateFDiv(lValue, rValue);
        else if (expr->op == ">")
            return m_Builder->CreateFCmpUGT(lValue, rValue);
        else if (expr->op == "<")
            return m_Builder->CreateFCmpULT(lValue, rValue);
        else if (expr->op == "&")
            return m_Builder->CreateBinOp(llvm::Instruction::And, lValue, rValue);
        else if (expr->op == "|")
            return m_Builder->CreateBinOp(llvm::Instruction::Or, lValue, rValue);
        else if (expr->op == "^")
            return m_Builder->CreateBinOp(llvm::Instruction::Xor, lValue, rValue);
        else if (expr->op == ">=")
            return m_Builder->CreateFCmpUGE(lValue, rValue);
        else if (expr->op == "<=")
            return m_Builder->CreateFCmpULE(lValue, rValue);
        else if (expr->op == "==")
            return m_Builder->CreateFCmpUEQ(lValue, rValue);
        else if (expr->op == "!=")
            return m_Builder->CreateFCmpUNE(lValue, rValue);
        else if (expr->op == "and")
            return m_Builder->CreateLogicalAnd(lValue, rValue);
        else if (expr->op == "or")
            return m_Builder->CreateLogicalOr(lValue, rValue);
    }

    return nullptr;
}
llvm::Value *LLVMCompiler::CompileNumExpr(NumExpr *expr)
{
    return llvm::ConstantFP::get(*m_Context, llvm::APFloat(expr->value));
}
llvm::Value *LLVMCompiler::CompileBoolExpr(BoolExpr *expr)
{
    return llvm::ConstantInt::get(*m_Context, llvm::APInt(1, expr->value ? 1 : 0));
}
llvm::Value *LLVMCompiler::CompilePrefixExpr(PrefixExpr *expr)
{
    auto rValue = CompileExpr(expr->right);
    if (expr->op == "-")
        return m_Builder->CreateUnOp(llvm::Instruction::UnaryOps::FNeg, rValue);
    else if (expr->op == "not")
        return m_Builder->CreateNot(rValue);
    else if (expr->op == "~")
    {
        rValue = m_Builder->CreateFPToUI(rValue, llvm::Type::getInt64Ty(*m_Context));
        return m_Builder->CreateUnOp(llvm::Instruction::UnaryOps::FNeg, rValue);
    }
    else
        ASSERT("Unrecognized prefix op");
}
llvm::Value *LLVMCompiler::CompileStrExpr(StrExpr *expr)
{
    return nullptr;
}
llvm::Value *LLVMCompiler::CompileNilExpr(NilExpr *expr)
{
    return llvm::ConstantPointerNull::get(llvm::PointerType::get(llvm::Type::getVoidTy(*m_Context), 0));
}
llvm::Value *LLVMCompiler::CompileGroupExpr(GroupExpr *expr)
{
    return CompileExpr(expr->expr);
}
llvm::Value *LLVMCompiler::CompileArrayExpr(ArrayExpr *expr)
{
    return nullptr;
}
llvm::Value *LLVMCompiler::CompileIndexExpr(IndexExpr *expr)
{
    return nullptr;
}
llvm::Value *LLVMCompiler::CompileIdentifierExpr(IdentifierExpr *expr)
{
    return nullptr;
}
llvm::Value *LLVMCompiler::CompileFunctionExpr(FunctionExpr *expr)
{
    return nullptr;
}
llvm::Value *LLVMCompiler::CompileFunctionCallExpr(FunctionCallExpr *expr)
{
    return nullptr;
}
llvm::Value *LLVMCompiler::CompileStructCallExpr(StructCallExpr *expr)
{
    return nullptr;
}
llvm::Value *LLVMCompiler::CompileRefExpr(RefExpr *expr)
{
    return nullptr;
}
llvm::Value *LLVMCompiler::CompileAnonyStructExpr(AnonyStructExpr *expr)
{
    return nullptr;
}
llvm::Value *LLVMCompiler::CompileDllImportExpr(DllImportExpr *expr)
{
    return nullptr;
}

llvm::Function *LLVMCompiler::GetFunction(const std::string &name)
{
    if (auto f = m_Module->getFunction(name))
        return f;
    return nullptr;
}
