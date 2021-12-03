from enum import IntEnum
from Utils import Assert


class OpCode(IntEnum):
    OP_NEW_NUM = 0,
    OP_NEW_STR = 1,
    OP_NEW_TRUE = 2,
    OP_NEW_FALSE = 3,
    OP_NEW_NIL = 4,
    OP_NEW_ARRAY = 5,
    OP_NEW_STRUCT = 6,
    OP_GET_VAR = 7,
    OP_SET_VAR = 8,
    OP_DEFINE_VAR = 9,
    OP_GET_INDEX_VAR = 10,
    OP_SET_INDEX_VAR = 11,
    OP_GET_STRUCT_VAR = 12,
    OP_SET_STRUCT_VAR = 13,
    OP_NEG = 14,
    OP_RETURN = 15,
    OP_ADD = 16,
    OP_SUB = 17,
    OP_MUL = 18,
    OP_DIV = 19,
    OP_GREATER = 20,
    OP_LESS = 21,
    OP_GREATER_EQUAL = 22,
    OP_LESS_EQUAL = 23,
    OP_EQUAL = 24,
    OP_NOT_EQUAL = 25,
    OP_NOT = 26,
    OP_OR = 27,
    OP_AND = 28,
    OP_ENTER_SCOPE = 29,
    OP_EXIT_SCOPE = 30,
    OP_JUMP = 30,
    OP_JUMP_IF_FALSE = 31,
    OP_FUNCTION_CALL = 32,


class Frame:
    upFrame: Frame = None
    functionFrames: dict[str, Frame] = {}
    structFrames: dict[str, Frame] = {}
    codes: list[int] = []
    nums: list[float] = []
    strings: list[str] = []

    def __init__(self, upFrame=None) -> None:
        self.upFrame = upFrame

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

    def AddFunctionFrame(self, name: str, frame: Frame) -> None:
        if self.functionFrames.get(name) != None:
            Assert("Redefinition function:"+name)
        self.functionFrames[name] = frame

    def GetFunctionFrame(self, name: str) -> Frame:
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

    def AddStructFrame(self, name: str, frame: Frame) -> None:
        if self.structFrames.get(name) != None:
            Assert("Redefinition function:"+name)
        self.functstructFramesionFrames[name] = frame

    def GetStructFrame(self, name: str) -> Frame:
        if self.structFrames.get(name) != None:
            return self.structFrames.get(name)
        elif self.upFrame != None:
            return self.upFrame.GetFunctionFrame(name)

    def HasStructFrame(self, name: str) -> bool:
        for key in self.structFrames:
            if key == name:
                return True
        if self.upFrame != None:
            return self.upFrame.HasStructFrame(name)
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
            result += ("%sFrame %d:\n" % (interval, key))
            result += value.Stringify(depth+1)

        for key, value in self.functionFrames.items():
            result += ("%sFrame %d:\n" % (interval, key))
            result += value.Stringify(depth+1)

        result += ("%sOpCodes:\n" % interval)

        for i in range(len(self.codes)):
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
                result += "%s\t%08d     OP_FUNCTION_CALL     %d\n" % (
                    interval, i, self.strings[self.codes[i+1]])
                i = i+1
            else:
                result += "%s\t%08d     UNKNOWN\n" % (interval, i)
        return result
