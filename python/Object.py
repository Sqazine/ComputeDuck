import abc
import ctypes
from enum import IntEnum
from typing import Union


class ObjectType(IntEnum):
    NUM = 0,
    STR = 1,
    BOOL = 2,
    NIL = 3,
    ARRAY = 4,
    STRUCT = 5,
    REF = 6,
    FUNCTION = 7,
    BUILTIN = 8,

class Object:
    type: ObjectType

    def __init__(self, type) -> None:
        self.type = type
    
    def __eq__(self, other) -> bool:
        if other==None or (not isinstance(other,Object)) or other.type != self.type:
            return False
        return True

class NumObject(Object):
    value = 0.0

    def __init__(self, value) -> None:
        super().__init__(ObjectType.NUM)
        self.value = value

    def __str__(self) -> str:
        return str(self.value)

    def __eq__(self, other) -> bool:
        if other==None or other.type != ObjectType.NUM:
            return False
        return self.value == other.value


class StrObject(Object):
    value = ""

    def __init__(self, value) -> None:
        super().__init__(ObjectType.STR)
        self.value = value

    def __str__(self) -> str:
        return self.value

    def __eq__(self, other) -> bool:
        return super.__eq__(other) and self.value == other.value

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

    def __eq__(self, other) -> bool:
        return super.__eq__(other) and self.value == other.value


class NilObject(Object):

    def __init__(self) -> None:
        super().__init__(ObjectType.NIL)

    def __str__(self) -> str:
        return "nil"


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

    def __eq__(self, other) -> bool:
        if super.__eq__(other) == False:
            return False

        if len(self.elements) != len(other.elements):
            return False

        for i in range(0,len(self.elements)):
            if not (self.elements[i]==other.elements[i]):
                return False
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

    def __eq__(self, other) -> bool:
        return self.__eq__(other) and self.pointer == other.pointer


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

    def __eq__(self, other) -> bool:
        if self.__eq__(other) == False:
            return False
        otherOpCodes = other.opCodes
        if len(self.opCodes) != len(otherOpCodes):
            return False
        for i in range(0, len(self.opCodes)):
            if self.opCodes[i] != otherOpCodes[i]:
                return False
        return True
    
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

    def __eq__(self, other) -> bool:
        if self.__eq__(other) == False:
            return False
        if len(self.members) != len(other.members):
            return False
        for key1, value1 in self.members.items():
            v2 = other.members.get(key1)
            if (v2 == None or (not v2==value1)):
                return False
        return True

class BuiltinObject(Object):
    data:any #builtin function,builtin native data or builtin constant value
    destroyFunc:any #only available on builtin native data 

    def __init__(self, data, destroyFunc=None) -> None:
        super().__init__(ObjectType.BUILTIN)
        self.data = data
        self.destroyFunc=destroyFunc

    def __del__(self) -> None:
        if self.destroyFunc != None:
            self.destroyFunc(self.data)

    def __str__(self) -> str:
        return "Builtin:(0x"+str(id(self))+")"

    def __eq__(self, other) -> bool:
        return self.__eq__(other) and self.data == other.data and self.destroyFunc==other.destroyFunction

# class BuiltinFunctionObject(Object):
#     fn: any

#     def __init__(self, fn: any) -> None:
#         super().__init__(ObjectType.BUILTIN_FUNCTION)
#         self.fn = fn

#     def __str__(self) -> str:
#         return "Builtin Function:(0x"+str(id(self))+")"

#     def __eq__(self, other) -> bool:
#         if other==None or other.type != ObjectType.BUILTIN_FUNCTION:
#             return False
#         return self.fn == other.fn
    

# class BuiltinDataObject(Object):
#     nativeData: any
#     destroyFunc: any

#     def __init__(self, nativeData, destroyFunc) -> None:
#         super().__init__(ObjectType.BUILTIN)
#         self.nativeData = nativeData
#         self.destroyFunc = destroyFunc

#     def __del__(self) -> None:
#         if self.destroyFunc!=None:
#             self.destroyFunc(self.nativeData)

#     def __str__(self) -> str:
#         return "Builtin Data:(0x"+id(self.nativeData)+")"

#     def __eq__(self, other):
#         if other==None or other.type != self.type:
#             return False
#         return self.nativeData == other.nativeData


# class BuiltinVariableObject(Object):
#     obj: Object

#     def __init__(self, obj) -> None:
#         super().__init__(ObjectType.BUILTIN)
#         self.obj = obj

#     def __str__(self) -> None:
#         return "Builtin Variable:("+self.obj.__str__()+")"

#     def __eq__(self, other: Object):
#         if other==None or other.type != self.type:
#             return False
#         return self.obj == other.obj