using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace ComputeDuck
{
    public class CallFrame
    {
        public FunctionObject fn;
        public int ip;
        public int slot;

        public CallFrame()
        {
            this.fn = null;
            this.ip = -1;
            this.slot = -1;
        }

        public CallFrame(FunctionObject fn, int slot)
        {
            this.fn = fn;
            this.ip = 0;
            this.slot = slot;
        }

        public bool IsAtEnd()
        {
            if (ip < fn.opCodes.Count)
                return false;
            return true;
        }
    }
    public class VM
    {
        const int STACK_MAX = 512;

        private Chunk m_Chunk;
        private Object[] m_GlobalVariables = new Object[STACK_MAX];

        private int m_StackTop;
        private Object[] m_ObjectStack = new Object[STACK_MAX];

        private int m_CallFrameTop;
        private CallFrame[] m_CallFrames = new CallFrame[STACK_MAX];

        private void ResetStatus()
        {
            m_StackTop = 0;
            for (int i = 0; i < STACK_MAX; ++i)
                m_ObjectStack[i] = new NilObject();

            for (int i = 0; i < STACK_MAX; ++i)
                m_GlobalVariables[i] = new NilObject();

            for (int i = 0; i < STACK_MAX; ++i)
                m_CallFrames[i] = new CallFrame();
        }

        public void Run(Chunk chunk)
        {
            ResetStatus();

            m_Chunk = chunk;

            var mainFn = new FunctionObject(chunk.opCodes);
            var mainCallFrame = new CallFrame(mainFn, m_StackTop);
            
            PushCallFrame(mainCallFrame);

            Execute();
        }

        private void Execute()
        {
            while (true)
            {
                CallFrame frame = PeekCallFrame(1);

                if (frame.IsAtEnd())
                    return;

                int instruction = frame.fn.opCodes[frame.ip++];
                switch (instruction)
                {
                    case (int)OpCode.OP_RETURN:
                        {
                            var returnCounnt = frame.fn.opCodes[frame.ip++];
                            Object obj = null;
                            if (returnCounnt == 1)
                                obj = Pop();

                            var callFrame = PopCallFrame();

                            if (m_CallFrameTop == 0)
                                return;

                            m_StackTop = callFrame.slot - 1;

                            Push(obj);
                            break;
                        }
                    case (int)OpCode.OP_CONSTANT:
                        {
                            var idx = frame.fn.opCodes[frame.ip++];
                            var value = m_Chunk.constants[idx];

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
                            var left =FindActualObject(Pop());
                            var right =FindActualObject(Pop());

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
                            var numElements = frame.fn.opCodes[frame.ip++];
                            List<Object> elements = new List<Object>(m_ObjectStack[(m_StackTop - numElements)..m_StackTop]);
                            
                            m_StackTop -= numElements;

                            var array = new ArrayObject(elements);
                            Push(array);
                            break;
                        }
                    case (int)OpCode.OP_INDEX:
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
                    case (int)OpCode.OP_JUMP_IF_FALSE:
                        {
                            var address = frame.fn.opCodes[frame.ip++];
                            var obj = Pop();
                            if (obj.type != ObjectType.BOOL)
                                Utils.Assert("The if condition not a boolean value:" + obj.ToString());
                            if (((BoolObject)obj).value != true)
                                frame.ip = address + 1;
                            break;
                        }
                    case (int)OpCode.OP_JUMP:
                        {
                            var address = frame.fn.opCodes[frame.ip++];
                            frame.ip = address + 1;
                            break;
                        }
                    case (int)OpCode.OP_SET_GLOBAL:
                        {
                            var index = frame.fn.opCodes[frame.ip++];
                            var obj = Pop();

                            if (m_GlobalVariables[index].type == ObjectType.REF)
                            {
                                var gRefObj = m_GlobalVariables[index];
                                var gRefAddress = new IntPtr(-1);
                                while (gRefObj.type == ObjectType.REF)
                                {
                                    gRefAddress = ((RefObject)gRefObj).pointer;
                                    gRefObj = SearchObjectByAddress(((RefObject)gRefObj).pointer);
                                }
                                AssignObjectByAddress(gRefAddress, obj);

                                var refObjs = SearchRefObjectListByAddress(gRefAddress);
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
                            var index = frame.fn.opCodes[frame.ip++];
                            Push(m_GlobalVariables[index]);
                            break;
                        }
                    case (int)OpCode.OP_FUNCTION_CALL:
                        {
                            var argCount = frame.fn.opCodes[frame.ip++];
                            var obj = m_ObjectStack[m_StackTop - 1 - argCount];
                            if (obj.type == ObjectType.FUNCTION)
                            {
                                if (argCount != ((FunctionObject)obj).parameterCount)
                                    Utils.Assert("Non matching function parameters for calling arguments,parameter count:" + ((FunctionObject)obj).parameterCount + ",argument count:" + argCount);

                                var callFrame = new CallFrame(((FunctionObject)obj), m_StackTop - argCount);
                                PushCallFrame(callFrame);
                                m_StackTop = callFrame.slot + ((FunctionObject)obj).localVarCount;
                            }
                            else if (obj.type == ObjectType.BUILTIN)
                            {
                                var args = new List<Object>(m_ObjectStack[(m_StackTop-argCount)..m_StackTop]);
                               
                                m_StackTop -= (argCount + 1);

                                var (hasReturnValue,returnValue) = ((BuiltinObject)obj).GetBuiltinFn()(args);

                                if (hasReturnValue)
                                    Push(returnValue);
                            }
                            else
                                Utils.Assert("Calling not a function or a builtinFn");
                            break;
                        }
                    case (int)OpCode.OP_SET_LOCAL:
                        {
                            var scopeDepth = frame.fn.opCodes[frame.ip++];
                            var index = frame.fn.opCodes[frame.ip++];

                            var obj = Pop();

                            var slot = PeekCallFrame(scopeDepth).slot + index;

                            if (m_ObjectStack[slot].type == ObjectType.REF)
                            {
                                var lRefObj = m_ObjectStack[slot];
                                var lRefAddress = new IntPtr(-1);
                                while (lRefObj.type == ObjectType.REF)
                                {
                                    lRefAddress = ((RefObject)lRefObj).pointer;
                                    lRefObj = SearchObjectByAddress(((RefObject)lRefObj).pointer);
                                }
                                AssignObjectByAddress(lRefAddress, obj);

                                var refObjs = SearchRefObjectListByAddress(lRefAddress);
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
                            var scopeDepth = frame.fn.opCodes[frame.ip++];
                            var index = frame.fn.opCodes[frame.ip++];

                            int slot =  PeekCallFrame(scopeDepth).slot + index;

                            Push(m_ObjectStack[slot]);

                            break;
                        }
                    case (int)OpCode.OP_SP_OFFSET:
                        {
                            var offset = frame.fn.opCodes[frame.ip++];
                            m_StackTop += offset;
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
                            var memberCount = frame.fn.opCodes[frame.ip++];

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

                            while (instance.type == ObjectType.REF)
                                instance = SearchObjectByAddress(((RefObject)instance).pointer);
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

                            while (instance.type == ObjectType.REF)
                                instance = SearchObjectByAddress(((RefObject)instance).pointer);
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
                            var index = frame.fn.opCodes[frame.ip++];
                            Push(new RefObject(m_GlobalVariables[index].GetAddress()));
                            break;
                        }
                    case (int)OpCode.OP_REF_LOCAL:
                        {
                            var scopeDepth = frame.fn.opCodes[frame.ip++];
                            var index = frame.fn.opCodes[frame.ip++];

                            int slot =  PeekCallFrame(scopeDepth).slot + index;

                            Push(new RefObject(m_ObjectStack[slot].GetAddress()));

                            break;
                        }
                    case (int)OpCode.OP_REF_INDEX_GLOBAL:
                        {
                            var index = frame.fn.opCodes[frame.ip++];
                            var idxValue = Pop();

                            var obj = m_GlobalVariables[index];
                            while (obj.type == ObjectType.REF)
                                obj = SearchObjectByAddress(((RefObject)obj).pointer);

                            if (obj.type == ObjectType.ARRAY)
                            {
                                if (idxValue.type != ObjectType.NUM)
                                    Utils.Assert("Invalid idx for array,only integer is available.");
                                if (((NumObject)idxValue).value < 0 || ((NumObject)idxValue).value >= ((ArrayObject)obj).elements.Count)
                                    Utils.Assert("Idx out of range.");
                                Push(new RefObject(((ArrayObject)obj).elements[(int)((NumObject)idxValue).value].GetAddress()));
                            }

                            break;
                        }
                    case (int)OpCode.OP_REF_INDEX_LOCAL:
                        {
                            var scopeDepth = frame.fn.opCodes[frame.ip++];
                            var index = frame.fn.opCodes[frame.ip++];

                            var idxValue = Pop();

                            int slot = PeekCallFrame(scopeDepth).slot + index;

                            while (m_ObjectStack[slot].type == ObjectType.REF)
                                m_ObjectStack[slot] = SearchObjectByAddress(((RefObject)m_ObjectStack[slot]).pointer);


                            if (m_ObjectStack[slot].type == ObjectType.ARRAY)
                            {
                                if (idxValue.type != ObjectType.NUM)
                                    Utils.Assert("Invalid idx for array,only integer is available.");
                                if (((NumObject)idxValue).value < 0 || ((NumObject)idxValue).value >= ((ArrayObject)m_ObjectStack[slot]).elements.Count)
                                    Utils.Assert("Idx out of range.");
                                Push(new RefObject(((ArrayObject)m_ObjectStack[slot]).elements[(int)((NumObject)idxValue).value].GetAddress()));
                            }
                            else
                                Utils.Assert("Invalid indexed reference type:" + m_ObjectStack[slot].ToString() + " not a array value.");

                            break;
                        }

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

        private Object FindActualObject(Object obj)
        {
            var actual = obj;
            while (actual.type == ObjectType.REF)
                actual = SearchObjectByAddress(((RefObject)actual).pointer);
           
            if (actual.type == ObjectType.BUILTIN && ((BuiltinObject)actual).IsBuiltinVar())
                actual = ((BuiltinObject)actual).GetBuiltinVar();

            return actual;
        }

        private Object SearchObjectByAddress(IntPtr address)
        {
            Object obj = GCHandle.FromIntPtr(address).Target as Object;
            return obj;
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

        }
    }
}