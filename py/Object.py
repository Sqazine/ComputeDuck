import abc
import ctypes
from enum import IntEnum
from typing import List


class ObjectType(IntEnum):
    NUM = 0,
    STR = 1,
    BOOL = 2,
    NIL = 3,
    ARRAY = 4,
    STRUCT_INSTANCE = 5,
    REF = 6,
    FUNCTION = 7,
    BUILTIN = 8


class Object:
    @abc.abstractmethod
    def Type(self) -> ObjectType:
        pass

    @abc.abstractmethod
    def IsEqualTo(self, other) -> bool:
        pass


class NumObject(Object):
    value = 0.0

    def __init__(self, value) -> None:
        self.value = value

    def __str__(self) -> str:
        return str(self.value)

    def Type(self) -> ObjectType:
        return ObjectType.NUM

    def IsEqualTo(self, other) -> bool:
        if other.Type() != ObjectType.NUM:
            return False
        return self.value == other.value


class StrObject(Object):
    value = ""

    def __init__(self, value) -> None:
        self.value = value

    def __str__(self) -> str:
        return self.value

    def Type(self) -> ObjectType:
        return ObjectType.STR

    def IsEqualTo(self, other) -> bool:
        if other.Type() != ObjectType.STR:
            return False
        return self.value == other.value


class BoolObject(Object):
    value = False

    def __init__(self, value) -> None:
        self.value = value

    def __str__(self) -> str:
        if self.value == True:
            return "true"
        else:
            return "false"

    def Type(self) -> ObjectType:
        return ObjectType.BOOL

    def IsEqualTo(self, other) -> bool:
        if other.Type() != ObjectType.BOOL:
            return False
        return self.value == other.value


class NilObject(Object):

    def __str__(self) -> str:
        return "nil"

    def Type(self) -> ObjectType:
        return ObjectType.NIL

    def IsEqualTo(self, other) -> bool:
        if other.Type() != ObjectType.NIL:
            return False
        return True


class ArrayObject(Object):
    elements: list = []

    def __init__(self, elements) -> None:
        self.elements = elements

    def __str__(self) -> str:
        result = "["
        if len(self.elements) > 0:
            for e in self.elements:
                result += e.__str__()+","
            result = result[0:len(result)-1]
        return result+"]"

    def Type(self) -> ObjectType:
        return ObjectType.ARRAY

    def IsEqualTo(self, other) -> bool:
        if other.Type() != ObjectType.ARRAY:
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
    #TODO
    # this a bit different from C++ part,C++ allow pointer to pointing to the actual object
    # but in python the RefObject only record the actual object's address,and search the object in runtime
    # this is not a good way to implemente reference feature,because it waste much time to search the global object and local object in vm's __globalVariable and __objectStack
    # but now i don't have better propose
    # welcome for better idea!
    pointer:int 

    def __init__(self,pointer: int) -> None:
        self.pointer=pointer

    def __str__(self) -> str:
        return str(ctypes.cast(self.pointer,ctypes.py_object).value)

    def Type(self) -> ObjectType:
        return ObjectType.REF

    def IsEqualTo(self, other) -> bool:
        if other.Type() != ObjectType.REF:
            return False
        return self.pointer==other.pointer


class FunctionObject(Object):
    opCodes: List[int]
    localVarCount: int
    parameterCount: int

    def __init__(self, opCodes: List[int], localVarCount: int = 0, parameterCount: int = 0) -> None:
        self.opCodes = opCodes
        self.localVarCount = localVarCount
        self.parameterCount = parameterCount

    def __str__(self) -> str:
        return "function:(0x"+str(id(self))+")"

    def Type(self) -> ObjectType:
        return ObjectType.FUNCTION

    def IsEqualTo(self, other) -> bool:
        if other.Type() != ObjectType.FUNCTION:
            return False
        otherOpCodes = other.opCodes
        if len(self.opCodes) != len(otherOpCodes):
            return False

        for i in range(0, len(self.opCodes)):
            if self.opCodes[i] != otherOpCodes[i]:
                return False

        return True


class BuiltinObject(Object):
    name: str
    fn: any

    def __init__(self, name: str, fn: any) -> None:
        self.name = name
        self.fn = fn

    def __str__(self) -> str:
        return "builtin function:(0x"+str(id(self))+")"

    def Type(self) -> ObjectType:
        return ObjectType.BUILTIN

    def IsEqualTo(self, other) -> bool:
        if other.Type() != ObjectType.BUILTIN:
            return False
        return self.name == other.name


class StructInstanceObject(Object):
    members: dict[str, Object]

    def __init__(self, members: dict[str, Object]) -> None:
        self.members = members

    def __str__(self) -> str:
        result = "struct instance(0x"+str(id(self))+"):\n{\n"
        for k, v in self.members.items():
            result += k+":"+v.__str__()+"\n"
        result = result[0:len(result)-1]
        result += "\n}\n"
        return result

    def Type(self) -> ObjectType:
        return ObjectType.STRUCT_INSTANCE

    def IsEqualTo(self, other) -> bool:
        if other.Type() != ObjectType.STRUCT_INSTANCE:
            return False

        if len(self.members) != len(other.members):
            return False

        for key1, value1 in self.members.items():
            v2 = other.members.get(key1)
            if(v2 == None or (not v2.IsEqualTo(value1))):
                return False

        return True
