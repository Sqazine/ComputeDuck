
def Assert(msg):
    print(msg)
    exit(1)

def ReadFile(path):
    file=open(path,"r")
    contents=file.read()
    file.close()
    return contents