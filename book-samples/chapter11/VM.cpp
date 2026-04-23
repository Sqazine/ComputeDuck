#include "VM.h"
#include "Allocator.h"
#include "Object.h"
#include "BuiltinManager.h"
void VM::Run(FunctionObject *fn)
{
    Allocator::GetInstance()->ResetStatus();
    
    auto mainCallFrame = CallFrame(fn, GET_STACK_TOP());
    PUSH_CALL_FRAME(mainCallFrame);
    SET_STACK_TOP(mainCallFrame.slot + fn->localVarCount);
    

    Execute();
}

void VM::Execute()
{
    while (1)
    {
        auto frame = PEEK_CALL_FRAME(1);

        if (frame->IsEnd())
            return;
        
        int32_t instruction = *frame->ip++;

        switch (instruction)
        {
        case OP_CONSTANT:
        {
            auto idx = *frame->ip++;
            auto value = frame->function->chunk.constants[idx];
            PUSH(value);
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
            auto numElements = *frame->ip++;
            Value *elements = new Value[numElements];

            int32_t i = numElements - 1;
            for (Value *p = GET_STACK_TOP() - 1; p >= GET_STACK_TOP() - numElements && i >= 0; --p, --i)
                elements[i] = *p;

            auto array = Allocator::GetInstance()->AllocateObject<ArrayObject>(elements, numElements);

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
            auto index = *frame->ip++;
            auto value = POP();
            auto ptr = GET_GLOBAL_VARIABLE_SLOT(index);
            *ptr = value;
            break;
        }
        case OP_SET_GLOBAL:
        {
            auto index = *frame->ip++;
            auto value = POP();
            auto ptr = GET_GLOBAL_VARIABLE_SLOT(index);
            *ptr = value;
            break;
        }
        case OP_GET_GLOBAL:
        {
            auto index = *frame->ip++;
            PUSH(*GET_GLOBAL_VARIABLE_SLOT(index));
            break;
        }
        // ++ 删除内容
        //case OP_PRINT:
        //{
        //    auto value = POP();
        //    std::cout << value.Stringify() << std::endl;
        //    break;
        //}
        // -- 删除内容
        case OP_DEF_LOCAL:
        {
            auto index = *frame->ip++;
            auto value = POP();
            Value *slot = GET_LOCAL_VARIABLE_SLOT(index);
            *slot = value;
            break;
        }
        case OP_SET_LOCAL:
        {
            auto index = *frame->ip++;
            auto value = POP();
            Value *slot = GET_LOCAL_VARIABLE_SLOT(index);
            *slot = value;
            break;
        }
        case OP_GET_LOCAL:
        {
            auto index = *frame->ip++;
            Value *slot = GET_LOCAL_VARIABLE_SLOT(index);
            PUSH(*slot);
            break;
        }
        case OP_JUMP_IF_FALSE:
        {
            auto address = *frame->ip++;
            auto value = POP();
            if (!IS_BOOL_VALUE(value))
                ASSERT("The if condition not a boolean value");
            if (!TO_BOOL_VALUE(value))
                frame->ip = frame->function->chunk.opCodeList.data() + address;
            break;
        }
        case OP_JUMP:
        {
            auto address = *frame->ip++;
            frame->ip = frame->function->chunk.opCodeList.data() + address;
            break;
        }
        case OP_RETURN:
        {
            auto returnCount = *frame->ip++;

            Value value;
            if (returnCount == 1)
            {
                value = POP();
            }

            auto callFrame = POP_CALL_FRAME();

            if (Allocator::GetInstance()->IsCallFrameStackEmpty())
                return;

            SET_STACK_TOP(callFrame->slot - 1);

            if (returnCount == 1)
                PUSH(value);
            break;
        }
        // ++ 新增内容
        case OP_GET_BUILTIN:
        {
            auto idx = *frame->ip++;
            auto name = TO_STR_VALUE(frame->function->chunk.constants[idx]);
            auto builtinObj = BuiltinManager::GetInstance()->FindBuiltinObject(name);
            PUSH(builtinObj);
            break;
        }
        // -- 新增内容
        case OP_FUNCTION_CALL:
        {
            auto argCount = (uint8_t)*frame->ip++;

            auto value = *(GET_STACK_TOP() - argCount - 1);
            if (IS_FUNCTION_VALUE(value))
            {
                auto function = TO_FUNCTION_VALUE(value);

                if (argCount != function->parameterCount)
                    ASSERT("Non matching function parameters for calling arguments,parameter count:%d,argument count:%d", function->parameterCount, argCount);

                auto callFrame = CallFrame(function, GET_STACK_TOP() - argCount);
                PUSH_CALL_FRAME(callFrame);
                SET_STACK_TOP(callFrame.slot + function->localVarCount);
            }
            // ++ 新增内容
            else if (IS_BUILTIN_VALUE(value))
            {
                auto builtin = TO_BUILTIN_VALUE(value);

                Value *slot = GET_STACK_TOP() - argCount;

                Value returnValue;
                auto hasRet = builtin->Get()(slot, argCount, returnValue);

                STACK_TOP_JUMP(-(argCount + 1));

                if (hasRet)
                    PUSH(returnValue);
            }
            // -- 新增内容
            else
                ASSERT("Calling not a function");
            break;
        }
        
        default:
            break;
        }
    }
}