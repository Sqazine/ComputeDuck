#include "Chunk.h"
#include "Object.h"
#include <format>

Chunk::Chunk(OpCodes opCodes, const std::vector<Value> &constants)
    : opCodes(opCodes), constants(constants)
{
}

std::string Chunk::Stringify()
{
    std::string result = OpCodeStringify(opCodes);
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
        case OP_ARRAY:
            cout << std::format("{:08}\tOP_ARRAY\t{}\n", i, opcodes[++i]);
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
        case OP_GET_INDEX:
            cout << std::format("{:08}\tOP_GET_INDEX\n", i);
            break;
        case OP_SET_INDEX:
            cout << std::format("{:08}\tOP_SET_INDEX\n", i);
            break;
        case OP_JUMP:
#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
            cout << std::format("{:08}\tOP_JUMP\t{}\t{}\n", i, opcodes[++i], opcodes[++i]);
#else
             cout << std::format("{:08}\tOP_JUMP\t{}\n", i, opcodes[++i]);
#endif
            break;
        case OP_JUMP_IF_FALSE:
#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
            cout << std::format("{:08}\tOP_JUMP_IF_FALSE\t{}\t{}\n", i, opcodes[++i], opcodes[++i]);
#else
            cout << std::format("{:08}\tOP_JUMP_IF_FALSE\t{}\n", i, opcodes[++i]);
#endif
            break;
#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
        case OP_JUMP_START:
            std::cout << std::format("{:08}\tOP_JUMP_START\t{}\n", i, opcodes[++i]);
            break;
        case OP_JUMP_END:
            std::cout << std::format("{:08}\tOP_JUMP_END\n", i);
            break;
#endif
        case OP_RETURN:
            cout << std::format("{:08}\tOP_RETURN\n", i);
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
        case OP_DEF_LOCAL:
            cout << std::format("{:08}\tOP_DEF_LOCAL\t{}\n", i, opcodes[++i]);
            break;
        case OP_SET_LOCAL:
            cout << std::format("{:08}\tOP_SET_LOCAL\t{}\t{}\t{}\n", i, opcodes[++i],opcodes[++i],opcodes[++i]);
            break;
        case OP_GET_LOCAL:
            cout << std::format("{:08}\tOP_GET_LOCAL\t{}\t{}\t{}\n", i, opcodes[++i],opcodes[++i],opcodes[++i]);
            break;
        case OP_FUNCTION_CALL:
            cout << std::format("{:08}\tOP_FUNCTION_CALL\t{}\n", i, opcodes[++i]);
            break;
        case OP_GET_BUILTIN:
            cout << std::format("{:08}\tOP_GET_BUILTIN\t'{}'\n", i, constants[opcodes[++i]].Stringify());
            break;
        case OP_STRUCT:
            cout << std::format("{:08}\tOP_STRUCT\t{}\n", i, opcodes[++i]);
            break;
        case OP_GET_STRUCT:
            cout << std::format("{:08}\tOP_GET_STRUCT\n", i);
            break;
        case OP_SET_STRUCT:
            cout << std::format("{:08}\tOP_SET_STRUCT\n", i);
            break;
        case OP_REF_GLOBAL:
            cout << std::format("{:08}\tOP_REF_GLOBAL\t{}\n", i, opcodes[++i]);
            break;
        case OP_REF_LOCAL:
            cout << std::format("{:08}\tOP_REF_LOCAL\t{}\t{}\t{}\n", i, opcodes[++i],opcodes[++i],opcodes[++i]);
            break;
        case OP_REF_INDEX_GLOBAL:
            cout << std::format("{:08}\tOP_REF_INDEX_GLOBAL\t{}\n", i, opcodes[++i]);
            break;
        case OP_REF_INDEX_LOCAL:
            cout << std::format("{:08}\tOP_REF_INDEX_LOCAL\t{}\t{}\t{}\n", i, opcodes[++i],opcodes[++i],opcodes[++i]);
            break;
        case OP_SP_OFFSET:
            cout << std::format("{:08}\tOP_SP_OFFSET\t{}\n", i, opcodes[++i]);
            break;
        case OP_DLL_IMPORT:
            cout << std::format("{:08}\tOP_DLL_IMPORT\n", i);
            break;
        default:
            break;
        }
    }

    return cout.str();
}