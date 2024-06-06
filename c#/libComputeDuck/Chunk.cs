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
        OP_GET_BUILTIN_FUNCTION,
        OP_GET_BUILTIN_VARIABLE,
        OP_STRUCT,
        OP_GET_STRUCT,
        OP_SET_STRUCT,
        OP_REF_GLOBAL,
        OP_REF_LOCAL,
        OP_REF_INDEX_GLOBAL,
        OP_REF_INDEX_LOCAL,
        OP_SP_OFFSET,
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
                    Console.WriteLine("=======constant idx:{0}    {1}=======", i, constant.Stringify());
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
                            Console.WriteLine("{0}    OP_CONSTANT    {1}    '{2}'", i.ToString().PadLeft(8), pos, constants[pos].Stringify());
                            ++i;
                            break;
                        }
                    case (int)OpCode.OP_ADD:
                        {
                            Console.WriteLine("{0}    OP_ADD", i.ToString().PadLeft(8));
                            break;
                        }
                    case (int)OpCode.OP_SUB:
                        {
                            Console.WriteLine("{0}    OP_SUB", i.ToString().PadLeft(8));
                            break;
                        }
                    case (int)OpCode.OP_MUL:
                        {
                            Console.WriteLine("{0}    OP_MUL", i.ToString().PadLeft(8));
                            break;
                        }
                    case (int)OpCode.OP_DIV:
                        {
                            Console.WriteLine("{0}    OP_DIV", i.ToString().PadLeft(8));
                            break;
                        }
                    case (int)OpCode.OP_LESS:
                        {
                            Console.WriteLine("{0}    OP_LESS", i.ToString().PadLeft(8));
                            break;
                        }
                    case (int)OpCode.OP_GREATER:
                        {
                            Console.WriteLine("{0}    OP_GREATER", i.ToString().PadLeft(8));
                            break;
                        }
                    case (int)OpCode.OP_NOT:
                        {
                            Console.WriteLine("{0}    OP_NOT", i.ToString().PadLeft(8));
                            break;
                        }
                    case (int)OpCode.OP_MINUS:
                        {
                            Console.WriteLine("{0}    OP_MINUS", i.ToString().PadLeft(8));
                            break;
                        }
                    case (int)OpCode.OP_EQUAL:
                        {
                            Console.WriteLine("{0}    OP_EQUAL", i.ToString().PadLeft(8));
                            break;
                        }
                    case (int)OpCode.OP_ARRAY:
                        {
                            var count = opCodes[i + 1];
                            Console.WriteLine("{0}    OP_CONSTANT    {1}", i.ToString().PadLeft(8), count);
                            ++i;
                            break;
                        }
                    case (int)OpCode.OP_INDEX:
                        {
                            Console.WriteLine("{0}    OP_INDEX", i.ToString().PadLeft(8));
                            break;
                        }
                    case (int)OpCode.OP_JUMP:
                        {
                            var address = opCodes[i + 1];
                            Console.WriteLine("{0}    OP_JUMP    {1}", i.ToString().PadLeft(8), address);
                            ++i;
                            break;
                        }
                    case (int)OpCode.OP_JUMP_IF_FALSE:
                        {
                            var address = opCodes[i + 1];
                            Console.WriteLine("{0}    OP_JUMP_IF_FALSE    {1}", i.ToString().PadLeft(8), address);
                            ++i;
                            break;
                        }
                    case (int)OpCode.OP_RETURN:
                        {
                            var count = opCodes[i + 1];
                            Console.WriteLine("{0}    OP_RETURN    {1}", i.ToString().PadLeft(8), count);
                            ++i;
                            break;
                        }
                    case (int)OpCode.OP_SET_GLOBAL:
                        {
                            var pos = opCodes[i + 1];
                            Console.WriteLine("{0}    OP_SET_GLOBAL    {1}", i.ToString().PadLeft(8), pos);
                            ++i;
                            break;
                        }
                    case (int)OpCode.OP_GET_GLOBAL:
                        {
                            var pos = opCodes[i + 1];
                            Console.WriteLine("{0}    OP_GET_GLOBAL    {1}", i.ToString().PadLeft(8), pos);
                            ++i;
                            break;
                        }
                    case (int)OpCode.OP_SET_LOCAL:
                        {
                            var isInUpScope = opCodes[i + 1];
                            var scopeDepth = opCodes[i + 2];
                            var index = opCodes[i + 3];
                            Console.WriteLine("{0}    OP_SET_LOCAL    {1}    {2}    {3}", i.ToString().PadLeft(8), isInUpScope, scopeDepth, index);
                            i += 3;
                            break;
                        }
                    case (int)OpCode.OP_GET_LOCAL:
                        {
                            var isInUpScope = opCodes[i + 1];
                            var scopeDepth = opCodes[i + 2];
                            var index = opCodes[i + 3];
                            Console.WriteLine("{0}    OP_GET_LOCAL    {1}    {2}    {3}", i.ToString().PadLeft(8), isInUpScope, scopeDepth, index);
                            i += 3;
                            break;
                        }
                    case (int)OpCode.OP_FUNCTION_CALL:
                        {
                            var argCount = opCodes[i + 1];
                            Console.WriteLine("{0}    OP_FUNCTION_CALL    {1}", i.ToString().PadLeft(8), argCount);
                            ++i;
                            break;
                        }
                    case (int)OpCode.OP_GET_BUILTIN_FUNCTION:
                        {
                            var builtinIdx = opCodes[i + 1];
                            Console.WriteLine("{0}    OP_GET_BUILTIN_FUNCTION    {1}", i.ToString().PadLeft(8), builtinIdx);
                            ++i;
                            break;
                        }
                    case (int)OpCode.OP_GET_BUILTIN_VARIABLE:
                        {
                            var builtinIdx = opCodes[i + 1];
                            Console.WriteLine("{0}    OP_GET_BUILTIN_VARIABLE    {1}", i.ToString().PadLeft(8), builtinIdx);
                            ++i;
                            break;
                        }
                    case (int)OpCode.OP_STRUCT:
                        {
                            var memberCount = opCodes[i + 1];
                            Console.WriteLine("{0}    OP_STRUCT    {1}", i.ToString().PadLeft(8), memberCount);
                            ++i;
                            break;
                        }
                    case (int)OpCode.OP_GET_STRUCT:
                        {
                            Console.WriteLine("{0}    OP_GET_STRUCT", i.ToString().PadLeft(8));
                            break;
                        }
                    case (int)OpCode.OP_SET_STRUCT:
                        {
                            Console.WriteLine("{0}    OP_SET_STRUCT", i.ToString().PadLeft(8));
                            break;
                        }
                    case (int)OpCode.OP_REF_GLOBAL:
                        {
                            var idx = opCodes[i + 1];
                            Console.WriteLine("{0}    OP_STRUCT    {1}", i.ToString().PadLeft(8), idx);
                            ++i;
                            break;
                        }
                    case (int)OpCode.OP_REF_LOCAL:
                        {
                            var isInUpScope = opCodes[i + 1];
                            var scopeDepth = opCodes[i + 2];
                            var index = opCodes[i + 3];
                            Console.WriteLine("{0}    OP_REF_LOCAL    {1}    {2}    {3}", i.ToString().PadLeft(8), isInUpScope, scopeDepth, index);
                            i += 3;
                            break;
                        }
                    case (int)OpCode.OP_REF_INDEX_GLOBAL:
                        {
                            var pos = opCodes[i + 1];
                            Console.WriteLine("{0}    OP_REF_INDEX_GLOBAL    {1}", i.ToString().PadLeft(8), pos);
                            ++i;
                            break;
                        }
                    case (int)OpCode.OP_REF_INDEX_LOCAL:
                        {
                            var isInUpScope = opCodes[i + 1];
                            var scopeDepth = opCodes[i + 2];
                            var index = opCodes[i + 3];
                            Console.WriteLine("{0}    OP_REF_INDEX_LOCAL    {1}    {2}    {3}", i.ToString().PadLeft(8), isInUpScope, scopeDepth, index);
                            i += 3;
                            break;
                        }
                    case (int)OpCode.OP_SP_OFFSET:
                        {
                            var offset = opCodes[i + 1];
                            Console.WriteLine("{0}    OP_SP_OFFSET    {1}", i.ToString().PadLeft(8), offset);
                            ++i;
                            break;
                        }
                    default:
                        break;
                }
            }
        }
    }

}