from audioop import add
from enum import IntEnum
import abc
from xml.etree.ElementTree import tostring
from Utils import Assert

class ObjectType(IntEnum):
    NUM = 0,
    STR = 1,
    BOOL = 2,
    NIL = 3,
    ARRAY = 4,
    STRUCT = 5,
    REF=6,
    LAMBDA=7


class Object:
    @abc.abstractmethod
    def Stringify(self) -> str:
        pass

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

    def Stringify(self) -> str:
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

    def Stringify(self) -> str:
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

    def Stringify(self) -> str:
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

    def Stringify(self) -> str:
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

    def Stringify(self) -> str:
        result = "["
        if len(self.elements) > 0:
            for e in self.elements:
                result += e.Stringify()+","
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
            i=i+1
        return True

class RefObject(Object):
    name:str

    def __init__(self,name:str) -> None:
        self.name=name

    def Stringify(self) -> str:
        return self.name

    def Type(self) -> ObjectType:
        return ObjectType.REF

    def IsEqualTo(self, other) -> bool:
        if other.Type()!=ObjectType.REF:
            return False
        return self.name==other.name

class LambdaObject(Object):
    idx:int=-1

    def __init__(self,idx:int) -> None:
        self.idx=idx

    def Stringify(self) -> str:
        return "lambda:"+ str(self.idx)

    def Type(self) -> ObjectType:
        return ObjectType.LAMBDA

    def IsEqualTo(self, other) -> bool:
        if other.Type()!=ObjectType.LAMBDA:
            return False
        return self.idx==other.idx

class StructObject(Object):
    name = ""
    members: dict[str, Object] = {}

    def __init__(self, name, members) -> None:
        self.name = name
        self.members = members

    def Stringify(self) -> str:
        result = "struct instance "+self.name
        if len(self.members) > 0:
            result += ":\n"
            for key, value in self.members.items():
                result += key + "="+value.Stringify()+"\n"
            result = result[0:len(result)-1]
        return result

    def Type(self) -> ObjectType:
        return ObjectType.STRUCT

    def IsEqualTo(self, other) -> bool:
        if other.Type() != ObjectType.STRUCT:
            return False

        if len(self.members) != len(other.members):
            return False

        for key1,value1 in self.members.items():
            for key2,value2 in other.members.items():
                if (key1==key2):
                    if(not value1.IsEqualTo(value2)):
                        return False
        return True
    
    def DefineMember(self,name:str,value:Object):
        if self.members.get(name)!=None:
            Assert("Redefined struct member:" + name)
        else:
            self.members[name]=value

    def AssignMember(self,name:str,value:Object):
        if self.members.get(name)!=None:
            self.members[name]=value
        else:
            Assert("Undefine struct member:" + name)

    def GetMember(self,name)->Object:
        if self.members.get(name)!=None:
            return self.members[name]
        else:
            return None
        