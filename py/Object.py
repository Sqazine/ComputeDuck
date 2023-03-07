import abc
import ctypes
from enum import IntEnum


class ObjectType(IntEnum):
    NUM = 0,
    STR = 1,
    BOOL = 2,
    NIL = 3,
    ARRAY = 4,
    STRUCT = 5,
    REF = 6,
    FUNCTION = 7,
    BUILTIN_FUNCTION = 8,
    BUILTIN_DATA = 9
    BUILTIN_VARIABLE = 10


class Object:
    type: ObjectType

    def __init__(self, type) -> None:
        self.type = type

    @abc.abstractmethod
    def IsEqualTo(self, other) -> bool:
        pass


class NumObject(Object):
    value = 0.0

    def __init__(self, value) -> None:
        super().__init__(ObjectType.NUM)
        self.value = value

    def __str__(self) -> str:
        return str(self.value)

    def IsEqualTo(self, other) -> bool:
        if other.type != ObjectType.NUM:
            return False
        return self.value == other.value


class StrObject(Object):
    value = ""

    def __init__(self, value) -> None:
        super().__init__(ObjectType.STR)
        self.value = value

    def __str__(self) -> str:
        return self.value

    def IsEqualTo(self, other) -> bool:
        if other.type != ObjectType.STR:
            return False
        return self.value == other.value


class BoolObject(Object):
    value = False

    def __init__(self, value) -> None:
        super().__init__(ObjectType.BOOL)
        self.value = value

    def __str__(self) -> str:
        if self.value == True:
            return "true"
        else:
            return "false"

    def IsEqualTo(self, other) -> bool:
        if other.type != ObjectType.BOOL:
            return False
        return self.value == other.value


class NilObject(Object):

    def __init__(self) -> None:
        super().__init__(ObjectType.NIL)

    def __str__(self) -> str:
        return "nil"

    def IsEqualTo(self, other) -> bool:
        if other.type != ObjectType.NIL:
            return False
        return True


class ArrayObject(Object):
    elements: list = []

    def __init__(self, elements) -> None:
        super().__init__(ObjectType.ARRAY)
        self.elements = elements

    def __str__(self) -> str:
        result = "["
        if len(self.elements) > 0:
            for e in self.elements:
                result += e.__str__()+","
            result = result[0:len(result)-1]
        return result+"]"

    def IsEqualTo(self, other) -> bool:
        if other.type != ObjectType.ARRAY:
            return False

        if len(self.elements) != len(other.elements):
            return False

        i = 0
        while i < (len(self.elements)):
            if not self.elements[i].IsEqualTo(other.elements[i]):
                return False
            i = i+1
        return True


class RefObject(Object):
    # TODO
    # this a bit different from C++ part,C++ allow pointer to pointing to the actual object
    # but in python the RefObject only record the actual object's address,and search the object in runtime
    # this is not a good way to implemente reference feature,because it waste much time to search the global object and local object in vm's __globalVariable and __objectStack
    # but now i don't have better propose
    # welcome for better idea!
    pointer: int

    def __init__(self, pointer: int) -> None:
        super().__init__(ObjectType.REF)
        self.pointer = pointer

    def __str__(self) -> str:
        return str(ctypes.cast(self.pointer, ctypes.py_object).value)

    def IsEqualTo(self, other) -> bool:
        if other.type != ObjectType.REF:
            return False
        return self.pointer == other.pointer


class FunctionObject(Object):
    opCodes: list[int]
    localVarCount: int
    parameterCount: int

    def __init__(self, opCodes: list[int], localVarCount: int = 0, parameterCount: int = 0) -> None:
        super().__init__(ObjectType.FUNCTION)
        self.opCodes = opCodes
        self.localVarCount = localVarCount
        self.parameterCount = parameterCount

    def __str__(self) -> str:
        return "function:(0x"+str(id(self))+")"

    def IsEqualTo(self, other) -> bool:
        if other.type != ObjectType.FUNCTION:
            return False
        otherOpCodes = other.opCodes
        if len(self.opCodes) != len(otherOpCodes):
            return False

        for i in range(0, len(self.opCodes)):
            if self.opCodes[i] != otherOpCodes[i]:
                return False

        return True


class BuiltinFunctionObject(Object):
    name: str
    fn: any

    def __init__(self, name: str, fn: any) -> None:
        super().__init__(ObjectType.BUILTIN_FUNCTION)
        self.name = name
        self.fn = fn

    def __str__(self) -> str:
        return "Builtin Function:(0x"+str(id(self))+")"

    def IsEqualTo(self, other) -> bool:
        if other.type != ObjectType.BUILTIN_FUNCTION:
            return False
        return self.name == other.name


class BuiltinDataObject(Object):

    nativeData: any
    destroyFunc: any

    def __init__(self, nativeData, destroyFunc) -> None:
        super().__init__(ObjectType.BUILTIN_DATA)
        self.nativeData = nativeData
        self.destroyFunc = destroyFunc

    def __del__(self) -> None:
        if self.destroyFunc!=None:
            self.destroyFunc(self.nativeData)

    def __str__(self) -> str:
        return "Builtin Data:(0x"+id(self.nativeData)+")"

    def IsEqualTo(self, other: Object) -> bool:
        if other.type != self.type:
            return False
        return self.nativeData == other.nativeData


class BuiltinVariableObject(Object):

    name: str
    obj: Object

    def __init__(self, name, obj) -> None:
        super().__init__(ObjectType.BUILTIN_VARIABLE)
        self.name = name
        self.obj = obj

    def __str__(self) -> None:
        return "Builtin Variable:("+self.name+":"+self.obj.__str__()+")"

    def IsEqualTo(self, other: Object) -> bool:
        if other.type != self.type:
            return False
        return self.name == other.name and self.obj == other.obj


class StructObject(Object):
    members: dict[str, Object]

    def __init__(self, members: dict[str, Object]) -> None:
        super().__init__(ObjectType.STRUCT)
        self.members = members

    def __str__(self) -> str:
        result = "struct instance(0x"+str(id(self))+"):\n{\n"
        for k, v in self.members.items():
            result += k+":"+v.__str__()+"\n"
        result = result[0:len(result)-1]
        result += "\n}\n"
        return result

    def IsEqualTo(self, other) -> bool:
        if other.type != ObjectType.STRUCT:
            return False

        if len(self.members) != len(other.members):
            return False

        for key1, value1 in self.members.items():
            v2 = other.members.get(key1)
            if (v2 == None or (not v2.IsEqualTo(value1))):
                return False

        return True
