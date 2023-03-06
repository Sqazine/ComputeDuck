import ctypes
from Utils import Assert
from Chunk import Chunk
from Chunk import OpCode
from Object import *
from BuiltinManager import *

STACK_MAX = 512
INITIAL_GC_THRESHOLD = 256
GLOBAL_VARIABLE_MAX = 1024


class CallFrame:
    fn: FunctionObject
    ip: int
    slot: int

    def __init__(self, fn=None, slot=None) -> None:
        self.fn = fn
        self.slot = slot
        self.ip = 0


class VM:
    __constants: list[Object] = []
    __globalVariables: list[Object] = []
    __stackTop: int
    __objectStack: list[Object] = []
    __callFrames: list[Object] = []
    __callFrameTop: int

    def __init__(self) -> None:
        self.ResetStatus()

    def ResetStatus(self) -> None:
        self.__stackTop = 0
        self.__objectStack = [NilObject()]*STACK_MAX
        self.__globalVariables = [NilObject()]*GLOBAL_VARIABLE_MAX
        self.__callFrames = [CallFrame()]*STACK_MAX

    def Run(self, chunk: Chunk) -> None:
        self.ResetStatus()

        mainFn = FunctionObject(chunk.opCodes)
        mainCallFrame = CallFrame(mainFn, 0)
        mainCallFrame.slot = self.__stackTop

        self.__callFrames[0] = mainCallFrame
        self.__callFrameTop = 1

        self.__constants = [None]*len(chunk.constants)
        for i in range(0, len(chunk.constants)):
            self.__constants[i] = chunk.constants[i]

        self.__Execute()

    def __Execute(self) -> None:
        while True:

            frame = self.__callFrames[self.__callFrameTop-1]

            instruction = frame.fn.opCodes[frame.ip]
            frame.ip += 1

            if instruction == OpCode.OP_RETURN:
                returnCount = frame.fn.opCodes[frame.ip]
                frame.ip += 1

                object = NilObject()
                if returnCount == 1:
                    object = self.__Pop()

                callFrame = self.__PopCallFrame()

                if self.__callFrameTop == 0:
                    return

                frame = self.__callFrames[self.__callFrameTop-1]

                self.__stackTop = callFrame.slot-1

                self.__Push(object)

            elif instruction == OpCode.OP_CONSTANT:
                idx = frame.fn.opCodes[frame.ip]
                frame.ip += 1

                object = self.__constants[idx]
                self.__Push(object)

            elif instruction == OpCode.OP_ADD:
                left = self.__Pop()
                right = self.__Pop()
                while left.type == ObjectType.REF:
                    left = self.SearchObjectByAddress(left.pointer)
                while left.type == ObjectType.REF:
                    right = self.SearchObjectByAddress(right.pointer)

                if left.type == ObjectType.BUILTIN_VARIABLE:
                    left = left.obj

                if right.type == ObjectType.BUILTIN_VARIABLE:
                    right = right.obj

                if left.type == ObjectType.NUM and right.type == ObjectType.NUM:
                    self.__Push(NumObject(left.value+right.value))
                elif left.type == ObjectType.STR and right.type == ObjectType.STR:
                    self.__Push(StrObject(left.value+right.value))
                else:
                    Assert("Invalid binary op:" +
                           left.__str__()+"+"+right.__str__())

            elif instruction == OpCode.OP_SUB:
                left = self.__Pop()
                right = self.__Pop()
                while left.type == ObjectType.REF:
                    left = self.SearchObjectByAddress(left.pointer)
                while left.type == ObjectType.REF:
                    right = self.SearchObjectByAddress(right.pointer)

                if left.type == ObjectType.BUILTIN_VARIABLE:
                    left = left.obj

                if right.type == ObjectType.BUILTIN_VARIABLE:
                    right = right.obj

                if left.type == ObjectType.NUM and right.type == ObjectType.NUM:
                    self.__Push(NumObject(left.value-right.value))
                else:
                    Assert("Invalid binary op:" +
                           left.__str__()+"-"+right.__str__())

            elif instruction == OpCode.OP_MUL:
                left = self.__Pop()
                right = self.__Pop()

                while left.type == ObjectType.REF:
                    left = self.SearchObjectByAddress(left.pointer)
                while left.type == ObjectType.REF:
                    right = self.SearchObjectByAddress(right.pointer)

                if left.type == ObjectType.BUILTIN_VARIABLE:
                    left = left.obj

                if right.type == ObjectType.BUILTIN_VARIABLE:
                    right = right.obj

                if left.type == ObjectType.NUM and right.type == ObjectType.NUM:
                    self.__Push(NumObject(left.value*right.value))
                else:
                    Assert("Invalid binary op:" +
                           left.__str__()+"*"+right.__str__())

            elif instruction == OpCode.OP_DIV:
                left = self.__Pop()
                right = self.__Pop()

                while left.type == ObjectType.REF:
                    left = self.SearchObjectByAddress(left.pointer)
                while left.type == ObjectType.REF:
                    right = self.SearchObjectByAddress(right.pointer)

                if left.type == ObjectType.BUILTIN_VARIABLE:
                    left = left.obj

                if right.type == ObjectType.BUILTIN_VARIABLE:
                    right = right.obj

                if left.type == ObjectType.NUM and right.type == ObjectType.NUM:
                    self.__Push(NumObject(left.value/right.value))
                else:
                    Assert("Invalid binary op:" +
                           left.__str__()+"/"+right.__str__())

            elif instruction == OpCode.OP_GREATER:
                left = self.__Pop()
                right = self.__Pop()

                while left.type == ObjectType.REF:
                    left = self.SearchObjectByAddress(left.pointer)
                while left.type == ObjectType.REF:
                    right = self.SearchObjectByAddress(right.pointer)

                if left.type == ObjectType.BUILTIN_VARIABLE:
                    left = left.obj

                if right.type == ObjectType.BUILTIN_VARIABLE:
                    right = right.obj

                if left.type == ObjectType.NUM and right.type == ObjectType.NUM:
                    self.__Push(BoolObject(left.value > right.value))
                else:
                    self.__Push(BoolObject(False))

            elif instruction == OpCode.OP_LESS:
                left = self.__Pop()
                right = self.__Pop()

                while left.type == ObjectType.REF:
                    left = self.SearchObjectByAddress(left.pointer)
                while left.type == ObjectType.REF:
                    right = self.SearchObjectByAddress(right.pointer)

                if left.type == ObjectType.BUILTIN_VARIABLE:
                    left = left.obj

                if right.type == ObjectType.BUILTIN_VARIABLE:
                    right = right.obj

                if left.type == ObjectType.NUM and right.type == ObjectType.NUM:
                    self.__Push(BoolObject(left.value < right.value))
                else:
                    self.__Push(BoolObject(False))

            elif instruction == OpCode.OP_EQUAL:
                left = self.__Pop()
                right = self.__Pop()

                while left.type == ObjectType.REF:
                    left = self.SearchObjectByAddress(left.pointer)
                while left.type == ObjectType.REF:
                    right = self.SearchObjectByAddress(right.pointer)

                if left.type == ObjectType.BUILTIN_VARIABLE:
                    left = left.obj

                if right.type == ObjectType.BUILTIN_VARIABLE:
                    right = right.obj

                self.__Push(BoolObject(left.IsEqualTo(right)))

            elif instruction == OpCode.OP_NOT:
                obj = self.__Pop()
                while obj.type == ObjectType.REF:
                    obj = self.SearchObjectByAddress(obj.pointer)

                if left.type == ObjectType.BUILTIN_VARIABLE:
                    left = left.obj

                if obj.type != ObjectType.BOOL:
                    Assert("Not a boolean obj of the obj:" + obj.__str__())
                self.__Push(BoolObject(not obj.value))

            elif instruction == OpCode.OP_MINUS:
                obj = self.__Pop()
                if obj.type == ObjectType.REF:
                    obj = self.SearchObjectByAddress(obj.pointer)
                if obj.type != ObjectType.NUM:
                    Assert("Not a valid op:'-'"+obj.__str__())
                self.__Push(-obj.value)

            elif instruction == OpCode.OP_AND:
                left = self.__Pop()
                right = self.__Pop()
                while left.type == ObjectType.REF:
                    left = self.SearchObjectByAddress(left.pointer)
                while left.type == ObjectType.REF:
                    right = self.SearchObjectByAddress(right.pointer)

                if left.type == ObjectType.BUILTIN_VARIABLE:
                    left = left.obj

                if right.type == ObjectType.BUILTIN_VARIABLE:
                    right = right.obj

                if left.type == ObjectType.BOOL and right.type == ObjectType.BOOL:
                    self.__Push(BoolObject(left.value and right.value))
                else:
                    Assert("Invalid op:" + left.__str__() +
                           " and " + right.__str__())

            elif instruction == OpCode.OP_OR:
                left = self.__Pop()
                right = self.__Pop()

                while left.type == ObjectType.REF:
                    left = self.SearchObjectByAddress(left.pointer)
                while left.type == ObjectType.REF:
                    right = self.SearchObjectByAddress(right.pointer)

                if left.type == ObjectType.BUILTIN_VARIABLE:
                    left = left.obj

                if right.type == ObjectType.BUILTIN_VARIABLE:
                    right = right.obj

                if left.type == ObjectType.BOOL and right.type == ObjectType.BOOL:
                    self.__Push(BoolObject(left.value or right.value))
                else:
                    Assert("Invalid op:" + left.__str__() +
                           " or " + right.__str__())

            elif instruction == OpCode.OP_ARRAY:
                numElements = frame.fn.opCodes[frame.ip]
                frame.ip += 1

                self.__stackTop -= numElements

                elements = [None]*numElements

                for i in range(0, numElements):
                    elements[i] = self.__objectStack[self.__stackTop+i]

                array = ArrayObject(elements)
                self.__Push(array)

            elif instruction == OpCode.OP_INDEX:
                index = self.__Pop()
                ds = self.__Pop()
                if ds.type == ObjectType.ARRAY and index.type == ObjectType.NUM:
                    i = int(index.value)
                    if i < 0 or i >= len(ds.elements):
                        self.__Push(NilObject())
                    else:
                        self.__Push(ds.elements[i])

            elif instruction == OpCode.OP_JUMP_IF_FALSE:
                address = frame.fn.opCodes[frame.ip]
                frame.ip += 1
                obj = self.__Pop()
                if obj.type != ObjectType.BOOL:
                    Assert("The if condition not a boolean value:"+obj.__str__())
                if obj.value != True:
                    frame.ip = address+1

            elif instruction == OpCode.OP_JUMP:
                address = frame.fn.opCodes[frame.ip]
                frame.ip = address+1

            elif instruction == OpCode.OP_SET_GLOBAL:
                index = frame.fn.opCodes[frame.ip]
                frame.ip += 1
                obj = self.__Pop()

                if self.__globalVariables[index].type == ObjectType.REF:
                    gRefObj = self.__globalVariables[index]
                    gRefAddress = -1
                    while gRefObj.type == ObjectType.REF:
                        gRefAddress = gRefObj.pointer
                        gRefObj = self.SearchObjectByAddress(gRefObj.pointer)
                    self.AssignObjectByAddress(gRefAddress, obj)

                    refObjs = self.SearchRefObjectByAddress(gRefAddress)
                    for i in range(0, len(refObjs)):
                        refObjs[i].pointer = id(obj)

                    self.__globalVariables[index].pointer = id(obj)
                else:
                    self.UpdateRefAddress(
                        id(self.__globalVariables[index]), id(obj))
                    self.__globalVariables[index] = obj

            elif instruction == OpCode.OP_GET_GLOBAL:
                index = frame.fn.opCodes[frame.ip]
                frame.ip += 1
                self.__Push(self.__globalVariables[index])

            elif instruction == OpCode.OP_FUNCTION_CALL:

                argCount = frame.fn.opCodes[frame.ip]
                frame.ip += 1

                obj = self.__objectStack[self.__stackTop-1-argCount]
                if obj.type == ObjectType.FUNCTION:
                    if argCount != obj.parameterCount:
                        Assert("Non matching function parameters for calling arguments,parameter count:" +
                               obj.parameterCount + ",argument count:" + argCount)
                    callFrame = CallFrame(obj, self.__stackTop-argCount)
                    self.__callFrames[self.__callFrameTop] = callFrame
                    self.__callFrameTop += 1

                    self.__stackTop = callFrame.slot+obj.localVarCount

                elif obj.type == ObjectType.BUILTIN_FUNCTION:
                    j = 0
                    args = [NilObject()]*argCount
                    for i in range(self.__stackTop-argCount, self.__stackTop):
                        args[j] = self.__objectStack[i]
                        j += 1

                    self.__stackTop -= (argCount+1)

                    hasReturnValue, returnValue = obj.fn(args)
                    if hasReturnValue == True:
                        self.__Push(returnValue)
                else:
                    Assert("Calling not a function or a builtinFn")

            elif instruction == OpCode.OP_SET_LOCAL:

                isInUpScope = frame.fn.opCodes[frame.ip]
                frame.ip += 1
                scopeDepth = frame.fn.opCodes[frame.ip]
                frame.ip += 1
                index = frame.fn.opCodes[frame.ip]
                frame.ip += 1

                obj = self.__Pop()

                slot = 0
                if isInUpScope == 0:
                    slot = frame.slot+index
                else:
                    slot = self.__PeekCallFrame(scopeDepth).slot+index

                if self.__objectStack[slot].type == ObjectType.REF:
                    lRefObj = self.__objectStack[slot]
                    lRefAddress = -1
                    while gRefObj.type == ObjectType.REF:
                        lRefAddress = lRefObj.pointer
                        lRefObj = self.SearchObjectByAddress(lRefObj.pointer)
                    self.AssignObjectByAddress(lRefAddress, obj)

                    refObjs = self.SearchRefObjectByAddress(lRefAddress)
                    for i in range(0, len(refObjs)):
                        refObjs[i].pointer = id(obj)

                    self.__objectStack[slot].pointer = id(obj)
                else:
                    self.UpdateRefAddress(
                        id(self.__objectStack[slot]), id(obj))
                    self.__objectStack[slot] = obj

            elif instruction == OpCode.OP_GET_LOCAL:
                isInUpScope = frame.fn.opCodes[frame.ip]
                frame.ip += 1
                scopeDepth = frame.fn.opCodes[frame.ip]
                frame.ip += 1
                index = frame.fn.opCodes[frame.ip]
                frame.ip += 1

                slot = 0
                if isInUpScope == 0:
                    slot = frame.slot+index
                else:
                    slot = self.__PeekCallFrame(scopeDepth).slot+index

                self.__Push(self.__objectStack[slot])

            elif instruction == OpCode.OP_SP_OFFSET:
                offset = frame.fn.opCodes[frame.ip]
                frame.ip += 1
                self.__stackTop += offset

            elif instruction == OpCode.OP_GET_BUILTIN_FUNCTION:
                idx = frame.fn.opCodes[frame.ip]
                frame.ip += 1

                builtinObj = gBuiltinManager.builtinFunctions[idx]
                self.__Push(builtinObj)

            elif instruction == OpCode.OP_GET_BUILTIN_VARIABLE:
                idx = frame.fn.opCodes[frame.ip]
                frame.ip += 1

                builtinObj = gBuiltinManager.builtinVariables[idx]
                self.__Push(builtinObj)

            elif instruction == OpCode.OP_STRUCT:
                members: dict[str, Object] = {}

                memberCount = frame.fn.opCodes[frame.ip]
                frame.ip += 1

                tmpPtr = self.__stackTop

                for i in range(0, memberCount):
                    tmpPtr -= 1
                    name = self.__objectStack[tmpPtr].value
                    tmpPtr -= 1
                    obj = self.__objectStack[tmpPtr]
                    members[name] = obj

                structInstance = StructObject(members)
                self.__stackTop = tmpPtr
                self.__Push(structInstance)

            elif instruction == OpCode.OP_GET_STRUCT:
                memberName = self.__Pop()
                instance = self.__Pop()
                while instance.type == ObjectType.REF:
                    instance = self.SearchObjectByAddress(instance.pointer)
                if memberName.type == ObjectType.STR:
                    iter = instance.members.get(memberName.value, None)
                    if iter == None:
                        Assert("no member named:(" + memberName.__str__() +
                               ") in struct instance:" + instance.__str__())
                    self.__Push(iter)

            elif instruction == OpCode.OP_SET_STRUCT:
                memberName = self.__Pop()
                instance = self.__Pop()
                while instance.type == ObjectType.REF:
                    instance = self.SearchObjectByAddress(instance.pointer)
                obj = self.__Pop()
                if memberName.type == ObjectType.STR:
                    iter = instance.members.get(memberName.value, None)
                    if iter == None:
                        Assert("no member named:(" + memberName.__str__() +
                               ") in struct instance:" + instance.__str__())
                    instance.members[memberName.value] = obj

            elif instruction == OpCode.OP_REF_GLOBAL:
                index = frame.fn.opCodes[frame.ip]
                frame.ip += 1
                self.__Push(RefObject(id(self.__globalVariables[index])))

            elif instruction == OpCode.OP_REF_LOCAL:
                isInUpScope = frame.fn.opCodes[frame.ip]
                frame.ip += 1
                scopeDepth = frame.fn.opCodes[frame.ip]
                frame.ip += 1
                index = frame.fn.opCodes[frame.ip]
                frame.ip += 1

                slot = 0
                if isInUpScope == 0:
                    slot = frame.slot+index
                else:
                    slot = self.__PeekCallFrame(scopeDepth).slot+index

                self.__Push(RefObject(id(self.__objectStack[slot])))

            elif instruction == OpCode.OP_REF_INDEX_GLOBAL:
                index = frame.fn.opCodes[frame.ip]
                frame.ip += 1
                idxValue = self.__Pop()

                obj = self.__globalVariables[index]
                while obj.type == ObjectType.REF:
                    obj = self.SearchObjectByAddress(obj.pointer)

                if obj.type == ObjectType.ARRAY:
                    if idxValue.type != ObjectType.NUM:
                        Assert("Invalid idx for array,only integer is available.")
                    if idxValue.value < 0 or idxValue.value >= len(obj.elements):
                        Assert("Idx out of range.")
                    self.__Push(
                        RefObject(id(obj.elements[int(idxValue.value)])))

            elif instruction == OpCode.OP_REF_INDEX_LOCAL:
                isInUpScope = frame.fn.opCodes[frame.ip]
                frame.ip += 1
                scopeDepth = frame.fn.opCodes[frame.ip]
                frame.ip += 1
                index = frame.fn.opCodes[frame.ip]
                frame.ip += 1

                idxValue = self.__Pop()

                slot = 0
                if isInUpScope == 0:
                    slot = frame.slot+index
                else:
                    slot = self.__PeekCallFrame(scopeDepth).slot+index

                while self.__objectStack[slot].type == ObjectType.REF:
                    self.__objectStack[slot] = self.SearchObjectByAddress(
                        self.__objectStack[slot].pointer)

                if self.__objectStack[slot].type == ObjectType.ARRAY:
                    if idxValue.type != ObjectType.NUM:
                        Assert("Invalid idx for array,only integer is available.")
                    if idxValue.value < 0 or idxValue.value >= len(self.__objectStack[slot].elements):
                        Assert("Idx out of range.")
                    self.__Push(
                        RefObject(id(self.__objectStack[slot].elements[idxValue])))
                else:
                    Assert("Invalid indexed reference type:" +
                           self.__objectStack[slot].__str__()+" not a table or array value.")

    def __Push(self, obj: Object) -> None:
        self.__objectStack[self.__stackTop] = obj
        self.__stackTop += 1

    def __Pop(self) -> Object:
        self.__stackTop -= 1
        return self.__objectStack[self.__stackTop]

    def __PopCallFrame(self) -> CallFrame:
        self.__callFrameTop -= 1
        return self.__callFrames[self.__callFrameTop]

    def __PeekCallFrame(self, distance: int) -> CallFrame:
        return self.__callFrames[self.__callFrameTop-distance]

    def SearchObjectByAddress(self, address):
        return ctypes.cast(address, ctypes.py_object).value

    def AssignObjectByAddress(self, address, obj: Object):
        for i in range(0, len(self.__globalVariables)):
            if self.__globalVariables[i].type == ObjectType.ARRAY:
                for j in range(0, len(self.__globalVariables[i].elements)):
                    if id(self.__globalVariables[i].elements[j]) == address:
                        self.__globalVariables[i].elements[j] = obj
            elif id(self.__globalVariables[i]) == address:
                self.__globalVariables[i] = obj
                return

        for p in range(0, self.__stackTop):
            if self.__objectStack[p].type == ObjectType.ARRAY:
                for j in range(0, len(self.__objectStack[p].elements)):
                    if id(self.__objectStack[p].elements[j]) == address:
                        self.__objectStack[p].elements[j] = obj
            elif id(self.__objectStack[p]) == address:
                self.__objectStack[p] = obj

    def SearchRefObjectByAddress(self, address: int) -> list:
        result: list = []
        for i in range(0, len(self.__globalVariables)):
            if self.__globalVariables[i].type == ObjectType.ARRAY:
                for j in range(0, len(self.__globalVariables[i].elements)):
                    if self.__globalVariables[i].elements[j].type == ObjectType.REF:
                        if self.__globalVariables[i].elements[j].pointer == address:
                            result.append(
                                self.__globalVariables[i].elements[j])
            elif self.__globalVariables[i].type == ObjectType.REF:
                if self.__globalVariables[i].pointer == address:
                    result.append(self.__globalVariables[i])

        for p in range(0, self.__stackTop):
            if self.__objectStack[p].type == ObjectType.ARRAY:
                for j in range(0, len(self.__objectStack[p].elements)):
                    if self.__objectStack[p].elements[j].type == ObjectType.REF:
                        if self.__objectStack[p].elements[j].pointer == address:
                            result.append(self.__objectStack[p].elements[j])

            elif self.__objectStack[p].type == ObjectType.REF:
                if self.__objectStack[p].pointer == address:
                    result.append(self.__objectStack[p])

        return result

    def UpdateRefAddress(self, originAddress, newAddress):
        for i in range(0, len(self.__globalVariables)):
            if self.__globalVariables[i].type == ObjectType.ARRAY:
                for j in range(0, len(self.__globalVariables[i].elements)):
                    if self.__globalVariables[i].elements[j].type == ObjectType.REF:
                        if self.__globalVariables[i].elements[j].pointer == originAddress:
                            self.__globalVariables[i].elements[j].pointer = newAddress
                            pass
            elif self.__globalVariables[i].type == ObjectType.REF:
                if self.__globalVariables[i].pointer == originAddress:
                    self.__globalVariables[i].pointer = newAddress
                    pass

        for p in range(0, self.__stackTop):
            if self.__objectStack[p].type == ObjectType.ARRAY:
                for j in range(0, len(self.__objectStack[p].elements)):
                    if self.__objectStack[p].elements[j].type == ObjectType.REF:
                        if self.__objectStack[p].elements[j].pointer == originAddress:
                            self.__objectStack[p].elements[j].pointer = newAddress
                            pass

            elif self.__objectStack[p].type == ObjectType.REF:
                if self.__objectStack[p].pointer == originAddress:
                    self.__objectStack[p].pointer = newAddress
                    pass
