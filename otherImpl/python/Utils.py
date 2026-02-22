import importlib
import sys
import ctypes

UINT8_COUNT:int = 256

def error(msg):
    print(msg)
    exit(1)


def read_file(path):
    file = open(path, "r")
    contents = file.read()
    file.close()
    return contents


def register_dlls(dllPath):
    modules = sys.modules.keys()
    if dllPath in modules:
        return True
    mod = importlib.import_module(dllPath)
    mod.register_builtins()


def search_object_by_address(address):
    return ctypes.cast(address, ctypes.py_object).value
