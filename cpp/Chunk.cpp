#include "Chunk.h"
#include "Object.h"

Chunk::Chunk(OpCodes opCodes, const std::vector<Value> &constants)
    : opCodes(opCodes), constants(constants)
{
}

std::string Chunk::Stringify()
{
    std::string result = OpCodeStringify(opCodes);
    for (const auto& c : constants)
        if (IS_FUNCTION_VALUE(c))
            result += c.Stringify(
#ifndef NDEBUG
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
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_CONSTANT\t'" << constants[opcodes[++i]].Stringify() << "'" << std::endl;
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
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_ARRAY\t" << opcodes[++i] << std::endl;
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
#ifdef BUILD_WITH_LLVM
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_JUMP\t" << opcodes[++i]<< "\t" << opCodes[++i] << std::endl;
#else
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_JUMP\t" << opcodes[++i]<< std::endl;
#endif
            break;
        case OP_JUMP_IF_FALSE:
#ifdef BUILD_WITH_LLVM
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_JUMP_IF_FALSE\t" << opcodes[++i]<< "\t" <<opCodes[++i] << std::endl;
#else
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_JUMP_IF_FALSE\t" << opcodes[++i] << std::endl;
#endif
            break;
#ifdef BUILD_WITH_LLVM
        case OP_JUMP_START:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_JUMP_START\t"<< opcodes[++i] << std::endl;
            break;
        case OP_JUMP_END:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_JUMP_END" << std::endl;
            break;
#endif
        case OP_RETURN:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_RETURN\t" << opcodes[++i] << std::endl;
            break;
        case OP_SET_GLOBAL:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_SET_GLOBAL\t" << opcodes[++i] << std::endl;
            break;
        case OP_GET_GLOBAL:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_GET_GLOBAL\t" << opcodes[++i] << std::endl;
            break;
        case OP_SET_LOCAL:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_SET_LOCAL\t" << opcodes[++i] << "\t" << opcodes[++i] << "\t" << opcodes[++i] << std::endl;
            break;
        case OP_GET_LOCAL:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_GET_LOCAL\t" << opcodes[++i] << "\t" << opcodes[++i] << "\t" << opcodes[++i] << std::endl;
            break;
        case OP_FUNCTION_CALL:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_FUNCTION_CALL\t" << opcodes[++i] << std::endl;
            break;
        case OP_GET_BUILTIN:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_GET_BUILTIN\t'" << constants[opcodes[++i]].Stringify() << "'" << std::endl;
            break;
        case OP_STRUCT:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_STRUCT\t" << opcodes[++i] << std::endl;
            break;
        case OP_GET_STRUCT:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_GET_STRUCT" << std::endl;
            break;
        case OP_SET_STRUCT:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_SET_STRUCT" << std::endl;
            break;
        case OP_REF_GLOBAL:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_REF_GLOBAL\t" << opcodes[++i] << std::endl;
            break;
        case OP_REF_LOCAL:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_REF_LOCAL\t" << opcodes[++i] << "\t" << opcodes[++i] << "\t" << opcodes[++i] << std::endl;
            break;
        case OP_REF_INDEX_GLOBAL:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_REF_INDEX_GLOBAL\t" << opcodes[++i] << std::endl;
            break;
        case OP_REF_INDEX_LOCAL:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_REF_INDEX_LOCAL\t" << opcodes[++i] << "\t" << opcodes[++i] << "\t" << opcodes[++i] << std::endl;
            break;
        case OP_SP_OFFSET:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_SP_OFFSET\t" << opcodes[++i] << std::endl;
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