import os


class Config(object):
    __instance = None
    __cur_execute_file_path: str

    def __new__(cls, *args, **kw):
        if not cls.__instance:
            cls.__instance = super(Config, cls).__new__(cls, *args, **kw)
        return cls.__instance

    def set_execute_file_path(self, path: str):
        self.__cur_execute_file_path = path

    def to_full_path(self, path: str) -> str:
        fullPath = path
        if not os.path.isabs(fullPath):
            fullPath = self.__cur_execute_file_path+fullPath
        return fullPath


gConfig = Config()
