from enum import IntEnum
from Object import *

class OpCode(IntEnum):
    OP_CONSTANT=0,
    OP_ADD=1,
    OP_SUB=2,
    OP_MUL=3,
    OP_DIV=4,
    OP_EQUAL=5,
    OP_GREATER=6,
    OP_LESS=7,
    OP_NOT=8,
    OP_MINUS=9,
    OP_AND=10,
    OP_OR=11,
    OP_BIT_AND=12,
    OP_BIT_OR=13,
    OP_BIT_NOT=14,
    OP_BIT_XOR=15,
    OP_JUMP_IF_FALSE=16,
    OP_JUMP=17,
    OP_SET_GLOBAL=18,
    OP_GET_GLOBAL=19,
    OP_SET_LOCAL=20,
    OP_GET_LOCAL=21,
    OP_ARRAY=22,
    OP_INDEX=23,
    OP_FUNCTION_CALL=24,
    OP_RETURN=25,
    OP_GET_BUILTIN=26,
    OP_STRUCT=27,
    OP_GET_STRUCT=28,
    OP_SET_STRUCT=29,
    OP_REF_GLOBAL=30,
    OP_REF_LOCAL=31,
    OP_REF_INDEX_GLOBAL=32,
    OP_REF_INDEX_LOCAL=33,
    OP_SP_OFFSET=34,

class Chunk:
    opCodes:list[int]
    constants:list[Object]


    def __init__(self,opCodes:list[int],constants:list[Object]) -> None:
        self.constants=constants
        self.opCodes=opCodes

    def stringify(self)->None:
        for i in range(0,len(self.constants)):
            if self.constants[i].type==ObjectType.FUNCTION:
                print(self.constants[i])
                self.__opcode_stringify(self.constants[i].opCodes)
                print()

        self.__opcode_stringify(self.opCodes)

    def __opcode_stringify(self,opcodes:list[int])->None:
        i = 0
        while i < (len(opcodes)):
            if opcodes[i]==OpCode.OP_CONSTANT:
                pos=opcodes[i+1]
                print("%8d    OP_CONSTANT    %d    '%s'" % (i,pos,self.constants[pos]))
                i=i+1
            elif opcodes[i]==OpCode.OP_ADD:
                print("%8d    OP_ADD" % (i))
            elif opcodes[i]==OpCode.OP_SUB:
                print("%8d    OP_SUB" % (i))
            elif opcodes[i]==OpCode.OP_MUL:
                print("%8d    OP_MUL" % (i))
            elif opcodes[i]==OpCode.OP_DIV:
                print("%8d    OP_DIV" % (i))
            elif opcodes[i]==OpCode.OP_LESS:
                print("%8d    OP_LESS" % (i))
            elif opcodes[i]==OpCode.OP_GREATER:
                print("%8d    OP_GREATER" % (i))
            elif opcodes[i]==OpCode.OP_MINUS:
                print("%8d    OP_MINUS" % (i))
            elif opcodes[i]==OpCode.OP_EQUAL:
                print("%8d    OP_EQUAL" % (i))
            elif opcodes[i]==OpCode.OP_AND:
                print("%8d    OP_AND" % (i))
            elif opcodes[i]==OpCode.OP_OR:
                print("%8d    OP_OR" % (i))
            elif opcodes[i]==OpCode.OP_NOT:
                print("%8d    OP_NOT" % (i))
            elif opcodes[i]==OpCode.OP_BIT_AND:
                print("%8d    OP_BIT_AND" % (i))
            elif opcodes[i]==OpCode.OP_BIT_OR:
                print("%8d    OP_BIT_OR" % (i))
            elif opcodes[i]==OpCode.OP_BIT_XOR:
                print("%8d    OP_BIT_XOR" % (i))
            elif opcodes[i]==OpCode.OP_BIT_NOT:
                print("%8d    OP_BIT_NOT" % (i))
            elif opcodes[i]==OpCode.OP_ARRAY:
                count=opcodes[i+1]
                print("%8d    OP_ARRAY    %d" % (i,count))
                i=i+1
            elif opcodes[i]==OpCode.OP_INDEX:
                print("%8d    OP_INDEX" % (i))
            elif opcodes[i]==OpCode.OP_JUMP:
                address=opcodes[i+1]
                print("%8d    OP_JUMP    %d" % (i,address))
                i=i+1
            elif opcodes[i]==OpCode.OP_JUMP_IF_FALSE:
                address=opcodes[i+1]
                print("%8d    OP_JUMP_IF_FALSE    %d" % (i,address))
                i=i+1
            elif opcodes[i]==OpCode.OP_RETURN:
                count=opcodes[i+1]
                print("%8d    OP_RETURN    %d" % (i,count))
                i=i+1
            elif opcodes[i]==OpCode.OP_SET_GLOBAL:
                pos=opcodes[i+1]
                print("%8d    OP_SET_GLOBAL    %d" % (i,pos))
                i=i+1
            elif opcodes[i]==OpCode.OP_GET_GLOBAL:
                pos=opcodes[i+1]
                print("%8d    OP_GET_GLOBAL    %d" % (i,pos))
                i=i+1
            elif opcodes[i]==OpCode.OP_SET_LOCAL:
                scopeDepth=opcodes[i+1]
                index=opcodes[i+2]
                print("%8d    OP_SET_LOCAL    %d    %d" % (i,scopeDepth,index))
                i=i+2
            elif opcodes[i]==OpCode.OP_GET_LOCAL:
                scopeDepth=opcodes[i+1]
                index=opcodes[i+2]
                print("%8d    OP_GET_LOCAL    %d    %d" % (i,scopeDepth,index))
                i=i+2
            elif opcodes[i]==OpCode.OP_FUNCTION_CALL:
                argCount=opcodes[i+1]
                print("%8d    OP_FUNCTION_CALL    %d" % (i,argCount))
                i=i+1
            elif opcodes[i]==OpCode.OP_GET_BUILTIN:
                print("%8d    OP_GET_BUILTIN" % i)
            elif opcodes[i]==OpCode.OP_STRUCT:
                memberCount=opcodes[i+1]
                print("%8d    OP_STRUCT    %d" % (i,memberCount))
                i+=1
            elif opcodes[i]==OpCode.OP_GET_STRUCT:
                print("%8d    OP_GET_STRUCT" % (i))
            elif opcodes[i]==OpCode.OP_SET_STRUCT:
                print("%8d    OP_SET_STRUCT" % (i))
            elif opcodes[i]==OpCode.OP_REF_GLOBAL:
                idx=opcodes[i+1]
                print("%8d    OP_REF_GLOBAL    %d" % (i,idx))
                i=i+1
            elif opcodes[i]==OpCode.OP_REF_LOCAL:
                scopeDepth=opcodes[i+1]
                index=opcodes[i+2]
                print("%8d    OP_REF_LOCAL    %d    %d" % (i,scopeDepth,index))
                i=i+2
            elif opcodes[i]==OpCode.OP_REF_INDEX_GLOBAL:
                pos=opcodes[i+1]
                print("%8d    OP_REF_INDEX_GLOBAL    %d" % (i,pos))
                i=i+1
            elif opcodes[i]==OpCode.OP_REF_INDEX_LOCAL:
                scopeDepth=opcodes[i+1]
                index=opcodes[i+2]
                print("%8d    OP_REF_INDEX_LOCAL    %d    %d" % (i,scopeDepth,index))
                i=i+2
            elif opcodes[i]==OpCode.OP_SP_OFFSET:
                offset=opcodes[i+1]
                print("%8d    OP_SP_OFFSET    %d" % (i,offset))
                i=i+1
            i=i+1