#include "Chunk.h"
#include <format>
#include <sstream>
Chunk::Chunk(OpCodes opCodes, const std::vector<Value> &constants)
    : opCodes(opCodes), constants(constants)
{
}

std::string Chunk::Stringify()
{
    std::string result = OpCodeStringify(opCodes);
    return result;
}

std::string Chunk::OpCodeStringify(const OpCodes &opcodes)
{
    std::stringstream cout;
    for (int32_t i = 0; i < opcodes.size(); ++i)
    {
        switch (opcodes[i])
        {
        case OP_CONSTANT:
            cout << std::format("{:08}\tOP_CONSTANT\t'{}'\n", i, constants[opcodes[++i]].Stringify());
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
        case OP_LESS:
            cout << std::format("{:08}\tOP_LESS\n", i);
            break;
        case OP_GREATER:
            cout << std::format("{:08}\tOP_GREATER\n", i);
            break;
        case OP_NOT:
            cout << std::format("{:08}\tOP_NOT\n", i);
            break;
        case OP_MINUS:
            cout << std::format("{:08}\tOP_MINUS\n", i);
            break;
        case OP_EQUAL:
            cout << std::format("{:08}\tOP_EQUAL\n", i);
            break;
        case OP_AND:
            cout << std::format("{:08}\tOP_AND\n", i);
            break;
        case OP_OR:
            cout << std::format("{:08}\tOP_OR\n", i);
            break;
        case OP_BIT_AND:
            cout << std::format("{:08}\tOP_BIT_AND\n", i);
            break;
        case OP_BIT_OR:
            cout << std::format("{:08}\tOP_BIT_OR\n", i);
            break;
        case OP_BIT_NOT:
            cout << std::format("{:08}\tOP_BIT_NOT\n", i);
            break;
        case OP_BIT_XOR:
            cout << std::format("{:08}\tOP_BIT_XOR\n", i);
            break;
        case OP_DEF_GLOBAL:
            cout << std::format("{:08}\tOP_DEF_GLOBAL\t{}\n", i, opcodes[++i]);
            break;
        case OP_SET_GLOBAL:
            cout << std::format("{:08}\tOP_SET_GLOBAL\t{}\n", i, opcodes[++i]);
            break;
        case OP_GET_GLOBAL:
            cout << std::format("{:08}\tOP_GET_GLOBAL\t{}\n", i, opcodes[++i]);
            break;
        default:
            break;
        }
    }

    return cout.str();
}