#include "Chunk.h"
#include "Object.h"

Chunk::Chunk(OpCodes opCodes, Value *constants, int32_t constantCount)
    : opCodes(opCodes), constantCount(constantCount)
{
    for (int32_t i = 0; i < constantCount; ++i)
        this->constants[i] = constants[i];
}

void Chunk::Stringify()
{
    for (int32_t i = 0; i < constantCount; ++i)
    {
        auto constant = constants[i];
        if (IS_FUNCTION_VALUE(constant))
        {
            std::cout << "=======constant idx:" << i << "    " << TO_FUNCTION_VALUE(constant)->Stringify() << "=======" << std::endl;
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
            std::cout << std::setfill('0') << std::setw(8) << i << "    "
                      << "OP_CONSTANT    " << pos << "    '" << constants[pos].Stringify() << "'" << std::endl;
            ++i;
            break;
        }
        case OP_ADD:
            std::cout << std::setfill('0') << std::setw(8) << i << "    "
                      << "OP_ADD" << std::endl;
            break;
        case OP_SUB:
            std::cout << std::setfill('0') << std::setw(8) << i << "    "
                      << "OP_SUB" << std::endl;
            break;
        case OP_MUL:
            std::cout << std::setfill('0') << std::setw(8) << i << "    "
                      << "OP_MUL" << std::endl;
            break;
        case OP_DIV:
            std::cout << std::setfill('0') << std::setw(8) << i << "    "
                      << "OP_DIV" << std::endl;
            break;
        case OP_LESS:
            std::cout << std::setfill('0') << std::setw(8) << i << "    "
                      << "OP_LESS" << std::endl;
            break;
        case OP_GREATER:
            std::cout << std::setfill('0') << std::setw(8) << i << "    "
                      << "OP_GREATER" << std::endl;
            break;
        case OP_NOT:
            std::cout << std::setfill('0') << std::setw(8) << i << "    "
                      << "OP_NOT" << std::endl;
            break;
        case OP_MINUS:
            std::cout << std::setfill('0') << std::setw(8) << i << "    "
                      << "OP_MINUS" << std::endl;
            break;
        case OP_EQUAL:
            std::cout << std::setfill('0') << std::setw(8) << i << "    "
                      << "OP_EQUAL" << std::endl;
            break;
        case OP_ARRAY:
        {
            auto count = opcodes[i + 1];
            std::cout << std::setfill('0') << std::setw(8) << i << "    "
                      << "OP_ARRAY    " << count << std::endl;
            ++i;
            break;
        }
        case OP_INDEX:
            std::cout << std::setfill('0') << std::setw(8) << i << "    "
                      << "OP_INDEX" << std::endl;
            break;
        case OP_JUMP:
        {
            auto address = opcodes[i + 1];
            std::cout << std::setfill('0') << std::setw(8) << i << "    "
                      << "OP_JUMP    " << address << std::endl;
            ++i;
            break;
        }
        case OP_JUMP_IF_FALSE:
        {
            auto address = opcodes[i + 1];
            std::cout << std::setfill('0') << std::setw(8) << i << "    "
                      << "OP_JUMP_IF_FALSE    " << address << std::endl;
            ++i;
            break;
        }
        case OP_RETURN:
        {
            auto count = opcodes[i + 1];
            std::cout << std::setfill('0') << std::setw(8) << i << "    "
                      << "OP_RETURN    " << count << std::endl;
            ++i;
            break;
        }
        case OP_SET_GLOBAL:
        {
            auto pos = opcodes[i + 1];
            std::cout << std::setfill('0') << std::setw(8) << i << "    "
                      << "OP_SET_GLOBAL    " << pos << "    " << std::endl;
            ++i;
            break;
        }
        case OP_GET_GLOBAL:
        {
            auto pos = opcodes[i + 1];
            std::cout << std::setfill('0') << std::setw(8) << i << "    "
                      << "OP_GET_GLOBAL    " << pos << "    " << std::endl;
            ++i;
            break;
        }
        case OP_SET_LOCAL:
        {
            auto isInUpScope=opcodes[i+1];
            auto scopeDepth = opcodes[i + 2];
            auto index = opcodes[i + 3];
            std::cout << std::setfill('0') << std::setw(8) << i << "    "
                      << "OP_SET_LOCAL    "<<isInUpScope<<"    " << scopeDepth << "    "<< index<< std::endl;
            i+=3;
            break;
        }
        case OP_GET_LOCAL:
        {
            auto isInUpScope=opcodes[i+1];
            auto scopeDepth = opcodes[i + 2];
            auto index = opcodes[i + 3];
            std::cout << std::setfill('0') << std::setw(8) << i << "    "
                      << "OP_GET_LOCAL    " <<isInUpScope<<"    "<< scopeDepth << "    "<<index << std::endl;
             i+=3;
            break;
        }
        case OP_FUNCTION_CALL:
        {
            auto argCount = opcodes[i + 1];
            std::cout << std::setfill('0') << std::setw(8) << i << "    "
                      << "OP_FUNCTION_CALL    " << argCount << std::endl;
            ++i;
            break;
        }
        case OP_GET_BUILTIN_FUNCTION:
        {
            auto builtinIdx = opcodes[i + 1];
            std::cout << std::setfill('0') << std::setw(8) << i << "    "
                      << "OP_GET_BUILTIN_FUNCTION    " << builtinIdx << std::endl;
            ++i;
            break;
        }
        case OP_STRUCT:
        {
            auto memberCount = opcodes[i + 1];
            std::cout << std::setfill('0') << std::setw(8) << i << "    "
                      << "OP_STRUCT    " << memberCount << std::endl;
            ++i;
            break;
        }
        case OP_GET_STRUCT:
        {
            std::cout << std::setfill('0') << std::setw(8) << i << "    "
                      << "OP_GET_STRUCT" << std::endl;
            break;
        }
        case OP_SET_STRUCT:
        {
            std::cout << std::setfill('0') << std::setw(8) << i << "    "
                      << "OP_SET_STRUCT" << std::endl;
            break;
        }
        case OP_REF_GLOBAL:
        {
            auto idx = opcodes[i + 1];
            std::cout << std::setfill('0') << std::setw(8) << i << "    "
                      << "OP_REF_GLOBAL    " << idx << std::endl;
            ++i;
            break;
        }
        case OP_REF_LOCAL:
        {
            auto isInUpScope=opcodes[i+1];
            auto scopeDepth = opcodes[i + 2];
            auto index = opcodes[i + 3];
            std::cout << std::setfill('0') << std::setw(8) << i << "    "
                      << "OP_REF_LOCAL    "<<isInUpScope<<"    " << scopeDepth<<"    "<<index << std::endl;
            i+=3;
            break;
        }
        case OP_SP_OFFSET:
        {
            auto offset = opcodes[i + 1];
            std::cout << std::setfill('0') << std::setw(8) << i << "    "
                      << "OP_SP_OFFSET    " << offset << std::endl;
            ++i;
            break;
        }
        default:
            break;
        }
    }
}