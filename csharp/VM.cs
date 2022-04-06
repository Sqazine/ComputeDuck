namespace ComputeDuck
{
    delegate Object NativeFunction(List<Object> args);
    public class VM
    {

        //helper class 
        public static class Lists
        {
            public static List<T> RepeatedDefault<T>(int count)
            {
                return Repeated(default(T), count);
            }

            public static List<T> Repeated<T>(T value, int count)
            {
                List<T> ret = new List<T>(count);
                ret.AddRange(Enumerable.Repeat(value, count));
                return ret;
            }
        }

        public VM()
        {
            this.ResetStatus();

            m_NativeFunctions = new Dictionary<string, NativeFunction>();

            m_NativeFunctions["println"] = (List<Object> args) =>
            {
                if (args.Count == 0)
                    return null;
                Console.WriteLine(args[0].Stringify());
                return null;
            };

            m_NativeFunctions["sizeof"] = (List<Object> args) =>
            {
                if (args.Count != 1)
                    Utils.Assert("[Native function 'sizeof']:Expect a argument.");
                if (args[0].Type() == ObjectType.ARRAY)
                    return new NumObject(((ArrayObject)args[0]).elements.Count);
                else if (args[0].Type() == ObjectType.STR)
                    return new NumObject(((StrObject)args[0]).value.Length);
                else
                    Utils.Assert("[Native function 'sizeof']:Expect a array or string argument.");
                return new NilObject();
            };

            m_NativeFunctions["insert"] = (List<Object> args) =>
            {
                if (args.Count != 3)
                    Utils.Assert("[Native function 'insert']:Expect 3 arguments,the arg0 must be array,table or string object.The arg1 is the index object.The arg2 is the value object.");
                if (args[0].Type() == ObjectType.ARRAY)
                {
                    ArrayObject array = (ArrayObject)args[0];
                    if (args[1].Type() != ObjectType.NUM)
                        Utils.Assert("[Native function 'insert']:Arg1 must be integer type while insert to a array");

                    int iIndex = (int)((NumObject)args[1]).value;

                    if (iIndex < 0 || iIndex >= array.elements.Count)
                        Utils.Assert("[Native function 'insert']:Index out of array's range");
                    array.elements.Insert(iIndex, args[2]);
                }
                else if (args[0].Type() == ObjectType.STR)
                {
                    StrObject str = (StrObject)args[0];

                    int iIndex = (int)((NumObject)args[1]).value;

                    if (iIndex < 0 || iIndex >= str.value.Length)
                        Utils.Assert("[Native function 'insert']:Index out of array's range");
                    str.value.Insert(iIndex, args[2].Stringify());
                }
                else
                    Utils.Assert("[Native function 'insert']:Expect a array,table ot string argument.");
                return null;
            };

            m_NativeFunctions["erase"] = (List<Object> args) =>
            {
                if (args.Count != 2)
                    Utils.Assert("[Native function 'erase']:Expect 2 arguments,the arg0 must be array,table or string object.The arg1 is the index object.");
                if (args[0].Type() == ObjectType.ARRAY)
                {
                    ArrayObject array = (ArrayObject)args[0];
                    if (args[1].Type() != ObjectType.NUM)
                        Utils.Assert("[Native function 'erase']:Arg1 must be integer type while erasing an array element");

                    int iIndex = (int)((NumObject)args[1]).value;

                    if (iIndex < 0 || iIndex >= array.elements.Count)
                        Utils.Assert("[Native function 'erase']:Index out of array's range");
                    array.elements.RemoveAt(iIndex);
                }
                else if (args[0].Type() == ObjectType.STR)
                {
                    StrObject str = (StrObject)args[0];

                    int iIndex = (int)((NumObject)args[1]).value;

                    if (iIndex < 0 || iIndex >= str.value.Length)
                        Utils.Assert("[Native function 'erase']:Index out of array's range");
                    str.value.Remove(iIndex);
                }
                else
                    Utils.Assert("[Native function 'erase']:Expect a array,table ot string argument.");
                return null;
            };
        }

        public void ResetStatus()
        {
            this.sp = 0;
            m_ObjectStack = Lists.RepeatedDefault<Object>(4096);
            m_Context = new Context();
        }


        public Object Execute(Frame frame)
        {
            for (var ip = 0; ip < frame.m_Codes.Count; ++ip)
            {
                int instruction = frame.m_Codes[ip];
                switch (instruction)
                {
                    case (int)OpCode.OP_RETURN:
                        if (m_Context.m_UpContext != null)
                            m_Context = m_Context.m_UpContext;
                        return PopObject();
                    case (int)OpCode.OP_NEW_NUM:
                        PushObject(new NumObject(frame.m_Nums[frame.m_Codes[++ip]]));
                        break;
                    case (int)OpCode.OP_NEW_STR:
                        PushObject(new StrObject(frame.m_Strings[frame.m_Codes[++ip]]));
                        break;
                    case (int)OpCode.OP_NEW_TRUE:
                        PushObject(new BoolObject(true));
                        break;
                    case (int)OpCode.OP_NEW_FALSE:
                        PushObject(new BoolObject(false));
                        break;
                    case (int)OpCode.OP_NEW_NIL:
                        PushObject(new NilObject());
                        break;
                    case (int)OpCode.OP_NEG:
                        {
                            var obj = PopObject();
                            if (obj.Type() == ObjectType.NUM)
                                PushObject(new NumObject(-((NumObject)obj).value));
                            else
                                Utils.Assert("Invalid op:'-'" + obj.Stringify());
                            break;
                        }
                    case (int)OpCode.OP_NOT:
                        {
                            Object obj = PopObject();
                            if (obj.Type() != ObjectType.BOOL)
                                PushObject(new BoolObject(!((BoolObject)obj).value));
                            else
                                Utils.Assert("Invalid op:'not' " + obj.Stringify());
                            break;
                        }
                    case (int)OpCode.OP_ADD:
                        {
                            var left = PopObject();
                            var right = PopObject();
                            if (right.Type() == ObjectType.NUM && right.Type() == ObjectType.NUM)
                                PushObject(new NumObject(((NumObject)left).value + ((NumObject)right).value));
                            else
                                Utils.Assert("Invalid binary op:" + left.Stringify() + "+" + right.Stringify());
                            break;
                        }
                    case (int)OpCode.OP_SUB:
                        {
                            var left = PopObject();
                            var right = PopObject();
                            if (right.Type() == ObjectType.NUM && right.Type() == ObjectType.NUM)
                                PushObject(new NumObject(((NumObject)left).value - ((NumObject)right).value));
                            else
                                Utils.Assert("Invalid binary op:" + left.Stringify() + "-" + right.Stringify());
                            break;
                        }
                    case (int)OpCode.OP_MUL:
                        {
                            var left = PopObject();
                            var right = PopObject();
                            if (right.Type() == ObjectType.NUM && right.Type() == ObjectType.NUM)
                                PushObject(new NumObject(((NumObject)left).value * ((NumObject)right).value));
                            else
                                Utils.Assert("Invalid binary op:" + left.Stringify() + "*" + right.Stringify());
                            break;
                        }
                    case (int)OpCode.OP_DIV:
                        {
                            var left = PopObject();
                            var right = PopObject();
                            if (right.Type() == ObjectType.NUM && right.Type() == ObjectType.NUM)
                                PushObject(new NumObject(((NumObject)left).value / ((NumObject)right).value));
                            else
                                Utils.Assert("Invalid binary op:" + left.Stringify() + "/" + right.Stringify());
                            break;
                        }
                    case (int)OpCode.OP_GREATER:
                        {
                            var left = PopObject();
                            var right = PopObject();
                            if (right.Type() == ObjectType.NUM && right.Type() == ObjectType.NUM)
                                PushObject(((NumObject)left).value > ((NumObject)right).value ? new BoolObject(true) : new BoolObject(false));
                            else
                                PushObject(new BoolObject(false));
                            break;
                        }
                    case (int)OpCode.OP_LESS:
                        {
                            var left = PopObject();
                            var right = PopObject();
                            if (right.Type() == ObjectType.NUM && right.Type() == ObjectType.NUM)
                                PushObject(((NumObject)left).value < ((NumObject)right).value ? new BoolObject(true) : new BoolObject(false));
                            else
                                PushObject(new BoolObject(false));
                            break;
                        }
                    case (int)OpCode.OP_GREATER_EQUAL:
                        {
                            var left = PopObject();
                            var right = PopObject();
                            if (right.Type() == ObjectType.NUM && right.Type() == ObjectType.NUM)
                                PushObject(((NumObject)left).value >= ((NumObject)right).value ? new BoolObject(true) : new BoolObject(false));
                            else
                                PushObject(new BoolObject(false));
                            break;
                        }
                    case (int)OpCode.OP_LESS_EQUAL:
                        {
                            var left = PopObject();
                            var right = PopObject();
                            if (right.Type() == ObjectType.NUM && right.Type() == ObjectType.NUM)
                                PushObject(((NumObject)left).value <= ((NumObject)right).value ? new BoolObject(true) : new BoolObject(false));
                            else
                                PushObject(new BoolObject(false));
                            break;
                        }
                    case (int)OpCode.OP_EQUAL:
                        {
                            var left = PopObject();
                            var right = PopObject();
                            PushObject(new BoolObject(left.IsEqualTo(right)));
                            break;
                        }
                    case (int)OpCode.OP_NOT_EQUAL:
                        {
                            var left = PopObject();
                            var right = PopObject();
                            PushObject(new BoolObject(!left.IsEqualTo(right)));
                            break;
                        }
                    case (int)OpCode.OP_AND:
                        {
                            var left = PopObject();
                            var right = PopObject();
                            if (right.Type() == ObjectType.BOOL && right.Type() == ObjectType.BOOL)
                                PushObject(((BoolObject)left).value && ((BoolObject)right).value ? new BoolObject(true) : new BoolObject(false));
                            else
                                PushObject(new BoolObject(false));
                            break;
                        }
                    case (int)OpCode.OP_OR:
                        {
                            var left = PopObject();
                            var right = PopObject();
                            if (right.Type() == ObjectType.BOOL && right.Type() == ObjectType.BOOL)
                                PushObject(((BoolObject)left).value || ((BoolObject)right).value ? new BoolObject(true) : new BoolObject(false));
                            else
                                PushObject(new BoolObject(false));
                            break;
                        }
                    case (int)OpCode.OP_DEFINE_VAR:
                        {
                            var value = PopObject();
                            m_Context.DefineVariableByName(frame.m_Strings[frame.m_Codes[++ip]], value);
                            break;
                        }
                    case (int)OpCode.OP_SET_VAR:
                        {
                            string name = frame.m_Strings[frame.m_Codes[++ip]];
                            var value = PopObject();
                            var variable = m_Context.GetVariableByName(name);
                            if (variable.Type() == ObjectType.REF)
                                m_Context.AssignVariableByName(((RefObject)variable).name, value);
                            else
                                m_Context.AssignVariableByName(name, value);
                            break;
                        }
                    case (int)OpCode.OP_GET_VAR:
                        {
                            string name = frame.m_Strings[frame.m_Codes[++ip]];

                            var varObject = m_Context.GetVariableByName(name);
                            //create a struct object
                            if (varObject == null)
                            {
                                if (frame.HasStructFrame(name))
                                    PushObject(Execute(frame.GetStructFrame(name)));
                                else
                                    Utils.Assert("No struct definition:" + name);
                            }
                            else if (varObject.Type() == ObjectType.REF)
                            {
                                varObject = m_Context.GetVariableByName(((RefObject)varObject).name);
                                PushObject(varObject);
                            }
                            else
                                PushObject(varObject);
                            break;
                        }
                    case (int)OpCode.OP_NEW_ARRAY:
                        {
                            List<Object> elements = new List<Object>();
                            int arraySize = (int)frame.m_Nums[frame.m_Codes[++ip]];
                            for (var i = 0; i < arraySize; ++i)
                                elements.Insert(0, PopObject());
                            PushObject(new ArrayObject(elements));
                            break;
                        }
                    case (int)OpCode.OP_NEW_STRUCT:
                        PushObject(new StructObject(frame.m_Strings[frame.m_Codes[++ip]], m_Context.m_Values));
                        break;
                    case (int)OpCode.OP_NEW_LAMBDA:
                        PushObject(new LambdaObject((int)frame.m_Nums[frame.m_Codes[++ip]]));
                        break;
                    case (int)OpCode.OP_GET_INDEX_VAR:
                        {
                            var index = PopObject();
                            var obj = PopObject();
                            if (obj.Type() == ObjectType.ARRAY)
                            {
                                var arrayObject = (ArrayObject)obj;
                                if (!(index.Type() == ObjectType.NUM))
                                    Utils.Assert("Invalid index op.The index type of the array object must ba a int num type,but got:" + index.Stringify());
                                int iIndex = (int)((NumObject)index).value;
                                if (iIndex < 0 || iIndex >= arrayObject.elements.Count)
                                    Utils.Assert("Index out of array range,array size:" + arrayObject.elements.Count.ToString() + ",index:" + iIndex.ToString());
                                PushObject(arrayObject.elements[iIndex]);
                            }
                            else if (obj.Type() == ObjectType.STR)
                            {
                                var strObject = (StrObject)obj;
                                if (!(index.Type() == ObjectType.NUM))
                                    Utils.Assert("Invalid index op.The index type of the array object must ba a int num type,but got:" + index.Stringify());
                                int iIndex = (int)((NumObject)index).value;
                                if (iIndex < 0 || iIndex >= strObject.value.Length)
                                    Utils.Assert("Index out of array range,array size:" + strObject.value.Length.ToString() + ",index:" + iIndex.ToString());
                                PushObject(new StrObject(strObject.value.Substring(iIndex, 1)));
                            }
                            else
                                Utils.Assert("Invalid index op.The indexed object isn't a array or a string object:" + obj.Stringify());
                            break;
                        }
                    case (int)OpCode.OP_SET_INDEX_VAR:
                        {
                            Object index = PopObject();
                            Object obj = PopObject();
                            Object assigner = PopObject();

                            if (obj.Type() == ObjectType.ARRAY)
                            {
                                var arrayObject = (ArrayObject)obj;
                                if (!(index.Type() == ObjectType.NUM))
                                    Utils.Assert("Invalid index op.The index type of the array object must ba a int num type,but got:" + index.Stringify());
                                int iIndex = (int)((NumObject)index).value;
                                if (iIndex < 0 || iIndex >= arrayObject.elements.Count)
                                    Utils.Assert("Index out of array range,array size:" + arrayObject.elements.Count.ToString() + ",index:" + iIndex.ToString());
                                arrayObject.elements[iIndex] = assigner;
                            }
                            else if (obj.Type() == ObjectType.STR)
                            {
                                StrObject strObject = (StrObject)obj;
                                if (!(index.Type() == ObjectType.NUM))
                                    Utils.Assert("Invalid index op.The index type of the array object must ba a int num type,but got:" + index.Stringify());
                                int iIndex = (int)((NumObject)index).value;
                                if (iIndex < 0 || iIndex >= strObject.value.Length)
                                    Utils.Assert("Index out of array range,array size:" + strObject.value.Length.ToString() + ",index:" + iIndex.ToString());

                                if (assigner.Type() != ObjectType.STR)
                                    Utils.Assert("The assigner isn't a string.");

                                StrObject strAssigner=(StrObject)assigner;

                                strObject.value=strObject.value.Substring(0,iIndex)+strAssigner.value+strObject.value.Substring(iIndex);
                            }
                            else
                                Utils.Assert("Invalid index op.The indexed object isn't a array object:" + obj.Stringify());
                            break;
                        }
                    case (int)OpCode.OP_GET_STRUCT_VAR:
                        {
                            string memberName = frame.m_Strings[frame.m_Codes[++ip]];
                            var stackTop = PopObject();
                            if (stackTop.Type() != ObjectType.STRUCT)
                                Utils.Assert("Not a struct object of the callee of:" + memberName);
                            PushObject(((StructObject)stackTop).GetMember(memberName));
                            break;
                        }
                    case (int)OpCode.OP_SET_STRUCT_VAR:
                        {
                            string memberName = frame.m_Strings[frame.m_Codes[++ip]];
                            var stackTop = PopObject();
                            if (stackTop.Type() != ObjectType.STRUCT)
                                Utils.Assert("Not a struct object of the callee of:" + memberName);
                            ((StructObject)stackTop).AssignMember(memberName, PopObject());
                            break;
                        }
                    case (int)OpCode.OP_ENTER_SCOPE:
                        {
                            m_Context = new Context(m_Context);
                            break;
                        }
                    case (int)OpCode.OP_EXIT_SCOPE:
                        {
                            m_Context = m_Context.m_UpContext;
                            break;
                        }
                    case (int)OpCode.OP_JUMP_IF_FALSE:
                        {
                            bool isJump = !((BoolObject)PopObject()).value;
                            int address = (int)(frame.m_Nums[frame.m_Codes[++ip]]);

                            if (isJump)
                                ip = address;
                            break;
                        }
                    case (int)OpCode.OP_JUMP:
                        {
                            int address = (int)(frame.m_Nums[frame.m_Codes[++ip]]);
                            ip = address;
                            break;
                        }
                    case (int)OpCode.OP_FUNCTION_CALL:
                        {
                            string fnName = frame.m_Strings[frame.m_Codes[++ip]];
                            NumObject argCount = ((NumObject)PopObject());
                            if (frame.HasFunctionFrame(fnName))
                                PushObject(Execute(frame.GetFunctionFrame(fnName)));
                            else if (HasNativeFunction(fnName))
                            {
                                List<Object> args = new List<Object>();
                                for (var i = 0; i < argCount.value; ++i)
                                    args.Insert(0, PopObject());
                                Object result = GetNativeFunction(fnName)(args);
                                if (result != null)
                                    PushObject(result);
                            }
                            else if (m_Context.GetVariableByName(fnName) != null && m_Context.GetVariableByName(fnName).Type() == ObjectType.LAMBDA)//lambda
                            {
                                var lambdaObject = (LambdaObject)m_Context.GetVariableByName(fnName);
                                PushObject(Execute(frame.GetLambdaFrame(lambdaObject.idx)));
                            }
                            else
                                Utils.Assert("No function:" + fnName);
                            break;
                        }
                    case (int)OpCode.OP_STRUCT_LAMBDA_CALL:
                        {
                            string fnName = frame.m_Strings[frame.m_Codes[++ip]];
                            var stackTop = PopObject();
                            NumObject argCount = (NumObject)PopObject();
                            if (stackTop.Type() != ObjectType.STRUCT)
                                Utils.Assert("Cannot call a struct lambda function:" + fnName + ",the callee isn't a struct object");
                            var structObj = (StructObject)stackTop;
                            Object member = structObj.GetMember(fnName);
                            if (member == null)
                                Utils.Assert("No member in struct:" + structObj.name);
                            if (member.Type() != ObjectType.LAMBDA)
                                Utils.Assert("Not a lambda function:" + fnName + " in struct:" + structObj.name);

                            LambdaObject lambdaObject = (LambdaObject)member;
                            PushObject(Execute(frame.GetLambdaFrame(lambdaObject.idx)));
                            break;
                        }
                    case (int)OpCode.OP_REF:
                        PushObject(new RefObject(frame.m_Strings[frame.m_Codes[++ip]]));
                        break;
                    default:
                        break;
                }
            }

            return new NilObject();
        }

        NativeFunction GetNativeFunction(string fnName)
        {

            if (m_NativeFunctions.ContainsKey(fnName))
                return m_NativeFunctions[fnName];
            Utils.Assert("No native function:" + fnName);
            return null;
        }

        bool HasNativeFunction(string name)
        {
            if (m_NativeFunctions.ContainsKey(name))
                return true;
            return false;
        }

        private void PushObject(Object obj)
        {
            m_ObjectStack[sp++] = obj;
        }

        private Object PopObject()
        {
            return m_ObjectStack[--sp];
        }

        private int sp;
        private List<Object> m_ObjectStack;
        private Context m_Context;
        private Dictionary<string, NativeFunction> m_NativeFunctions;
    }

}