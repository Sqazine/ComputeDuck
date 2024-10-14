#pragma once
#include "Utils.h"
#include "Value.h"
#include <vector>
#include <string>
// OpCode表示虚拟机要执行的操作码,每个操作码带有0个或多个操作数
// 比如OP_CONSTANT操作码带有一个操作数,这个操作数是一个指向常量的索引
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
    // ++ 新增内容
    OP_PRINT,
    // -- 新增内容
};

// 使用OpCodes表示操作码列表使用int16_t是为了跳转操作码允许带有负数的操作数且允许更大的操作数索引,使用int16_t会比较浪费空间,这里使用主要为了简单方便
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