from Utils import error, STACK_COUNT
from Chunk import OpCode
from Object import *
from BuiltinManager import *


class CallFrame:
    closure: ClosureObject
    ip: int
    slot: int

    def __init__(self, closure=None, slot=None) -> None:
        self.closure = closure
        self.slot = slot
        self.ip = 0

    def is_at_end(self):
        if self.ip < len(self.closure.function.chunk.opCodeList):
            return False
        return True


class VM:
    __globalVariables: list[Object] = []
    __stackTop: int
    __objectStack: list[Object] = []
    __callFrameTop: int
    __callFrameStack: list[CallFrame] = []
    __open_upvalues: UpvalueObject

    def __init__(self) -> None:
        self.__reset_status()

    def __reset_status(self) -> None:
        self.__globalVariables = [NilObject()]*STACK_COUNT
        self.__stackTop = 0
        self.__objectStack = [NilObject()]*STACK_COUNT
        self.__callFrameTop = 0
        self.__callFrameStack = [CallFrame()]*STACK_COUNT
        self.__open_upvalues = None

    def run(self, mainFn: FunctionObject) -> None:
        self.__reset_status()

        mainClosure = ClosureObject(mainFn)
        mainCallFrame = CallFrame(mainClosure, self.__stackTop)
        self.__push_call_frame(mainCallFrame)
        self.__stackTop = mainCallFrame.slot + mainFn.localVarCount

        self.__execute()

    def __execute(self) -> None:
        while True:
            frame = self.__peek_call_frame(1)

            if frame.is_at_end():
                return

            instruction = frame.closure.function.chunk.opCodeList[frame.ip]
            frame.ip += 1

            if instruction == OpCode.OP_RETURN:
                returnCount = frame.closure.function.chunk.opCodeList[frame.ip]
                frame.ip += 1

                object = NilObject()
                if returnCount == 1:
                    object = self.__pop()

                self.__closed_upvalues(id(self.__objectStack[frame.slot]))

                callFrame = self.__pop_call_frame()

                if self.__callFrameTop == 0:
                    return

                self.__stackTop = callFrame.slot-1

                if returnCount == 1:
                    self.__push(object)

            elif instruction == OpCode.OP_CONSTANT:
                idx = frame.closure.function.chunk.opCodeList[frame.ip]
                frame.ip += 1

                object = frame.closure.function.chunk.constants[idx]
                self.__push(object)

            elif instruction == OpCode.OP_ADD:
                left = self.__find_actual_object(self.__pop())
                right = self.__find_actual_object(self.__pop())

                if left.type == ObjectType.NUM and right.type == ObjectType.NUM:
                    self.__push(NumObject(left.value+right.value))
                elif left.type == ObjectType.STR and right.type == ObjectType.STR:
                    self.__push(StrObject(left.value+right.value))
                else:
                    error("Invalid binary op:" +
                          left.__str__() + "+" + right.__str__())

            elif instruction == OpCode.OP_SUB:
                left = self.__find_actual_object(self.__pop())
                right = self.__find_actual_object(self.__pop())

                if left.type == ObjectType.NUM and right.type == ObjectType.NUM:
                    self.__push(NumObject(left.value-right.value))
                else:
                    error("Invalid binary op:" +
                          left.__str__()+"-"+right.__str__())

            elif instruction == OpCode.OP_MUL:
                left = self.__find_actual_object(self.__pop())
                right = self.__find_actual_object(self.__pop())

                if left.type == ObjectType.NUM and right.type == ObjectType.NUM:
                    self.__push(NumObject(left.value*right.value))
                else:
                    error("Invalid binary op:" +
                          left.__str__()+"*"+right.__str__())

            elif instruction == OpCode.OP_DIV:
                left = self.__find_actual_object(self.__pop())
                right = self.__find_actual_object(self.__pop())

                if left.type == ObjectType.NUM and right.type == ObjectType.NUM:
                    self.__push(NumObject(left.value/right.value))
                else:
                    error("Invalid binary op:" +
                          left.__str__()+"/"+right.__str__())

            elif instruction == OpCode.OP_GREATER:
                left = self.__find_actual_object(self.__pop())
                right = self.__find_actual_object(self.__pop())

                if left.type == ObjectType.NUM and right.type == ObjectType.NUM:
                    self.__push(BoolObject(left.value > right.value))
                else:
                    self.__push(BoolObject(False))

            elif instruction == OpCode.OP_LESS:
                left = self.__find_actual_object(self.__pop())
                right = self.__find_actual_object(self.__pop())

                if left.type == ObjectType.NUM and right.type == ObjectType.NUM:
                    self.__push(BoolObject(left.value < right.value))
                else:
                    self.__push(BoolObject(False))

            elif instruction == OpCode.OP_EQUAL:
                left = self.__find_actual_object(self.__pop())
                right = self.__find_actual_object(self.__pop())

                self.__push(BoolObject(left == right))

            elif instruction == OpCode.OP_NOT:
                obj = self.__find_actual_object(self.__pop())

                if obj.type != ObjectType.BOOL:
                    error("Not a boolean obj of:" + obj.__str__())
                self.__push(BoolObject(not obj.value))

            elif instruction == OpCode.OP_MINUS:
                obj = self.__find_actual_object(self.__pop())
                if obj.type != ObjectType.NUM:
                    error("Invalid op:'-'"+obj.__str__())
                self.__push(NumObject(-obj.value))

            elif instruction == OpCode.OP_AND:
                left = self.__find_actual_object(self.__pop())
                right = self.__find_actual_object(self.__pop())

                if left.type == ObjectType.BOOL and right.type == ObjectType.BOOL:
                    self.__push(BoolObject(left.value and right.value))
                else:
                    error("Invalid op:" + left.__str__() +
                          " and " + right.__str__())

            elif instruction == OpCode.OP_OR:
                left = self.__find_actual_object(self.__pop())
                right = self.__find_actual_object(self.__pop())

                if left.type == ObjectType.BOOL and right.type == ObjectType.BOOL:
                    self.__push(BoolObject(left.value or right.value))
                else:
                    error("Invalid op:" + left.__str__() +
                          " or " + right.__str__())

            elif instruction == OpCode.OP_BIT_AND:
                left = self.__find_actual_object(self.__pop())
                right = self.__find_actual_object(self.__pop())

                if left.type == ObjectType.NUM and right.type == ObjectType.NUM:
                    self.__push(NumObject(int(left.value) & int(right.value)))
                else:
                    error("Invalid op:" + left.__str__() +
                          " & " + right.__str__())

            elif instruction == OpCode.OP_BIT_OR:
                left = self.__find_actual_object(self.__pop())
                right = self.__find_actual_object(self.__pop())

                if left.type == ObjectType.NUM and right.type == ObjectType.NUM:
                    self.__push(NumObject(int(left.value) | int(right.value)))
                else:
                    error("Invalid op:" + left.__str__() +
                          " | " + right.__str__())

            elif instruction == OpCode.OP_BIT_XOR:
                left = self.__find_actual_object(self.__pop())
                right = self.__find_actual_object(self.__pop())

                if left.type == ObjectType.NUM and right.type == ObjectType.NUM:
                    self.__push(NumObject(int(left.value) ^ int(right.value)))
                else:
                    error("Invalid op:" + left.__str__() +
                          " ^ " + right.__str__())

            elif instruction == OpCode.OP_BIT_NOT:
                obj = self.__find_actual_object(self.__pop())

                if obj.type != ObjectType.NUM:
                    error("Not a number obj of:" + obj.__str__())
                self.__push(NumObject(~int(obj.value)))

            elif instruction == OpCode.OP_ARRAY:
                numElements = frame.closure.function.chunk.opCodeList[frame.ip]
                frame.ip += 1

                self.__stackTop -= numElements

                elements = [None]*numElements

                for i in range(0, numElements):
                    elements[i] = self.__objectStack[self.__stackTop+i]

                self.__push(ArrayObject(elements))

            elif instruction == OpCode.OP_GET_INDEX:
                index = self.__pop()
                ds = self.__pop()
                if ds.type == ObjectType.ARRAY and index.type == ObjectType.NUM:
                    i = int(index.value)
                    if i < 0 or i >= len(ds.elements):
                        self.__push(NilObject())
                    else:
                        self.__push(ds.elements[i])
                else:
                    error("Invalid index op:" + ds.__str__() +
                          "[" + index.__str__() + "]")

            elif instruction == OpCode.OP_SET_INDEX:
                index = self.__pop()
                ds = self.__pop()
                v = self.__pop()
                if ds.type == ObjectType.ARRAY and index.type == ObjectType.NUM:
                    i = int(index.value)
                    if i < 0 or i >= len(ds.elements):
                        error("Invalid index:" + str(i) +
                              " outside of array's:" + str(len(ds.elements)))
                    else:
                        if ds.elements[i].type == ObjectType.REF:
                            self.__set_value(ds.elements[i], v)
                        else:
                            originAddress = id(ds.elements[i])
                            ds.elements[i] = v
                            self.__update_ref_address(originAddress, id(v))

                else:
                    error("Invalid index op:" + ds.__str__() +
                          "[" + index.__str__() + "]")

            elif instruction == OpCode.OP_JUMP_IF_FALSE:
                address = frame.closure.function.chunk.opCodeList[frame.ip]
                frame.ip += 1
                obj = self.__pop()
                if obj.type != ObjectType.BOOL:
                    error("The if condition not a boolean value:" + obj.__str__())
                if obj.value != True:
                    frame.ip = address+1

            elif instruction == OpCode.OP_JUMP:
                address = frame.closure.function.chunk.opCodeList[frame.ip]
                frame.ip = address+1

            elif instruction == OpCode.OP_DEF_GLOBAL:
                index = frame.closure.function.chunk.opCodeList[frame.ip]
                frame.ip += 1
                obj = self.__pop()
                self.__globalVariables[index] = obj

            elif instruction == OpCode.OP_SET_GLOBAL:
                index = frame.closure.function.chunk.opCodeList[frame.ip]
                frame.ip += 1
                obj = self.__pop()

                if self.__globalVariables[index].type == ObjectType.REF:
                    self.__set_value(self.__globalVariables[index], obj)
                else:
                    originAddress = id(self.__globalVariables[index])
                    self.__globalVariables[index] = obj
                    self.__update_ref_address(originAddress, id(obj))

            elif instruction == OpCode.OP_GET_GLOBAL:
                index = frame.closure.function.chunk.opCodeList[frame.ip]
                frame.ip += 1
                self.__push(self.__globalVariables[index])

            elif instruction == OpCode.OP_FUNCTION_CALL:
                argCount = frame.closure.function.chunk.opCodeList[frame.ip]
                frame.ip += 1

                obj = self.__objectStack[self.__stackTop-1-argCount]
                if obj.type == ObjectType.CLOSURE:
                    if argCount != obj.function.parameterCount:
                        error("Non matching function parameters for calling arguments,parameter count:" +
                              obj.function.parameterCount + ",argument count:" + argCount)
                    callFrame = CallFrame(obj, self.__stackTop-argCount)
                    self.__push_call_frame(callFrame)
                    self.__stackTop = callFrame.slot+obj.function.localVarCount

                elif obj.type == ObjectType.BUILTIN:
                    args = self.__objectStack[self.__stackTop -
                                              argCount:self.__stackTop]
                    self.__stackTop -= (argCount+1)

                    hasReturnValue, returnValue = obj.data(args)
                    if hasReturnValue == True:
                        self.__push(returnValue)
                else:
                    error("Calling not a function or a builtinFn")

            elif instruction == OpCode.OP_DEF_LOCAL:
                index = frame.closure.function.chunk.opCodeList[frame.ip]
                frame.ip += 1

                obj = self.__pop()

                slot = self.__get_local_variable_slot(index)
                self.__objectStack[slot] = obj

            elif instruction == OpCode.OP_SET_LOCAL:
                index = frame.closure.function.chunk.opCodeList[frame.ip]
                frame.ip += 1

                obj = self.__pop()

                if self.__objectStack[self.__get_local_variable_slot(index)].type == ObjectType.REF:
                    self.__set_value(
                        self.__objectStack[self.__get_local_variable_slot(index)], obj)
                else:
                    originAddress = self.__objectStack[self.__get_local_variable_slot(
                        index)]
                    self.__objectStack[self.__get_local_variable_slot(
                        index)] = obj
                    self.__update_ref_address(originAddress, id(obj))

            elif instruction == OpCode.OP_GET_LOCAL:
                index = frame.closure.function.chunk.opCodeList[frame.ip]
                frame.ip += 1

                slot = self.__get_local_variable_slot(index)

                self.__push(self.__objectStack[slot])

            elif instruction == OpCode.OP_GET_BUILTIN:
                name = self.__pop().value
                builtinObj = gBuiltinManager.builtinObjects[name]
                self.__push(builtinObj)

            elif instruction == OpCode.OP_STRUCT:
                members: dict[str, Object] = {}

                memberCount = frame.closure.function.chunk.opCodeList[frame.ip]
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
                self.__push(structInstance)

            elif instruction == OpCode.OP_GET_STRUCT:
                memberName = self.__pop()
                instance = self.__pop()
                instance, _ = self.__get_end_of_ref_object(instance)

                if memberName.type == ObjectType.STR:
                    iter = instance.members.get(memberName.value, None)
                    if iter == None:
                        error("no member named:(" + memberName.__str__() +
                              ") in struct instance:" + instance.__str__())
                    self.__push(iter)

            elif instruction == OpCode.OP_SET_STRUCT:
                memberName = self.__pop()
                instance = self.__pop()
                instance, _ = self.__get_end_of_ref_object(instance)

                obj = self.__pop()
                if memberName.type == ObjectType.STR:
                    iter = instance.members.get(memberName.value, None)
                    if iter == None:
                        error("no member named:(" + memberName.__str__() +
                              ") in struct instance:" + instance.__str__())

                    if instance.members[memberName.value].type == ObjectType.REF:
                        self.__set_value(
                            instance.members[memberName.value], obj)
                    else:
                        originAddress = id(instance.members[memberName.value])
                        instance.members[memberName.value] = obj
                        self.__update_ref_address(originAddress, id(obj))

            elif instruction == OpCode.OP_REF_GLOBAL:
                index = frame.closure.function.chunk.opCodeList[frame.ip]
                frame.ip += 1
                obj, _ = self.__get_end_of_ref_object(self.__globalVariables[index])
                self.__push(RefObject(id(obj)))

            elif instruction == OpCode.OP_REF_LOCAL:
                index = frame.closure.function.chunk.opCodeList[frame.ip]
                frame.ip += 1

                slot = self.__get_local_variable_slot(index)
                obj, _ = self.__get_end_of_ref_object(self.__objectStack[slot])
                self.__push(RefObject(id(obj)))

            elif instruction == OpCode.OP_REF_INDEX_GLOBAL:
                index = frame.closure.function.chunk.opCodeList[frame.ip]
                frame.ip += 1
                idxValue = self.__pop()

                obj, _ = self.__get_end_of_ref_object(self.__globalVariables[index])

                self.__push(self.__allocate_index_ref_object(obj, idxValue))

            elif instruction == OpCode.OP_REF_INDEX_LOCAL:
                index = frame.closure.function.chunk.opCodeList[frame.ip]
                frame.ip += 1

                idxValue = self.__pop()
                slot = self.__get_local_variable_slot(index)

                obj, _ = self.__get_end_of_ref_object(self.__objectStack[slot])

                self.__push(self.__allocate_index_ref_object(obj, idxValue))

            elif instruction == OpCode.OP_DLL_IMPORT:
                name = self.__pop().value
                register_dlls(name)

            elif instruction == OpCode.OP_CLOSURE:
                pos = frame.closure.function.chunk.opCodeList[frame.ip]
                frame.ip += 1
                upvalueCount = frame.closure.function.chunk.opCodeList[frame.ip]
                frame.ip += 1
                closureObj = ClosureObject(
                    frame.closure.function.chunk.constants[pos])

                for i in range(0, upvalueCount):
                    index = frame.closure.function.chunk.opCodeList[frame.ip]
                    frame.ip += 1
                    scopeDepth = frame.closure.function.chunk.opCodeList[frame.ip]
                    frame.ip += 1

                    upvalue = self.__capture_upvalue(
                        id(self.__objectStack[self.__get_upvalue_variable_slot(index, scopeDepth)]))
                    closureObj.upvalues[i] = upvalue

                self.__push(closureObj)

            elif instruction == OpCode.OP_GET_UPVALUE:
                index = frame.closure.function.chunk.opCodeList[frame.ip]
                frame.ip += 1
                upvalue = frame.closure.upvalues[index]
                self.__push(search_object_by_address(upvalue.pointer))

            elif instruction == OpCode.OP_SET_UPVALUE:
                index = frame.closure.function.chunk.opCodeList[frame.ip]
                frame.ip += 1

                obj = self.__pop()

                frame.closure.upvalues[index].pointer = id(obj)

            elif instruction == OpCode.OP_REF_UPVALUE:
                index = frame.closure.function.chunk.opCodeList[frame.ip]
                frame.ip += 1
                slot, _ = self.__get_end_of_ref_object(search_object_by_address(frame.closure.upvalues[index].pointer))
                self.__push(RefObject(id(slot)))

            elif instruction == OpCode.OP_REF_INDEX_UPVALUE:
                index = frame.closure.function.chunk.opCodeList[frame.ip]
                frame.ip += 1
                idxValue = self.__pop()
                slot, _ = self.__get_end_of_ref_object(
                    search_object_by_address(frame.closure.upvalues[index].pointer))
                self.__push(self.__allocate_index_ref_object(slot, idxValue))

    def __push(self, obj: Object) -> None:
        self.__objectStack[self.__stackTop] = obj
        self.__stackTop += 1

    def __pop(self) -> Object:
        self.__stackTop -= 1
        return self.__objectStack[self.__stackTop]

    def __push_call_frame(self, callframe: CallFrame) -> None:
        self.__callFrameStack[self.__callFrameTop] = callframe
        self.__callFrameTop += 1

    def __pop_call_frame(self) -> CallFrame:
        self.__callFrameTop -= 1
        return self.__callFrameStack[self.__callFrameTop]

    def __peek_call_frame(self, distance: int) -> CallFrame:
        return self.__callFrameStack[self.__callFrameTop - distance]

    def __allocate_index_ref_object(self, obj: Object, idxValue: Object) -> RefObject:
        if obj.type == ObjectType.ARRAY:
            if idxValue.type != ObjectType.NUM:
                error("Invalid idx for array,only integer is available.")
            if idxValue.value < 0 or idxValue.value >= len(obj.elements):
                error("Idx out of range.")
            return RefObject(id(obj.elements[int(idxValue.value)]))
        else:
            error("Invalid indexed reference type:" +
                  obj.__str__()+" not a array value.")

    def __find_actual_object(self, obj):
        # find actual object of RefObject or BuiltinVariable
        actual, _ = self.__get_end_of_ref_object(obj)
        if actual.type == ObjectType.BUILTIN:
            actual = actual.data
        return actual

    def __get_end_of_ref_object(self, obj: Object) -> tuple[Object, int]:
        address = -1
        refObj = obj
        while refObj.type == ObjectType.REF:
            address = refObj.pointer
            refObj = search_object_by_address(refObj.pointer)
        return refObj, address

    def __assign_struct_member_by_address(self, srcObject: StructObject, address, obj: Object):
        for k, v in srcObject.members.items():
            if id(v) == address:
                srcObject.members[k] = obj
                return True

        for k, v in srcObject.members.items():
            if v.type == ObjectType.STRUCT:
                subStructMember = srcObject.members[k]
                return self.__assign_struct_member_by_address(subStructMember, address, obj)

        return False

    def __assign_object_by_address(self, address, obj: Object):
        for i in range(0, len(self.__globalVariables)):
            if self.__globalVariables[i] != None and id(self.__globalVariables[i]) == address:
                self.__globalVariables[i] = obj
                return
            elif self.__globalVariables[i] != None and self.__globalVariables[i].type == ObjectType.ARRAY:
                for j in range(0, len(self.__globalVariables[i].elements)):
                    if id(self.__globalVariables[i].elements[j]) == address:
                        self.__globalVariables[i].elements[j] = obj
                        return
            elif self.__globalVariables[i] != None and self.__globalVariables[i].type == ObjectType.STRUCT:
                if self.__assign_struct_member_by_address(self.__globalVariables[i], address, obj):
                    return

        for p in range(0, self.__stackTop):
            if self.__objectStack[p] != None and id(self.__objectStack[p]) == address:
                self.__objectStack[p] = obj
                return
            elif self.__objectStack[p] != None and self.__objectStack[p].type == ObjectType.ARRAY:
                for j in range(0, len(self.__objectStack[p].elements)):
                    if id(self.__objectStack[p].elements[j]) == address:
                        self.__objectStack[p].elements[j] = obj
                        return
            elif self.__objectStack[p] != None and self.__objectStack[p].type == ObjectType.STRUCT:
                if self.__assign_struct_member_by_address(self.__objectStack[p], address, obj):
                    return

        for p in range(0, self.__callFrameTop):
            if self.__callFrameStack[p] != None and self.__callFrameStack[p].closure != None and self.__callFrameStack[p].closure.upvalues != None:
                for j in range(0, len(self.__callFrameStack[p].closure.upvalues)):
                    if self.__callFrameStack[p].closure.upvalues[j] != None:
                        if self.__callFrameStack[p].closure.upvalues[j].pointer == address:
                            self.__callFrameStack[p].closure.upvalues[j].pointer = id(
                                obj)
                            return
                        else:
                            rawObject = search_object_by_address(
                                self.__callFrameStack[p].closure.upvalues[j].pointer)
                            if rawObject != None and rawObject.type == ObjectType.ARRAY:
                                for k in range(0, len(rawObject.elements)):
                                    if id(rawObject.elements[k]) == address:
                                        rawObject.elements[k] = obj
                                        return
                            elif rawObject != None and rawObject.type == ObjectType.STRUCT:
                                if self.__assign_struct_member_by_address(rawObject, address, obj):
                                    return

    def __search_ref_object_list_by_address(self, address: int) -> list:
        result: list = []
        for i in range(0, len(self.__globalVariables)):
            if self.__globalVariables[i] != None and self.__globalVariables[i].type == ObjectType.REF:
                if self.__globalVariables[i].pointer == address:
                    result.append(self.__globalVariables[i])
            elif self.__globalVariables[i] != None and self.__globalVariables[i].type == ObjectType.ARRAY:
                for j in range(0, len(self.__globalVariables[i].elements)):
                    if self.__globalVariables[i].elements[j].type == ObjectType.REF:
                        if self.__globalVariables[i].elements[j].pointer == address:
                            result.append(
                                self.__globalVariables[i].elements[j])
            elif self.__globalVariables[i] != None and self.__globalVariables[i].type == ObjectType.STRUCT:
                for k, v in self.__globalVariables[i].members.items():
                    if v.type == ObjectType.REF:
                        if v.pointer == address:
                            result.append(v)

        for p in range(0, self.__stackTop):
            if self.__objectStack[p] != None and self.__objectStack[p].type == ObjectType.REF:
                if self.__objectStack[p].pointer == address:
                    result.append(self.__objectStack[p])

            elif self.__objectStack[p] != None and self.__objectStack[p].type == ObjectType.ARRAY:
                for j in range(0, len(self.__objectStack[p].elements)):
                    if self.__objectStack[p].elements[j].type == ObjectType.REF:
                        if self.__objectStack[p].elements[j].pointer == address:
                            result.append(self.__objectStack[p].elements[j])

            elif self.__objectStack[p] != None and self.__objectStack[p].type == ObjectType.STRUCT:
                for k,v in self.__objectStack[p].members.items():
                    if v.type == ObjectType.REF:
                        if v.pointer == address:
                            result.append(v)

        for p in range(0, self.__callFrameTop):
            if self.__callFrameStack[p] != None and self.__callFrameStack[p].closure != None and self.__callFrameStack[p].closure.upvalues != None:
                for j in range(0, len(self.__callFrameStack[p].closure.upvalues)):
                    if self.__callFrameStack[p].closure.upvalues[j] != None:
                        if self.__callFrameStack[p].closure.upvalues[j].pointer == address:
                            result.append(
                                self.__callFrameStack[p].closure.upvalues[j])
                        else:
                            rawObj = search_object_by_address(
                                self.__callFrameStack[p].closure.upvalues[j].pointer)
                            if rawObj != None and rawObj.type == ObjectType.ARRAY:
                                for k in range(0, len(rawObj.elements)):
                                    if rawObj.elements[k].type == ObjectType.REF:
                                        if rawObj.elements[k].pointer == address:
                                            result.append(rawObj.elements[k])
                            elif rawObj != None and rawObj.type == ObjectType.STRUCT:
                                for k,v in rawObj.members.items():
                                    if v.type == ObjectType.REF:
                                        if v.pointer == address:
                                            result.append(v)

        return result

    def __update_ref_address(self, originAddress, newAddress):
        for i in range(0, len(self.__globalVariables)):
            if self.__globalVariables[i] != None and self.__globalVariables[i].type == ObjectType.REF:
                if self.__globalVariables[i].pointer == originAddress:
                    self.__globalVariables[i].pointer = newAddress
            elif self.__globalVariables[i] != None and self.__globalVariables[i].type == ObjectType.ARRAY:
                for j in range(0, len(self.__globalVariables[i].elements)):
                    if self.__globalVariables[i].elements[j].type == ObjectType.REF:
                        if self.__globalVariables[i].elements[j].pointer == originAddress:
                            self.__globalVariables[i].elements[j].pointer = newAddress
            elif self.__globalVariables[i] != None and self.__globalVariables[i].type == ObjectType.STRUCT:
                for k,v in self.__globalVariables[i].members.items():
                    if v.type == ObjectType.REF:
                        if v.pointer == originAddress:
                            v.pointer = newAddress
        for p in range(0, self.__stackTop):
            if self.__objectStack[p] != None and self.__objectStack[p].type == ObjectType.REF:
                if self.__objectStack[p].pointer == originAddress:
                    self.__objectStack[p].pointer = newAddress
            elif self.__objectStack[p] != None and self.__objectStack[p].type == ObjectType.ARRAY:
                for j in range(0, len(self.__objectStack[p].elements)):
                    if self.__objectStack[p].elements[j].type == ObjectType.REF:
                        if self.__objectStack[p].elements[j].pointer == originAddress:
                            self.__objectStack[p].elements[j].pointer = newAddress
            elif self.__objectStack[p] != None and self.__objectStack[p].type == ObjectType.STRUCT:
                for k,v in self.__objectStack[p].members.items():
                    if v.type == ObjectType.REF:
                        if v.pointer == originAddress:
                            v.pointer = newAddress

        for p in range(0, self.__callFrameTop):
            if self.__callFrameStack[p] != None and self.__callFrameStack[p].closure != None and self.__callFrameStack[p].closure.upvalues != None:
                for j in range(0, len(self.__callFrameStack[p].closure.upvalues)):
                    if self.__callFrameStack[p].closure.upvalues[j] != None:
                        if self.__callFrameStack[p].closure.upvalues[j].pointer == originAddress:
                            self.__callFrameStack[p].closure.upvalues[j].pointer = newAddress
                        else:
                            rawObj = search_object_by_address(self.__callFrameStack[p].closure.upvalues[j].pointer)
                            if rawObj != None and rawObj.type == ObjectType.ARRAY:
                                for k in range(0, len(rawObj.elements)):
                                    if rawObj.elements[k].type == ObjectType.REF:
                                        if rawObj.elements[k].pointer == originAddress:
                                            rawObj.elements[k].pointer = newAddress
                            elif rawObj != None and rawObj.type == ObjectType.STRUCT:
                                for k,v in rawObj.members.items():
                                    if v.type == ObjectType.REF:
                                        if v.pointer == originAddress:
                                            v.pointer = newAddress

    def __set_value(self, slot: RefObject, value: Object):
        if slot != None and slot.type == ObjectType.REF:
            address = 0
            if value != None and value.type != ObjectType.REF:
                _, address = self.__get_end_of_ref_object(slot)
            else:
                address = id(slot)

            self.__assign_object_by_address(address, value)

            refObjs = self.__search_ref_object_list_by_address(address)
            for i in range(0, len(refObjs)):
                refObjs[i].pointer = id(value)

    def __get_local_variable_slot(self, index) -> int:
        return self.__peek_call_frame(1).slot+index

    def __get_upvalue_variable_slot(self, index, scopeDepth) -> int:
        return self.__callFrameStack[scopeDepth].slot+index

    def __capture_upvalue(self, pointer: int) -> UpvalueObject:
        prevUpValue: UpvalueObject = None
        upvalue: UpvalueObject = self.__open_upvalues
        while upvalue != None and upvalue.pointer > pointer:
            prevUpValue = upvalue
            upvalue = upvalue.nextUpvalue

        if upvalue != None and upvalue.pointer == pointer:
            return upvalue

        createdUpvalue = UpvalueObject(pointer)

        if prevUpValue == None:
            self.__open_upvalues = createdUpvalue
        else:
            prevUpValue.nextUpvalue = createdUpvalue
        return createdUpvalue

    def __closed_upvalues(self, last: int) -> None:
        while self.__open_upvalues != None and self.__open_upvalues.pointer >= last:
            upvalue: UpvalueObject = self.__open_upvalues
            upvalue.closed = search_object_by_address(upvalue.pointer)
            upvalue.pointer = id(upvalue.closed)
            self.__open_upvalues = upvalue.nextUpvalue
