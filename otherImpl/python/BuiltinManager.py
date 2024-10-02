from Object import *
from Utils import *
import time


class BuiltinManager(object):
    __instance = None
    builtinObjects: dict[str, BuiltinObject] = {}

    def __new__(cls, *args, **kw):
        if not cls.__instance:
            cls.__instance = super(
                BuiltinManager, cls).__new__(cls, *args, **kw)
        return cls.__instance

    def __init__(self) -> None:
        self.register("print", self.__print)
        self.register("println", self.__println)
        self.register("sizeof", self.__sizeof)
        self.register("insert", self.__insert)
        self.register("erase", self.__erase)
        self.register("clock", self.__clock)

    def __del__(self) -> None:
        self.builtinObjects.clear()

    def register(self, name: str, obj: Object) -> None:
        if name in self.builtinObjects:
            error("Redefined builtin:"+name)
        self.builtinObjects[name] = BuiltinObject(obj)

    def __println(self, args: list[Object]):
        if len(args) == 0:
            return False, None
        print(args[0])
        return False, None

    def __print(self, args: list[Object]):
        if len(args) == 0:
            return False, None
        print(args[0], end="")
        return False, None

    def __sizeof(self, args: list[Object]):
        if len(args) == 0 or len(args) > 1:
            error("Native function 'siezeof':Expect a argument.")
        if args[0].type == ObjectType.ARRAY:
            result = NumObject(len(args[0].elements))
        elif args[0].type == ObjectType.STR:
            result = NumObject(len(args[0].value))
        else:
            error("[Native function 'sizeof']:Expect a array or string argument.")
        return True, result

    def __insert(self, args: list[Object]):
        if len(args) == 0 or len(args) != 3:
            error("[Native function 'insert']:Expect 3 arguments,the arg0 must be array,table or string obj.The arg1 is the index obj.The arg2 is the value obj.")

        if args[0].type == ObjectType.ARRAY:
            if args[1].type != ObjectType.NUM:
                error("[Native function 'insert']:Arg1 must be integer type while insert to a array")
            iIndex = args[1].value
            if iIndex < 0 or iIndex >= len(args[0].elements):
                error("[Native function 'insert']:Index out of array's range")
            args[0].elements.insert(iIndex, args[2])
        elif args[0].type == ObjectType.STR:
            if args[1].type != ObjectType.NUM:
                error("[Native function 'insert']:Arg1 must be integer type while insert to a string")
            iIndex = args[1].value
            if iIndex < 0 or iIndex >= len(args[0].values):
                error("[Native function 'insert']:Index out of array's range")
            args[0].values.insert(iIndex, args[2].__str__())
        return False, None

    def __erase(self, args: list[Object]):
        if len(args) or len(args) != 2:
            error("[Native function 'erase']:Expect 2 arguments,the arg0 must be array,table or string obj.The arg1 is the corresponding index obj.")

        if args[0].type == ObjectType.ARRAY:
            if args[1].type != ObjectType.NUM:
                error("[Native function 'insert']:Arg1 must be integer type while insert to a array")
            iIndex = args[1].value
            if iIndex < 0 or iIndex >= len(args[0].elements):
                error("[Native function 'insert']:Index out of array's range")
            args[0].elements.pop(iIndex)
        elif args[0].type == ObjectType.STR:
            if args[1].type != ObjectType.NUM:
                error("[Native function 'insert']:Arg1 must be integer type while insert to a string")
            iIndex = args[1].value
            if iIndex < 0 or iIndex >= len(args[0].values):
                error("[Native function 'insert']:Index out of array's range")
            args[0].values.pop(iIndex)
        else:
            error("[Native function 'erase']:Expect a array,table ot string argument.")
        return False, None

    def __clock(self, args: list[Object]):
        result = NumObject(time.perf_counter())
        return True, result


gBuiltinManager = BuiltinManager()
