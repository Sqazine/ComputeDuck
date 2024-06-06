
def error(msg):
    print(msg)
    exit(1)

def read_file(path):
    file = open(path, "r")
    contents = file.read()
    file.close()
    return contents
