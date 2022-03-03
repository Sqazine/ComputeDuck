from email.headerregistry import Address
from tkinter.messagebox import NO
from Object import Object,ObjectType
from Utils import Assert


class Context:
    values: dict = {}
    upContext = None

    def __init__(self, upContext=None) -> None:
        self.values = {}
        self.upContext = upContext

    def DefineVariableByName(self, name, value: Object):
        if self.values.get(name) != None:
            Assert("Redefined variable:"+name+" in current context.")
        else:
            self.values[name] = value

    def AssignVariableByName(self, name, value: Object):
        if self.values.get(name) != None:
            self.values[name] = value
        elif self.upContext != None:
            self.upContext.AssignVariableByName(name, value)
        else:
            Assert("Undefine variable:"+name+" in current context.")

    def GetVariableByName(self, name: str) -> Object:
        if self.values.get(name) != None:
            return self.values[name]
        if self.upContext != None:
            return self.upContext.GetVariableByName(name)
        return None