#include "VM.h"
#include <iostream>
#include "BuiltinManager.h"
#include "Config.h"
#include "Allocator.h"

#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
#include "JitUtils.h"
#include "Jit.h"
#endif

VM::~VM()
{
#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
    SAFE_DELETE(m_Jit);
#endif
}

void VM::Run(FunctionObject *fn)
{
#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
    if (Config::GetInstance()->IsUseJit())
    {
        SAFE_DELETE(m_Jit);
        m_Jit = new Jit();
    }
#endif

    Allocator::GetInstance()->ResetStatus();

    auto closure = ALLOCATE_OBJECT(ClosureObject, fn);
    auto mainCallFrame = CallFrame(closure, GET_STACK_TOP());
    PUSH_CALL_FRAME(mainCallFrame);

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
        case OP_RETURN:
        {
            auto returnCount = *frame->ip++;
#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
            if (frame->closure->function->probableReturnTypeSet == nullptr)
                frame->closure->function->probableReturnTypeSet = new TypeSet();
#endif
            Value value;
            if (returnCount == 1)
            {
                value = POP();
#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
                if (IS_OBJECT_VALUE(value))
                {
                    if (IS_FUNCTION_VALUE(value))
                        frame->closure->function->probableReturnTypeSet->Insert(TO_FUNCTION_VALUE(value)->probableReturnTypeSet);
                    else
                        frame->closure->function->probableReturnTypeSet->Insert(TO_OBJECT_VALUE(value)->type);
                }
                else
                    frame->closure->function->probableReturnTypeSet->Insert(value.type);
#endif
            }

            Allocator::GetInstance()->ClosedUpvalues(frame->slot);

            auto callFrame = POP_CALL_FRAME();

            if (Allocator::GetInstance()->IsCallFrameStackEmpty())
                return;

            SET_STACK_TOP(callFrame->slot - 1);

            if (returnCount == 1)
                PUSH(value);
            break;
        }
        case OP_CONSTANT:
        {
            auto idx = *frame->ip++;
            auto value = frame->closure->function->chunk.constants[idx];
            PUSH(value);
            break;
        }
        case OP_ADD:
        {
            auto l = POP();
            auto r = POP();
            Value ret;
            ValueAdd(l, r, ret);
            PUSH(ret);
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

            auto array = ALLOCATE_OBJECT(ArrayObject, elements, numElements);

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
                frame->ip = frame->closure->function->chunk.opCodeList.data() + address;
            break;
        }
        case OP_JUMP:
        {
            auto address = *frame->ip++;
#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
            auto mode = *frame->ip++;
#endif
            frame->ip = frame->closure->function->chunk.opCodeList.data() + address;
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
            ptr = GetEndOfRefValuePtr(ptr);
            *ptr = value;
            break;
        }
        case OP_GET_GLOBAL:
        {
            auto index = *frame->ip++;
            PUSH(*GET_GLOBAL_VARIABLE_SLOT(index));
            break;
        }
        case OP_FUNCTION_CALL:
        {
            auto argCount = (uint8_t)*frame->ip++;

            auto value = *(GET_STACK_TOP() - argCount - 1);
            if (IS_CLOSURE_VALUE(value))
            {
                auto closure = TO_CLOSURE_VALUE(value);

                if (argCount != closure->function->parameterCount)
                    ASSERT("Non matching function parameters for calling arguments,parameter count:%d,argument count:%d", closure->function->parameterCount, argCount);

                auto callFrame = CallFrame(closure, GET_STACK_TOP() - argCount);
                PUSH_CALL_FRAME(callFrame);
                SET_STACK_TOP(callFrame.slot + closure->function->localVarCount);

#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
                RunJit(callFrame);
#endif
            }
            else if (IS_BUILTIN_VALUE(value))
            {
                auto builtin = TO_BUILTIN_VALUE(value);

                if (!builtin->Is<BuiltinFn>())
                    ASSERT("Invalid builtin function");

                Value *slot = GET_STACK_TOP() - argCount;

                Value returnValue;
                auto hasRet = builtin->Get<BuiltinFn>()(slot, argCount, returnValue);

                STACK_TOP_JUMP(-(argCount + 1));

                if (hasRet)
                    PUSH(returnValue);
            }
            else
                ASSERT("Calling not a function or a builtinFn");
            break;
        }
        case OP_CLOSURE:
        {
            auto idx = *frame->ip++;
            auto upvalueCount = *frame->ip++;
            auto function = TO_FUNCTION_VALUE(frame->closure->function->chunk.constants[idx]);
            auto closure = ALLOCATE_OBJECT(ClosureObject, function);

            for (uint8_t i = 0; i < upvalueCount; ++i)
            {
                auto index = *frame->ip++;
                auto scopeDepth = *frame->ip++;

                auto upvalue = Allocator::GetInstance()->CaptureUpvalue(GET_UPVALUE_VARIABLE_SLOT(index, scopeDepth));

                closure->upvalues[i] = upvalue;
            }

            PUSH(closure);
            break;
        }
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
            slot = GetEndOfRefValuePtr(slot);
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
        case OP_GET_UPVALUE:
        {
            auto index = *frame->ip++;
            auto upvalue = frame->closure->upvalues[index];
            PUSH(*upvalue->location);
            break;
        }
        case OP_SET_UPVALUE:
        {
            auto value = POP();
            auto index = *frame->ip++;
            auto slot = frame->closure->upvalues[index]->location;
            slot = GetEndOfRefValuePtr(slot);
            *slot = value;
            break;
        }
        case OP_GET_BUILTIN:
        {
            auto idx = *frame->ip++;
            auto name = TO_STR_VALUE(frame->closure->function->chunk.constants[idx])->value;
            auto builtinObj = BuiltinManager::GetInstance()->FindBuiltinObject(name);
            PUSH(builtinObj);
            break;
        }
        case OP_STRUCT:
        {
            Table *members = new Table();
            auto memberCount = 2 * (*frame->ip++);
            for (auto slot = GET_STACK_TOP(); slot > GET_STACK_TOP() - memberCount;)
            {
                auto name = TO_STR_VALUE(*--slot);
                auto value = *--slot;
                members->Set(name, value);
            }
            auto structInstance = ALLOCATE_OBJECT(StructObject, members);

            STACK_TOP_JUMP(-memberCount);

            PUSH(structInstance);
            break;
        }
        case OP_GET_STRUCT:
        {
            auto memberName = POP();
            Value instance;
            GetEndOfRefValue(POP(), instance);
            if (IS_STR_VALUE(memberName))
            {
                auto structInstance = TO_STRUCT_VALUE(instance);

                Value value;
                bool isSuccess = structInstance->members->Get(TO_STR_VALUE(memberName), value);
                if (!isSuccess)
                    ASSERT("no member named:(%s) in struct instance:%s", memberName.Stringify().c_str(), instance.Stringify().c_str());
                PUSH(value);
            }
            break;
        }
        case OP_SET_STRUCT:
        {
            auto memberName = POP();
            Value instance;
            GetEndOfRefValue(POP(), instance);
            auto structInstance = TO_STRUCT_VALUE(instance);
            auto value = POP();
            if (IS_STR_VALUE(memberName))
            {
                bool isSuccess = structInstance->members->Find(TO_STR_VALUE(memberName));
                if (!isSuccess)
                    ASSERT("no member named:(%s) in struct instance:(0x%s)", memberName.Stringify().c_str(), PointerAddressToString(structInstance).c_str());
                structInstance->members->Set(TO_STR_VALUE(memberName), value);
            }
            break;
        }
        case OP_REF_GLOBAL:
        {
            auto index = *frame->ip++;
            PUSH(ALLOCATE_OBJECT(RefObject, GET_GLOBAL_VARIABLE_SLOT(index)));
            break;
        }
        case OP_REF_LOCAL:
        {
            auto index = *frame->ip++;
            Value *slot = GET_LOCAL_VARIABLE_SLOT(index);
            PUSH(ALLOCATE_OBJECT(RefObject, slot));
            break;
        }
        case OP_REF_INDEX_GLOBAL:
        {
            auto index = *frame->ip++;
            auto idxValue = POP();
            auto ptr = GetEndOfRefValuePtr(GET_GLOBAL_VARIABLE_SLOT(index));
            PUSH(ALLOCATE_INDEX_REF_OBJECT(ptr, idxValue));
            break;
        }
        case OP_REF_INDEX_LOCAL:
        {
            auto index = *frame->ip++;

            auto idxValue = POP();
            Value *slot = GetEndOfRefValuePtr(GET_LOCAL_VARIABLE_SLOT(index));
            PUSH(ALLOCATE_INDEX_REF_OBJECT(slot, idxValue));
            break;
        }
        case OP_DLL_IMPORT:
        {
            auto name = TO_STR_VALUE(POP())->value;
            Allocator::GetInstance()->DisableGC();
            RegisterDLLs(name);
            Allocator::GetInstance()->EnableGC();
            break;
        }
        default:
            return;
        }
    }
}

#ifdef COMPUTEDUCK_BUILD_WITH_LLVM
void VM::RunJit(const CallFrame &frame)
{
    if (!Config::GetInstance()->IsUseJit() ||
        frame.closure->function->callCount < JIT_TRIGGER_COUNT ||
        !frame.closure->function->probableReturnTypeSet ||
        frame.closure->function->parameterCount > JIT_FUNCTION_MAX_PARAMETER_COUNT)
        return;

    // get function name by hashing arguments
    size_t paramTypeHash = HashValueList(frame.slot, frame.slot + frame.closure->function->parameterCount);
    auto fnName = GenerateFunctionName(frame.closure->function->uuid, frame.closure->function->probableReturnTypeSet->Hash(), paramTypeHash);

    // compile jit function
    {
        auto iter = frame.closure->function->jitCache.find(paramTypeHash);
        if ((iter == frame.closure->function->jitCache.end() && !frame.closure->function->probableReturnTypeSet->IsMultiplyType()) ||
            iter->second.state == JitCompileState::DEPEND)
        {
            m_Jit->ResetStatus();

            frame.closure->function->jitCache[paramTypeHash] = JitFnDecl();

            frame.closure->function->jitCache[paramTypeHash] = m_Jit->Compile(frame, fnName);
            iter = frame.closure->function->jitCache.find(paramTypeHash);
        }

        if (iter->second.state != JitCompileState::SUCCESS) //means current paramTypeHash compile error,not try compiling anymore
            return;
    }

    //execute jit function
    {
        //TODO:maybe in jit compiler need to stop GC,but NOT test yet!
        // here we enable GC as default

        // Allocator::GetInstance()->DisableGC();

        if (frame.closure->function->probableReturnTypeSet->IsOnly(ValueType::NUM))
            ExecuteJitFunction<double>(frame, fnName);
        else if (frame.closure->function->probableReturnTypeSet->IsOnly(ValueType::BOOL))
            ExecuteJitFunction<bool>(frame, fnName);
        else if (frame.closure->function->probableReturnTypeSet->IsOnly(ObjectType::STR))
            ExecuteJitFunction<StrObject *>(frame, fnName);
        else if (frame.closure->function->probableReturnTypeSet->IsOnly(ObjectType::ARRAY))
            ExecuteJitFunction<ArrayObject *>(frame, fnName);
        else if (frame.closure->function->probableReturnTypeSet->IsOnly(ObjectType::REF))
            ExecuteJitFunction<RefObject *>(frame, fnName);
        else if (frame.closure->function->probableReturnTypeSet->IsOnly(ObjectType::STRUCT))
            ExecuteJitFunction<StructObject *>(frame, fnName);
        else if (frame.closure->function->probableReturnTypeSet->IsOnly(ValueType::NIL))
        {
            ExecuteJitFunction<void>(frame, fnName);
            PUSH(Value());
        }
        else
            ExecuteJitFunction<void>(frame, fnName);

        // Allocator::GetInstance()->EnableGC();
    }
}

#endif