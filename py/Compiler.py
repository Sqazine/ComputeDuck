from enum import IntEnum
from Utils import Assert
from Frame import Frame
from Ast import Stmt
class ObjectState(IntEnum):
    INIT=0,
    READ=1,
    WRITE=2,
    STRUCT_READ=3,
    STRUCT_WRITE=4
 
class Compiler:
    __rootFrame:Frame
    
    def __init__(self,stmts:list[Stmt]) -> None:
        pass
    
    def ResetStatus()->None:
        pass
    
    def CompileStmt(stmt:Stmt,frame:Frame)->None:
        pass
    
    