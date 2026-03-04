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
    OP_MINUS,
    OP_BIT_AND,
    OP_BIT_OR,
    OP_BIT_NOT,
    OP_BIT_XOR,
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