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
        OP_SET_GLOBAL,
        OP_GET_GLOBAL,
        OP_SET_LOCAL,
        OP_GET_LOCAL,
        OP_ARRAY,
        OP_INDEX,
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
        public Chunk(OpCodes opCodes, List<Object> constants)
        {
            this.opCodes = opCodes;
            this.constants = constants;
        }

        public void Stringify()
        {
            for (int i = 0; i < constants.Count; ++i)
            {
                var constant = constants[i];
                if (constant.type == ObjectType.FUNCTION)
                {
                    Console.WriteLine("{0}", constant.ToString());
                    OpCodeStringify(((FunctionObject)constant).opCodes);
                    Console.WriteLine();
                }
            }

            OpCodeStringify(opCodes);
        }

        private void OpCodeStringify(OpCodes opCodes)
        {
            for (int i = 0; i < opCodes.Count; ++i)
            {
                switch (opCodes[i])
                {
                    case (int)OpCode.OP_CONSTANT:
                        {
                            var pos = opCodes[i + 1];
                            Console.WriteLine("{0}\tOP_CONSTANT\t{1}    '{2}'", i.ToString().PadLeft(8), pos, constants[pos].ToString());
                            ++i;
                            break;
                        }
                    case (int)OpCode.OP_ADD:
                        {
                            Console.WriteLine("{0}\tOP_ADD", i.ToString().PadLeft(8));
                            break;
                        }
                    case (int)OpCode.OP_SUB:
                        {
                            Console.WriteLine("{0}\tOP_SUB", i.ToString().PadLeft(8));
                            break;
                        }
                    case (int)OpCode.OP_MUL:
                        {
                            Console.WriteLine("{0}\tOP_MUL", i.ToString().PadLeft(8));
                            break;
                        }
                    case (int)OpCode.OP_DIV:
                        {
                            Console.WriteLine("{0}\tOP_DIV", i.ToString().PadLeft(8));
                            break;
                        }
                    case (int)OpCode.OP_LESS:
                        {
                            Console.WriteLine("{0}\tOP_LESS", i.ToString().PadLeft(8));
                            break;
                        }
                    case (int)OpCode.OP_GREATER:
                        {
                            Console.WriteLine("{0}\tOP_GREATER", i.ToString().PadLeft(8));
                            break;
                        }
                    case (int)OpCode.OP_NOT:
                        {
                            Console.WriteLine("{0}\tOP_NOT", i.ToString().PadLeft(8));
                            break;
                        }
                    case (int)OpCode.OP_MINUS:
                        {
                            Console.WriteLine("{0}\tOP_MINUS", i.ToString().PadLeft(8));
                            break;
                        }
                    case (int)OpCode.OP_EQUAL:
                        {
                            Console.WriteLine("{0}\tOP_EQUAL", i.ToString().PadLeft(8));
                            break;
                        }
                    case (int)OpCode.OP_AND:
                        {
                            Console.WriteLine("{0}\tOP_AND", i.ToString().PadLeft(8));
                            break;
                        }
                    case (int)OpCode.OP_OR:
                        {
                            Console.WriteLine("{0}\tOP_OR", i.ToString().PadLeft(8));
                            break;
                        }
                    case (int)OpCode.OP_BIT_AND:
                        {
                            Console.WriteLine("{0}\tOP_BIT_AND", i.ToString().PadLeft(8));
                            break;
                        }
                    case (int)OpCode.OP_BIT_OR:
                        {
                            Console.WriteLine("{0}\tOP_BIT_OR", i.ToString().PadLeft(8));
                            break;
                        }
                    case (int)OpCode.OP_BIT_XOR:
                        {
                            Console.WriteLine("{0}\tOP_BIT_XOR", i.ToString().PadLeft(8));
                            break;
                        }
                    case (int)OpCode.OP_BIT_NOT:
                        {
                            Console.WriteLine("{0}\tOP_BIT_NOT", i.ToString().PadLeft(8));
                            break;
                        }
                    case (int)OpCode.OP_ARRAY:
                        {
                            var count = opCodes[i + 1];
                            Console.WriteLine("{0}\tOP_ARRAY\t{1}", i.ToString().PadLeft(8), count);
                            ++i;
                            break;
                        }
                    case (int)OpCode.OP_INDEX:
                        {
                            Console.WriteLine("{0}\tOP_INDEX", i.ToString().PadLeft(8));
                            break;
                        }
                    case (int)OpCode.OP_JUMP:
                        {
                            var address = opCodes[i + 1];
                            Console.WriteLine("{0}\tOP_JUMP\t{1}", i.ToString().PadLeft(8), address);
                            ++i;
                            break;
                        }
                    case (int)OpCode.OP_JUMP_IF_FALSE:
                        {
                            var address = opCodes[i + 1];
                            Console.WriteLine("{0}\tOP_JUMP_IF_FALSE\t{1}", i.ToString().PadLeft(8), address);
                            ++i;
                            break;
                        }
                    case (int)OpCode.OP_RETURN:
                        {
                            var count = opCodes[i + 1];
                            Console.WriteLine("{0}\tOP_RETURN\t{1}", i.ToString().PadLeft(8), count);
                            ++i;
                            break;
                        }
                    case (int)OpCode.OP_SET_GLOBAL:
                        {
                            var pos = opCodes[i + 1];
                            Console.WriteLine("{0}\tOP_SET_GLOBAL\t{1}", i.ToString().PadLeft(8), pos);
                            ++i;
                            break;
                        }
                    case (int)OpCode.OP_GET_GLOBAL:
                        {
                            var pos = opCodes[i + 1];
                            Console.WriteLine("{0}\tOP_GET_GLOBAL\t{1}", i.ToString().PadLeft(8), pos);
                            ++i;
                            break;
                        }
                    case (int)OpCode.OP_SET_LOCAL:
                        {
                            var scopeDepth = opCodes[i + 1];
                            var index = opCodes[i + 2];
                            Console.WriteLine("{0}\tOP_SET_LOCAL\t{1}\t{2}", i.ToString().PadLeft(8), scopeDepth, index);
                            i += 2;
                            break;
                        }
                    case (int)OpCode.OP_GET_LOCAL:
                        {
                            var scopeDepth = opCodes[i + 1];
                            var index = opCodes[i + 2];
                            Console.WriteLine("{0}\tOP_GET_LOCAL\t{1}\t{2}", i.ToString().PadLeft(8), scopeDepth, index);
                            i += 2;
                            break;
                        }
                    case (int)OpCode.OP_FUNCTION_CALL:
                        {
                            var argCount = opCodes[i + 1];
                            Console.WriteLine("{0}\tOP_FUNCTION_CALL\t{1}", i.ToString().PadLeft(8), argCount);
                            ++i;
                            break;
                        }
                    case (int)OpCode.OP_GET_BUILTIN:
                        {
                            Console.WriteLine("{0}\tOP_GET_BUILTIN", i.ToString().PadLeft(8));
                            break;
                        }
                    case (int)OpCode.OP_STRUCT:
                        {
                            var memberCount = opCodes[i + 1];
                            Console.WriteLine("{0}\tOP_STRUCT\t{1}", i.ToString().PadLeft(8), memberCount);
                            ++i;
                            break;
                        }
                    case (int)OpCode.OP_GET_STRUCT:
                        {
                            Console.WriteLine("{0}\tOP_GET_STRUCT", i.ToString().PadLeft(8));
                            break;
                        }
                    case (int)OpCode.OP_SET_STRUCT:
                        {
                            Console.WriteLine("{0}\tOP_SET_STRUCT", i.ToString().PadLeft(8));
                            break;
                        }
                    case (int)OpCode.OP_REF_GLOBAL:
                        {
                            var idx = opCodes[i + 1];
                            Console.WriteLine("{0}\tOP_REF_GLOBAL\t{1}", i.ToString().PadLeft(8), idx);
                            ++i;
                            break;
                        }
                    case (int)OpCode.OP_REF_LOCAL:
                        {
                            var scopeDepth = opCodes[i + 1];
                            var index = opCodes[i + 2];
                            Console.WriteLine("{0}\tOP_REF_LOCAL\t{1}\t{2}", i.ToString().PadLeft(8), scopeDepth, index);
                            i += 2;
                            break;
                        }
                    case (int)OpCode.OP_REF_INDEX_GLOBAL:
                        {
                            var pos = opCodes[i + 1];
                            Console.WriteLine("{0}\tOP_REF_INDEX_GLOBAL\t{1}", i.ToString().PadLeft(8), pos);
                            ++i;
                            break;
                        }
                    case (int)OpCode.OP_REF_INDEX_LOCAL:
                        {
                            var scopeDepth = opCodes[i + 1];
                            var index = opCodes[i + 2];
                            Console.WriteLine("{0}\tOP_REF_INDEX_LOCAL\t{1}\t{2}", i.ToString().PadLeft(8), scopeDepth, index);
                            i += 2;
                            break;
                        }
                    case (int)OpCode.OP_SP_OFFSET:
                        {
                            var offset = opCodes[i + 1];
                            Console.WriteLine("{0}\tOP_SP_OFFSET\t{1}", i.ToString().PadLeft(8), offset);
                            ++i;
                            break;
                        }
                    case (int)OpCode.OP_DLL_IMPORT:
                        {
                            Console.WriteLine("{0}\tOP_DLL_IMPORT", i.ToString().PadLeft(8));
                            break;
                        }
                    default:
                        break;
                }
            }
        }
    }

}