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
        default:
            break;
        }
    }
}