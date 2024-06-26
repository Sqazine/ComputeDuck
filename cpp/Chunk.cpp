#include "Chunk.h"
#include "Object.h"

Chunk::Chunk(OpCodes opCodes, const std::vector<Value> &constants)
    : opCodes(opCodes), constants(constants)
{
}

std::string Chunk::Stringify()
{
    std::string result=OpCodeStringify(opCodes);
    for (const auto& c : constants)
        if (IS_FUNCTION_VALUE(c))
            result += ::Stringify(TO_FUNCTION_VALUE(c)
#ifndef NDEBUG
                ,true
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
        {
            auto idx = opcodes[i + 1];
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_CONSTANT\t" << idx << "\t'" << constants[idx].Stringify() << "'" << std::endl;
            ++i;
            break;
        }
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
        {
            auto count = opcodes[i + 1];
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_ARRAY\t" << count << std::endl;
            ++i;
            break;
        }
        case OP_AND:
        {
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_AND" << std::endl;
            break;
        }
        case OP_OR:
        {
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_OR" << std::endl;
            break;
        }
        case OP_BIT_AND:
        {
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_BIT_AND" << std::endl;
            break;
        }
        case OP_BIT_OR:
        {
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_BIT_OR" << std::endl;
            break;
        }
        case OP_BIT_NOT:
        {
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_BIT_NOT" << std::endl;
            break;
        }
        case OP_BIT_XOR:
        {
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_BIT_XOR" << std::endl;
            break;
        }
        case OP_INDEX:
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_INDEX" << std::endl;
            break;
        case OP_JUMP:
        {
            auto address = opcodes[i + 1];
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_JUMP\t" << address << std::endl;
            ++i;
            break;
        }
        case OP_JUMP_IF_FALSE:
        {
            auto address = opcodes[i + 1];
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_JUMP_IF_FALSE\t" << address << std::endl;
            ++i;
            break;
        }
        case OP_RETURN:
        {
            auto count = opcodes[i + 1];
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_RETURN\t" << count << std::endl;
            ++i;
            break;
        }
        case OP_SET_GLOBAL:
        {
            auto idx = opcodes[i + 1];
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_SET_GLOBAL\t" << idx << std::endl;
            ++i;
            break;
        }
        case OP_GET_GLOBAL:
        {
            auto idx = opcodes[i + 1];
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_GET_GLOBAL\t" << idx << std::endl;
            ++i;
            break;
        }
        case OP_SET_LOCAL:
        {
            auto scopeDepth = opcodes[i + 1];
            auto index = opcodes[i + 2];
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_SET_LOCAL\t" << scopeDepth << "\t" << index << std::endl;
            i += 2;
            break;
        }
        case OP_GET_LOCAL:
        {
            auto scopeDepth = opcodes[i + 1];
            auto index = opcodes[i + 2];
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_GET_LOCAL\t" << scopeDepth << "\t" << index << std::endl;
            i += 2;
            break;
        }
        case OP_FUNCTION_CALL:
        {
            auto argCount = opcodes[i + 1];
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_FUNCTION_CALL\t" << argCount << std::endl;
            ++i;
            break;
        }
        case OP_GET_BUILTIN:
        {
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_GET_BUILTIN" << std::endl;
            break;
        }
        case OP_STRUCT:
        {
            auto memberCount = opcodes[i + 1];
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_STRUCT\t" << memberCount << std::endl;
            ++i;
            break;
        }
        case OP_GET_STRUCT:
        {
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_GET_STRUCT" << std::endl;
            break;
        }
        case OP_SET_STRUCT:
        {
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_SET_STRUCT" << std::endl;
            break;
        }
        case OP_REF_GLOBAL:
        {
            auto idx = opcodes[i + 1];
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_REF_GLOBAL\t" << idx << std::endl;
            ++i;
            break;
        }
        case OP_REF_LOCAL:
        {
            auto scopeDepth = opcodes[i + 1];
            auto index = opcodes[i + 2];
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_REF_LOCAL\t" << scopeDepth << "\t" << index << std::endl;
            i += 2;
            break;
        }
        case OP_REF_INDEX_GLOBAL:
        {
            auto idx = opcodes[i + 1];
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_REF_INDEX_GLOBAL\t" << idx << std::endl;
            ++i;
            break;
        }
        case OP_REF_INDEX_LOCAL:
        {
            auto scopeDepth = opcodes[i + 1];
            auto index = opcodes[i + 2];
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_REF_INDEX_LOCAL\t" << scopeDepth << "\t" << index << std::endl;
            i += 2;
            break;
        }
        case OP_SP_OFFSET:
        {
            auto offset = opcodes[i + 1];
            cout << std::setfill('0') << std::setw(8) << i << "\tOP_SP_OFFSET\t" << offset << std::endl;
            ++i;
            break;
        }
        case OP_DLL_IMPORT:
        {
            cout << std::setfill('0') << std::setw(8) << i << "\OP_DLL_IMPORT" << std::endl;
            break;
        }
        default:
            break;
        }
    }

    return cout.str();
}