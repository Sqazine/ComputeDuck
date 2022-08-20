from audioop import add
import ctypes
import time
from typing import Any, List
from Object import Object
from Utils import Assert
from Object import ObjectType
from Object import BoolObject, NilObject, NumObject, StrObject
from Compiler import CONSTANT_MAX
from Object import FunctionObject
from Chunk import Chunk
from Object import BuiltinObject
from Object import RefObject
from Chunk import OpCode
from Object import ArrayObject
from Object import StructInstanceObject


def println(args: list[Object]):
    if len(args) == 0:
        return False,None
    print(args[0])
    return False,None


def print_(args: list[Object]) -> bool:
    if len(args) == 0:
        return False,None
    print(args[0], end="")
    return False,None


def sizeof(args: list[Object]):
    if len(args) == 0 or len(args) > 1:
        Assert("Native function 'siezeof':Expect a argument.")
    if args[0].Type() == ObjectType.ARRAY:
        result = NumObject(len(args[0].elements))
    elif args[0].Type == ObjectType.STR:
        result = NumObject(len(args[0].value))
    else:
        Assert("[Native function 'sizeof']:Expect a array or string argument.")
    return True,result


def insert(args: list[Object]):
    if len(args) == 0 or len(args) != 3:
        Assert("[Native function 'insert']:Expect 3 arguments,the arg0 must be array,table or string obj.The arg1 is the index obj.The arg2 is the value obj.")

    if args[0].Type() == ObjectType.ARRAY:
        if args[1].Type() != ObjectType.NUM:
            Assert(
                "[Native function 'insert']:Arg1 must be integer type while insert to a array")
        iIndex = args[1].value
        if iIndex < 0 or iIndex >= len(args[0].elements):
            Assert("[Native function 'insert']:Index out of array's range")
        args[0].elements.insert(iIndex, args[2])
    elif args[0].Type == ObjectType.STR:
        if args[1].Type() != ObjectType.NUM:
            Assert(
                "[Native function 'insert']:Arg1 must be integer type while insert to a string")
        iIndex = args[1].value
        if iIndex < 0 or iIndex >= len(args[0].values):
            Assert("[Native function 'insert']:Index out of array's range")
        args[0].values.insert(iIndex, args[2].Stringify())
    return False,None


def erase(args: list[Object] ) -> bool:
    if len(args) or len(args) != 2:
        Assert("[Native function 'erase']:Expect 2 arguments,the arg0 must be array,table or string obj.The arg1 is the corresponding index obj.")

    if args[0].Type() == ObjectType.ARRAY:
        if args[1].Type() != ObjectType.NUM:
            Assert(
                "[Native function 'insert']:Arg1 must be integer type while insert to a array")
        iIndex = args[1].value
        if iIndex < 0 or iIndex >= len(args[0].elements):
            Assert("[Native function 'insert']:Index out of array's range")
        args[0].elements.pop(iIndex)
    elif args[0].Type == ObjectType.STR:
        if args[1].Type() != ObjectType.NUM:
            Assert(
                "[Native function 'insert']:Arg1 must be integer type while insert to a string")
        iIndex = args[1].value
        if iIndex < 0 or iIndex >= len(args[0].values):
            Assert("[Native function 'insert']:Index out of array's range")
        args[0].values.pop(iIndex)
    else:
        Assert("[Native function 'erase']:Expect a array,table ot string argument.")
    return False,None


def clock(args: list[Object] ) -> bool:
    result = NumObject(time.perf_counter())
    return True,result


STACK_MAX = 512
INITIAL_GC_THRESHOLD = 256
GLOBAL_VARIABLE_MAX = 1024


class CallFrame:
    fn: FunctionObject
    ip: int
    basePtr: int

    def __init__(self, fn=None, basePtr=0) -> None:
        self.fn = fn
        self.basePtr = basePtr
        self.ip = -1

    def GetOpCodes(self) -> List[int]:
        return self.fn.opCodes


class VM:

    __constants: List[Object] = []
    __globalVariables: List[Object] = []
    __sp: int
    __objectStack: List[Object] = []
    __callFrames: List[Object] = []
    __callFrameIndex: int
    __builtins: List[BuiltinObject] = []
    __curCallFrame: CallFrame = None

    def __init__(self) -> None:
        self.ResetStatus()

        self.__builtins.append(BuiltinObject("print",print_))
        self.__builtins.append(BuiltinObject("println",println))
        self.__builtins.append(BuiltinObject("sizeof",sizeof))
        self.__builtins.append(BuiltinObject("insert",insert))
        self.__builtins.append(BuiltinObject("erase",erase))
        self.__builtins.append(BuiltinObject("clock",clock))

    def ResetStatus(self) -> None:
        self.__sp = 0
        self.__objectStack = [NilObject()]*STACK_MAX
        self.__globalVariables = [NilObject()]*GLOBAL_VARIABLE_MAX
        self.__callFrames = [CallFrame()]*STACK_MAX

    def Run(self, chunk: Chunk) -> None:
        self.ResetStatus()

        mainFn = FunctionObject(chunk.opCodes)
        mainCallFrame = CallFrame(mainFn, 0)
        self.__callFrames[0] = mainCallFrame
        self.__callFrameIndex = 1
        self.__curCallFrame = self.__callFrames[self.__callFrameIndex-1]

        self.__constants = [None]*chunk.constantCount
        for i in range(0, chunk.constantCount):
            self.__constants[i] = chunk.constants[i]

        self.__Execute()

    def __Execute(self) -> None:
        while self.__curCallFrame.ip < len(self.__curCallFrame.GetOpCodes())-1:

            self.__curCallFrame.ip += 1
            instruction = self.__curCallFrame.GetOpCodes()[
                self.__curCallFrame.ip]

            if instruction == OpCode.OP_RETURN:
                self.__curCallFrame.ip += 1
                returnCount = self.__curCallFrame.GetOpCodes()[
                    self.__curCallFrame.ip]
                object = NilObject()
                if returnCount == 1:
                    object = self.__Pop()

                callFrame = self.__PopCallFrame()
                self.__sp = callFrame.basePtr-1
                self.__Push(object)

            elif instruction == OpCode.OP_CONSTANT:
                self.__curCallFrame.ip += 1
                idx = self.__curCallFrame.GetOpCodes()[self.__curCallFrame.ip]
                object = self.__constants[idx]
                self.__Push(object)

            elif instruction == OpCode.OP_ADD:
                left = self.__Pop()
                right = self.__Pop()
                if left.Type() == ObjectType.REF:
                    left = self.SearchObjectByAddress(left.pointer)
                if left.Type() == ObjectType.REF:
                    right = self.SearchObjectByAddress(right.pointer)
                if left.Type() == ObjectType.NUM and right.Type() == ObjectType.NUM:
                    self.__Push(NumObject(left.value+right.value))
                elif left.Type() == ObjectType.STR and right.Type() == ObjectType.STR:
                    self.__Push(StrObject(left.value+right.value))
                else:
                    Assert("Invalid binary op:" +
                           left.Stringify()+"+"+right.Stringify())

            elif instruction == OpCode.OP_SUB:
                left = self.__Pop()
                right = self.__Pop()
                if left.Type() == ObjectType.REF:
                    left = self.SearchObjectByAddress(left.pointer)
                if left.Type() == ObjectType.REF:
                    right = self.SearchObjectByAddress(right.pointer)
                if left.Type() == ObjectType.NUM and right.Type() == ObjectType.NUM:
                    self.__Push(NumObject(left.value-right.value))
                else:
                    Assert("Invalid binary op:" +
                           left.Stringify()+"-"+right.Stringify())

            elif instruction == OpCode.OP_MUL:
                left = self.__Pop()
                right = self.__Pop()
                if left.Type() == ObjectType.REF:
                    left = self.SearchObjectByAddress(left.pointer)
                if left.Type() == ObjectType.REF:
                    right = self.SearchObjectByAddress(right.pointer)
                if left.Type() == ObjectType.NUM and right.Type() == ObjectType.NUM:
                    self.__Push(NumObject(left.value*right.value))
                else:
                    Assert("Invalid binary op:" +
                           left.Stringify()+"*"+right.Stringify())

            elif instruction == OpCode.OP_DIV:
                left = self.__Pop()
                right = self.__Pop()
                if left.Type() == ObjectType.REF:
                    left = self.SearchObjectByAddress(left.pointer)
                if left.Type() == ObjectType.REF:
                    right =self.SearchObjectByAddress( right.pointer)
                if left.Type() == ObjectType.NUM and right.Type() == ObjectType.NUM:
                    self.__Push(NumObject(left.value/right.value))
                else:
                    Assert("Invalid binary op:" +
                           left.Stringify()+"/"+right.Stringify())

            elif instruction == OpCode.OP_GREATER:
                left = self.__Pop()
                right = self.__Pop()
                if left.Type() == ObjectType.REF:
                    left =self.SearchObjectByAddress( left.pointer)
                if left.Type() == ObjectType.REF:
                    right =self.SearchObjectByAddress( right.pointer)
                if left.Type() == ObjectType.NUM and right.Type() == ObjectType.NUM:
                    self.__Push(BoolObject(left.value > right.value))
                else:
                    self.__Push(BoolObject(False))

            elif instruction == OpCode.OP_LESS:
                left = self.__Pop()
                right = self.__Pop()
                if left.Type() == ObjectType.REF:
                    left = self.SearchObjectByAddress(left.pointer)
                if left.Type() == ObjectType.REF:
                    right = self.SearchObjectByAddress(right.pointer)
                if left.Type() == ObjectType.NUM and right.Type() == ObjectType.NUM:
                    self.__Push(BoolObject(left.value < right.value))
                else:
                    self.__Push(BoolObject(False))

            elif instruction == OpCode.OP_EQUAL:
                left = self.__Pop()
                right = self.__Pop()
                if left.Type() == ObjectType.REF:
                    left = self.SearchObjectByAddress(left.pointer)
                if left.Type() == ObjectType.REF:
                    right =self.SearchObjectByAddress( right.pointer)
                self.__Push(BoolObject(left.IsEqualTo(right)))

            elif instruction == OpCode.OP_NOT:
                obj = self.__Pop()
                if obj.Type() == ObjectType.REF:
                    obj = self.SearchObjectByAddress(obj.pointer)
                if obj.Type() != ObjectType.BOOL:
                    Assert("Not a boolean obj of the obj:" +
                           obj.Stringify())
                self.__Push(not obj.value)

            elif instruction == OpCode.OP_MINUS:
                obj = self.__Pop()
                if obj.Type() == ObjectType.REF:
                    obj = self.SearchObjectByAddress(obj.pointer)
                if obj.Type() != ObjectType.NUM:
                    Assert("Not a valid op:'-'"+obj.Stringify())
                self.__Push(-obj.value)

            elif instruction == OpCode.OP_AND:
                left = self.__Pop()
                right = self.__Pop()
                if left.Type() == ObjectType.REF:
                    left = self.SearchObjectByAddress(left.pointer)
                if left.Type() == ObjectType.REF:
                    right = self.SearchObjectByAddress(right.pointer)
                if left.Type() == ObjectType.BOOL and right.Type() == ObjectType.BOOL:
                    self.__Push(BoolObject(left.value and right.value))
                else:
                    Assert("Invalid op:" + left.Stringify() +
                           " and " + right.Stringify())

            elif instruction == OpCode.OP_OR:
                left = self.__Pop()
                right = self.__Pop()
                if left.Type() == ObjectType.REF:
                    left =self.SearchObjectByAddress( left.pointer)
                if left.Type() == ObjectType.REF:
                    right =self.SearchObjectByAddress( right.pointer)
                if left.Type() == ObjectType.BOOL and right.Type() == ObjectType.BOOL:
                    self.__Push(BoolObject(left.value or right.value))
                else:
                    Assert("Invalid op:" + left.Stringify() +
                           " or " + right.Stringify())

            elif instruction == OpCode.OP_ARRAY:
                self.__curCallFrame.ip += 1
                numElements = self.__curCallFrame.GetOpCodes()[
                    self.__curCallFrame.ip]
                startIndex = self.__sp-numElements
                endIndex = self.__sp
                elements = [Object]*numElements

                for i in range(startIndex, endIndex):
                    elements[i-startIndex] = self.__objectStack[i]
                array = ArrayObject(elements)
                self.__sp -= numElements
                self.__Push(array)

            elif instruction == OpCode.OP_INDEX:
                index = self.__Pop()
                ds = self.__Pop()
                if ds.Type() == ObjectType.ARRAY and index.Type() == ObjectType.NUM:
                    i = int(index.value)
                    if i < 0 or i >= len(ds.elements):
                        self.__Push(NilObject())
                    else:
                        self.__Push(ds.elements[i])

            elif instruction == OpCode.OP_JUMP_IF_FALSE:
                self.__curCallFrame.ip += 1
                address = self.__curCallFrame.GetOpCodes()[
                    self.__curCallFrame.ip]
                obj = self.__Pop()
                if obj.Type() != ObjectType.BOOL:
                    Assert("The if condition not a boolean value:"+obj.Stringify())
                if obj.value != True:
                    self.__curCallFrame.ip = address

            elif instruction == OpCode.OP_JUMP:
                self.__curCallFrame.ip += 1
                address = self.__curCallFrame.GetOpCodes()[
                    self.__curCallFrame.ip]
                self.__curCallFrame.ip = address

            elif instruction == OpCode.OP_SET_GLOBAL:
                self.__curCallFrame.ip += 1
                index = self.__curCallFrame.GetOpCodes()[
                    self.__curCallFrame.ip]
                obj = self.__Pop()
                if self.__globalVariables[index].Type() == ObjectType.REF:
                    self.AssignObjectByAddress(self.__globalVariables[index].pointer,obj)
                    self.__globalVariables[index].pointer=id(obj)
                else:
                    self.UpdateRefAddress(id(self.__globalVariables[index]),id(obj))
                    self.__globalVariables[index] = obj

            elif instruction == OpCode.OP_GET_GLOBAL:
                self.__curCallFrame.ip += 1
                index = self.__curCallFrame.GetOpCodes()[
                    self.__curCallFrame.ip]
                self.__Push(self.__globalVariables[index])

            elif instruction == OpCode.OP_FUNCTION_CALL:
                self.__curCallFrame.ip += 1
                argCount = self.__curCallFrame.GetOpCodes()[self.__curCallFrame.ip]
                obj = self.__objectStack[self.__sp-1-argCount]
                if obj.Type() == ObjectType.FUNCTION:
                    if argCount != obj.parameterCount:
                        Assert("Non matching function parameters for calling arguments,parameter count:" +
                               obj.parameterCount + ",argument count:" + argCount)
                    callFrame = CallFrame(obj, self.__sp-argCount)
                    self.__PushCallFrame(callFrame)
                    self.__sp = callFrame.basePtr+obj.localVarCount
                elif obj.Type() == ObjectType.BUILTIN:
                    args = [Object]*argCount

                    j = 0
                    for i in range(self.__sp-argCount, self.__sp):
                        args[j] = self.__objectStack[i]
                        j += 1

                    self.__sp = self.__sp-argCount-1

                    hasReturnValue,returnValue = obj.fn(args)
                    if hasReturnValue == True:
                        self.__Push(returnValue)
                else:
                    Assert("Calling not a function or a builtinFn")

            elif instruction == OpCode.OP_SET_LOCAL:
                self.__curCallFrame.ip += 1
                isInUpScope = self.__curCallFrame.GetOpCodes()[self.__curCallFrame.ip]
                self.__curCallFrame.ip += 1
                scopeDepth = self.__curCallFrame.GetOpCodes()[self.__curCallFrame.ip]
                self.__curCallFrame.ip += 1
                index = self.__curCallFrame.GetOpCodes()[self.__curCallFrame.ip]
                obj = self.__Pop()

                fullIdx=0
                if isInUpScope==0:
                    fullIdx = self.__curCallFrame.basePtr+index
                else:
                    fullIdx=self.__PeekCallFrame(scopeDepth).basePtr+index

                if self.__objectStack[fullIdx].Type() == ObjectType.REF:
                    self.AssignObjectByAddress(self.__objectStack[fullIdx].pointer,obj)
                    self.__objectStack[fullIdx].pointer = id(obj)
                else:
                    self.UpdateRefAddress(id(self.__objectStack[fullIdx]),id(obj))
                    self.__objectStack[fullIdx] = obj

            elif instruction == OpCode.OP_GET_LOCAL:
                self.__curCallFrame.ip += 1
                isInUpScope = self.__curCallFrame.GetOpCodes()[self.__curCallFrame.ip]
                self.__curCallFrame.ip += 1
                scopeDepth = self.__curCallFrame.GetOpCodes()[self.__curCallFrame.ip]
                self.__curCallFrame.ip += 1
                index = self.__curCallFrame.GetOpCodes()[self.__curCallFrame.ip]
                
                fullIdx=0
                if isInUpScope==0:
                    fullIdx = self.__curCallFrame.basePtr+index
                else:
                    fullIdx=self.__PeekCallFrame(scopeDepth).basePtr+index
                
                self.__Push(
                    self.__objectStack[fullIdx])

            elif instruction == OpCode.OP_SP_OFFSET:
                self.__curCallFrame.ip += 1
                offset = self.__curCallFrame.GetOpCodes()[
                    self.__curCallFrame.ip]
                self.__sp += offset

            elif instruction == OpCode.OP_GET_BUILTIN:
                self.__curCallFrame.ip += 1
                idx = self.__curCallFrame.GetOpCodes()[self.__curCallFrame.ip]
                builtinObj = self.__builtins[idx]
                self.__Push(builtinObj)

            elif instruction == OpCode.OP_GET_CURRENT_FUNCTION:
                self.__Push(self.__curCallFrame.fn)

            elif instruction == OpCode.OP_STRUCT:
                members:dict[str, Object]={}
                self.__curCallFrame.ip += 1
                memberCount = self.__curCallFrame.GetOpCodes()[
                    self.__curCallFrame.ip]
                tmpPtr = self.__sp

                for i in range(0, memberCount):
                    tmpPtr -= 1
                    name = self.__objectStack[tmpPtr].value
                    tmpPtr -= 1
                    obj = self.__objectStack[tmpPtr]
                    members[name] = obj

                structInstance = StructInstanceObject(members)
                self.__sp = tmpPtr
                self.__Push(structInstance)

            elif instruction == OpCode.OP_GET_STRUCT:
                memberName = self.__Pop()
                instance = self.__Pop()
                if instance.Type() == ObjectType.REF:
                    instance =self.SearchObjectByAddress( instance.pointer)
                if memberName.Type() == ObjectType.STR:
                    iter = instance.members.get(memberName.value, None)
                    if iter == None:
                        Assert("no member named:(" + memberName.Stringify() +
                               ") in struct instance:" + instance.Stringify())
                    self.__Push(iter)

            elif instruction == OpCode.OP_SET_STRUCT:
                memberName = self.__Pop()
                instance = self.__Pop()
                if instance.Type() == ObjectType.REF:
                    instance = self.SearchObjectByAddress(instance.pointer)
                obj = self.__Pop()
                if memberName.Type() == ObjectType.STR:
                    iter = instance.members.get(memberName.value, None)
                    if iter == None:
                        Assert("no member named:(" + memberName.Stringify() +
                               ") in struct instance:" + instance.Stringify())
                    instance.members[memberName.value] = obj

            elif instruction == OpCode.OP_REF_GLOBAL:
                self.__curCallFrame.ip += 1
                index = self.__curCallFrame.GetOpCodes()[
                    self.__curCallFrame.ip]
                self.__Push(RefObject(id(self.__globalVariables[index])))
            elif instruction == OpCode.OP_REF_LOCAL:
                self.__curCallFrame.ip += 1
                isInUpScope = self.__curCallFrame.GetOpCodes()[self.__curCallFrame.ip]
                self.__curCallFrame.ip += 1
                scopeDepth = self.__curCallFrame.GetOpCodes()[self.__curCallFrame.ip]
                self.__curCallFrame.ip += 1
                index = self.__curCallFrame.GetOpCodes()[self.__curCallFrame.ip]
                
                fullIdx=0
                if isInUpScope==0:
                    fullIdx = self.__curCallFrame.basePtr+index
                else:
                    fullIdx=self.__PeekCallFrame(scopeDepth).basePtr+index

                self.__Push(RefObject(id(self.__objectStack[fullIdx])))

    def __Push(self, obj: Object) -> None:
        self.__objectStack[self.__sp] = obj
        self.__sp += 1

    def __Pop(self) -> Object:
        self.__sp -= 1
        return self.__objectStack[self.__sp]

    def __PushCallFrame(self, callFrame: CallFrame) -> None:
        self.__callFrames[self.__callFrameIndex] = callFrame
        self.__callFrameIndex += 1
        self.__curCallFrame = self.__callFrames[self.__callFrameIndex-1]

    def __PopCallFrame(self) -> CallFrame:
        self.__callFrameIndex -= 1
        self.__curCallFrame = self.__callFrames[self.__callFrameIndex-1]
        return self.__callFrames[self.__callFrameIndex]

    def __PeekCallFrame(self, distance: int) -> CallFrame:
        return self.__callFrames[distance]

    def SearchObjectByAddress(self,address):
         return ctypes.cast(address,ctypes.py_object).value

    def AssignObjectByAddress(self,address,obj:Object):
        for i in range(0,len(self.__globalVariables)):
            if id(self.__globalVariables[i])==address:
                self.__globalVariables[i]=obj
                pass
        
        for p in range(0,self.__sp):
            if id(self.__objectStack[p])==address:
                self.__objectStack[p]=obj
    
    def UpdateRefAddress(self,originAddress,newAddress):
        for i in range(0,len(self.__globalVariables)):
            if self.__globalVariables[i].Type()==ObjectType.REF:
                if self.__globalVariables[i].pointer==originAddress:
                    self.__globalVariables[i].pointer=newAddress
                    pass
        
        for p in range(0,self.__sp):
            if self.__objectStack[p].Type()==ObjectType.REF:
                if self.__objectStack[p].pointer==originAddress:
                    self.__objectStack[p].pointer=newAddress
                    pass