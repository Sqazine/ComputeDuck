#pragma once
#include <vector>
#include <iomanip>
#include <array>
#include "Value.h"
#include "Config.h"
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
    OP_SET_GLOBAL,
    OP_GET_GLOBAL,
    OP_SET_LOCAL,
    OP_GET_LOCAL,
    OP_ARRAY,
    OP_INDEX,
    OP_FUNCTION_CALL,
    OP_RETURN,
    OP_GET_BUILTIN_FUNCTION,
    OP_GET_BUILTIN_VARIABLE,
    OP_STRUCT,
    OP_GET_STRUCT,
    OP_SET_STRUCT,
    OP_REF_GLOBAL,
    OP_REF_LOCAL,
    OP_REF_INDEX_GLOBAL,
    OP_REF_INDEX_LOCAL,
    OP_SP_OFFSET,
};

typedef std::vector<int32_t> OpCodes;

#define CONSTANT_MAX 8192

class COMPUTE_DUCK_API Chunk
{
public:
    Chunk(OpCodes opCodes, const std::vector<Value>& constants);

    void Stringify();

    OpCodes opCodes;

    std::vector<Value> constants;

private:
    void OpCodeStringify(const OpCodes &opcodes);
};
