#include "Chunk.h"
#include "Object.h"

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
    for (int32_t i = 0; i < opCodeList.size(); ++i)
    {
        switch (opCodeList[i])
        {
        case OP_CONSTANT:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_CONSTANT\t'" << constants[opCodeList[++i]].Stringify() << "'" << std::endl;
            break;
        case OP_ADD:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_ADD" << std::endl;
            break;
        case OP_SUB:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_SUB" << std::endl;
            break;
        case OP_MUL:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_MUL" << std::endl;
            break;
        case OP_DIV:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_DIV" << std::endl;
            break;
        case OP_LESS:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_LESS" << std::endl;
            break;
        case OP_GREATER:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_GREATER" << std::endl;
            break;
        case OP_NOT:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_NOT" << std::endl;
            break;
        case OP_MINUS:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_MINUS" << std::endl;
            break;
        case OP_EQUAL:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_EQUAL" << std::endl;
            break;
        case OP_ARRAY:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_ARRAY\t" << opCodeList[++i] << std::endl;
            break;
        case OP_AND:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_AND" << std::endl;
            break;
        case OP_OR:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_OR" << std::endl;
            break;
        case OP_BIT_AND:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_BIT_AND" << std::endl;
            break;
        case OP_BIT_OR:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_BIT_OR" << std::endl;
            break;
        case OP_BIT_NOT:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_BIT_NOT" << std::endl;
            break;
        case OP_BIT_XOR:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_BIT_XOR" << std::endl;
            break;
        case OP_GET_INDEX:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_GET_INDEX" << std::endl;
            break;
        case OP_SET_INDEX:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_SET_INDEX" << std::endl;
            break;
        case OP_JUMP:
#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_JUMP\t" << opCodeList[++i] << "\t" << opCodeList[++i] << std::endl;
#else
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_JUMP\t" << opCodeList[++i] << std::endl;
#endif
            break;
        case OP_JUMP_IF_FALSE:
#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_JUMP_IF_FALSE\t" << opCodeList[++i] << "\t" << opCodeList[++i] << std::endl;
#else
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_JUMP_IF_FALSE\t" << opCodeList[++i] << std::endl;
#endif
            break;
#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
        case OP_JUMP_START:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_JUMP_START\t" << opCodeList[++i] << std::endl;
            break;
        case OP_JUMP_END:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_JUMP_END" << std::endl;
            break;
#endif
        case OP_RETURN:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_RETURN\t" << opCodeList[++i] << std::endl;
            break;
        case OP_DEF_GLOBAL:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_DEF_GLOBAL\t" << opCodeList[++i] << std::endl;
            break;
        case OP_SET_GLOBAL:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_SET_GLOBAL\t" << opCodeList[++i] << std::endl;
            break;
        case OP_GET_GLOBAL:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_GET_GLOBAL\t" << opCodeList[++i] << std::endl;
            break;
        case OP_DEF_LOCAL:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_DEF_LOCAL\t" << opCodeList[++i] << std::endl;
            break;
        case OP_SET_LOCAL:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_SET_LOCAL\t" << opCodeList[++i] << std::endl;
            break;
        case OP_GET_LOCAL:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_GET_LOCAL\t" << opCodeList[++i] << std::endl;
            break;
        case OP_GET_UPVALUE:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_GET_UPVALUE\t" << opCodeList[++i] << std::endl;
            break;
        case OP_SET_UPVALUE:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_SET_UPVALUE\t" << opCodeList[++i] << std::endl;
            break;
        case OP_CLOSURE:
        {
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_CLOSURE\t" << opCodeList[++i] << "\t" << opCodeList[++i] << std::endl;
            for (uint8_t j = 0; j < opCodeList[i]; ++j)
                cout << std::setfill('0') << std::setw(8) << i << "\t|\t" << opCodeList[++i] << "\t" << opCodeList[++i] << std::endl;
            break;
        }
        case OP_FUNCTION_CALL:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_FUNCTION_CALL\t" << opCodeList[++i] << std::endl;
            break;
        case OP_GET_BUILTIN:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_GET_BUILTIN\t'" << constants[opCodeList[++i]].Stringify() << "'" << std::endl;
            break;
        case OP_STRUCT:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_STRUCT\t" << opCodeList[++i] << std::endl;
            break;
        case OP_GET_STRUCT:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_GET_STRUCT" << std::endl;
            break;
        case OP_SET_STRUCT:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_SET_STRUCT" << std::endl;
            break;
        case OP_REF_GLOBAL:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_REF_GLOBAL\t" << opCodeList[++i] << std::endl;
            break;
        case OP_REF_LOCAL:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_REF_LOCAL\t" << opCodeList[++i] << std::endl;
            break;
        case OP_REF_INDEX_GLOBAL:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_REF_INDEX_GLOBAL\t" << opCodeList[++i] << std::endl;
            break;
        case OP_REF_INDEX_LOCAL:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_REF_INDEX_LOCAL\t" << opCodeList[++i] << std::endl;
            break;
        case OP_DLL_IMPORT:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_DLL_IMPORT" << std::endl;
            break;
        default:
            break;
        }
    }

    return cout.str();
}