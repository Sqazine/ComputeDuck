#include "Chunk.h"
#include "Object.h"

Chunk::Chunk(OpCodes opCodes, const std::vector<Value> &constants)
    : opCodes(opCodes), constants(constants)
{
}

void Chunk::Stringify()
{
    for (int32_t i = 0; i < constants.size(); ++i)
    {
        auto constant = constants[i];
        if (IS_FUNCTION_VALUE(constant))
        {
            std::cout << TO_FUNCTION_VALUE(constant)->Stringify() << ":" << std::endl;
            OpCodeStringify(TO_FUNCTION_VALUE(constant)->opCodes);
            std::cout << std::endl;
        }
    }

    OpCodeStringify(opCodes);
}

void Chunk::OpCodeStringify(const OpCodes &opcodes)
{
    for (int32_t i = 0; i < opcodes.size(); ++i)
    {

        switch (opcodes[i])
        {
        case OP_CONSTANT:
        {
            auto pos = opcodes[i + 1];
            std::cout << std::setfill('0') << std::setw(8) << i << "\tOP_CONSTANT\t" << pos << "\t'" << constants[pos].Stringify() << "'" << std::endl;
            ++i;
            break;
        }
        case OP_ADD:
            std::cout << std::setfill('0') << std::setw(8) << i << "\tOP_ADD" << std::endl;
            break;
        case OP_SUB:
            std::cout << std::setfill('0') << std::setw(8) << i << "\tOP_SUB" << std::endl;
            break;
        case OP_MUL:
            std::cout << std::setfill('0') << std::setw(8) << i << "\tOP_MUL" << std::endl;
            break;
        case OP_DIV:
            std::cout << std::setfill('0') << std::setw(8) << i << "\tOP_DIV" << std::endl;
            break;
        case OP_LESS:
            std::cout << std::setfill('0') << std::setw(8) << i << "\tOP_LESS" << std::endl;
            break;
        case OP_GREATER:
            std::cout << std::setfill('0') << std::setw(8) << i << "\tOP_GREATER" << std::endl;
            break;
        case OP_NOT:
            std::cout << std::setfill('0') << std::setw(8) << i << "\tOP_NOT" << std::endl;
            break;
        case OP_MINUS:
            std::cout << std::setfill('0') << std::setw(8) << i << "\tOP_MINUS" << std::endl;
            break;
        case OP_EQUAL:
            std::cout << std::setfill('0') << std::setw(8) << i << "\tOP_EQUAL" << std::endl;
            break;
        case OP_ARRAY:
        {
            auto count = opcodes[i + 1];
            std::cout << std::setfill('0') << std::setw(8) << i << "\tOP_ARRAY\t" << count << std::endl;
            ++i;
            break;
        }
        case OP_AND:
        {
            std::cout << std::setfill('0') << std::setw(8) << i << "\tOP_AND" << std::endl;
            break;
        }
        case OP_OR:
        {
            std::cout << std::setfill('0') << std::setw(8) << i << "\tOP_OR" << std::endl;
            break;
        }
        case OP_BIT_AND:
        {
            std::cout << std::setfill('0') << std::setw(8) << i << "\tOP_BIT_AND" << std::endl;
            break;
        }
        case OP_BIT_OR:
        {
            std::cout << std::setfill('0') << std::setw(8) << i << "\tOP_BIT_OR" << std::endl;
            break;
        }
        case OP_BIT_NOT:
        {
            std::cout << std::setfill('0') << std::setw(8) << i << "\tOP_BIT_NOT" << std::endl;
            break;
        }
        case OP_BIT_XOR:
        {
            std::cout << std::setfill('0') << std::setw(8) << i << "\tOP_BIT_XOR" << std::endl;
            break;
        }
        case OP_INDEX:
            std::cout << std::setfill('0') << std::setw(8) << i << "\tOP_INDEX" << std::endl;
            break;
        case OP_JUMP:
        {
            auto address = opcodes[i + 1];
            std::cout << std::setfill('0') << std::setw(8) << i << "\tOP_JUMP\t" << address << std::endl;
            ++i;
            break;
        }
        case OP_JUMP_IF_FALSE:
        {
            auto address = opcodes[i + 1];
            std::cout << std::setfill('0') << std::setw(8) << i << "\tOP_JUMP_IF_FALSE\t" << address << std::endl;
            ++i;
            break;
        }
        case OP_RETURN:
        {
            auto count = opcodes[i + 1];
            std::cout << std::setfill('0') << std::setw(8) << i << "\tOP_RETURN\t" << count << std::endl;
            ++i;
            break;
        }
        case OP_SET_GLOBAL:
        {
            auto pos = opcodes[i + 1];
            std::cout << std::setfill('0') << std::setw(8) << i << "\tOP_SET_GLOBAL\t" << pos << std::endl;
            ++i;
            break;
        }
        case OP_GET_GLOBAL:
        {
            auto pos = opcodes[i + 1];
            std::cout << std::setfill('0') << std::setw(8) << i << "\tOP_GET_GLOBAL\t" << pos << std::endl;
            ++i;
            break;
        }
        case OP_SET_LOCAL:
        {
            auto scopeDepth = opcodes[i + 1];
            auto index = opcodes[i + 2];
            std::cout << std::setfill('0') << std::setw(8) << i << "\tOP_SET_LOCAL\t" << scopeDepth << "\t" << index << std::endl;
            i += 2;
            break;
        }
        case OP_GET_LOCAL:
        {
            auto scopeDepth = opcodes[i + 1];
            auto index = opcodes[i + 2];
            std::cout << std::setfill('0') << std::setw(8) << i << "\tOP_GET_LOCAL\t" << scopeDepth << "\t" << index << std::endl;
            i += 2;
            break;
        }
        case OP_FUNCTION_CALL:
        {
            auto argCount = opcodes[i + 1];
            std::cout << std::setfill('0') << std::setw(8) << i << "\tOP_FUNCTION_CALL\t" << argCount << std::endl;
            ++i;
            break;
        }
        case OP_GET_BUILTIN:
        {
            std::cout << std::setfill('0') << std::setw(8) << i << "\tOP_GET_BUILTIN" << std::endl;
            break;
        }
        case OP_STRUCT:
        {
            auto memberCount = opcodes[i + 1];
            std::cout << std::setfill('0') << std::setw(8) << i << "\tOP_STRUCT\t" << memberCount << std::endl;
            ++i;
            break;
        }
        case OP_GET_STRUCT:
        {
            std::cout << std::setfill('0') << std::setw(8) << i << "\tOP_GET_STRUCT" << std::endl;
            break;
        }
        case OP_SET_STRUCT:
        {
            std::cout << std::setfill('0') << std::setw(8) << i << "\tOP_SET_STRUCT" << std::endl;
            break;
        }
        case OP_REF_GLOBAL:
        {
            auto idx = opcodes[i + 1];
            std::cout << std::setfill('0') << std::setw(8) << i << "\tOP_REF_GLOBAL\t" << idx << std::endl;
            ++i;
            break;
        }
        case OP_REF_LOCAL:
        {
            auto scopeDepth = opcodes[i + 1];
            auto index = opcodes[i + 2];
            std::cout << std::setfill('0') << std::setw(8) << i << "\tOP_REF_LOCAL\t" << scopeDepth << "\t" << index << std::endl;
            i += 2;
            break;
        }
        case OP_REF_INDEX_GLOBAL:
        {
            auto pos = opcodes[i + 1];
            std::cout << std::setfill('0') << std::setw(8) << i << "\tOP_REF_INDEX_GLOBAL\t" << pos << std::endl;
            ++i;
            break;
        }
        case OP_REF_INDEX_LOCAL:
        {
            auto scopeDepth = opcodes[i + 1];
            auto index = opcodes[i + 2];
            std::cout << std::setfill('0') << std::setw(8) << i << "\tOP_REF_INDEX_LOCAL\t" << scopeDepth << "\t" << index << std::endl;
            i += 2;
            break;
        }
        case OP_SP_OFFSET:
        {
            auto offset = opcodes[i + 1];
            std::cout << std::setfill('0') << std::setw(8) << i << "\tOP_SP_OFFSET\t" << offset << std::endl;
            ++i;
            break;
        }
        default:
            break;
        }
    }
}