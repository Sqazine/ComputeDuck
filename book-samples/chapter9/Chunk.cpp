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
        case OP_ARRAY:
            cout << std::format("{:08}\tOP_ARRAY\t{}\n", i, opCodeList[++i]);
            break;
        case OP_GET_INDEX:
            cout << std::format("{:08}\tOP_GET_INDEX\n", i);
            break;
        case OP_SET_INDEX:
            cout << std::format("{:08}\tOP_SET_INDEX\n", i);
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
            cout << std::format("{:08}\tOP_DEF_GLOBAL\t{}\n", i, opCodeList[++i]);
            break;
        case OP_SET_GLOBAL:
            cout << std::format("{:08}\tOP_SET_GLOBAL\t{}\n", i, opCodeList[++i]);
            break;
        case OP_GET_GLOBAL:
            cout << std::format("{:08}\tOP_GET_GLOBAL\t{}\n", i, opCodeList[++i]);
            break;

        case OP_DEF_LOCAL:
            cout << std::format("{:08}\tOP_DEF_LOCAL\t{}\n", i, opCodeList[++i]);
            break;
        case OP_SET_LOCAL:
            cout << std::format("{:08}\tOP_SET_LOCAL\t{}\n", i, opCodeList[++i]);
            break;
        case OP_GET_LOCAL:
            cout << std::format("{:08}\tOP_GET_LOCAL\t{}\n", i, opCodeList[++i]);
            break;

        case OP_PRINT:
            cout << std::format("{:08}\tOP_PRINT\n", i);
            break;
        // ++ 新增内容
        case OP_JUMP:
            cout << std::format("{:08}\tOP_JUMP\t{}\n", i, opCodeList[++i]);
            break;
        case OP_JUMP_IF_FALSE:
            cout << std::format("{:08}\tOP_JUMP_IF_FALSE\t{}\n", i, opCodeList[++i]);
            break;
        // -- 新增内容
        default:
            break;
        }
    }

    return cout.str();
}