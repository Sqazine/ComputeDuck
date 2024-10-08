using System;
using System.Collections.Generic;

namespace ComputeDuck
{
    using OpCodes = List<int>;
    public enum OpCode
    {
        OP_CONSTANT = 0,
        OP_ADD,
        OP_SUB,
        OP_MUL,
        OP_DIV,
        OP_EQUAL,
        OP_GREATER,
        OP_LESS,
        OP_NOT,
        OP_MINUS,
        OP_AND,
        OP_OR,
        OP_BIT_AND,
        OP_BIT_OR,
        OP_BIT_NOT,
        OP_BIT_XOR,
        OP_JUMP_IF_FALSE,
        OP_JUMP,
        OP_DEF_GLOBAL,
        OP_SET_GLOBAL,
        OP_GET_GLOBAL,
        OP_DEF_LOCAL,
        OP_SET_LOCAL,
        OP_GET_LOCAL,
        OP_ARRAY,
        OP_GET_INDEX,
        OP_SET_INDEX,
        OP_FUNCTION_CALL,
        OP_RETURN,
        OP_GET_BUILTIN,
        OP_STRUCT,
        OP_GET_STRUCT,
        OP_SET_STRUCT,
        OP_REF_GLOBAL,
        OP_REF_LOCAL,
        OP_REF_INDEX_GLOBAL,
        OP_REF_INDEX_LOCAL,
        OP_SP_OFFSET,
        OP_DLL_IMPORT,
    };

    public class Chunk
    {
        public OpCodes opCodes;
        public List<Object> constants;

        public Chunk()
        {
            this.opCodes = new OpCodes();
            this.constants = new List<Object>();
        }
        public Chunk(OpCodes opCodes, List<Object> constants)
        {
            this.opCodes = opCodes;
            this.constants = constants;
        }

        public override string ToString()
        {
            string result = OpCodeStringify(opCodes);
            for (int i = 0; i < constants.Count; ++i)
                if (constants[i].type == ObjectType.FUNCTION)
                    result += ((FunctionObject)constants[i]).ToStringWithChunk();

            return result;
        }

        private string OpCodeStringify(OpCodes opCodes)
        {
            string result = "";
            for (int i = 0; i < opCodes.Count; ++i)
            {
                switch (opCodes[i])
                {
                    case (int)OpCode.OP_CONSTANT:
                        result += string.Format("{0}\tOP_CONSTANT\t'{1}'\t\n", i.ToString().PadLeft(8), constants[opCodes[++i]].ToString());
                        break;
                    case (int)OpCode.OP_ADD:
                        result += string.Format("{0}\tOP_ADD\n", i.ToString().PadLeft(8));
                        break;
                    case (int)OpCode.OP_SUB:
                        result += string.Format("{0}\tOP_SUB\n", i.ToString().PadLeft(8));
                        break;
                    case (int)OpCode.OP_MUL:
                        result += string.Format("{0}\tOP_MUL\n", i.ToString().PadLeft(8));
                        break;
                    case (int)OpCode.OP_DIV:
                        result += string.Format("{0}\tOP_DIV\n", i.ToString().PadLeft(8));
                        break;
                    case (int)OpCode.OP_LESS:
                        result += string.Format("{0}\tOP_LESS\n", i.ToString().PadLeft(8));
                        break;
                    case (int)OpCode.OP_GREATER:
                        result += string.Format("{0}\tOP_GREATER\n", i.ToString().PadLeft(8));
                        break;
                    case (int)OpCode.OP_NOT:
                        result += string.Format("{0}\tOP_NOT\n", i.ToString().PadLeft(8));
                        break;
                    case (int)OpCode.OP_MINUS:
                        result += string.Format("{0}\tOP_MINUS\n", i.ToString().PadLeft(8));
                        break;
                    case (int)OpCode.OP_EQUAL:
                        result += string.Format("{0}\tOP_EQUAL\n", i.ToString().PadLeft(8));
                        break;
                    case (int)OpCode.OP_AND:
                        result += string.Format("{0}\tOP_AND\n", i.ToString().PadLeft(8));
                        break;
                    case (int)OpCode.OP_OR:
                        result += string.Format("{0}\tOP_OR\n", i.ToString().PadLeft(8));
                        break;
                    case (int)OpCode.OP_BIT_AND:
                        result += string.Format("{0}\tOP_BIT_AND\n", i.ToString().PadLeft(8));
                        break;
                    case (int)OpCode.OP_BIT_OR:
                        result += string.Format("{0}\tOP_BIT_OR\n", i.ToString().PadLeft(8));
                        break;
                    case (int)OpCode.OP_BIT_XOR:
                        result += string.Format("{0}\tOP_BIT_XOR\n", i.ToString().PadLeft(8));
                        break;
                    case (int)OpCode.OP_BIT_NOT:
                        result += string.Format("{0}\tOP_BIT_NOT\n", i.ToString().PadLeft(8));
                        break;
                    case (int)OpCode.OP_ARRAY:
                        result += string.Format("{0}\tOP_ARRAY\t{1}\n", i.ToString().PadLeft(8), opCodes[++i]);
                        break;
                    case (int)OpCode.OP_GET_INDEX:
                        result += string.Format("{0}\tOP_GET_INDEX\n", i.ToString().PadLeft(8));
                        break;
                    case (int)OpCode.OP_SET_INDEX:
                        result += string.Format("{0}\tOP_SET_INDEX\n", i.ToString().PadLeft(8));
                        break;
                    case (int)OpCode.OP_JUMP:
                        result += string.Format("{0}\tOP_JUMP\t{1}\n", i.ToString().PadLeft(8), opCodes[++i]);
                        break;
                    case (int)OpCode.OP_JUMP_IF_FALSE:
                        result += string.Format("{0}\tOP_JUMP_IF_FALSE\t{1}\n", i.ToString().PadLeft(8), opCodes[++i]);
                        break;
                    case (int)OpCode.OP_RETURN:
                        result += string.Format("{0}\tOP_RETURN\t{1}\n", i.ToString().PadLeft(8), opCodes[++i]);
                        break;
                    case (int)OpCode.OP_DEF_GLOBAL:
                        result += string.Format("{0}\tOP_DEF_GLOBAL\t{1}\n", i.ToString().PadLeft(8), opCodes[++i]);
                        break;
                    case (int)OpCode.OP_SET_GLOBAL:
                        result += string.Format("{0}\tOP_SET_GLOBAL\t{1}\n", i.ToString().PadLeft(8), opCodes[++i]);
                        break;
                    case (int)OpCode.OP_GET_GLOBAL:
                        result += string.Format("{0}\tOP_GET_GLOBAL\t{1}\n", i.ToString().PadLeft(8), opCodes[++i]);
                        break;
                    case (int)OpCode.OP_DEF_LOCAL:
                        result += string.Format("{0}\tOP_DEF_LOCAL\t{1}\t{2}\n", i.ToString().PadLeft(8), opCodes[++i], opCodes[++i]);
                        break;
                    case (int)OpCode.OP_SET_LOCAL:
                        result += string.Format("{0}\tOP_SET_LOCAL\t{1}\t{2}\t{3}\n", i.ToString().PadLeft(8), opCodes[++i], opCodes[++i], opCodes[++i]);
                        break;
                    case (int)OpCode.OP_GET_LOCAL:
                        result += string.Format("{0}\tOP_GET_LOCAL\t{1}\t{2}\t{3}\n", i.ToString().PadLeft(8), opCodes[++i], opCodes[++i], opCodes[++i]);
                        break;
                    case (int)OpCode.OP_FUNCTION_CALL:
                        result += string.Format("{0}\tOP_FUNCTION_CALL\t{1}\n", i.ToString().PadLeft(8), opCodes[++i]);
                        break;
                    case (int)OpCode.OP_GET_BUILTIN:
                        result += string.Format("{0}\tOP_GET_BUILTIN\n", i.ToString().PadLeft(8));
                        break;
                    case (int)OpCode.OP_STRUCT:
                        result += string.Format("{0}\tOP_STRUCT\t{1}\n", i.ToString().PadLeft(8), opCodes[++i]);
                        break;
                    case (int)OpCode.OP_GET_STRUCT:
                        result += string.Format("{0}\tOP_GET_STRUCT\n", i.ToString().PadLeft(8));
                        break;
                    case (int)OpCode.OP_SET_STRUCT:
                        result += string.Format("{0}\tOP_SET_STRUCT\n", i.ToString().PadLeft(8));
                        break;
                    case (int)OpCode.OP_REF_GLOBAL:
                        result += string.Format("{0}\tOP_REF_GLOBAL\t{1}\n", i.ToString().PadLeft(8), opCodes[++i]);
                        break;
                    case (int)OpCode.OP_REF_LOCAL:
                        result += string.Format("{0}\tOP_REF_LOCAL\t{1}\t{2}\t{3}\n", i.ToString().PadLeft(8), opCodes[++i], opCodes[++i], opCodes[++i]);
                        break;
                    case (int)OpCode.OP_REF_INDEX_GLOBAL:
                        result += string.Format("{0}\tOP_REF_INDEX_GLOBAL\t{1}\n", i.ToString().PadLeft(8), opCodes[++i]);
                        break;
                    case (int)OpCode.OP_REF_INDEX_LOCAL:
                        result += string.Format("{0}\tOP_REF_INDEX_LOCAL\t{1}\t{2}\t{3}\n", i.ToString().PadLeft(8), opCodes[++i], opCodes[++i], opCodes[++i]);
                        break;
                    case (int)OpCode.OP_SP_OFFSET:
                        result += string.Format("{0}\tOP_SP_OFFSET\t{1}\n", i.ToString().PadLeft(8), opCodes[++i]);
                        break;
                    case (int)OpCode.OP_DLL_IMPORT:
                        result += string.Format("{0}\tOP_DLL_IMPORT\n", i.ToString().PadLeft(8));
                        break;
                    default:
                        break;
                }
            }

            return result;
        }

    }

}