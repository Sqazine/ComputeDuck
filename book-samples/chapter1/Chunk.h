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