#include "VM.h"
#include "Allocator.h"
#include "Object.h"
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
        if (ip - m_Chunk.opCodes.data() >= m_Chunk.opCodes.size())
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
        case OP_ARRAY:
        {
            auto numElements = *ip++;
            Value *elements = new Value[numElements];

            int32_t i = numElements - 1;
            for (Value *p = STACK_TOP() - 1; p >= STACK_TOP() - numElements && i >= 0; --p, --i)
                elements[i] = *p;

            auto array = Allocator::GetInstance()->CreateObject<ArrayObject>(elements, numElements);

            STACK_TOP_JUMP(-numElements);

            PUSH(array);
            break;
        }
        case OP_GET_INDEX:
        {
            auto index = POP();
            auto ds = POP();
            Value ret;
            GetArrayObjectElement(ds, index, ret);
            PUSH(ret);
            break;
        }
        case OP_SET_INDEX:
        {
            auto index = POP();
            auto ds = POP();
            auto v = POP();
            if (IS_ARRAY_VALUE(ds) && IS_NUM_VALUE(index))
            {
                auto array = TO_ARRAY_VALUE(ds);
                auto i = (size_t)TO_NUM_VALUE(index);
                if (i < 0 || i >= array->len)
                    ASSERT("Invalid index:%ld outside of array's size:%ld", i, array->len)
                else
                    array->elements[i] = v;
            }
            else
                ASSERT("Invalid index op: %s[%s]", ds.Stringify().c_str(), index.Stringify().c_str());
            break;
        }
        case OP_DEF_GLOBAL:
        {
            auto index = *ip++;
            auto value = POP();
            auto ptr = GET_GLOBAL_VARIABLE_REF(index);
            *ptr = value;
            break;
        }
        case OP_SET_GLOBAL:
        {
            auto index = *ip++;
            auto value = POP();
            auto ptr = GET_GLOBAL_VARIABLE_REF(index);
            *ptr = value;
            break;
        }
        case OP_GET_GLOBAL:
        {
            auto index = *ip++;
            PUSH(*GET_GLOBAL_VARIABLE_REF(index));
            break;
        }
        default:
            break;
        }
    }
}