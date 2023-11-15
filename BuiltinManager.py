from Object import *
from Utils import *
import time

class BuiltinManager(object):
    instance=None
    builtinFunctions:list[BuiltinFunctionObject]=[]
    builtinFunctionNames:list[str]=[]
    builtinVariables:list[BuiltinVariableObject]=[]
    builtinVariableNames:list[str]=[]

    def __new__(cls, *args, **kw):
        if not cls.instance:
            cls.instance = super(BuiltinManager, cls).__new__(cls, *args, **kw)
        return cls.instance
    
    def __init__(self) -> None:
        self.RegisterFunction("print",self.__Print)
        self.RegisterFunction("println",self.__Println)
        self.RegisterFunction("sizeof",self.__Sizeof)
        self.RegisterFunction("insert",self.__Insert)
        self.RegisterFunction("erase",self.__Erase)
        self.RegisterFunction("clock",self.__Clock)

    def __del__(self)->None:
        self.builtinFunctions.clear()
        self.builtinFunctionNames.clear()
        self.builtinVariables.clear()
        self.builtinVariableNames.clear()

    def RegisterFunction(self,name:str,fn:any)->None:
        self.builtinFunctions.append(BuiltinFunctionObject(name,fn))
        self.builtinFunctionNames.append(name)

    def RegisterVariable(self,name:str,obj:Object)->None:
        self.builtinVariables.append(BuiltinVariableObject(name,obj))
        self.builtinVariableNames.append(name)

    
    def __Println(self,args: list[Object]):
        if len(args) == 0:
            return False,None
        print(args[0])
        return False,None


    def __Print(self,args: list[Object]) -> bool:
        if len(args) == 0:
            return False,None
        print(args[0], end="")
        return False,None

    def __Sizeof(self,args: list[Object]):
        if len(args) == 0 or len(args) > 1:
            Assert("Native function 'siezeof':Expect a argument.")
        if args[0].type == ObjectType.ARRAY:
            result = NumObject(len(args[0].elements))
        elif args[0].type == ObjectType.STR:
            result = NumObject(len(args[0].value))
        else:
            Assert("[Native function 'sizeof']:Expect a array or string argument.")
        return True,result


    def __Insert(self,args: list[Object]):
        if len(args) == 0 or len(args) != 3:
            Assert("[Native function 'insert']:Expect 3 arguments,the arg0 must be array or string obj.The arg1 is the index obj.The arg2 is the value obj.")

        if args[0].type == ObjectType.ARRAY:
            if args[1].type != ObjectType.NUM:
                Assert(
                    "[Native function 'insert']:Arg1 must be integer type while insert to a array")
            iIndex = args[1].value
            if iIndex < 0 or iIndex >= len(args[0].elements):
                Assert("[Native function 'insert']:Index out of array's range")
            args[0].elements.insert(iIndex, args[2])
        elif args[0].type == ObjectType.STR:
            if args[1].type != ObjectType.NUM:
                Assert(
                    "[Native function 'insert']:Arg1 must be integer type while insert to a string")
            iIndex = args[1].value
            if iIndex < 0 or iIndex >= len(args[0].values):
                Assert("[Native function 'insert']:Index out of array's range")
            args[0].values.insert(iIndex, args[2].__str__())
        return False,None


    def __Erase(self,args: list[Object] ) -> bool:
        if len(args) or len(args) != 2:
            Assert("[Native function 'erase']:Expect 2 arguments,the arg0 must be array or string obj.The arg1 is the corresponding index obj.")

        if args[0].type == ObjectType.ARRAY:
            if args[1].type != ObjectType.NUM:
                Assert(
                    "[Native function 'insert']:Arg1 must be integer type while insert to a array")
            iIndex = args[1].value
            if iIndex < 0 or iIndex >= len(args[0].elements):
                Assert("[Native function 'insert']:Index out of array's range")
            args[0].elements.pop(iIndex)
        elif args[0].type == ObjectType.STR:
            if args[1].type != ObjectType.NUM:
                Assert(
                    "[Native function 'insert']:Arg1 must be integer type while insert to a string")
            iIndex = args[1].value
            if iIndex < 0 or iIndex >= len(args[0].values):
                Assert("[Native function 'insert']:Index out of array's range")
            args[0].values.pop(iIndex)
        else:
            Assert("[Native function 'erase']:Expect a array or string argument.")
        return False,None


    def __Clock(self,args: list[Object] ) -> bool:
        result = NumObject(time.perf_counter())
        return True,result

gBuiltinManager=BuiltinManager()