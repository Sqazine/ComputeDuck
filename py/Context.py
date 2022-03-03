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

    def AssignVariableByAddress(self, address: str, value: Object):
        for k, v in self.values.items():
            if id(v) == address:
                self.values[k] = value
                return
            elif v.Type() == ObjectType.ARRAY:
                for i in range(0, len(v.elements)):
                    if id(v.elements[i]) == address:
                        v.elements[i] = value
                        return

        if self.upContext != None:
            self.upContext.AssignVariableByAddress(address, value)
        else:
            Assert("Undefine variable(address:"+address+") in current context.")

    def GetVariableByAddress(self, address: str):
        for k, v in self.values.items():
            if id(v) == address:
                return v
            elif v.Type() == address:
                for arrV in v.elements:
                    if id(arrV) == address:
                        return arrV

        if self.upContext != None:
            return self.upContext.GetVariableByAddress(address)
        return None
