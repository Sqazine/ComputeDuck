#include "Chunk.h"
#include "Object.h"
#include <format>
#include <sstream>
Chunk::Chunk(OpCodeList opCodeList, const std::vector<Value> &constants)
    : opCodeList(opCodeList), constants(constants)
{
}

std::string Chunk::Stringify()
{
    std::string result = OpCodeStringify(opCodeList);
    for (const auto &c : constants)
        if (IS_FUNCTION_VALUE(c))
            result += ObjectStringify(TO_FUNCTION_VALUE(c)
#ifndef NDEBUG
                                          ,
                                      true
#endif
            );
    return result;
}

std::string Chunk::OpCodeStringify(const OpCodeList &opCodeList)
{
    std::stringstream cout;
    for (size_t i = 0; i < opCodeList.size(); ++i)
    {
        size_t curAddress = i;
        switch (opCodeList[curAddress])
        {
        case OP_CONSTANT:
            cout << std::format("{:08}\tOP_CONSTANT\t'{}'\n", curAddress, constants[opCodeList[++i]].Stringify());
            break;
        case OP_ADD:
            cout << std::format("{:08}\tOP_ADD\n", curAddress);
            break;
        case OP_SUB:
            cout << std::format("{:08}\tOP_SUB\n", curAddress);
            break;
        case OP_MUL:
            cout << std::format("{:08}\tOP_MUL\n", curAddress);
            break;
        case OP_DIV:
            cout << std::format("{:08}\tOP_DIV\n", curAddress);
            break;
        case OP_LESS:
            cout << std::format("{:08}\tOP_LESS\n", curAddress);
            break;
        case OP_GREATER:
            cout << std::format("{:08}\tOP_GREATER\n", curAddress);
            break;
        case OP_NOT:
            cout << std::format("{:08}\tOP_NOT\n", curAddress);
            break;
        case OP_MINUS:
            cout << std::format("{:08}\tOP_MINUS\n", curAddress);
            break;
        case OP_EQUAL:
            cout << std::format("{:08}\tOP_EQUAL\n", curAddress);
            break;
        case OP_ARRAY:
            cout << std::format("{:08}\tOP_ARRAY\t{}\n", curAddress, opCodeList[++i]);
            break;
        case OP_GET_INDEX:
            cout << std::format("{:08}\tOP_GET_INDEX\n", curAddress);
            break;
        case OP_SET_INDEX:
            cout << std::format("{:08}\tOP_SET_INDEX\n", curAddress);
            break;
        case OP_AND:
            cout << std::format("{:08}\tOP_AND\n", curAddress);
            break;
        case OP_OR:
            cout << std::format("{:08}\tOP_OR\n", curAddress);
            break;
        case OP_BIT_AND:
            cout << std::format("{:08}\tOP_BIT_AND\n", curAddress);
            break;
        case OP_BIT_OR:
            cout << std::format("{:08}\tOP_BIT_OR\n", curAddress);
            break;
        case OP_BIT_NOT:
            cout << std::format("{:08}\tOP_BIT_NOT\n", curAddress);
            break;
        case OP_BIT_XOR:
            cout << std::format("{:08}\tOP_BIT_XOR\n", curAddress);
            break;
        case OP_DEF_GLOBAL:
            cout << std::format("{:08}\tOP_DEF_GLOBAL\t{}\n", curAddress, opCodeList[++i]);
            break;
        case OP_SET_GLOBAL:
            cout << std::format("{:08}\tOP_SET_GLOBAL\t{}\n", curAddress, opCodeList[++i]);
            break;
        case OP_GET_GLOBAL:
            cout << std::format("{:08}\tOP_GET_GLOBAL\t{}\n", curAddress, opCodeList[++i]);
            break;
        case OP_DEF_LOCAL:
            cout << std::format("{:08}\tOP_DEF_LOCAL\t{}\n", curAddress, opCodeList[++i]);
            break;
        case OP_SET_LOCAL:
            cout << std::format("{:08}\tOP_SET_LOCAL\t{}\n", curAddress, opCodeList[++i]);
            break;
        case OP_GET_LOCAL:
            cout << std::format("{:08}\tOP_GET_LOCAL\t{}\n", curAddress, opCodeList[++i]);
            break;
        case OP_JUMP:
            cout << std::format("{:08}\tOP_JUMP\t{}\n", curAddress, opCodeList[++i]);
            break;
        case OP_JUMP_IF_FALSE:
            cout << std::format("{:08}\tOP_JUMP_IF_FALSE\t{}\n", curAddress, opCodeList[++i]);
            break;
        case OP_RETURN:
            cout << std::format("{:08}\tOP_RETURN\t{}\n", curAddress, opCodeList[++i]);
            break;
        case OP_FUNCTION_CALL:
            cout << std::format("{:08}\tOP_FUNCTION_CALL\t{}\n", curAddress, opCodeList[++i]);
            break;
        case OP_GET_BUILTIN:
            cout << std::format("{:08}\tOP_GET_BUILTIN\t{}\n", curAddress, opCodeList[++i]);
            break;
        case OP_CLOSURE:
        {
            cout << std::format("{:08}\tOP_CLOSURE\t{}\t{}\n", curAddress, opCodeList[++i], opCodeList[++i]);
            auto upvalueCount = opCodeList[i];
            for (uint8_t j = 0; j < upvalueCount; ++j)
                cout << std::format("\t\t\t|\t{}\t{}\n", opCodeList[++i], opCodeList[++i]);
            break;
        }
        case OP_GET_UPVALUE:
            cout << std::format("{:08}\tOP_GET_UPVALUE\t{}\n", curAddress, opCodeList[++i]);
            break;
        case OP_SET_UPVALUE:
            cout << std::format("{:08}\tOP_SET_UPVALUE\t{}\n", curAddress, opCodeList[++i]);
            break;
        default:
            break;
        }
    }

    return cout.str();
}