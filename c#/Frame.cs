namespace ComputeDuck
{
    public enum OpCode
    {
        OP_NEW_NUM = 0,
        OP_NEW_STR,
        OP_NEW_TRUE,
        OP_NEW_FALSE,
        OP_NEW_NIL,
        OP_NEW_ARRAY,
        OP_NEW_STRUCT,
        OP_NEW_LAMBDA,
        OP_GET_VAR,
        OP_SET_VAR,
        OP_DEFINE_VAR,
        OP_GET_INDEX_VAR,
        OP_SET_INDEX_VAR,
        OP_GET_STRUCT_VAR,
        OP_SET_STRUCT_VAR,
        OP_NEG,
        OP_RETURN,
        OP_ADD,
        OP_SUB,
        OP_MUL,
        OP_DIV,
        OP_GREATER,
        OP_LESS,
        OP_GREATER_EQUAL,
        OP_LESS_EQUAL,
        OP_EQUAL,
        OP_NOT_EQUAL,
        OP_NOT,
        OP_OR,
        OP_AND,
        OP_ENTER_SCOPE,
        OP_EXIT_SCOPE,
        OP_JUMP,
        OP_JUMP_IF_FALSE,
        OP_FUNCTION_CALL,
        OP_STRUCT_LAMBDA_CALL,
        OP_REF,
    };


    public class Frame
    {
        public Frame()
        {
            Clear();
        }
        public Frame(ref Frame frame)
        {
            Clear();
            this.m_UpFrame = frame;
        }

        public void AddOpCode(int code)
        {
            m_Codes.Add(code);
        }

        public int AddNum(double value)
        {
            m_Nums.Add(value);
            return m_Nums.Count - 1;
        }
        public int AddString(string value)
        {
            m_Strings.Add(value);
            return m_Strings.Count - 1;
        }


        public void AddFunctionFrame(string name, Frame frame)
        {
            if (m_FunctionFrames.ContainsKey(name))
                Utils.Assert("Redefinition function:" + name);
            m_FunctionFrames[name] = frame;
        }
        public Frame GetFunctionFrame(string name)
        {
            if (m_FunctionFrames.ContainsKey(name))
                return m_FunctionFrames[name];
            else if (this.m_UpFrame != null)
                return m_UpFrame.GetFunctionFrame(name);
            Utils.Assert("No function definition:" + name);
            return new Frame();//avoid compiler warning
        }
        public bool HasFunctionFrame(string name)
        {
            foreach (var entry in m_FunctionFrames)
                if (entry.Key == name)
                    return true;
            if (m_UpFrame != null)
                return m_UpFrame.HasFunctionFrame(name);
            return false;
        }

        public void AddStructFrame(string name, Frame frame)
        {
            if (m_StructFrames.ContainsKey(name))
                Utils.Assert("Redefinition struct:" + name);
            m_StructFrames[name] = frame;
        }
        public Frame GetStructFrame(string name)
        {
            if (m_StructFrames.ContainsKey(name))
                return m_StructFrames[name];
            else if (this.m_UpFrame != null)
                return m_UpFrame.GetStructFrame(name);
            Utils.Assert("No struct definition:" + name);
            return new Frame();//avoid compiler warning
        }
        public bool HasStructFrame(string name)
        {
            foreach (var entry in m_StructFrames)
                if (entry.Key == name)
                    return true;
            if (m_UpFrame != null)
                return m_UpFrame.HasStructFrame(name);
            return false;
        }

        public int AddLambdaFrame(Frame frame)
        {
            Frame rootFrame = this;
            //lambda frame save to rootframe
            if (rootFrame.m_UpFrame != null)
            {
                while (rootFrame.m_UpFrame != null)
                    rootFrame = rootFrame.m_UpFrame;
            }
            rootFrame.m_LambdaFrames.Add(frame);
            return rootFrame.m_LambdaFrames.Count - 1;
        }
        public Frame GetLambdaFrame(int idx)
        {
            if (m_UpFrame != null)
            {
                Frame rootFrame = this;
                while (rootFrame.m_UpFrame != null)
                    rootFrame = rootFrame.m_UpFrame;
                return rootFrame.GetLambdaFrame(idx);
            }
            else if (idx >= 0 || idx < m_LambdaFrames.Count)
                return m_LambdaFrames[idx];
            else return null;
        }
        public bool HasLambdaFrame(int idx)
        {
            if (m_UpFrame != null)
            {
                Frame rootFrame = this;
                while (rootFrame.m_UpFrame != null)
                    rootFrame = rootFrame.m_UpFrame;
                return rootFrame.HasLambdaFrame(idx);
            }
            else if (idx >= 0 || idx < m_LambdaFrames.Count)
                return true;
            else return false;
        }

        public string Stringify(int depth = 0)
        {
            string interval = "";
            for (var i = 0; i < depth; ++i)
                interval += "\t";

            string result = "";

            foreach (var entry in m_StructFrames)
            {
                result += interval + "Frame " + entry.Key + ":\n";
                result += entry.Value.Stringify(depth + 1);
            }

            foreach (var entry in m_FunctionFrames)
            {
                result += interval + "Frame " + entry.Key + ":\n";
                result += entry.Value.Stringify(depth + 1);
            }

            for (var i = 0; i < m_LambdaFrames.Count; ++i)
            {
                result += interval + "Frame " + i.ToString() + ":\n";
                result += m_LambdaFrames[i].Stringify(depth + 1);
            }

            result += interval + "OpCodes:\n";

            for (var i = 0; i < m_Codes.Count; ++i)
            {
                switch (m_Codes[i])
                {
                    case (int)OpCode.OP_RETURN:
                        result += interval + "\t" + string.Format("{0:D8}", i) + "     " + "OP_RETURN\n";
                        break;
                    case (int)OpCode.OP_NEW_NUM:
                        result += interval + "\t" + string.Format("{0:D8}", i) + "     " + "OP_NEW_NUM" + "     " + m_Nums[m_Codes[++i]] + "\n";
                        break;
                    case (int)OpCode.OP_NEW_STR:
                        result += interval + "\t" + string.Format("{0:D8}", i) + "     " + "OP_NEW_STR" + "     " + m_Strings[m_Codes[++i]] + "\n";
                        break;
                    case (int)OpCode.OP_NEW_TRUE:
                        result += interval + "\t" + string.Format("{0:D8}", i) + "     " + "OP_NEW_TRUE\n";
                        break;
                    case (int)OpCode.OP_NEW_FALSE:
                        result += interval + "\t" + string.Format("{0:D8}", i) + "     " + "OP_NEW_FALSE\n";
                        break;
                    case (int)OpCode.OP_NEW_NIL:
                        result += interval + "\t" + string.Format("{0:D8}", i) + "     " + "OP_NEW_NIL\n";
                        break;
                    case (int)OpCode.OP_NEW_STRUCT:
                        result += interval + "\t" + string.Format("{0:D8}", i) + "     " + "OP_NEW_STRUCT" + "     " + m_Strings[m_Codes[++i]] + "\n";
                        break;
                    case (int)OpCode.OP_NEW_LAMBDA:
                        result += interval + "\t" + string.Format("{0:D8}", i) + "     " + "OP_NEW_LAMBDA" + "     " + m_Nums[m_Codes[++i]] + "\n";
                        break;
                    case (int)OpCode.OP_NEG:
                        result += interval + "\t" + string.Format("{0:D8}", i) + "     " + "OP_NEG\n";
                        break;
                    case (int)OpCode.OP_ADD:
                        result += interval + "\t" + string.Format("{0:D8}", i) + "     " + "OP_ADD\n";
                        break;
                    case (int)OpCode.OP_SUB:
                        result += interval + "\t" + string.Format("{0:D8}", i) + "     " + "OP_SUB\n";
                        break;
                    case (int)OpCode.OP_MUL:
                        result += interval + "\t" + string.Format("{0:D8}", i) + "     " + "OP_MUL\n";
                        break;
                    case (int)OpCode.OP_DIV:
                        result += interval + "\t" + string.Format("{0:D8}", i) + "     " + "OP_DIV\n";
                        break;
                    case (int)OpCode.OP_GREATER:
                        result += interval + "\t" + string.Format("{0:D8}", i) + "     " + "OP_GREATER\n";
                        break;
                    case (int)OpCode.OP_LESS:
                        result += interval + "\t" + string.Format("{0:D8}", i) + "     " + "OP_LESS\n";
                        break;
                    case (int)OpCode.OP_GREATER_EQUAL:
                        result += interval + "\t" + string.Format("{0:D8}", i) + "     " + "OP_GREATER_EQUAL\n";
                        break;
                    case (int)OpCode.OP_LESS_EQUAL:
                        result += interval + "\t" + string.Format("{0:D8}", i) + "     " + "OP_LESS_EQUAL\n";
                        break;
                    case (int)OpCode.OP_EQUAL:
                        result += interval + "\t" + string.Format("{0:D8}", i) + "     " + "OP_EQUAL\n";
                        break;
                    case (int)OpCode.OP_NOT:
                        result += interval + "\t" + string.Format("{0:D8}", i) + "     " + "OP_NOT\n";
                        break;
                    case (int)OpCode.OP_NOT_EQUAL:
                        result += interval + "\t" + string.Format("{0:D8}", i) + "     " + "OP_NOT_EQUAL\n";
                        break;
                    case (int)OpCode.OP_AND:
                        result += interval + "\t" + string.Format("{0:D8}", i) + "     " + "OP_AND\n";
                        break;
                    case (int)OpCode.OP_OR:
                        result += interval + "\t" + string.Format("{0:D8}", i) + "     " + "OP_OR\n";
                        break;
                    case (int)OpCode.OP_GET_VAR:
                        result += interval + "\t" + string.Format("{0:D8}", i) + "     " + "OP_GET_VAR" + "     " + m_Strings[m_Codes[++i]] + "\n";
                        break;
                    case (int)OpCode.OP_DEFINE_VAR:
                        result += interval + "\t" + string.Format("{0:D8}", i) + "     " + "OP_DEFINE_VAR" + "     " + m_Strings[m_Codes[++i]] + "\n";
                        break;
                    case (int)OpCode.OP_SET_VAR:
                        result += interval + "\t" + string.Format("{0:D8}", i) + "     " + "OP_SET_VAR" + "     " + m_Strings[m_Codes[++i]] + "\n";
                        break;
                    case (int)OpCode.OP_NEW_ARRAY:
                        result += interval + "\t" + string.Format("{0:D8}", i) + "     " + "OP_NEW_ARRAY" + "     " + m_Nums[m_Codes[++i]] + "\n";
                        break;
                    case (int)OpCode.OP_GET_INDEX_VAR:
                        result += interval + "\t" + string.Format("{0:D8}", i) + "     " + "OP_GET_INDEX_VAR\n";
                        break;
                    case (int)OpCode.OP_SET_INDEX_VAR:
                        result += interval + "\t" + string.Format("{0:D8}", i) + "     " + "OP_SET_INDEX_VAR\n";
                        break;
                    case (int)OpCode.OP_GET_STRUCT_VAR:
                        result += interval + "\t" + string.Format("{0:D8}", i) + "     " + "OP_GET_STRUCT_VAR" + "     " + m_Strings[m_Codes[++i]] + "\n";
                        break;
                    case (int)OpCode.OP_SET_STRUCT_VAR:
                        result += interval + "\t" + string.Format("{0:D8}", i) + "     " + "OP_SET_STRUCT_VAR" + "     " + m_Strings[m_Codes[++i]] + "\n";
                        break;
                    case (int)OpCode.OP_ENTER_SCOPE:
                        result += interval + "\t" + string.Format("{0:D8}", i) + "     " + "OP_ENTER_SCOPE\n";
                        break;
                    case (int)OpCode.OP_EXIT_SCOPE:
                        result += interval + "\t" + string.Format("{0:D8}", i) + "     " + "OP_EXIT_SCOPE\n";
                        break;
                    case (int)OpCode.OP_JUMP:
                        result += interval + "\t" + string.Format("{0:D8}", i) + "     " + "OP_JUMP" + "     " + m_Nums[m_Codes[++i]] + "\n";
                        break;
                    case (int)OpCode.OP_JUMP_IF_FALSE:
                        result += interval + "\t" + string.Format("{0:D8}", i) + "     " + "OP_JUMP_IF_FALSE" + "     " + m_Nums[m_Codes[++i]] + "\n";
                        break;
                    case (int)OpCode.OP_FUNCTION_CALL:
                        result += interval + "\t" + string.Format("{0:D8}", i) + "     " + "OP_FUNCTION_CALL" + "     " + m_Strings[m_Codes[++i]] + "\n";
                        break;
                    case (int)OpCode.OP_STRUCT_LAMBDA_CALL:
                        result += interval + "\t" + string.Format("{0:D8}", i) + "     " + "OP_STRUCT_LAMBDA_CALL" + "     " + m_Strings[m_Codes[++i]] + "\n";
                        break;
                    case (int)OpCode.OP_REF:
                        result += interval + "\t" + string.Format("{0:D8}", i) + "     " + "OP_REF" + "     " + m_Strings[m_Codes[++i]] + "\n";
                        break;
                    default:
                        result += interval + "\t" + string.Format("{0:D8}", i) + "     " + "OP_UNKNOWN\n";
                        break;
                }
            }

            return result;
        }

        public void Clear()
        {
            m_Codes = new List<int>();
            m_Nums = new List<double>();
            m_Strings = new List<string>();
            m_LambdaFrames = new List<Frame>();
            m_FunctionFrames = new Dictionary<string, Frame>();
            m_StructFrames = new Dictionary<string, Frame>();
            m_UpFrame = null;
        }

        public List<int> m_Codes;
        public List<double> m_Nums;
        public List<string> m_Strings;
        public List<Frame> m_LambdaFrames;
        public Dictionary<string, Frame> m_FunctionFrames;
        public Dictionary<string, Frame> m_StructFrames;
        public Frame m_UpFrame;
    }
}