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