#pragma once
#include "Utils.h"
#include "Value.h"
#include <vector>
#include <string>

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
    OP_DEF_GLOBAL,
    OP_SET_GLOBAL,
    OP_GET_GLOBAL,
    OP_ARRAY,
    OP_GET_INDEX,
    OP_SET_INDEX,
    OP_DEF_LOCAL,
    OP_SET_LOCAL,
    OP_GET_LOCAL,
    OP_JUMP_IF_FALSE,
    OP_JUMP,
    OP_FUNCTION_CALL,
    OP_RETURN,
    OP_GET_BUILTIN,
};

using OpCodeList = std::vector<int16_t>;

class COMPUTEDUCK_API Chunk
{
public:
    Chunk() = default;
    Chunk(OpCodeList opCodeList, const std::vector<Value> &constants);
    ~Chunk()
    {
        OpCodeList().swap(opCodeList);
    }

    std::string Stringify();

    OpCodeList opCodeList;

    std::vector<Value> constants;
private:
    std::string OpCodeStringify(const OpCodeList &opCodeList);
};