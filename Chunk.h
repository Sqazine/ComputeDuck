#pragma once
#include <vector>
#include <iomanip>
#include <array>
#include "Value.h"

enum OpCode
{
    OP_CONSTANT,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_NOT,
    OP_MINUS,
    OP_AND,
    OP_OR,
    OP_BIT_AND,
    OP_BIT_OR,
    OP_BIT_NOT,
    OP_BIT_XOR,
    OP_JUMP_IF_FALSE,
    OP_JUMP,
    OP_DEF_GLOBAL,
    OP_SET_GLOBAL,
    OP_GET_GLOBAL,
    OP_DEF_LOCAL,
    OP_SET_LOCAL,
    OP_GET_LOCAL,
    OP_ARRAY,
    OP_GET_INDEX,
    OP_SET_INDEX,
    OP_FUNCTION_CALL,
    OP_RETURN,
    OP_GET_BUILTIN,
    OP_STRUCT,
    OP_GET_STRUCT,
    OP_SET_STRUCT,
    OP_REF_GLOBAL,
    OP_REF_LOCAL,
    OP_REF_INDEX_GLOBAL,
    OP_REF_INDEX_LOCAL,
    OP_SP_OFFSET,
    OP_DLL_IMPORT,
#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
    OP_JUMP_START,
    OP_JUMP_END,
#endif
};

using OpCodes = std::vector<int16_t>;

class COMPUTEDUCK_API Chunk
{
public:
    Chunk() = default;
    Chunk(OpCodes opCodes, const std::vector<Value> &constants);
    ~Chunk()
    {
        OpCodes().swap(opCodes);
    }

    std::string Stringify();

    OpCodes opCodes;

    std::vector<Value> constants;

private:
    std::string OpCodeStringify(const OpCodes &opcodes);
};
