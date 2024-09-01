#include "VM.h"
#include <iostream>
#include "BuiltinManager.h"
#include "Config.h"
#include "Allocator.h"

VM::~VM()
{
#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
    SAFE_DELETE(m_Jit);
#endif
    Allocator::GetInstance()->FreeAllObjects();
}

void VM::Run(FunctionObject *fn)
{
#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
    SAFE_DELETE(m_Jit);
    m_Jit = new Jit();
#endif

    Allocator::GetInstance()->ResetStack();
    Allocator::GetInstance()->ResetFrame();

    auto mainCallFrame = CallFrame(fn, STACK_TOP());
    PUSH_CALL_FRAME(mainCallFrame);

    Execute();
}

void VM::Execute()
{
    //  - * /
#define COMMON_BINARY(op)                                                                                     \
    do                                                                                                        \
    {                                                                                                         \
        auto left = FindActualValue(POP());                                                                   \
        auto right = FindActualValue(POP());                                                                  \
        if (IS_NUM_VALUE(right) && IS_NUM_VALUE(left))                                                        \
            PUSH(TO_NUM_VALUE(left) op TO_NUM_VALUE(right));                                                  \
        else                                                                                                  \
            ASSERT("Invalid binary op:%s %s %s", left.Stringify().c_str(), (#op), right.Stringify().c_str()); \
    } while (0);

// > >= < <=
#define COMPARE_BINARY(op)                                                                \
    do                                                                                    \
    {                                                                                     \
        Value left = FindActualValue(POP());                                              \
        Value right = FindActualValue(POP());                                             \
        if (IS_NUM_VALUE(right) && IS_NUM_VALUE(left))                                    \
            PUSH(TO_NUM_VALUE(left) op TO_NUM_VALUE(right) ? Value(true) : Value(false)); \
        else                                                                              \
            PUSH(false);                                                                  \
    } while (0);

// and or
#define LOGIC_BINARY(op)                                                                               \
    do                                                                                                 \
    {                                                                                                  \
        Value left = FindActualValue(POP());                                                           \
        Value right = FindActualValue(POP());                                                          \
        if (IS_BOOL_VALUE(right) && IS_BOOL_VALUE(left))                                               \
            PUSH(TO_BOOL_VALUE(left) op TO_BOOL_VALUE(right) ? Value(true) : Value(false));            \
        else                                                                                           \
            ASSERT("Invalid op:%s %s %s", left.Stringify().c_str(), (#op), right.Stringify().c_str()); \
    } while (0);

#define BIT_BINARY(op)                                                                                 \
    do                                                                                                 \
    {                                                                                                  \
        Value left = FindActualValue(POP());                                                           \
        Value right = FindActualValue(POP());                                                          \
        if (IS_NUM_VALUE(right) && IS_NUM_VALUE(left))                                                 \
            PUSH((uint64_t)TO_NUM_VALUE(left) op(uint64_t) TO_NUM_VALUE(right));                       \
        else                                                                                           \
            ASSERT("Invalid op:%s %s %s", left.Stringify().c_str(), (#op), right.Stringify().c_str()); \
    } while (0);

    while (1)
    {
        auto frame = PEEK_CALL_FRAME_FROM_BACK(1);

        if (frame->IsEnd())
            return;

        int32_t instruction = *frame->ip++;
        switch (instruction)
        {
        case OP_RETURN:
        {
            auto returnCount = *frame->ip++;

#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
            if (frame->GetFnObject()->probableReturnTypeSet == nullptr)
                frame->GetFnObject()->probableReturnTypeSet = new TypeSet();
#endif

            Value value;
            if (returnCount == 1)
            {
                value = POP();
#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
                if (IS_OBJECT_VALUE(value))
                {
                    if (IS_FUNCTION_VALUE(value))
                        frame->GetFnObject()->probableReturnTypeSet->Insert(TO_FUNCTION_VALUE(value)->probableReturnTypeSet);
                    else
                        frame->GetFnObject()->probableReturnTypeSet->Insert(TO_OBJECT_VALUE(value)->type);
                }
                else
                    frame->GetFnObject()->probableReturnTypeSet->Insert(value.type);
#endif
            }

            auto callFrame = POP_CALL_FRAME();

            if (Allocator::GetInstance()->IsCallFrameStackEmpty())
                return;

            SET_STACK_TOP(callFrame->slot - 1);

            PUSH(value);
            break;
        }
        case OP_CONSTANT:
        {
            auto idx = *frame->ip++;
            auto value = frame->GetFnObject()->chunk.constants[idx];

            PUSH(value);
            break;
        }
        case OP_ADD:
        {
            Value left = FindActualValue(POP());
            Value right = FindActualValue(POP());
            if (IS_NUM_VALUE(right) && IS_NUM_VALUE(left))
                PUSH(TO_NUM_VALUE(left) + TO_NUM_VALUE(right));
            else if (IS_STR_VALUE(right) && IS_STR_VALUE(left))
                PUSH(StrAdd(TO_STR_VALUE(left), TO_STR_VALUE(right)));
            else
                ASSERT("Invalid binary op:%s+%s", left.Stringify().c_str(), right.Stringify().c_str());
            break;
        }
        case OP_SUB:
        {
            COMMON_BINARY(-);
            break;
        }
        case OP_MUL:
        {
            COMMON_BINARY(*);
            break;
        }
        case OP_DIV:
        {
            COMMON_BINARY(/ );
            break;
        }
        case OP_GREATER:
        {
            COMPARE_BINARY(> );
            break;
        }
        case OP_LESS:
        {
            COMPARE_BINARY(< );
            break;
        }
        case OP_EQUAL:
        {
            Value left = FindActualValue(POP());
            Value right = FindActualValue(POP());
            PUSH(left == right);
            break;
        }
        case OP_NOT:
        {
            auto value = FindActualValue(POP());
            if (!IS_BOOL_VALUE(value))
                ASSERT("Invalid op:'!' %s", value.Stringify().c_str());
            PUSH(!TO_BOOL_VALUE(value));
            break;
        }
        case OP_MINUS:
        {
            auto value = FindActualValue(POP());
            if (!IS_NUM_VALUE(value))
                ASSERT("Invalid op:'-' %s", value.Stringify().c_str());
            PUSH(-TO_NUM_VALUE(value));
            break;
        }
        case OP_AND:
        {
            LOGIC_BINARY(&&);
            break;
        }
        case OP_OR:
        {
            LOGIC_BINARY(|| );
            break;
        }
        case OP_BIT_AND:
        {
            BIT_BINARY(&);
            break;
        }
        case OP_BIT_OR:
        {
            BIT_BINARY(| );
            break;
        }
        case OP_BIT_XOR:
        {
            BIT_BINARY(^);
            break;
        }
        case OP_BIT_NOT:
        {
            Value value = FindActualValue(POP());
            if (!IS_NUM_VALUE(value))
                ASSERT("Invalid op:~ %s", value.Stringify().c_str());
            PUSH(~(uint64_t)TO_NUM_VALUE(value));
            break;
        }
        case OP_ARRAY:
        {
            auto numElements = *frame->ip++;
            Value *elements = new Value[numElements];

            int32_t i = numElements - 1;
            for (Value *p = STACK_TOP() - 1; p >= STACK_TOP() - numElements && i >= 0; --p, --i)
                elements[i] = *p;

            auto array = Allocator::GetInstance()->CreateObject<ArrayObject>(elements, numElements);

            STACK_TOP_JUMP_BACK(numElements);

            PUSH(array);
            break;
        }
        case OP_GET_INDEX:
        {
            auto index = POP();
            auto ds = POP();

            if (IS_ARRAY_VALUE(ds) && IS_NUM_VALUE(index))
            {
                auto array = TO_ARRAY_VALUE(ds);
                auto i = (size_t)TO_NUM_VALUE(index);
                if (i < 0 || i >= array->len)
                    PUSH(Value());
                else
                    PUSH(array->elements[i]);
            }
            else
                ASSERT("Invalid index op: %s[%s]", ds.Stringify().c_str(), index.Stringify().c_str());
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
                    ASSERT("Invalid index:%d outside of array's size:%d", i, array->len)
                else
                    array->elements[i] = v;
            }
            else
                ASSERT("Invalid index op: %s[%s]", ds.Stringify().c_str(), index.Stringify().c_str());
            break;
        }
        case OP_JUMP_IF_FALSE:
        {
            auto address = *frame->ip++;
#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
            auto mode = *frame->ip++;
#endif
            auto value = POP();
            if (!IS_BOOL_VALUE(value))
                ASSERT("The if condition not a boolean value");
            if (!TO_BOOL_VALUE(value))
                frame->ip = frame->GetFnObject()->chunk.opCodes.data() + address;
            break;
        }
        case OP_JUMP:
        {
            auto address = *frame->ip++;
#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
            auto mode = *frame->ip++;
#endif
            frame->ip = frame->GetFnObject()->chunk.opCodes.data() + address;
            break;
        }
#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
        case OP_JUMP_START:
        {
            frame->ip++;
            break;
        }
        case OP_JUMP_END:
        {
            break;
        }
#endif
        case OP_SET_GLOBAL:
        {
            auto index = *frame->ip++;
            auto value = POP();

            auto ptr = GET_GLOBAL_VARIABLE_REF(index);

            if (IS_REF_VALUE(*ptr)) // if is a reference object,then set the actual value which the reference object points
                ptr = GetEndOfRefValue(ptr);
            *ptr = value;
            break;
        }
        case OP_GET_GLOBAL:
        {
            auto index = *frame->ip++;
            PUSH(*GET_GLOBAL_VARIABLE_REF(index));
            break;
        }
        case OP_FUNCTION_CALL:
        {
            auto argCount = (uint8_t)*frame->ip++;

            auto value = *(STACK_TOP() - argCount - 1);
            if (IS_FUNCTION_VALUE(value))
            {
                auto fn = TO_FUNCTION_VALUE(value);

                if (argCount != fn->parameterCount)
                    ASSERT("Non matching function parameters for calling arguments,parameter count:%d,argument count:%d", fn->parameterCount, argCount);

                auto callFrame = CallFrame(fn, STACK_TOP() - argCount);
                PUSH_CALL_FRAME(callFrame);
                SET_STACK_TOP(callFrame.slot + fn->localVarCount);

#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
                RunJit(callFrame);
#endif
            }
            else if (IS_BUILTIN_VALUE(value))
            {
                auto builtin = TO_BUILTIN_VALUE(value);

                if (!builtin->Is<BuiltinFn>())
                    ASSERT("Invalid builtin function");

                Value *slot = STACK_TOP() - argCount;

                STACK_TOP_JUMP_BACK(argCount + 1);

                Value returnValue;
                auto hasRet = builtin->Get<BuiltinFn>()(slot, argCount, returnValue);

                if (hasRet)
                    PUSH(returnValue);
            }
            else
                ASSERT("Calling not a function or a builtinFn");
            break;
        }
        case OP_SET_LOCAL:
        {
            auto scopeDepth = *frame->ip++;
            auto index = *frame->ip++;
            auto isUpValue = *frame->ip++;
            auto value = POP();

            Value *slot = nullptr;
            if (isUpValue)
                slot = PEEK_CALL_FRAME_FROM_FRONT(scopeDepth)->slot + index;
            else
                slot = PEEK_CALL_FRAME_FROM_BACK(scopeDepth)->slot + index;
            slot = GetEndOfRefValue(slot);

            *slot = value;
            break;
        }
        case OP_GET_LOCAL:
        {
            auto scopeDepth = *frame->ip++;
            auto index = *frame->ip++;
            auto isUpValue = *frame->ip++;

            Value *slot = nullptr;
            if (isUpValue)
                slot = PEEK_CALL_FRAME_FROM_FRONT(scopeDepth)->slot + index;
            else
                slot = PEEK_CALL_FRAME_FROM_BACK(scopeDepth)->slot + index;

            PUSH(*slot);
            break;
        }
        case OP_SP_OFFSET:
        {
            auto offset = *frame->ip++;
            STACK_TOP_JUMP(offset);
            break;
        }
        case OP_GET_BUILTIN:
        {
            auto idx = *frame->ip++;
            auto name = TO_STR_VALUE(frame->GetFnObject()->chunk.constants[idx])->value;

            auto builtinObj = BuiltinManager::GetInstance()->FindBuiltinObject(name);
            PUSH(builtinObj);
            break;
        }
        case OP_STRUCT:
        {
            std::unordered_map<std::string, Value> members;
            auto memberCount = *frame->ip++;

            auto tmpPtr = STACK_TOP(); // save the locale,to avoid gc system delete the tmp object before finish the struct instance creation

            for (int i = 0; i < memberCount; ++i)
            {
                auto name = TO_STR_VALUE(*--tmpPtr)->value;
                auto value = *--tmpPtr;
                members[name] = value;
            }

            auto structInstance = Allocator::GetInstance()->CreateObject<StructObject>(members);
            SET_STACK_TOP(tmpPtr); // recover the locale
            PUSH(structInstance);
            break;
        }
        case OP_GET_STRUCT:
        {
            auto memberName = POP();
            auto instance = GetEndOfRefValue(POP());
            if (IS_STR_VALUE(memberName))
            {
                auto structInstance = TO_STRUCT_VALUE(instance);
                auto iter = structInstance->members.find(TO_STR_VALUE(memberName)->value);
                if (iter == structInstance->members.end())
                    ASSERT("no member named:(%s) in struct instance:%s", memberName.Stringify().c_str(), instance.Stringify().c_str());
                PUSH(iter->second);
            }
            break;
        }
        case OP_SET_STRUCT:
        {
            auto memberName = POP();
            auto instance = GetEndOfRefValue(POP());
            auto structInstance = TO_STRUCT_VALUE(instance);
            auto value = POP();
            if (IS_STR_VALUE(memberName))
            {
                auto iter = structInstance->members.find(TO_STR_VALUE(memberName)->value);
                if (iter == structInstance->members.end())
                    ASSERT("no member named:(%s) in struct instance:(0x%s)", memberName.Stringify().c_str(), PointerAddressToString(structInstance).c_str());
                structInstance->members[TO_STR_VALUE(memberName)->value] = value;
            }
            break;
        }
        case OP_REF_GLOBAL:
        {
            auto index = *frame->ip++;
            PUSH(Allocator::GetInstance()->CreateObject<RefObject>(GET_GLOBAL_VARIABLE_REF(index)));
            break;
        }
        case OP_REF_LOCAL:
        {
            auto scopeDepth = *frame->ip++;
            auto index = *frame->ip++;
            auto isUpValue = *frame->ip++;
            Value *slot = nullptr;
            if (isUpValue)
                slot = PEEK_CALL_FRAME_FROM_FRONT(scopeDepth)->slot + index;
            else
                slot = PEEK_CALL_FRAME_FROM_BACK(scopeDepth)->slot + index;

            PUSH(Allocator::GetInstance()->CreateObject<RefObject>(slot));
            break;
        }
        case OP_REF_INDEX_GLOBAL:
        {
            auto index = *frame->ip++;
            auto idxValue = POP();

            auto ptr = GetEndOfRefValue(GET_GLOBAL_VARIABLE_REF(index));

            if (IS_ARRAY_VALUE(*ptr))
            {
                if (!IS_NUM_VALUE(idxValue))
                    ASSERT("Invalid idx for array,only integer is available.");
                auto intIdx = TO_NUM_VALUE(idxValue);
                if (intIdx < 0 || intIdx >= TO_ARRAY_VALUE(*ptr)->len)
                    ASSERT("Idx out of range.");
                PUSH(Allocator::GetInstance()->CreateObject<RefObject>(&(TO_ARRAY_VALUE(*ptr)->elements[(uint64_t)intIdx])));
            }
            else
                ASSERT("Invalid indexed reference type: %s not a array value.", ptr->Stringify().c_str());
            break;
        }
        case OP_REF_INDEX_LOCAL:
        {
            auto scopeDepth = *frame->ip++;
            auto index = *frame->ip++;
            auto isUpValue = *frame->ip++;

            auto idxValue = POP();

            Value *slot = nullptr;
            if (isUpValue)
                slot = PEEK_CALL_FRAME_FROM_FRONT(scopeDepth)->slot + index;
            else
                slot = PEEK_CALL_FRAME_FROM_BACK(scopeDepth)->slot + index;

            slot = GetEndOfRefValue(slot);

            if (IS_ARRAY_VALUE(*slot))
            {
                if (!IS_NUM_VALUE(idxValue))
                    ASSERT("Invalid idx for array,only integer is available.");
                auto intIdx = TO_NUM_VALUE(idxValue);
                if (intIdx < 0 || intIdx >= TO_ARRAY_VALUE(*slot)->len)
                    ASSERT("Idx out of range.");
                PUSH(Allocator::GetInstance()->CreateObject<RefObject>(&(TO_ARRAY_VALUE(*slot)->elements[(uint64_t)intIdx])));
            }
            else
                ASSERT("Invalid indexed reference type: %s not a array value.", slot->Stringify().c_str());
            break;
        }
        case OP_DLL_IMPORT:
        {
            auto name = TO_STR_VALUE(POP())->value;
            RegisterDLLs(name);
            break;
        }
        default:
            return;
        }
    }
}

Value VM::FindActualValue(const Value &v)
{
    auto value = GetEndOfRefValue(v);
    if (IS_BUILTIN_VALUE(value) && TO_BUILTIN_VALUE(value)->Is<Value>())
        value = TO_BUILTIN_VALUE(value)->Get<Value>();
    return value;
}

Value *VM::GetEndOfRefValue(Value *v)
{
    auto result = v;
    while (IS_REF_VALUE(*result))
        result = TO_REF_VALUE(*result)->pointer;
    return result;
}

Value VM::GetEndOfRefValue(const Value &v)
{
    auto value = v;
    while (IS_REF_VALUE(value))
        value = *TO_REF_VALUE(value)->pointer;
    return value;
}

#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
void VM::RunJit(const CallFrame &frame)
{
    if (!Config::GetInstance()->IsUseJit())
        return;
    if (frame.fn->callCount < JIT_TRIGGER_COUNT || !frame.fn->probableReturnTypeSet)
        return;

    size_t paramTypeHash = 0;
    for (Value *slot = frame.slot; slot < frame.slot + frame.fn->parameterCount; ++slot)
    {
        paramTypeHash ^= std::hash<ValueType>()(slot->type);
        if (IS_OBJECT_VALUE(*slot))
            paramTypeHash ^= std::hash<ObjectType>()(TO_OBJECT_VALUE(*slot)->type);
    }

    auto fnName = "function_" + frame.fn->uuid + "_" + std::to_string(paramTypeHash);

    auto iter = frame.fn->jitCache.find(paramTypeHash);
    if (iter == frame.fn->jitCache.end() && !frame.fn->probableReturnTypeSet->IsMultiplyType())
    {
        frame.fn->jitCache[paramTypeHash]=nullptr;
        
        auto llvmFn = m_Jit->Compile(frame, fnName);
        if (!llvmFn)
        {
            frame.fn->jitCache.erase(paramTypeHash);
            return;
        }
        else
            frame.fn->jitCache[paramTypeHash]= llvmFn;
    }

    auto curCallFrame = PEEK_CALL_FRAME_FROM_BACK(1);

    auto prevCallFrame = POP_CALL_FRAME();

    // Todo:Not solve function multiply argument
    if (frame.fn->probableReturnTypeSet->IsOnly(ValueType::NUM))
    {
        Value ret;
        if (frame.fn->parameterCount == 1)
        {
            auto arg0 = curCallFrame->slot + 0;
            if (IS_NUM_VALUE(*arg0))
            {
                double dArg0 = TO_NUM_VALUE(*arg0);
                ret = m_Jit->Run<double>(fnName, std::move(dArg0));
            }
        }
        else
            ret = m_Jit->Run<double>(fnName);
        SET_STACK_TOP(prevCallFrame->slot - 1);
        PUSH(ret);
    }
    else if (frame.fn->probableReturnTypeSet->IsOnly(ValueType::NIL))
    {
        m_Jit->Run<bool *>(fnName);
        SET_STACK_TOP(prevCallFrame->slot - 1);
        PUSH(Value());
    }
    else if (frame.fn->probableReturnTypeSet->IsOnly(ValueType::BOOL))
    {
        auto v = m_Jit->Run<bool>(fnName);
        SET_STACK_TOP(prevCallFrame->slot - 1);
        PUSH(v);
    }
    else if (frame.fn->probableReturnTypeSet->IsOnly(ObjectType::STR))
    {
        auto v = m_Jit->Run<Value *>(fnName);
        SET_STACK_TOP(prevCallFrame->slot - 1);
        if (v->IsValid())
            PUSH(*v);
        else
        {
            auto rawChars = reinterpret_cast<const char *>(v);
            PUSH(Allocator::GetInstance()->CreateObject<StrObject>(rawChars));
        }
    }
    else if (frame.fn->probableReturnTypeSet->IsOnly(ObjectType::ARRAY))
    {
        Value ret;
        if (frame.fn->parameterCount == 1)
        {
            auto arg0 = curCallFrame->slot + 0;
            if (IS_ARRAY_VALUE(*arg0))
            {
                auto aArg0 = TO_ARRAY_VALUE(*arg0);
                ret = m_Jit->Run<ArrayObject *>(fnName, std::move(aArg0));
            }
        }
        else
            ret = m_Jit->Run<ArrayObject *>(fnName);
        SET_STACK_TOP(prevCallFrame->slot - 1);
        PUSH(ret);
    }
    else
    {
        m_Jit->Run<void>(fnName);
        SET_STACK_TOP(prevCallFrame->slot - 1);
    }
}
#endif