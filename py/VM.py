from typing import Any
from Context import Context
from Frame import Frame
from Object import Object
from Utils import Assert
from Object import ObjectType
from Frame import OpCode
from Object import BoolObject, NilObject, NumObject, StrObject
from Object import ArrayObject, StructObject


def println(args: list[Object]) -> Object:
    if len(args) == 0:
        return None
    print(args[0].Stringify())
    return None


def sizeof(args: list[Object]) -> Object:
    if len(args) == 0 or len(args) > 1:
        Assert("Native function 'siezeof':Expect a argument.")
    if args[0].Type() == ObjectType.ARRAY:
        return NumObject(len(args[0].elements))
    elif args[0].Type == ObjectType.STR:
        return NumObject(len(args[0].value))
    else:
        Assert("[Native function 'sizeof']:Expect a array or string argument.")
    return NilObject()


def insert(args: list[Object]) -> Object:
    if len(args) == 0 or len(args) != 3:
        Assert("[Native function 'insert']:Expect 3 arguments,the arg0 must be array,table or string object.The arg1 is the index object.The arg2 is the value object.")

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
    return None


def erase(args: list[Object]) -> Object:
    if len(args) or len(args) != 2:
        Assert("[Native function 'erase']:Expect 2 arguments,the arg0 must be array,table or string object.The arg1 is the corresponding index object.")

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


class VM:
    __context: Context = Context()
    __sp: int = 0
    __objectStack: list = [None]*128
    nativeFunctions: dict[str, Any] = {}

    def __init__(self) -> None:
        self.ResetStatus()

        self.nativeFunctions["println"] = println
        self.nativeFunctions["sizeof"] = sizeof

    def ResetStatus(self) -> None:
        self.__sp = 0
        self.__objectStack = [None]*128
        self.__context = Context()

    def GetNativeFunction(self, fnName: str):
        if self.nativeFunctions.get(fnName) != None:
            return self.nativeFunctions[fnName]
        Assert("No native function:"+fnName)

    def HasNativeFunction(self, name: str) -> bool:
        return self.nativeFunctions.get(name) != None

    def PushObject(self, object: Object) -> None:
        self.__objectStack[self.__sp] = object
        self.__sp += 1

    def PopObject(self) -> Object:
        self.__sp -= 1
        return self.__objectStack[self.__sp]

    def Execute(self, frame: Frame) -> Object:
        ip = 0
        while ip < len(frame.codes):
            instruction = frame.codes[ip]
            if instruction == OpCode.OP_RETURN:
                if self.__context.upContext!=None:
                    self.__context=self.__context.upContext
                return self.PopObject()
            elif instruction==OpCode.OP_STRUCT_RETURN:
                return self.PopObject()
            elif instruction == OpCode.OP_NEW_NUM:
                ip = ip+1
                self.PushObject(NumObject(frame.nums[frame.codes[ip]]))
            elif instruction == OpCode.OP_NEW_STR:
                ip = ip+1
                self.PushObject(StrObject(frame.strings[frame.codes[ip]]))
            elif instruction == OpCode.OP_NEW_TRUE:
                self.PushObject(BoolObject(True))
            elif instruction == OpCode.OP_NEW_FALSE:
                self.PushObject(BoolObject(False))
            elif instruction == OpCode.OP_NEW_NIL:
                self.PushObject(NilObject())
            elif instruction == OpCode.OP_NEG:
                object = self.PopObject()
                if object.Type() == ObjectType.NUM:
                    self.PushObject(NumObject(-object.value))
                else:
                    Assert("Invalid op:'-'" + object.Stringify())
            elif instruction == OpCode.OP_NOT:
                object = self.PopObject()
                if object.Type() == ObjectType.BOOL:
                    self.PushObject(BoolObject(not object.value))
                else:
                    Assert("Invalid op:'!'" + object.Stringify())
            elif instruction == OpCode.OP_ADD:
                left = self.PopObject()
                right = self.PopObject()
                if right.Type() == ObjectType.NUM and left.Type() == ObjectType.NUM:
                    self.PushObject(NumObject(left.value+right.value))
                else:
                    Assert("Invalid binary op:" +
                           left.Stringify()+"+"+right.Stringify())
            elif instruction == OpCode.OP_SUB:
                left = self.PopObject()
                right = self.PopObject()
                if right.Type() == ObjectType.NUM and left.Type() == ObjectType.NUM:
                    self.PushObject(NumObject(left.value-right.value))
                else:
                    Assert("Invalid binary op:" +
                           left.Stringify()+"-"+right.Stringify())
            elif instruction == OpCode.OP_MUL:
                left = self.PopObject()
                right = self.PopObject()
                if right.Type() == ObjectType.NUM and left.Type() == ObjectType.NUM:
                    self.PushObject(NumObject(left.value*right.value))
                else:
                    Assert("Invalid binary op:" +
                           left.Stringify()+"*"+right.Stringify())
            elif instruction == OpCode.OP_DIV:
                left = self.PopObject()
                right = self.PopObject()
                if right.Type() == ObjectType.NUM and left.Type() == ObjectType.NUM:
                    self.PushObject(NumObject(left.value/right.value))
                else:
                    Assert("Invalid binary op:" +
                           left.Stringify()+"/"+right.Stringify())
            elif instruction == OpCode.OP_GREATER:
                left = self.PopObject()
                right = self.PopObject()
                if right.Type() == ObjectType.NUM and left.Type() == ObjectType.NUM:
                    if left.value > right.value:
                        self.PushObject(BoolObject(True))
                    else:
                        self.PushObject(BoolObject(False))
                else:
                    self.PushObject(BoolObject(False))
            elif instruction == OpCode.OP_LESS:
                left = self.PopObject()
                right = self.PopObject()
                if right.Type() == ObjectType.NUM and left.Type() == ObjectType.NUM:
                    if left.value < right.value:
                        self.PushObject(BoolObject(True))
                    else:
                        self.PushObject(BoolObject(False))
                else:
                    self.PushObject(BoolObject(False))
            elif instruction == OpCode.OP_GREATER_EQUAL:
                left = self.PopObject()
                right = self.PopObject()
                if right.Type() == ObjectType.NUM and left.Type() == ObjectType.NUM:
                    if left.value >= right.value:
                        self.PushObject(BoolObject(True))
                    else:
                        self.PushObject(BoolObject(False))
                else:
                    self.PushObject(BoolObject(False))
            elif instruction == OpCode.OP_LESS_EQUAL:
                left = self.PopObject()
                right = self.PopObject()
                if right.Type() == ObjectType.NUM and left.Type() == ObjectType.NUM:
                    if left.value <= right.value:
                        self.PushObject(BoolObject(True))
                    else:
                        self.PushObject(BoolObject(False))
                else:
                    self.PushObject(BoolObject(False))
            elif instruction == OpCode.OP_EQUAL:
                left = self.PopObject()
                right = self.PopObject()
                self.PushObject(BoolObject(left.IsEqualTo(right)))
            elif instruction == OpCode.OP_NOT_EQUAL:
                left = self.PopObject()
                right = self.PopObject()
                self.PushObject(BoolObject(not left.IsEqualTo(right)))
            elif instruction == OpCode.OP_AND:
                left = self.PopObject()
                right = self.PopObject()
                if left.Type() == ObjectType.BOOL and right.Type == ObjectType.BOOL:
                    self.PushObject(left.value and right.value)
                else:
                    Assert("Invalid op:"+left.Stringify() +
                           " and "+right.Stringify())
            elif instruction == OpCode.OP_OR:
                left = self.PopObject()
                right = self.PopObject()
                if left.Type() == ObjectType.BOOL and right.Type == ObjectType.BOOL:
                    self.PushObject(left.value or right.value)
                else:
                    Assert("Invalid op:"+left.Stringify() +
                           " or "+right.Stringify())
            elif instruction == OpCode.OP_DEFINE_VAR:
                value = self.PopObject()
                ip = ip+1
                self.__context.DefineVariable(
                    frame.strings[frame.codes[ip]], value)
            elif instruction == OpCode.OP_SET_VAR:
                ip = ip+1
                name = frame.strings[frame.codes[ip]]
                value = self.PopObject()
                self.__context.AssignVariable(name, value)
            elif instruction == OpCode.OP_GET_VAR:
                ip = ip+1
                name = frame.strings[frame.codes[ip]]
                varObject = self.__context.GetVariable(name)

                #create a struct object
                if varObject == None:
                    if frame.HasStructFrame(name):
                        self.PushObject(self.Execute(
                            frame.GetStructFrame(name)))
                    else:
                        Assert("No Struct definition:"+name)
                else:
                    self.PushObject(varObject)
            elif instruction == OpCode.OP_NEW_ARRAY:
                elements: list[Object] = []
                ip = ip+1
                arraySize = frame.nums[frame.codes[ip]]
                for i in range(arraySize):
                    elements.insert(0, self.PopObject())
                self.PushObject(ArrayObject(elements))
            elif instruction == OpCode.OP_NEW_STRUCT:
                ip = ip+1
                self.PushObject(StructObject(
                    frame.strings[frame.codes[ip]], self.__context.values))
                self.__context = self.__context.upContext
            elif instruction == OpCode.OP_GET_INDEX_VAR:
                index = self.PopObject()
                object = self.PopObject()
                if object.Type() == ObjectType.ARRAY:
                    if index.Type() != ObjectType.NUM:
                        Assert(
                            "Invalid index op.The index type of the array object must ba a int num type,but got:" + index.Stringify())
                    iIndex = int(index.value)
                    if iIndex < 0 or iIndex > len(object.elements):
                        Assert("Index out of array range,array size:" +
                               len(object.elements) + ",index:" + iIndex)
                    self.PushObject(object.elements[iIndex])
                elif object.Type() == ObjectType.STR:
                    if index.Type() != ObjectType.NUM:
                        Assert(
                            "Invalid index op.The index type of the string object must ba a int num type,but got:" + index.Stringify())
                    iIndex = int(index.value)
                    if iIndex < 0 or iIndex > len(object.value):
                        Assert("Index out of array range,array size:" +
                               len(object.value) + ",index:" + iIndex)
                    self.PushObject(object.value[iIndex])
                else:
                    Assert(
                        "Invalid index op.The indexed object isn't a array or a string object:" + object.Stringify())
            elif instruction == OpCode.OP_SET_INDEX_VAR:
                index = self.PopObject()
                object = self.PopObject()
                assigner = self.PopObject()

                if object.Type() == ObjectType.ARRAY:
                    if index.Type() != ObjectType.NUM:
                        Assert(
                            "Invalid index op.The index type of the array object must ba a int num type,but got:" + index.Stringify())
                    iIndex = int(index.value)
                    if iIndex < 0 or iIndex > len(object.elements):
                        Assert("Index out of array range,array size:" +
                               len(object.elements) + ",index:" + iIndex)
                    object.elements[iIndex] = assigner
                else:
                    Assert(
                        "Invalid index op.The indexed object isn't a array object:" + object.Stringify())
            elif instruction == OpCode.OP_GET_STRUCT_VAR:
                ip += 1
                memberName = frame.strings[frame.codes[ip]]
                stackTop = self.PopObject()
                if stackTop.Type() != ObjectType.STRUCT:
                    Assert("Not a class object of the callee of:" + memberName)
                self.PushObject(stackTop.GetMember(memberName))
            elif instruction == OpCode.OP_SET_STRUCT_VAR:
                ip += 1
                memberName = frame.strings[frame.codes[ip]]
                stackTop = self.PopObject()
                if stackTop.Type() != ObjectType.STRUCT:
                    Assert("Not a class object of the callee of:" + memberName)
                assigner = self.PopObject()
                stackTop.AssignMember(memberName, assigner)
            elif instruction == OpCode.OP_ENTER_SCOPE:
                self.__context = Context(self.__context)
            elif instruction == OpCode.OP_EXIT_SCOPE:
                self.__context = self.__context.upContext
            elif instruction == OpCode.OP_JUMP_IF_FALSE:
                isJump = not (self.PopObject()).value
                ip = ip+1
                address = int(frame.nums[frame.codes[ip]])
                if isJump:
                    ip = address
            elif instruction == OpCode.OP_JUMP:
                ip = ip+1
                address = int(frame.nums[frame.codes[ip]])
                ip = address
            elif instruction == OpCode.OP_FUNCTION_CALL:
                ip = ip+1
                fnName = frame.strings[frame.codes[ip]]
                argCount = self.PopObject()

                if frame.HasFunctionFrame(fnName):
                    self.PushObject(self.Execute(
                        frame.GetFunctionFrame(fnName)))
                elif self.HasNativeFunction(fnName):
                    args: list[Object] = []
                    for i in range(argCount.value):
                        args.insert(0, self.PopObject())
                    result = self.GetNativeFunction(fnName)(args)
                    if result != None:
                        self.PushObject(result)
                else:
                    Assert("No function:"+fnName)

            ip += 1
        return NilObject()
