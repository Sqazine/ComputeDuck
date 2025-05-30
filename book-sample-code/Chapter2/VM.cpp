#include "VM.h"
#include "Allocator.h"
void VM::Run(const Chunk &chunk)
{
    Allocator::GetInstance()->ResetStack();

    m_Chunk = chunk;

    Execute();
}

void VM::Execute()
{
    auto ip = m_Chunk.opCodes.data();
    while (1)
    {
        if(ip - m_Chunk.opCodes.data() >= m_Chunk.opCodes.size())
            return;

        int16_t instruction = *ip++;

        switch (instruction)
        {
        case OP_CONSTANT:
        {
            auto idx = *ip++;
            Value constant = m_Chunk.constants[idx];
            PUSH(constant);
            break;
        }
        case OP_ADD:
        {
            Value l = POP();
            Value r = POP();
            Value result;
            ValueAdd(l, r, result);
            PUSH(result);
            break;
        }
        case OP_SUB:
        {
            auto l = POP();
            auto r = POP();
            PUSH(ValueSub(l, r));
            break;
        }
        case OP_MUL:
        {
            auto l = POP();
            auto r = POP();
            PUSH(ValueMul(l, r));
            break;
        }
        case OP_DIV:
        {
            auto l = POP();
            auto r = POP();
            PUSH(ValueDiv(l, r));
            break;
        }
        case OP_GREATER:
        {
            auto l = POP();
            auto r = POP();
            PUSH(ValueGreater(l, r));
            break;
        }
        case OP_LESS:
        {
            auto l = POP();
            auto r = POP();
            PUSH(ValueLess(l, r));
            break;
        }
        case OP_EQUAL:
        {
            auto l = POP();
            auto r = POP();
            PUSH(ValueEqual(l, r));
            break;
        }
        case OP_NOT:
        {
            PUSH(ValueLogicNot(POP()));
            break;
        }
        case OP_MINUS:
        {
            PUSH(ValueMinus(POP()));
            break;
        }
        case OP_AND:
        {
            auto l = POP();
            auto r = POP();
            PUSH(ValueLogicAnd(l, r));
            break;
        }
        case OP_OR:
        {
            auto l = POP();
            auto r = POP();
            PUSH(ValueLogicOr(l, r));
            break;
        }
        case OP_BIT_AND:
        {
            auto l = POP();
            auto r = POP();
            PUSH(ValueBitAnd(l, r));
            break;
        }
        case OP_BIT_OR:
        {
            auto l = POP();
            auto r = POP();
            PUSH(ValueBitOr(l, r));
            break;
        }
        case OP_BIT_XOR:
        {
            auto l = POP();
            auto r = POP();
            PUSH(ValueBitXor(l, r));
            break;
        }
        case OP_BIT_NOT:
        {
            PUSH(ValueBitNot(POP()));
            break;
        }
        default:
            break;
        }
    }
}