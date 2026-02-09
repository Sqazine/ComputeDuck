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
    OP_DEF_GLOBAL=18,
    OP_SET_GLOBAL=19,
    OP_GET_GLOBAL=20,
    OP_DEF_LOCAL=21,
    OP_SET_LOCAL=22,
    OP_GET_LOCAL=23,
    OP_ARRAY=24,
    OP_GET_INDEX=25,
    OP_SET_INDEX=26,
    OP_FUNCTION_CALL=27,
    OP_RETURN=28,
    OP_GET_BUILTIN=29,
    OP_STRUCT=30,
    OP_GET_STRUCT=31,
    OP_SET_STRUCT=32,
    OP_REF_GLOBAL=33,
    OP_REF_LOCAL=34,
    OP_REF_INDEX_GLOBAL=35,
    OP_REF_INDEX_LOCAL=36,
    OP_DLL_IMPORT=37

class Chunk:
    opCodes:list[int]
    constants:list[Object]

    def __init__(self,opCodes:list[int]=[],constants:list[Object]=[]) -> None:
        if opCodes!=[]:
            self.opCodes=opCodes
        else:
            self.opCodes=[]
            
        if constants!=[]:
            self.constants=constants
        else:
            self.constants=[]

    def __str__(self)->str:
        result = self.__opcode_stringify(self.opCodes)
        for i in range(0,len(self.constants)):
            if self.constants[i].type==ObjectType.FUNCTION:
                result+=self.constants[i].str_with_chunk()

        return result

    def __opcode_stringify(self,opcodes:list[int])->str:
        result=""
        i = 0
        while i < (len(opcodes)):
            if opcodes[i]==OpCode.OP_CONSTANT:
                pos=opcodes[i+1]
                result+=("%8d\tOP_CONSTANT\t%d\t'%s'\n" % (i,pos,self.constants[pos]))
                i=i+1
            elif opcodes[i]==OpCode.OP_ADD:
                result+=("%8d\tOP_ADD\n" % (i))
            elif opcodes[i]==OpCode.OP_SUB:
                result+=("%8d\tOP_SUB\n" % (i))
            elif opcodes[i]==OpCode.OP_MUL:
                result+=("%8d\tOP_MUL\n" % (i))
            elif opcodes[i]==OpCode.OP_DIV:
                result+=("%8d\tOP_DIV\n" % (i))
            elif opcodes[i]==OpCode.OP_LESS:
                result+=("%8d\tOP_LESS\n" % (i))
            elif opcodes[i]==OpCode.OP_GREATER:
                result+=("%8d\tOP_GREATER\n" % (i))
            elif opcodes[i]==OpCode.OP_MINUS:
                result+=("%8d\tOP_MINUS\n" % (i))
            elif opcodes[i]==OpCode.OP_EQUAL:
                result+=("%8d\tOP_EQUAL\n" % (i))
            elif opcodes[i]==OpCode.OP_AND:
                result+=("%8d\tOP_AND\n" % (i))
            elif opcodes[i]==OpCode.OP_OR:
                result+=("%8d\tOP_OR\n" % (i))
            elif opcodes[i]==OpCode.OP_NOT:
                result+=("%8d\tOP_NOT\n" % (i))
            elif opcodes[i]==OpCode.OP_BIT_AND:
                result+=("%8d\tOP_BIT_AND\n" % (i))
            elif opcodes[i]==OpCode.OP_BIT_OR:
                result+=("%8d\tOP_BIT_OR\n" % (i))
            elif opcodes[i]==OpCode.OP_BIT_XOR:
                result+=("%8d\tOP_BIT_XOR\n" % (i))
            elif opcodes[i]==OpCode.OP_BIT_NOT:
                result+=("%8d\tOP_BIT_NOT\n" % (i))
            elif opcodes[i]==OpCode.OP_ARRAY:
                count=opcodes[i+1]
                result+=("%8d\tOP_ARRAY\t%d\n" % (i,count))
                i=i+1
            elif opcodes[i]==OpCode.OP_GET_INDEX:
                result+=("%8d\tOP_GET_INDEX\n" % (i))
            elif opcodes[i]==OpCode.OP_SET_INDEX:
                result+=("%8d\tOP_SET_INDEX\n" % (i))
            elif opcodes[i]==OpCode.OP_JUMP:
                address=opcodes[i+1]
                result+=("%8d\tOP_JUMP\t%d\n" % (i,address))
                i=i+1
            elif opcodes[i]==OpCode.OP_JUMP_IF_FALSE:
                address=opcodes[i+1]
                result+=("%8d\tOP_JUMP_IF_FALSE\t%d\n" % (i,address))
                i=i+1
            elif opcodes[i]==OpCode.OP_RETURN:
                count=opcodes[i+1]
                result+=("%8d\tOP_RETURN\t%d\n" % (i,count))
                i=i+1
            elif opcodes[i]==OpCode.OP_DEF_GLOBAL:
                pos=opcodes[i+1]
                result+=("%8d\tOP_DEF_GLOBAL\t%d\n" % (i,pos))
                i=i+1
            elif opcodes[i]==OpCode.OP_SET_GLOBAL:
                pos=opcodes[i+1]
                result+=("%8d\tOP_SET_GLOBAL\t%d\n" % (i,pos))
                i=i+1
            elif opcodes[i]==OpCode.OP_GET_GLOBAL:
                pos=opcodes[i+1]
                result+=("%8d\tOP_GET_GLOBAL\t%d\n" % (i,pos))
                i=i+1
            elif opcodes[i]==OpCode.OP_DEF_LOCAL:
                scopeDepth=opcodes[i+1]
                index=opcodes[i+2]
                result+=("%8d\tOP_DEF_LOCAL\t%d\t%d\n" % (i,scopeDepth,index))
                i=i+2
            elif opcodes[i]==OpCode.OP_SET_LOCAL:
                scopeDepth=opcodes[i+1]
                index=opcodes[i+2]
                isUpValue=opcodes[i+3]
                result+=("%8d\tOP_SET_LOCAL\t%d\t%d\t%d\n" % (i,scopeDepth,index,isUpValue))
                i=i+3
            elif opcodes[i]==OpCode.OP_GET_LOCAL:
                scopeDepth=opcodes[i+1]
                index=opcodes[i+2]
                isUpValue=opcodes[i+3]
                result+=("%8d\tOP_GET_LOCAL\t%d\t%d\t%d\n" % (i,scopeDepth,index,isUpValue))
                i=i+3
            elif opcodes[i]==OpCode.OP_FUNCTION_CALL:
                argCount=opcodes[i+1]
                result+=("%8d\tOP_FUNCTION_CALL\t%d\n" % (i,argCount))
                i=i+1
            elif opcodes[i]==OpCode.OP_GET_BUILTIN:
                result+=("%8d\tOP_GET_BUILTIN\n" % i)
            elif opcodes[i]==OpCode.OP_STRUCT:
                memberCount=opcodes[i+1]
                result+=("%8d\tOP_STRUCT\t%d\n" % (i,memberCount))
                i+=1
            elif opcodes[i]==OpCode.OP_GET_STRUCT:
                result+=("%8d\tOP_GET_STRUCT\n" % (i))
            elif opcodes[i]==OpCode.OP_SET_STRUCT:
                result+=("%8d\tOP_SET_STRUCT\n" % (i))
            elif opcodes[i]==OpCode.OP_REF_GLOBAL:
                idx=opcodes[i+1]
                result+=("%8d\tOP_REF_GLOBAL\t%d\n" % (i,idx))
                i=i+1
            elif opcodes[i]==OpCode.OP_REF_LOCAL:
                scopeDepth=opcodes[i+1]
                index=opcodes[i+2]
                isUpValue=opcodes[i+3]
                result+=("%8d\tOP_REF_LOCAL\t%d\t%d\t%d\n" % (i,scopeDepth,index,isUpValue))
                i=i+3
            elif opcodes[i]==OpCode.OP_REF_INDEX_GLOBAL:
                pos=opcodes[i+1]
                result+=("%8d\tOP_REF_INDEX_GLOBAL\t%d\n" % (i,pos))
                i=i+1
            elif opcodes[i]==OpCode.OP_REF_INDEX_LOCAL:
                scopeDepth=opcodes[i+1]
                index=opcodes[i+2]
                isUpValue=opcodes[i+3]
                result+=("%8d\tOP_REF_INDEX_LOCAL\t%d\t%d\t%d\n" % (i,scopeDepth,index,isUpValue))
                i=i+3
            elif opcodes[i]==OpCode.OP_DLL_IMPORT:
                result+=("%8d\tOP_DLL_IMPORT\n" % (i))
            i=i+1
        return result