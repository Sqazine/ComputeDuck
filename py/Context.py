from Object import Object
from Utils import Assert


class Context:
    values:dict={}
    upContext=None

    def __init__(self,upContext=None) -> None:
        self.values={}
        self.upContext=upContext

    def DefineVariable(self,name,value:Object):
        if self.values.get(name)!=None:
            Assert("Redefined variable:"+name+" in current context.")
        else:
            self.values[name]=value

    def AssignVariable(self,name,value:Object):
        if self.values.get(name)!=None:
            self.values[name]=value
        elif self.upContext!=None:
            self.upContext.AssignVariable(name,value)
        else:
            Assert("Undefine variable:"+name+" in current context.")
    
    def GetVariable(self,name:str)->Object:
        if self.values.get(name)!=None:
            return self.values[name]
        if self.upContext!=None:
            return self.upContext.GetVariable(name)
        return None

    def GetUpContext(self):
        return self.upContext

    def SetUpContext(self,env):
        self.upContext=env

    
    
    

