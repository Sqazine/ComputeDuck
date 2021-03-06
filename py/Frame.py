from enum import IntEnum
from logging import root
from operator import truediv
from Utils import Assert


class OpCode(IntEnum):
    OP_NEW_NUM = 0,
    OP_NEW_STR = 1,
    OP_NEW_TRUE = 2,
    OP_NEW_FALSE = 3,
    OP_NEW_NIL = 4,
    OP_NEW_ARRAY = 5,
    OP_NEW_STRUCT = 6,
    OP_NEW_LAMBDA = 7,
    OP_GET_VAR = 8,
    OP_SET_VAR = 9,
    OP_DEFINE_VAR = 10,
    OP_GET_INDEX_VAR = 11,
    OP_SET_INDEX_VAR = 12,
    OP_GET_STRUCT_VAR = 13,
    OP_SET_STRUCT_VAR = 14,
    OP_NEG = 15,
    OP_RETURN = 16,
    OP_ADD = 17,
    OP_SUB = 18,
    OP_MUL = 19,
    OP_DIV = 20,
    OP_GREATER = 21,
    OP_LESS = 22,
    OP_GREATER_EQUAL = 23,
    OP_LESS_EQUAL = 24,
    OP_EQUAL = 25,
    OP_NOT_EQUAL = 26,
    OP_NOT = 27,
    OP_OR = 28,
    OP_AND = 29,
    OP_ENTER_SCOPE = 30,
    OP_EXIT_SCOPE = 31,
    OP_JUMP = 32,
    OP_JUMP_IF_FALSE = 33,
    OP_FUNCTION_CALL = 34,
    OP_STRUCT_LAMBDA_CALL=35
    OP_REF = 36,


class Frame:
    upFrame = None
    functionFrames = {}
    structFrames = {}
    lambdaFrames=[]
    codes: list[int] = []
    nums: list[float] = []
    strings: list[str] = []

    def __init__(self, upFrame=None) -> None:
        self.upFrame = upFrame
        self.functionFrames = {}
        self.structFrames = {}
        self.lambdaFrames=[]
        self.codes: list[int] = []
        self.nums: list[float] = []
        self.strings: list[str] = []

    def AddOpCode(self, code: int) -> None:
        self.codes.append(code)

    def GetOpCodeSize(self) -> int:
        return len(self.codes)

    def AddNum(self, value: float) -> int:
        self.nums.append(value)
        return len(self.nums)-1

    def AddString(self, value: str) -> int:
        self.strings.append(value)
        return len(self.strings)-1

    def AddFunctionFrame(self, name: str, frame) -> None:
        if self.functionFrames.get(name) != None:
            Assert("Redefinition function:"+name)
        self.functionFrames[name] = frame

    def GetFunctionFrame(self, name: str):
        if self.functionFrames.get(name) != None:
            return self.functionFrames.get(name)
        elif self.upFrame != None:
            return self.upFrame.GetFunctionFrame(name)

    def HasFunctionFrame(self, name: str) -> bool:
        for key in self.functionFrames:
            if key == name:
                return True
        if self.upFrame != None:
            return self.upFrame.HasFunctionFrame(name)
        return False

    def AddStructFrame(self, name: str, frame) -> None:
        if self.structFrames.get(name) != None:
            Assert("Redefinition function:"+name)
        self.structFrames[name] = frame

    def GetStructFrame(self, name: str):
        if self.structFrames.get(name) != None:
            return self.structFrames.get(name)
        elif self.upFrame != None:
            return self.upFrame.GetStructFrame(name)

    def HasStructFrame(self, name: str) -> bool:
        for key in self.structFrames:
            if key == name:
                return True
        if self.upFrame != None:
            return self.upFrame.HasStructFrame(name)
        return False

    def AddLambdaFrame(self,frame)->int:
        rootFrame=self
        if rootFrame.upFrame!=None:
            while rootFrame.upFrame!=None:
                rootFrame=rootFrame.upFrame
        rootFrame.lambdaFrames.append(frame)
        return len(rootFrame.lambdaFrames)-1

    def GetLambdaFrame(self,idx):
        if self.upFrame!=None:
            rootFrame=self
            while rootFrame.upFrame:
                rootFrame=rootFrame.upFrame
            return rootFrame.GetLambdaFrame(idx)
        elif idx>=0 or idx<len(self.lambdaFrames):
            return self.lambdaFrames[idx]
        else: 
            return None

    def HasLambdaFrame(self,idx):
        if self.upFrame:
            rootframe=self
            while rootFrame.upFrame:
                rootFrame = rootFrame.upFrame
            return rootFrame.HasLambdaFrame(idx)
        elif idx>=0 or idx<len(self.lambdaFrames):
            return True
        else:
            return False

    def Clear(self) -> None:
        self.upFrame: Frame = None
        self.functionFrames: dict[str, Frame] = {}
        self.structFrames: dict[str, Frame] = {}
        self.codes: list[int] = []
        self.nums: list[float] = []
        self.strings: list[str] = []

    def Stringify(self, depth: int = 0) -> str:
        interval: str = ""
        for i in range(depth):
            interval += "\t"

        result: str = ""

        for key, value in self.structFrames.items():
            result += ("%sFrame %s:\n" % (interval, key))
            result += value.Stringify(depth+1)

        for key, value in self.functionFrames.items():
            result += ("%sFrame %s:\n" % (interval, key))
            result += value.Stringify(depth+1)

        for i in range(len(self.lambdaFrames)):
            result += ("%sFrame %d:\n" % (interval, i))
            result += self.lambdaFrames[i].Stringify(depth+1)

        result += ("%sOpCodes:\n" % interval)

        i = 0
        while i < (len(self.codes)):
            if self.codes[i] == OpCode.OP_RETURN:
                result += "%s\t%08d     OP_RETURN\n" % (interval, i)
            elif self.codes[i] == OpCode.OP_NEW_NUM:
                result += "%s\t%08d     OP_NEW_NUM     %d\n" % (
                    interval, i, self.nums[self.codes[i+1]])
                i = i+1
            elif self.codes[i] == OpCode.OP_NEW_STR:
                result += "%s\t%08d     OP_NEW_STR     %s\n" % (
                    interval, i, self.strings[self.codes[i+1]])
                i = i+1
            elif self.codes[i] == OpCode.OP_NEW_TRUE:
                result += "%s\t%08d     OP_NEW_TRUE\n" % (interval, i)
            elif self.codes[i] == OpCode.OP_NEW_FALSE:
                result += "%s\t%08d     OP_NEW_FALSE\n" % (interval, i)
            elif self.codes[i] == OpCode.OP_NEW_NIL:
                result += "%s\t%08d     OP_NEW_NIL\n" % (interval, i)
            elif self.codes[i] == OpCode.OP_NEW_STRUCT:
                result += "%s\t%08d     OP_NEW_STRUCT     %s\n" % (
                    interval, i, self.strings[self.codes[i+1]])
                i = i+1
            elif self.codes[i] == OpCode.OP_NEW_LAMBDA:
                result += "%s\t%08d     OP_NEW_LAMBDA     %s\n" % (
                    interval, i, self.nums[self.codes[i+1]])
                i = i+1
            elif self.codes[i] == OpCode.OP_NEG:
                result += "%s\t%08d     OP_NEG\n" % (interval, i)
            elif self.codes[i] == OpCode.OP_ADD:
                result += "%s\t%08d     OP_ADD\n" % (interval, i)
            elif self.codes[i] == OpCode.OP_SUB:
                result += "%s\t%08d     OP_SUB\n" % (interval, i)
            elif self.codes[i] == OpCode.OP_MUL:
                result += "%s\t%08d     OP_MUL\n" % (interval, i)
            elif self.codes[i] == OpCode.OP_DIV:
                result += "%s\t%08d     OP_DIV\n" % (interval, i)
            elif self.codes[i] == OpCode.OP_GREATER:
                result += "%s\t%08d     OP_GREATER\n" % (interval, i)
            elif self.codes[i] == OpCode.OP_LESS:
                result += "%s\t%08d     OP_LESS\n" % (interval, i)
            elif self.codes[i] == OpCode.OP_GREATER_EQUAL:
                result += "%s\t%08d     OP_GREATER_EQUAL\n" % (interval, i)
            elif self.codes[i] == OpCode.OP_LESS_EQUAL:
                result += "%s\t%08d     OP_LESS_EQUAL\n" % (interval, i)
            elif self.codes[i] == OpCode.OP_EQUAL:
                result += "%s\t%08d     OP_EQUAL\n" % (interval, i)
            elif self.codes[i] == OpCode.OP_NOT:
                result += "%s\t%08d     OP_NOT\n" % (interval, i)
            elif self.codes[i] == OpCode.OP_NOT_EQUAL:
                result += "%s\t%08d     OP_NOT_EQUAL\n" % (interval, i)
            elif self.codes[i] == OpCode.OP_AND:
                result += "%s\t%08d     OP_AND\n" % (interval, i)
            elif self.codes[i] == OpCode.OP_OR:
                result += "%s\t%08d     OP_OR\n" % (interval, i)
            elif self.codes[i] == OpCode.OP_GET_VAR:
                result += "%s\t%08d     OP_GET_VAR     %s\n" % (
                    interval, i, self.strings[self.codes[i+1]])
                i = i+1
            elif self.codes[i] == OpCode.OP_DEFINE_VAR:
                result += "%s\t%08d     OP_DEFINE_VAR     %s\n" % (
                    interval, i, self.strings[self.codes[i+1]])
                i = i+1
            elif self.codes[i] == OpCode.OP_SET_VAR:
                result += "%s\t%08d     OP_SET_VAR     %s\n" % (
                    interval, i, self.strings[self.codes[i+1]])
                i = i+1
            elif self.codes[i] == OpCode.OP_NEW_ARRAY:
                result += "%s\t%08d     OP_NEW_ARRAY     %d\n" % (
                    interval, i, self.nums[self.codes[i+1]])
                i = i+1
            elif self.codes[i] == OpCode.OP_GET_INDEX_VAR:
                result += "%s\t%08d     OP_GET_INDEX_VAR\n" % (interval, i)
            elif self.codes[i] == OpCode.OP_SET_INDEX_VAR:
                result += "%s\t%08d     OP_SET_INDEX_VAR\n" % (interval, i)
            elif self.codes[i] == OpCode.OP_GET_STRUCT_VAR:
                result += "%s\t%08d     OP_GET_STRUCT_VAR     %s\n" % (
                    interval, i, self.strings[self.codes[i+1]])
                i = i+1
            elif self.codes[i] == OpCode.OP_SET_STRUCT_VAR:
                result += "%s\t%08d     OP_SET_STRUCT_VAR     %s\n" % (
                    interval, i, self.strings[self.codes[i+1]])
                i = i+1
            elif self.codes[i] == OpCode.OP_ENTER_SCOPE:
                result += "%s\t%08d     OP_ENTER_SCOPE\n" % (interval, i)
            elif self.codes[i] == OpCode.OP_EXIT_SCOPE:
                result += "%s\t%08d     OP_EXIT_SCOPE\n" % (interval, i)
            elif self.codes[i] == OpCode.OP_JUMP:
                result += "%s\t%08d     OP_JUMP     %d\n" % (
                    interval, i, self.nums[self.codes[i+1]])
                i = i+1
            elif self.codes[i] == OpCode.OP_JUMP_IF_FALSE:
                result += "%s\t%08d     OP_JUMP_IF_FALSE     %d\n" % (
                    interval, i, self.nums[self.codes[i+1]])
                i = i+1
            elif self.codes[i] == OpCode.OP_FUNCTION_CALL:
                result += "%s\t%08d     OP_FUNCTION_CALL     %s\n" % (
                    interval, i, self.strings[self.codes[i+1]])
                i = i+1
            elif self.codes[i] == OpCode.OP_STRUCT_LAMBDA_CALL:
                result += "%s\t%08d     OP_STRUCT_LAMBDA_CALL     %s\n" % (
                    interval, i, self.strings[self.codes[i+1]])
                i = i+1
            elif self.codes[i] == OpCode.OP_REF:
                result += "%s\t%08d     OP_REF     %s\n" % (
                    interval, i, self.strings[self.codes[i+1]])
                i = i+1
            else:
                result += "%s\t%08d     UNKNOWN\n" % (interval, i)
            i = i+1
        return result
