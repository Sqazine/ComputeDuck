#include "Chunk.h"
#include <format>
#include <sstream>
Chunk::Chunk(OpCodeList opCodeList, const std::vector<Value> &constants)
    : opCodeList(opCodeList), constants(constants)
{
}

std::string Chunk::Stringify()
{
    std::string result = OpCodeStringify(opCodeList);
    return result;
}

std::string Chunk::OpCodeStringify(const OpCodeList &opCodeList)
{
    std::stringstream cout;
    for (int32_t i = 0; i < opCodeList.size(); ++i)
    {
        switch (opCodeList[i])
        {
        case OP_CONSTANT:
            cout << std::format("{:08}\tOP_CONSTANT\t'{}'\n", i, constants[opCodeList[++i]].Stringify());
            break;
        case OP_ADD:
            cout << std::format("{:08}\tOP_ADD\n", i);
            break;
        case OP_SUB:
            cout << std::format("{:08}\tOP_SUB\n", i);
            break;
        case OP_MUL:
            cout << std::format("{:08}\tOP_MUL\n", i);
            break;
        case OP_DIV:
            cout << std::format("{:08}\tOP_DIV\n", i);
            break;
        default:
            break;
        }
    }

    return cout.str();
}