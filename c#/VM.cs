namespace ComputeDuck
{
    delegate Value NativeFunction(List<Value> args);
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

             m_NativeFunctions["print"] = (List<Value> args) =>
            {
                if (args.Count == 0)
                    return new Value();
                Console.Write(args[0].Stringify());
                return Value.g_UnknownValue;
            };

            m_NativeFunctions["println"] = (List<Value> args) =>
            {
                if (args.Count == 0)
                    return new Value();
                Console.WriteLine(args[0].Stringify());
                return new Value();
            };

            m_NativeFunctions["sizeof"] = (List<Value> args) =>
            {
                if (args.Count != 1)
                    Utils.Assert("[Native function 'sizeof']:Expect a argument.");
                if ( args[0].Type() == ValueType.OBJECT&&args[0].obj.Type()==ObjectType.ARRAY)
                    return new Value(((ArrayObject)args[0].obj).elements.Count);
                else if (args[0].Type()==ValueType.OBJECT &&args[0].obj.Type() == ObjectType.STR)
                    return new Value(((StrObject)args[0].obj).value.Length);
                else
                    Utils.Assert("[Native function 'sizeof']:Expect a array or string argument.");
                return new Value();
            };

            m_NativeFunctions["insert"] = (List<Value> args) =>
            {
                if (args.Count != 3)
                    Utils.Assert("[Native function 'insert']:Expect 3 arguments,the arg0 must be array,table or string object.The arg1 is the index object.The arg2 is the value object.");
                if (args[0].Type()==ValueType.OBJECT&&args[0].obj.Type()== ObjectType.ARRAY)
                {
                    ArrayObject array = (ArrayObject)args[0].obj;
                    if (args[1].Type() != ValueType.NUM)
                        Utils.Assert("[Native function 'insert']:Arg1 must be integer type while insert to a array");

                    int iIndex = (int)args[1].number;

                    if (iIndex < 0 || iIndex >= array.elements.Count)
                        Utils.Assert("[Native function 'insert']:Index out of array's range");
                    array.elements.Insert(iIndex, args[2]);
                }
                else if (args[0].Type()==ValueType.OBJECT&& args[0].obj.Type() == ObjectType.STR)
                {
                    StrObject str = (StrObject)args[0].obj;

                    int iIndex = (int)args[1].number;

                    if (iIndex < 0 || iIndex >= str.value.Length)
                        Utils.Assert("[Native function 'insert']:Index out of array's range");
                    str.value.Insert(iIndex, args[2].Stringify());
                }
                else
                    Utils.Assert("[Native function 'insert']:Expect a array,table ot string argument.");
                return new Value();
            };

            m_NativeFunctions["erase"] = (List<Value> args) =>
            {
                if (args.Count != 2)
                    Utils.Assert("[Native function 'erase']:Expect 2 arguments,the arg0 must be array,table or string object.The arg1 is the index object.");
                if (args[0].Type()==ValueType.OBJECT&&args[0].obj.Type() == ObjectType.ARRAY)
                {
                    ArrayObject array = (ArrayObject)args[0].obj;
                    if (args[1].Type() != ValueType.NUM)
                        Utils.Assert("[Native function 'erase']:Arg1 must be integer type while erasing an array element");

                    int iIndex = (int)args[1].number;

                    if (iIndex < 0 || iIndex >= array.elements.Count)
                        Utils.Assert("[Native function 'erase']:Index out of array's range");
                    array.elements.RemoveAt(iIndex);
                }
                else if (args[0].Type()==ValueType.OBJECT&&args[0].obj.Type() == ObjectType.STR)
                {
                    StrObject str = (StrObject)args[0].obj;

                    int iIndex = (int)args[1].number;

                    if (iIndex < 0 || iIndex >= str.value.Length)
                        Utils.Assert("[Native function 'erase']:Index out of array's range");
                    str.value.Remove(iIndex);
                }
                else
                    Utils.Assert("[Native function 'erase']:Expect a array,table ot string argument.");
                return new Value();
            };
        }

        public void ResetStatus()
        {
            this.sp = 0;
            m_ValueStack = Lists.RepeatedDefault<Value>(4096);
            m_Context = new Context();
        }


        public Value Execute(Frame frame)
        {
            for (var ip = 0; ip < frame.m_Codes.Count; ++ip)
            {
                int instruction = frame.m_Codes[ip];
                switch (instruction)
                {
                    case (int)OpCode.OP_RETURN:
                        if (m_Context.m_UpContext != null)
                            m_Context = m_Context.m_UpContext;
                        return PopValue();
                    case (int)OpCode.OP_NEW_NUM:
                        PushValue(new Value(frame.m_Nums[frame.m_Codes[++ip]]));
                        break;
                    case (int)OpCode.OP_NEW_STR:
                        PushValue(new Value(new StrObject(frame.m_Strings[frame.m_Codes[++ip]])));
                        break;
                    case (int)OpCode.OP_NEW_TRUE:
                        PushValue(new Value(true));
                        break;
                    case (int)OpCode.OP_NEW_FALSE:
                        PushValue(new Value(false));
                        break;
                    case (int)OpCode.OP_NEW_NIL:
                        PushValue(new Value());
                        break;
                    case (int)OpCode.OP_NEG:
                        {
                            var value = PopValue();
                            if (value.Type() == ValueType.NUM)
                                PushValue(new Value(-value.number));
                            else
                                Utils.Assert("Invalid op:'-'" + value.Stringify());
                            break;
                        }
                    case (int)OpCode.OP_NOT:
                        {
                            var value = PopValue();
                            if (value.Type() != ValueType.BOOL)
                                PushValue(new Value(!value.boolean));
                            else
                                Utils.Assert("Invalid op:'not' " + value.Stringify());
                            break;
                        }
                    case (int)OpCode.OP_ADD:
                        {
                            var left = PopValue();
                            var right = PopValue();
                            if (right.Type() == ValueType.NUM && right.Type() == ValueType.NUM)
                                PushValue(new Value(left.number + right.number));
                            else
                                Utils.Assert("Invalid binary op:" + left.Stringify() + "+" + right.Stringify());
                            break;
                        }
                    case (int)OpCode.OP_SUB:
                        {
                            var left = PopValue();
                            var right = PopValue();
                            if (right.Type() == ValueType.NUM && right.Type() == ValueType.NUM)
                                PushValue(new Value(left.number - right.number));
                            else
                                Utils.Assert("Invalid binary op:" + left.Stringify() + "-" + right.Stringify());
                            break;
                        }
                    case (int)OpCode.OP_MUL:
                        {
                            var left = PopValue();
                            var right = PopValue();
                            if (right.Type() == ValueType.NUM && right.Type() == ValueType.NUM)
                                PushValue(new Value(left.number * right.number));
                            else
                                Utils.Assert("Invalid binary op:" + left.Stringify() + "*" + right.Stringify());
                            break;
                        }
                    case (int)OpCode.OP_DIV:
                        {
                            var left = PopValue();
                            var right = PopValue();
                            if (right.Type() == ValueType.NUM && right.Type() == ValueType.NUM)
                                PushValue(new Value(left.number / right.number));
                            else
                                Utils.Assert("Invalid binary op:" + left.Stringify() + "/" + right.Stringify());
                            break;
                        }
                    case (int)OpCode.OP_GREATER:
                        {
                            var left = PopValue();
                            var right = PopValue();
                            if (right.Type() == ValueType.NUM && right.Type() == ValueType.NUM)
                                PushValue(left.number > right.number ? new Value(true) : new Value(false));
                            else
                                PushValue(new Value(false));
                            break;
                        }
                    case (int)OpCode.OP_LESS:
                        {
                            var left = PopValue();
                            var right = PopValue();
                            if (right.Type() == ValueType.NUM && right.Type() == ValueType.NUM)
                                PushValue(left.number < right.number ? new Value(true) : new Value(false));
                            else
                                PushValue(new Value(false));
                            break;
                        }
                    case (int)OpCode.OP_GREATER_EQUAL:
                        {
                            var left = PopValue();
                            var right = PopValue();
                            if (right.Type() == ValueType.NUM && right.Type() == ValueType.NUM)
                                PushValue(left.number >= right.number ? new Value(true) : new Value(false));
                            else
                                PushValue(new Value(false));
                            break;
                        }
                    case (int)OpCode.OP_LESS_EQUAL:
                        {
                            var left = PopValue();
                            var right = PopValue();
                            if (right.Type() == ValueType.NUM && right.Type() == ValueType.NUM)
                                PushValue(left.number <= right.number ? new Value(true) : new Value(false));
                            else
                                PushValue(new Value(false));
                            break;
                        }
                    case (int)OpCode.OP_EQUAL:
                        {
                            var left = PopValue();
                            var right = PopValue();
                            PushValue(new Value(left.IsEqualTo(right)));
                            break;
                        }
                    case (int)OpCode.OP_NOT_EQUAL:
                        {
                            var left = PopValue();
                            var right = PopValue();
                            PushValue(new Value(!left.IsEqualTo(right)));
                            break;
                        }
                    case (int)OpCode.OP_AND:
                        {
                            var left = PopValue();
                            var right = PopValue();
                            if (right.Type() == ValueType.BOOL && right.Type() == ValueType.BOOL)
                                PushValue(left.boolean && right.boolean ? new Value(true) : new Value(false));
                            else
                                PushValue(new Value(false));
                            break;
                        }
                    case (int)OpCode.OP_OR:
                        {
                            var left = PopValue();
                            var right = PopValue();
                            if (right.Type() == ValueType.BOOL && right.Type() == ValueType.BOOL)
                                PushValue(left.boolean || right.boolean ? new Value(true) : new Value(false));
                            else
                                PushValue(new Value(false));
                            break;
                        }
                    case (int)OpCode.OP_DEFINE_VAR:
                            m_Context.DefineVariableByName(frame.m_Strings[frame.m_Codes[++ip]], PopValue());
                            break;
                    case (int)OpCode.OP_SET_VAR:
                        {
                            string name = frame.m_Strings[frame.m_Codes[++ip]];
                            var value = PopValue();
                            var variable = m_Context.GetVariableByName(name);
                            if (variable.Type()==ValueType.OBJECT && variable.obj.Type() == ObjectType.REF)
                                m_Context.AssignVariableByName(((RefObject)variable.obj).name, value);
                            else
                                m_Context.AssignVariableByName(name, value);
                            break;
                        }
                    case (int)OpCode.OP_GET_VAR:
                        {
                            string name = frame.m_Strings[frame.m_Codes[++ip]];

                            var varValue = m_Context.GetVariableByName(name);
                            //create a struct object
                            if (varValue == Value.g_UnknownValue)
                            {
                                if (frame.HasStructFrame(name))
                                    PushValue(Execute(frame.GetStructFrame(name)));
                                else
                                    Utils.Assert("No struct definition:" + name);
                            }
                            else if (varValue.Type()==ValueType.OBJECT&&varValue.obj.Type() == ObjectType.REF)
                            {
                                varValue = m_Context.GetVariableByName(((RefObject)varValue.obj).name);
                                PushValue(varValue);
                            }
                            else
                                PushValue(varValue);
                            break;
                        }
                    case (int)OpCode.OP_NEW_ARRAY:
                        {
                            List<Value> elements = new List<Value>();
                            int arraySize = (int)frame.m_Nums[frame.m_Codes[++ip]];
                            for (var i = 0; i < arraySize; ++i)
                                elements.Insert(0, PopValue());
                            PushValue(new Value(new ArrayObject(elements)));
                            break;
                        }
                    case (int)OpCode.OP_NEW_STRUCT:
                        PushValue(new Value(new StructObject(frame.m_Strings[frame.m_Codes[++ip]], m_Context.m_Values)));
                        break;
                    case (int)OpCode.OP_NEW_LAMBDA:
                        PushValue(new Value(new LambdaObject((int)frame.m_Nums[frame.m_Codes[++ip]])));
                        break;
                    case (int)OpCode.OP_GET_INDEX_VAR:
                        {
                            var index = PopValue();
                            var value = PopValue();
                            if (value.Type()==ValueType.OBJECT&&value.obj.Type() == ObjectType.ARRAY)
                            {
                                var arrayObject = (ArrayObject)value.obj;
                                if (!(index.Type() == ValueType.NUM))
                                    Utils.Assert("Invalid index op.The index type of the array object must ba a int num type,but got:" + index.Stringify());
                                int iIndex = (int)index.number;
                                if (iIndex < 0 || iIndex >= arrayObject.elements.Count)
                                    Utils.Assert("Index out of array range,array size:" + arrayObject.elements.Count.ToString() + ",index:" + iIndex.ToString());
                                PushValue(arrayObject.elements[iIndex]);
                            }
                            else if (value.Type()==ValueType.OBJECT&& value.obj.Type() == ObjectType.STR)
                            {
                                var strObject = (StrObject)value.obj;
                                if (!(index.Type() == ValueType.NUM))
                                    Utils.Assert("Invalid index op.The index type of the array object must ba a int num type,but got:" + index.Stringify());
                                int iIndex = (int)index.number;
                                if (iIndex < 0 || iIndex >= strObject.value.Length)
                                    Utils.Assert("Index out of array range,array size:" + strObject.value.Length.ToString() + ",index:" + iIndex.ToString());
                                PushValue(new Value(new StrObject(strObject.value.Substring(iIndex, 1))));
                            }
                            else
                                Utils.Assert("Invalid index op.The indexed object isn't a array or a string object:" + value.Stringify());
                            break;
                        }
                    case (int)OpCode.OP_SET_INDEX_VAR:
                        {
                            Value index = PopValue();
                            Value value = PopValue();
                            Value assigner = PopValue();

                            if (value.Type()==ValueType.OBJECT&&value.obj.Type() == ObjectType.ARRAY)
                            {
                                var arrayObject = (ArrayObject)value.obj;
                                if (!(index.Type() == ValueType.NUM))
                                    Utils.Assert("Invalid index op.The index type of the array object must ba a int num type,but got:" + index.Stringify());
                                int iIndex = (int)index.number;
                                if (iIndex < 0 || iIndex >= arrayObject.elements.Count)
                                    Utils.Assert("Index out of array range,array size:" + arrayObject.elements.Count.ToString() + ",index:" + iIndex.ToString());
                                arrayObject.elements[iIndex] = assigner;
                            }
                            else if (value.Type()==ValueType.OBJECT&&value.obj.Type() == ObjectType.STR)
                            {
                                StrObject strObject = (StrObject)value.obj;
                                if (!(index.Type() == ValueType.NUM))
                                    Utils.Assert("Invalid index op.The index type of the array object must ba a int num type,but got:" + index.Stringify());
                                int iIndex = (int)index.number;
                                if (iIndex < 0 || iIndex >= strObject.value.Length)
                                    Utils.Assert("Index out of array range,array size:" + strObject.value.Length.ToString() + ",index:" + iIndex.ToString());

                                if (assigner.Type()!=ValueType.OBJECT||assigner.obj.Type() != ObjectType.STR)
                                    Utils.Assert("The assigner isn't a string.");

                                StrObject strAssigner=(StrObject)assigner.obj;

                                strObject.value=strObject.value.Substring(0,iIndex)+strAssigner.value+strObject.value.Substring(iIndex);
                            }
                            else
                                Utils.Assert("Invalid index op.The indexed object isn't a array object:" + value.Stringify());
                            break;
                        }
                    case (int)OpCode.OP_GET_STRUCT_VAR:
                        {
                            string memberName = frame.m_Strings[frame.m_Codes[++ip]];
                            var stackTop = PopValue();
                            if (stackTop.Type()!=ValueType.OBJECT||stackTop.obj.Type() != ObjectType.STRUCT)
                                Utils.Assert("Not a struct object of the callee of:" + memberName);
                            PushValue(((StructObject)stackTop.obj).GetMember(memberName));
                            break;
                        }
                    case (int)OpCode.OP_SET_STRUCT_VAR:
                        {
                            string memberName = frame.m_Strings[frame.m_Codes[++ip]];
                            var stackTop = PopValue();
                            if (stackTop.Type() != ValueType.OBJECT || stackTop.obj.Type() != ObjectType.STRUCT)
                                Utils.Assert("Not a struct object of the callee of:" + memberName);
                            ((StructObject)stackTop.obj).AssignMember(memberName, PopValue());
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
                            bool isJump = !PopValue().boolean;
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
                            var argCount = PopValue().number;
                            if (frame.HasFunctionFrame(fnName))
                                PushValue(Execute(frame.GetFunctionFrame(fnName)));
                            else if (HasNativeFunction(fnName))
                            {
                                List<Value> args = new List<Value>();
                                for (var i = 0; i < argCount; ++i)
                                    args.Insert(0, PopValue());
                                Value result = GetNativeFunction(fnName)(args);
                                if (result != null)
                                    PushValue(result);
                            }
                            else if (m_Context.GetVariableByName(fnName) != Value.g_UnknownValue && m_Context.GetVariableByName(fnName).obj.Type() == ObjectType.LAMBDA)//lambda
                            {
                                var lambdaObject = (LambdaObject)m_Context.GetVariableByName(fnName).obj;
                                PushValue(Execute(frame.GetLambdaFrame(lambdaObject.idx)));
                            }
                            else
                                Utils.Assert("No function:" + fnName);
                            break;
                        }
                    case (int)OpCode.OP_STRUCT_LAMBDA_CALL:
                        {
                            string fnName = frame.m_Strings[frame.m_Codes[++ip]];
                            var stackTop = PopValue();
                            var argCount = PopValue().number;
                            if (stackTop.Type()!=ValueType.OBJECT||stackTop.obj.Type() != ObjectType.STRUCT)
                                Utils.Assert("Cannot call a struct lambda function:" + fnName + ",the callee isn't a struct object");
                            var structObj = (StructObject)stackTop.obj;
                            Value member = structObj.GetMember(fnName);
                            if (member == Value.g_UnknownValue)
                                Utils.Assert("No member in struct:" + structObj.name);
                            if (member.Type()!=ValueType.OBJECT|| member.obj.Type() != ObjectType.LAMBDA)
                                Utils.Assert("Not a lambda function:" + fnName + " in struct:" + structObj.name);

                            LambdaObject lambdaObject = (LambdaObject)member.obj;
                            PushValue(Execute(frame.GetLambdaFrame(lambdaObject.idx)));
                            break;
                        }
                    case (int)OpCode.OP_REF:
                        PushValue(new Value(new RefObject(frame.m_Strings[frame.m_Codes[++ip]])));
                        break;
                    default:
                        break;
                }
            }

            return new Value();
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

        private void PushValue(Value value)
        {
            m_ValueStack[sp++] = value;
        }

        private Value PopValue()
        {
            return m_ValueStack[--sp];
        }

        private int sp;
        private List<Value> m_ValueStack;
        private Context m_Context;
        private Dictionary<string, NativeFunction> m_NativeFunctions;
    }

}