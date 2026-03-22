using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Threading;

namespace ComputeDuck
{
    public class VM
    {
        public class CallFrame
        {
            public ClosureObject closure;
            public int ip;
            public int slot;

            public CallFrame()
            {
                this.closure = null;
                this.ip = -1;
                this.slot = -1;
            }

            public CallFrame(ClosureObject closure, int slot)
            {
                this.closure = closure;
                this.ip = 0;
                this.slot = slot;
            }

            public bool IsAtEnd()
            {
                if (ip < closure.function.chunk.opCodeList.Count)
                    return false;
                return true;
            }
        }

        private Object[] m_GlobalVariables = new Object[Utils.STACK_COUNT];

        private int m_StackTop;
        private Object[] m_ObjectStack = new Object[Utils.STACK_COUNT];

        private int m_CallFrameTop;
        private CallFrame[] m_CallFrames = new CallFrame[Utils.STACK_COUNT];

        private UpvalueObject m_OpenUpvalues = null;

        private void ResetStatus()
        {
            m_StackTop = 0;
            for (int i = 0; i < Utils.STACK_COUNT; ++i)
                m_ObjectStack[i] = new NilObject();

            for (int i = 0; i < Utils.STACK_COUNT; ++i)
                m_GlobalVariables[i] = new NilObject();

            for (int i = 0; i < Utils.STACK_COUNT; ++i)
                m_CallFrames[i] = new CallFrame();
        }

        public void Run(FunctionObject mainFn)
        {
            ResetStatus();

            var closure = new ClosureObject(mainFn);
            var mainCallFrame = new CallFrame(closure, m_StackTop);
            PushCallFrame(mainCallFrame);
            m_StackTop = mainCallFrame.slot + mainFn.localVarCount;

            Execute();
        }

        private void Execute()
        {
            while (true)
            {
                CallFrame frame = PeekCallFrame(1);

                if (frame.IsAtEnd())
                    return;

                int instruction = frame.closure.function.chunk.opCodeList[frame.ip++];
                switch (instruction)
                {
                    case (int)OpCode.OP_RETURN:
                        {
                            var returnCounnt = frame.closure.function.chunk.opCodeList[frame.ip++];
                            Object obj = null;
                            if (returnCounnt == 1)
                                obj = Pop();

                            ClosedUpvalues(m_ObjectStack[frame.slot].GetAddress());

                            var callFrame = PopCallFrame();

                            if (m_CallFrameTop == 0)
                                return;

                            m_StackTop = callFrame.slot - 1;

                            if (returnCounnt == 1)
                                Push(obj);
                            break;
                        }
                    case (int)OpCode.OP_CONSTANT:
                        {
                            var idx = frame.closure.function.chunk.opCodeList[frame.ip++];
                            var value = frame.closure.function.chunk.constants[idx];

                            Push(value);
                            break;
                        }
                    case (int)OpCode.OP_ADD:
                        {
                            var left = FindActualObject(Pop());
                            var right = FindActualObject(Pop());

                            if (left.type == ObjectType.NUM && right.type == ObjectType.NUM)
                                Push(new NumObject(((NumObject)left).value + ((NumObject)right).value));
                            else if (left.type == ObjectType.STR && right.type == ObjectType.STR)
                                Push(new StrObject(((StrObject)left).value + ((StrObject)right).value));
                            else
                                Utils.Assert("Invalid binary op:" + left.ToString() + "+" + right.ToString());

                            break;
                        }
                    case (int)OpCode.OP_SUB:
                        {
                            var left = FindActualObject(Pop());
                            var right = FindActualObject(Pop());

                            if (left.type == ObjectType.NUM && right.type == ObjectType.NUM)
                                Push(new NumObject(((NumObject)left).value - ((NumObject)right).value));
                            else
                                Utils.Assert("Invalid binary op:" + left.ToString() + "-" + right.ToString());

                            break;
                        }
                    case (int)OpCode.OP_MUL:
                        {
                            var left = FindActualObject(Pop());
                            var right = FindActualObject(Pop());

                            if (left.type == ObjectType.NUM && right.type == ObjectType.NUM)
                                Push(new NumObject(((NumObject)left).value * ((NumObject)right).value));
                            else
                                Utils.Assert("Invalid binary op:" + left.ToString() + "*" + right.ToString());

                            break;
                        }
                    case (int)OpCode.OP_DIV:
                        {
                            var left = FindActualObject(Pop());
                            var right = FindActualObject(Pop());

                            if (left.type == ObjectType.NUM && right.type == ObjectType.NUM)
                                Push(new NumObject(((NumObject)left).value / ((NumObject)right).value));
                            else
                                Utils.Assert("Invalid binary op:" + left.ToString() + "/" + right.ToString());

                            break;
                        }
                    case (int)OpCode.OP_GREATER:
                        {
                            var left = FindActualObject(Pop());
                            var right = FindActualObject(Pop());

                            if (left.type == ObjectType.NUM && right.type == ObjectType.NUM)
                                Push(new BoolObject(((NumObject)left).value > ((NumObject)right).value));
                            else
                                Push(new BoolObject(false));

                            break;
                        }
                    case (int)OpCode.OP_LESS:
                        {
                            var left = FindActualObject(Pop());
                            var right = FindActualObject(Pop());

                            if (left.type == ObjectType.NUM && right.type == ObjectType.NUM)
                                Push(new BoolObject(((NumObject)left).value < ((NumObject)right).value));
                            else
                                Push(new BoolObject(false));

                            break;
                        }

                    case (int)OpCode.OP_EQUAL:
                        {
                            var left = FindActualObject(Pop());
                            var right = FindActualObject(Pop());

                            Push(new BoolObject(left.IsEqualTo(right)));
                            break;
                        }
                    case (int)OpCode.OP_NOT:
                        {
                            var obj = FindActualObject(Pop());
                            if (obj.type != ObjectType.BOOL)
                                Utils.Assert("Not a boolean obj of the obj:" + obj.ToString());
                            Push(new BoolObject(!((BoolObject)obj).value));
                            break;
                        }
                    case (int)OpCode.OP_MINUS:
                        {
                            var obj = FindActualObject(Pop());
                            if (obj.type != ObjectType.NUM)
                                Utils.Assert("Invalid op:'-'" + obj.ToString());
                            Push(new NumObject(-((NumObject)obj).value));
                            break;
                        }
                    case (int)OpCode.OP_AND:
                        {
                            var left = FindActualObject(Pop());
                            var right = FindActualObject(Pop());

                            if (left.type == ObjectType.BOOL && right.type == ObjectType.BOOL)
                                Push(new BoolObject(((BoolObject)left).value && ((BoolObject)right).value));
                            else
                                Utils.Assert("Invalid op:" + left.ToString() + " and " + right.ToString());
                            break;
                        }
                    case (int)OpCode.OP_OR:
                        {
                            var left = FindActualObject(Pop());
                            var right = FindActualObject(Pop());

                            if (left.type == ObjectType.BOOL && right.type == ObjectType.BOOL)
                                Push(new BoolObject(((BoolObject)left).value || ((BoolObject)right).value));
                            else
                                Utils.Assert("Invalid op:" + left.ToString() + " or " + right.ToString());
                            break;
                        }
                    case (int)OpCode.OP_BIT_AND:
                        {
                            var left = FindActualObject(Pop());
                            var right = FindActualObject(Pop());

                            if (left.type == ObjectType.NUM && right.type == ObjectType.NUM)
                                Push(new NumObject((int)((NumObject)left).value & (int)((NumObject)right).value));
                            else
                                Utils.Assert("Invalid op:" + left.ToString() + " & " + right.ToString());
                            break;
                        }
                    case (int)OpCode.OP_BIT_OR:
                        {
                            var left = FindActualObject(Pop());
                            var right = FindActualObject(Pop());

                            if (left.type == ObjectType.NUM && right.type == ObjectType.NUM)
                                Push(new NumObject((int)((NumObject)left).value | (int)((NumObject)right).value));
                            else
                                Utils.Assert("Invalid op:" + left.ToString() + " | " + right.ToString());
                            break;
                        }
                    case (int)OpCode.OP_BIT_XOR:
                        {
                            var left = FindActualObject(Pop());
                            var right = FindActualObject(Pop());

                            if (left.type == ObjectType.NUM && right.type == ObjectType.NUM)
                                Push(new NumObject((int)((NumObject)left).value ^ (int)((NumObject)right).value));
                            else
                                Utils.Assert("Invalid op:" + left.ToString() + " ^ " + right.ToString());
                            break;
                        }
                    case (int)OpCode.OP_BIT_NOT:
                        {
                            var obj = FindActualObject(Pop());
                            if (obj.type != ObjectType.NUM)
                                Utils.Assert("Invalid op:'~'" + obj.ToString());
                            Push(new NumObject(~(int)((NumObject)obj).value));
                            break;
                        }
                    case (int)OpCode.OP_ARRAY:
                        {
                            var numElements = frame.closure.function.chunk.opCodeList[frame.ip++];
                            List<Object> elements = new List<Object>(m_ObjectStack[(m_StackTop - numElements)..m_StackTop]);

                            m_StackTop -= numElements;

                            var array = new ArrayObject(elements);
                            Push(array);
                            break;
                        }
                    case (int)OpCode.OP_GET_INDEX:
                        {
                            var index = Pop();
                            var ds = Pop();

                            if (ds.type == ObjectType.ARRAY && index.type == ObjectType.NUM)
                            {
                                int i = (int)((NumObject)index).value;
                                if (i < 0 || i >= ((ArrayObject)ds).elements.Count)
                                    Push(new NilObject());
                                else
                                    Push(((ArrayObject)ds).elements[i]);
                            }
                            else
                                Utils.Assert("Invalid index op:" + ds.ToString() + "[" + index.ToString() + "]");
                            break;
                        }
                    case (int)OpCode.OP_SET_INDEX:
                        {
                            var index = Pop();
                            var ds = Pop();
                            var v = Pop();

                            if (ds.type == ObjectType.ARRAY && index.type == ObjectType.NUM)
                            {
                                int i = (int)((NumObject)index).value;
                                if (i < 0 || i >= ((ArrayObject)ds).elements.Count)
                                    Utils.Assert("Invalid Index:" + i.ToString() + " outside of array's size:" + ((ArrayObject)ds).elements.Count.ToString());
                                else
                                    ((ArrayObject)ds).elements[i] = v;
                            }
                            else
                                Utils.Assert("Invalid index op:" + ds.ToString() + "[" + index.ToString() + "]");
                            break;
                        }
                    case (int)OpCode.OP_JUMP_IF_FALSE:
                        {
                            var address = frame.closure.function.chunk.opCodeList[frame.ip++];
                            var obj = Pop();
                            if (obj.type != ObjectType.BOOL)
                                Utils.Assert("The if condition not a boolean value:" + obj.ToString());
                            if (((BoolObject)obj).value != true)
                                frame.ip = address + 1;
                            break;
                        }
                    case (int)OpCode.OP_JUMP:
                        {
                            var address = frame.closure.function.chunk.opCodeList[frame.ip++];
                            frame.ip = address + 1;
                            break;
                        }
                    case (int)OpCode.OP_DEF_GLOBAL:
                        {
                            var index = frame.closure.function.chunk.opCodeList[frame.ip++];
                            var obj = Pop();
                            m_GlobalVariables[index] = obj;
                            break;
                        }
                    case (int)OpCode.OP_SET_GLOBAL:
                        {
                            var index = frame.closure.function.chunk.opCodeList[frame.ip++];
                            var obj = Pop();

                            if (m_GlobalVariables[index].type == ObjectType.REF)
                            {
                                var (refObj, address) = GetEndOfRefObject(m_GlobalVariables[index]);

                                AssignObjectByAddress(address, obj);

                                var refObjs = SearchRefObjectListByAddress(address);
                                for (int i = 0; i < refObjs.Count; ++i)
                                    ((RefObject)refObjs[i]).pointer = obj.GetAddress();
                            }
                            else
                            {
                                UpdateRefAddress(m_GlobalVariables[index].GetAddress(), obj.GetAddress());
                                m_GlobalVariables[index] = obj;
                            }
                            break;
                        }
                    case (int)OpCode.OP_GET_GLOBAL:
                        {
                            var index = frame.closure.function.chunk.opCodeList[frame.ip++];
                            Push(m_GlobalVariables[index]);
                            break;
                        }
                    case (int)OpCode.OP_CLOSURE:
                        {
                            var idx = frame.closure.function.chunk.opCodeList[frame.ip++];
                            var upvalueCount = frame.closure.function.chunk.opCodeList[frame.ip++];
                            var fn = (FunctionObject)frame.closure.function.chunk.constants[idx];
                            var closure = new ClosureObject(fn);

                            for (int i = 0; i < upvalueCount; ++i)
                            {
                                var index = frame.closure.function.chunk.opCodeList[frame.ip++];
                                var scopeDepth = frame.closure.function.chunk.opCodeList[frame.ip++];

                                var upvalue = CaptureUpvalue(m_ObjectStack[GetUpvalueVariableSlot(index, scopeDepth)].GetAddress());
                                closure.upvalues[i] = upvalue;
                            }

                            Push(closure);
                            break;
                        }
                    case (int)OpCode.OP_FUNCTION_CALL:
                        {
                            var argCount = frame.closure.function.chunk.opCodeList[frame.ip++];
                            var obj = m_ObjectStack[m_StackTop - 1 - argCount];
                            if (obj.type == ObjectType.CLOSURE)
                            {
                                var closure = (ClosureObject)obj;
                                if (argCount != closure.function.parameterCount)
                                    Utils.Assert("Non matching function parameters for calling arguments,parameter count:" + closure.function.parameterCount + ",argument count:" + argCount);

                                var callFrame = new CallFrame(closure, m_StackTop - argCount);
                                PushCallFrame(callFrame);
                                m_StackTop = callFrame.slot + closure.function.localVarCount;
                            }
                            else if (obj.type == ObjectType.BUILTIN)
                            {
                                var args = new List<Object>(m_ObjectStack[(m_StackTop - argCount)..m_StackTop]);

                                m_StackTop -= (argCount + 1);

                                var (hasReturnValue, returnValue) = ((BuiltinObject)obj).GetBuiltinFn()(args);

                                if (hasReturnValue)
                                    Push(returnValue);
                            }
                            else
                                Utils.Assert("Calling not a function or a builtinFn");
                            break;
                        }
                    case (int)OpCode.OP_DEF_LOCAL:
                        {
                            var index = frame.closure.function.chunk.opCodeList[frame.ip++];

                            var obj = Pop();
                            var slot = GetLocalVariableSlot(index);
                            m_ObjectStack[slot] = obj;
                            break;
                        }
                    case (int)OpCode.OP_SET_LOCAL:
                        {
                            var index = frame.closure.function.chunk.opCodeList[frame.ip++];

                            var obj = Pop();

                            int slot = GetLocalVariableSlot(index);

                            if (m_ObjectStack[slot].type == ObjectType.REF)
                            {
                                var (refObj, address) = GetEndOfRefObject(m_ObjectStack[slot]);
                                AssignObjectByAddress(address, obj);

                                var refObjs = SearchRefObjectListByAddress(address);
                                for (int i = 0; i < refObjs.Count; ++i)
                                    ((RefObject)refObjs[i]).pointer = obj.GetAddress();
                                ((RefObject)m_ObjectStack[slot]).pointer = obj.GetAddress();
                            }
                            else
                            {
                                UpdateRefAddress(m_ObjectStack[slot].GetAddress(), obj.GetAddress());
                                m_ObjectStack[slot] = obj;
                            }
                            break;
                        }
                    case (int)OpCode.OP_GET_LOCAL:
                        {
                            var index = frame.closure.function.chunk.opCodeList[frame.ip++];

                            int slot = GetLocalVariableSlot(index);
                            Push(m_ObjectStack[slot]);
                            break;
                        }
                    case (int)OpCode.OP_GET_BUILTIN:
                        {
                            var name = ((StrObject)Pop()).value;
                            var builtinObj = BuiltinManager.GetInstance().m_Builtins[name];
                            Push(builtinObj);
                            break;
                        }
                    case (int)OpCode.OP_STRUCT:
                        {
                            var members = new Dictionary<string, Object>();
                            var memberCount = frame.closure.function.chunk.opCodeList[frame.ip++];

                            var tmpPtr = m_StackTop;

                            for (int i = 0; i < memberCount; ++i)
                            {
                                var name = ((StrObject)m_ObjectStack[--tmpPtr]).value;
                                var obj = m_ObjectStack[--tmpPtr];

                                members[name] = obj;
                            }

                            var structInstance = new StructObject(members);
                            m_StackTop = tmpPtr;
                            Push(structInstance);
                            break;
                        }
                    case (int)OpCode.OP_GET_STRUCT:
                        {
                            var memberName = Pop();
                            var instance = Pop();

                            (instance, _) = GetEndOfRefObject(instance);

                            if (memberName.type == ObjectType.STR)
                            {
                                var obj = ((StructObject)instance).members.GetValueOrDefault(((StrObject)memberName).value, new NilObject());
                                if (obj.type == ObjectType.NIL)
                                    Utils.Assert("no member named:" + memberName.ToString() + ") in struct instance:" + instance.ToString());
                                Push(obj);
                            }
                            break;
                        }
                    case (int)OpCode.OP_SET_STRUCT:
                        {
                            var memberName = Pop();
                            var instance = Pop();

                            (instance, _) = GetEndOfRefObject(instance);
                            var obj = Pop();
                            if (memberName.type == ObjectType.STR)
                            {
                                var isFound = ((StructObject)instance).members.ContainsKey(((StrObject)memberName).value);
                                if (isFound == false)
                                    Utils.Assert("no member named:" + memberName.ToString() + " in struct instance:" + instance.ToString());
                                ((StructObject)instance).members[((StrObject)memberName).value] = obj;
                            }

                            break;
                        }
                    case (int)OpCode.OP_REF_GLOBAL:
                        {
                            var index = frame.closure.function.chunk.opCodeList[frame.ip++];
                            Push(new RefObject(m_GlobalVariables[index].GetAddress()));
                            break;
                        }
                    case (int)OpCode.OP_REF_LOCAL:
                        {
                            var index = frame.closure.function.chunk.opCodeList[frame.ip++];

                            int slot = GetLocalVariableSlot(index);

                            Push(new RefObject(m_ObjectStack[slot].GetAddress()));

                            break;
                        }
                    case (int)OpCode.OP_REF_INDEX_GLOBAL:
                        {
                            var index = frame.closure.function.chunk.opCodeList[frame.ip++];
                            var idxValue = Pop();

                            var (obj, _) = GetEndOfRefObject(m_GlobalVariables[index]);
                            Push(AllocateIndexRefObject(obj, idxValue));
                            break;
                        }
                    case (int)OpCode.OP_REF_INDEX_LOCAL:
                        {
                            var index = frame.closure.function.chunk.opCodeList[frame.ip++];
                            var idxValue = Pop();

                            int slot = GetLocalVariableSlot(index);
                            var (obj, _) = GetEndOfRefObject(m_ObjectStack[slot]);
                            Push(AllocateIndexRefObject(obj, idxValue));
                            break;
                        }
                    case (int)OpCode.OP_DLL_IMPORT:
                        {
                            var name = ((StrObject)Pop()).value;
                            Utils.RegisterDLLs(name);
                            break;
                        }
                    case (int)OpCode.OP_SET_UPVALUE:
                        {
                            var index = frame.closure.function.chunk.opCodeList[frame.ip++];

                            var obj = Pop();

                            frame.closure.upvalues[index].location = obj.GetAddress();
                            break;
                        }
                    case (int)OpCode.OP_GET_UPVALUE:
                        {
                            var index = frame.closure.function.chunk.opCodeList[frame.ip++];
                            var upvalue = frame.closure.upvalues[index];
                            Push(Utils.SearchObjectByAddress(upvalue.location));
                            break;
                        }
                    case (int)OpCode.OP_REF_UPVALUE:
                        {
                            var index = frame.closure.function.chunk.opCodeList[frame.ip++];
                            Push(new RefObject(frame.closure.upvalues[index].location));
                            break;
                        }
                    case (int)OpCode.OP_REF_INDEX_UPVALUE:
                        {
                            var index = frame.closure.function.chunk.opCodeList[frame.ip++];
                            var idxValue = Pop();

                            var (obj, _) = GetEndOfRefObject(Utils.SearchObjectByAddress(frame.closure.upvalues[index].location));
                            Push(AllocateIndexRefObject(obj, idxValue));
                            break;
                        }
                    default:
                        return;

                }
            }
        }

        private void Push(Object obj)
        {
            m_ObjectStack[m_StackTop++] = obj;
        }

        private Object Pop()
        {
            return m_ObjectStack[--m_StackTop];
        }

        private void PushCallFrame(CallFrame callFrame)
        {
            m_CallFrames[m_CallFrameTop++] = callFrame;
        }

        private CallFrame PopCallFrame()
        {
            return m_CallFrames[--m_CallFrameTop];
        }


        private CallFrame PeekCallFrame(int distance)
        {
            return m_CallFrames[m_CallFrameTop - distance];
        }

        private int GetLocalVariableSlot(int index)
        {
            return PeekCallFrame(1).slot + index;
        }

        private int GetUpvalueVariableSlot(int index, int scopeDepth)
        {
            return m_CallFrames[scopeDepth].slot + index;
        }

        private RefObject AllocateIndexRefObject(Object obj, Object idxValue)
        {
            if (obj.type == ObjectType.ARRAY)
            {
                if (idxValue.type != ObjectType.NUM)
                    Utils.Assert("Invalid idx for array,only integer is available.");
                if (((NumObject)idxValue).value < 0 || ((NumObject)idxValue).value >= ((ArrayObject)obj).elements.Count)
                    Utils.Assert("Idx out of range.");
                return new RefObject(((ArrayObject)obj).elements[(int)((NumObject)idxValue).value].GetAddress());
            }
            else
                Utils.Assert("Invalid indexed reference type:" + obj.ToString() + " not a array value.");
            return null;
        }

        private Object FindActualObject(Object obj)
        {
            var actual = obj;
            while (actual.type == ObjectType.REF)
                actual = Utils.SearchObjectByAddress(((RefObject)actual).pointer);

            if (actual.type == ObjectType.BUILTIN && ((BuiltinObject)actual).IsBuiltinVar())
                actual = ((BuiltinObject)actual).GetBuiltinVar();

            return actual;
        }

        private (Object, IntPtr) GetEndOfRefObject(Object obj)
        {
            Object retObj = obj;
            IntPtr address = 0;
            while (retObj.type == ObjectType.REF)
            {
                address = ((RefObject)retObj).pointer;
                retObj = Utils.SearchObjectByAddress(((RefObject)retObj).pointer);
            }
            return (retObj, address);
        }

        private void AssignObjectByAddress(IntPtr address, Object obj)
        {
            for (int i = 0; i < m_GlobalVariables.Count(); ++i)
            {
                if (m_GlobalVariables[i].type == ObjectType.ARRAY)
                {
                    for (int j = 0; j < ((ArrayObject)m_GlobalVariables[i]).elements.Count(); ++j)
                    {
                        if (((ArrayObject)m_GlobalVariables[i]).elements[j].GetAddress() == address)
                        {
                            ((ArrayObject)m_GlobalVariables[i]).elements[j] = obj;
                            return;
                        }
                    }
                }
                else if (m_GlobalVariables[i].GetAddress() == address)
                {
                    m_GlobalVariables[i] = obj;
                    return;
                }

            }

            for (int i = 0; i < m_StackTop; ++i)
            {
                if (m_ObjectStack[i].type == ObjectType.ARRAY)
                {
                    for (int j = 0; j < ((ArrayObject)m_ObjectStack[i]).elements.Count(); ++j)
                    {
                        if (((ArrayObject)m_ObjectStack[i]).elements[j].GetAddress() == address)
                        {
                            ((ArrayObject)m_ObjectStack[i]).elements[j] = obj;
                            return;
                        }
                    }
                }
                else if (m_ObjectStack[i].GetAddress() == address)
                {
                    m_ObjectStack[i] = obj;
                    return;
                }

            }

            for (int i = 0; i < m_CallFrameTop; ++i)
            {
                var callFrame = m_CallFrames[i];
                if (callFrame != null && callFrame.closure != null)
                {
                    for (int j = 0; j < callFrame.closure.upvalues.Count(); ++j)
                    {
                        if (callFrame.closure.upvalues[j] != null)
                        {
                            if (callFrame.closure.upvalues[j].location == address)
                            {
                                callFrame.closure.upvalues[j].location = obj.GetAddress();
                                return;
                            }
                            else
                            {
                                var arrayObj = Utils.SearchObjectByAddress(callFrame.closure.upvalues[j].location);
                                if (arrayObj != null && arrayObj.type == ObjectType.ARRAY)
                                {
                                    for (int k = 0; k < ((ArrayObject)arrayObj).elements.Count(); ++k)
                                    {
                                        if (((ArrayObject)arrayObj).elements[k].GetAddress() == address)
                                        {
                                            ((ArrayObject)arrayObj).elements[k] = obj;
                                            return;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            Utils.Assert("Invalid reference address:" + address.ToString() + " for assigning object:" + obj.ToString());
        }

        private List<Object> SearchRefObjectListByAddress(IntPtr address)
        {
            List<Object> result = new List<Object>();

            for (int i = 0; i < m_GlobalVariables.Count(); ++i)
            {
                if (m_GlobalVariables[i].type == ObjectType.ARRAY)
                {
                    for (int j = 0; j < ((ArrayObject)m_GlobalVariables[i]).elements.Count(); ++j)
                    {
                        if (((ArrayObject)m_GlobalVariables[i]).elements[j].type == ObjectType.REF)
                            if (((RefObject)((ArrayObject)m_GlobalVariables[i]).elements[j]).pointer == address)
                                result.Add(((ArrayObject)m_GlobalVariables[i]).elements[j]);
                    }
                }
                else if (m_GlobalVariables[i].type == ObjectType.REF)
                    if (((RefObject)m_GlobalVariables[i]).pointer == address)
                        result.Add(m_GlobalVariables[i]);
            }

            for (int i = 0; i < m_StackTop; ++i)
            {
                if (m_ObjectStack[i].type == ObjectType.ARRAY)
                {
                    for (int j = 0; j < ((ArrayObject)m_ObjectStack[i]).elements.Count(); ++j)
                    {
                        if (((ArrayObject)m_ObjectStack[i]).elements[j].type == ObjectType.REF)
                            if (((RefObject)((ArrayObject)m_ObjectStack[i]).elements[j]).pointer == address)
                                result.Add(((ArrayObject)m_ObjectStack[i]).elements[j]);
                    }
                }
                else if (m_ObjectStack[i].type == ObjectType.REF)
                    if (((RefObject)m_ObjectStack[i]).pointer == address)
                        result.Add(m_ObjectStack[i]);
            }

            for (int i = 0; i < m_CallFrameTop; ++i)
            {
                var callFrame = m_CallFrames[i];
                if (callFrame != null && callFrame.closure != null)
                {
                    for (int j = 0; j < callFrame.closure.upvalues.Count(); ++j)
                    {
                        if (callFrame.closure.upvalues[j] != null)
                        {
                            if (callFrame.closure.upvalues[j].location == address)
                                result.Add(callFrame.closure.upvalues[j]);
                            else
                            {
                                var arrayObj = Utils.SearchObjectByAddress(callFrame.closure.upvalues[j].location);
                                if (arrayObj != null && arrayObj.type == ObjectType.ARRAY)
                                {
                                    for (int k = 0; k < ((ArrayObject)arrayObj).elements.Count(); ++k)
                                    {
                                        if (((ArrayObject)arrayObj).elements[k].type == ObjectType.REF)
                                            if (((RefObject)((ArrayObject)m_ObjectStack[i]).elements[j]).pointer == address)
                                                result.Add(((ArrayObject)arrayObj).elements[k]);
                                    }
                                }
                            }
                        }
                    }
                }
            }

            return result;
        }

        private void UpdateRefAddress(IntPtr originAddress, IntPtr newAddress)
        {
            for (int i = 0; i < m_GlobalVariables.Count(); ++i)
            {
                if (m_GlobalVariables[i].type == ObjectType.ARRAY)
                {
                    for (int j = 0; j < ((ArrayObject)m_GlobalVariables[i]).elements.Count(); ++j)
                    {
                        if (((ArrayObject)m_GlobalVariables[i]).elements[j].type == ObjectType.REF)
                            if (((RefObject)((ArrayObject)m_GlobalVariables[i]).elements[j]).pointer == originAddress)
                                ((RefObject)((ArrayObject)m_GlobalVariables[i]).elements[j]).pointer = newAddress;
                    }
                }
                else if (m_GlobalVariables[i].type == ObjectType.REF)
                    if (((RefObject)m_GlobalVariables[i]).pointer == originAddress)
                        ((RefObject)m_GlobalVariables[i]).pointer = newAddress;
            }


            for (int i = 0; i < m_StackTop; ++i)
            {
                if (m_ObjectStack[i].type == ObjectType.ARRAY)
                {
                    for (int j = 0; j < ((ArrayObject)m_ObjectStack[i]).elements.Count(); ++j)
                    {
                        if (((ArrayObject)m_ObjectStack[i]).elements[j].type == ObjectType.REF)
                            if (((RefObject)((ArrayObject)m_ObjectStack[i]).elements[j]).pointer == originAddress)
                                ((RefObject)((ArrayObject)m_ObjectStack[i]).elements[j]).pointer = newAddress;
                    }
                }
                else if (m_ObjectStack[i].type == ObjectType.REF)
                    if (((RefObject)m_ObjectStack[i]).pointer == originAddress)
                        ((RefObject)m_ObjectStack[i]).pointer = newAddress;
            }

            for (int i = 0; i < m_CallFrameTop; ++i)
            {
                var callFrame = m_CallFrames[i];
                if (callFrame != null && callFrame.closure != null)
                {
                    for (int j = 0; j < callFrame.closure.upvalues.Count(); ++j)
                    {
                        if (callFrame.closure.upvalues[j] != null)
                        {
                            if (callFrame.closure.upvalues[j].location == originAddress)
                                callFrame.closure.upvalues[j].location = newAddress;
                            else
                            {
                                var arrayObj = Utils.SearchObjectByAddress(callFrame.closure.upvalues[j].location);
                                if (arrayObj != null && arrayObj.type == ObjectType.ARRAY)
                                {
                                    for (int k = 0; k < ((ArrayObject)arrayObj).elements.Count(); ++k)
                                    {
                                        if (((ArrayObject)arrayObj).elements[k].type == ObjectType.REF)
                                            if (((RefObject)((ArrayObject)m_ObjectStack[i]).elements[j]).pointer == originAddress)
                                                ((RefObject)((ArrayObject)arrayObj).elements[k]).pointer = newAddress;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }


        UpvalueObject CaptureUpvalue(IntPtr slot)
        {
            UpvalueObject prevUpvalue = null;
            UpvalueObject upvalue = m_OpenUpvalues;
            while (upvalue != null && upvalue.location > slot)
            {
                prevUpvalue = upvalue;
                upvalue = upvalue.nextUpvalue;
            }

            if (upvalue != null && upvalue.location == slot)
                return upvalue;

            var createdUpvalue = new UpvalueObject(slot);
            createdUpvalue.nextUpvalue = upvalue;

            if (prevUpvalue == null)
                m_OpenUpvalues = createdUpvalue;
            else
                prevUpvalue.nextUpvalue = createdUpvalue;

            return createdUpvalue;
        }

        void ClosedUpvalues(IntPtr last)
        {
            while (m_OpenUpvalues != null && m_OpenUpvalues.location >= last)
            {
                UpvalueObject upvalue = m_OpenUpvalues;
                upvalue.closed = Utils.SearchObjectByAddress(upvalue.location);
                upvalue.location = upvalue.closed.GetAddress();
                m_OpenUpvalues = upvalue.nextUpvalue;
            }
        }
    }
}