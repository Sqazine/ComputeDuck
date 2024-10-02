from BuiltinManager import *
from OpenGL import GL
import numpy as np
import Utils

def glad_load_gl_wrapper(args: list[Object]):
    return True, NumObject(1)


def gl_gen_vertex_arrays_wrapper(args: list[Object]):
    if args[0].type != ObjectType.NUM and args[1].type != ObjectType.REF:
        error("Invalid value of glGenVertexArrays(args[0],args[1]).")

    count = int(args[0].value)
    ref = search_object_by_address(args[1].pointer)

    if ref.type == ObjectType.ARRAY:
        vaos=GL.glGenVertexArrays(count)
        ref.elements = []
        for i in range(0, len(vaos)):
            ref.elements.append(NumObject(vaos[i]))
        assert (GL.glGetError() == 0)
    elif ref.type == ObjectType.NUM:
        vao= GL.glGenVertexArrays(1)
        ref.value=vao
        assert (GL.glGetError() == 0)
    else:
        error("Invalid value of glGenVertexArrays:args[1].only number array or number is available")
    return False,None


def gl_bind_vertex_array_wrapper(args: list[Object]):
    if args[0].type!=ObjectType.NUM:
        error("Invalid value of glBindVertexArray(args[0]).")
    vao=args[0].value
    GL.glBindVertexArray(int(vao))
    assert (GL.glGetError() == 0)
    return False,None


def gl_vertex_attrib_pointer_wrapper(args: list[Object]):
    if args[0].type != ObjectType.NUM and args[1].type != ObjectType.NUM and args[2].type != ObjectType.BUILTIN and args[3].type != ObjectType.BUILTIN and args[4].type !=ObjectType.NUM and not (args[5].type == ObjectType.REF or args[5].type == ObjectType.NIL):
        assert("Invalid value of glVertexAttribPointer(args[0],args[1],args[2],args[3],args[4],args[5]).")

    arg0=int(args[0].value)
    arg1=int(args[1].value)
    arg2=int(args[2].data.value)
    arg3=int(args[3].data.value)
    arg4=int(args[4].value)

    if args[5].type==ObjectType.NIL:
        GL.glVertexAttribPointer(arg0,arg1,arg2,arg3,arg4,None)
    else:
        if arg2 == GL.GL_FLOAT:
            arg5 = args[5].pointer
            arrArg5=(ArrayObject)(Utils.search_object_by_address(arg5))
            rawArg5=[]
            for i in range(0,len(arrArg5.elements)):
                rawArg5.append((float)(arrArg5.elements[i].value))
            GL.glVertexAttribPointer(arg0,arg1,arg2,arg3,arg4,rawArg5)
        elif arg2.value==GL.GL_UNSIGNED_INT:
            arg5 = args[5].pointer
            arrArg5=(ArrayObject)(Utils.search_object_by_address(arg5))
            rawArg5=[]
            for i in range(0,len(arrArg5.elements)):
                rawArg5.append((int)(arrArg5.elements[i].value))
            GL.glVertexAttribPointer(arg0,arg1,arg2,arg3,arg4,rawArg5)
        elif  arg2 == GL.GL_INT:
            arg5 = args[5].pointer
            arrArg5=(ArrayObject)(Utils.search_object_by_address(arg5))
            rawArg5=[]
            for i in range(0,len(arrArg5.elements)):
                rawArg5.append((int)(arrArg5.elements[i].value))
            GL.glVertexAttribPointer(arg0,arg1,arg2,arg3,arg4,rawArg5)
    return False,None

def gl_enable_vertex_attrib_array_wrapper(args: list[Object]):
    if args[0].type!=ObjectType.NUM:
        error("Invalid value of glEnableVertexAttribArray(args[0]).")
    vao=int(args[0].value)
    GL.glEnableVertexAttribArray(vao)
    assert (GL.glGetError() == 0)
    return False,None


def gl_gen_buffers_wrapper(args: list[Object]):
    if args[0].type != ObjectType.NUM and args[1].type != ObjectType.REF:
        error("Invalid value of glGenBuffers(args[0],args[1]).")

    count = int(args[0].value)
    ref = search_object_by_address(args[1].pointer)

    if ref.type == ObjectType.ARRAY:
        vbos=GL.glGenBuffers(count)
        elements = []
        for i in range(0, len(vbos)):
            elements.append(NumObject(vbos[i]))
        ref.elements=elements
        assert (GL.glGetError() == 0)
    elif ref.type == ObjectType.NUM:
        vbo=GL.glGenBuffers(1)
        ref.value = vbo
        assert (GL.glGetError() == 0)
    else:
        error("Invalid value of glGenBuffers:args[1].only number array or number is available")
    return False,None


def gl_bind_buffer_wrapper(args: list[Object]):
    if args[0].type!=ObjectType.NUM and args[1].type!=ObjectType.NUM:
        error("Invalid value of glBindBuffer(args[0],args[1]).")
    arg0=int(args[0].data.value)
    arg1=int(args[1].value)
    GL.glBindBuffer(arg0,arg1)
    assert (GL.glGetError() == 0)
    return False,None


def gl_buffer_data_wrapper(args: list[Object]):
    if args[0].type!=ObjectType.BUILTIN or args[1].type!= ObjectType.NUM or args[2].type!=ObjectType.ARRAY or args[3].type!=ObjectType.BUILTIN:
        error("Invalid value of glBufferData(args[0],args[1],args[2],args[3]).")
    
    arg0=args[0].data.value
    arg1=args[1].value
    arg2=args[2].elements
    arg3=args[3].data.value

    if arg0 == GL.GL_ELEMENT_ARRAY_BUFFER:
        rawArg2=[]
        for i in range(0,len(arg2)):
            rawArg2.append(int(arg2[i].value))
        rawArg2=np.array(rawArg2).astype(np.int32)
        GL.glBufferData(arg0,int(arg1),rawArg2.ravel(),arg3)
    else:
        rawArg2=[]
        for i in range(0,len(arg2)):
            rawArg2.append(float(arg2[i].value))
        rawArg2=np.array(rawArg2).astype(np.float32)
        GL.glBufferData(arg0,int(arg1),rawArg2.ravel(),arg3)
    assert (GL.glGetError() == 0)
    return False,None


def gl_create_shader_wrapper(args: list[Object]):
    if args[0].type!=ObjectType.BUILTIN:
        error("Invalid value of glCreateShader(args[0]).")
    arg0=args[0].data.value
    result = GL.glCreateShader(arg0)
    assert (GL.glGetError() == 0)
    return True,NumObject(result)

def gl_shader_source_wrapper(args: list[Object]):
    if args[0].type!=ObjectType.NUM or args[1].type!=ObjectType.NUM or args[2].type != ObjectType.REF or not (args[3].type != ObjectType.REF or args[3].type!=ObjectType.NIL):
        error("Invalid value of glCreateShader(args[0]).")

    arg0 = int(args[0].value)
    arg1 = int(args[1].value)
    arg2 = Utils.search_object_by_address(args[2].pointer).value
    
    arg3=None
    if args[3].type==ObjectType.REF:
        arg3=Utils.search_object_by_address(args[3].pointer).value
    
    GL.glShaderSource(arg0,arg2)
    assert (GL.glGetError() == 0)
    return False,None
    

def gl_compile_shader_wrapper(args: list[Object]):
    if args[0].type!=ObjectType.NUM:
        error("Invalid value of glCompileShader(args[0]).")
    
    arg0=args[0].value
    GL.glCompileShader(arg0)
    
    isSuccess=GL.glGetShaderiv(arg0,GL.GL_COMPILE_STATUS)
    if not isSuccess:
        infoLog=GL.glGetShaderInfoLog(arg0,512,None)
        print("ERROR::SHADER::COMPILATION_FAILED\n"+ infoLog)
    assert (GL.glGetError() == 0)
    return False,None


def gl_create_program_wrapper(args: list[Object]):
    result = GL.glCreateProgram()
    assert (GL.glGetError() == 0)
    return True,NumObject(result)


def gl_attach_shader_wrapper(args: list[Object]):
    if args[0].type!=ObjectType.NUM or args[1].type!=ObjectType.NUM:
        error("Invalid value of glAttachShader(args[0],args[1]).")
    
    arg0 = int(args[0].value)
    arg1 = int(args[1].value)
    
    GL.glAttachShader(arg0,arg1)
    
    assert (GL.glGetError() == 0)
    return False,None


def gl_link_program_wrapper(args: list[Object]):
    if args[0].type!=ObjectType.NUM :
        error("Invalid value of glLinkProgram(args[0]).")
    
    arg0 = int(args[0].value)
    
    GL.glLinkProgram(arg0)
    
    success=GL.glGetProgramiv(arg0,GL.GL_LINK_STATUS)
    if success ==False:
        infoLog=GL.glGetProgramInfoLog(arg0)
        print("ERROR::SHADER::PROGRAM::LINKING_FAILED\n"+infoLog)
        
    assert (GL.glGetError() == 0)
    return False,None


def gl_clear_color_wrapper(args: list[Object]):
    if args[0].type!=ObjectType.NUM or args[1].type!=ObjectType.NUM or args[2].type!=ObjectType.NUM or args[3].type!=ObjectType.NUM:
        error("Invalid value of glClearColor(args[0],arg[1],arg[2],arg[3]).")
        
    arg0=float(args[0].value)
    arg1=float(args[1].value)
    arg2=float(args[2].value)
    arg3=float(args[3].value)
        
    GL.glClearColor(arg0,arg1,arg2,arg3)
    
    assert (GL.glGetError() == 0)
    return False,None


def gl_clear_wrapper(args: list[Object]):
    if args[0].type==ObjectType.BUILTIN:
        arg=int(args[0].data.value)
        GL.glClear(arg)
    elif args[0].type==ObjectType.NUM:
        arg=int(args[0].value)
        GL.glClear(arg)
    else:
        error("Invalid value of glClear(args[0]).")
    assert (GL.glGetError() == 0)
    return False,None


def gl_use_program_wrapper(args: list[Object]):
    if args[0].type!=ObjectType.NUM :
        error("Invalid value of glUseProgram(args[0]).")
    arg0=int(args[0].value)
    GL.glUseProgram(arg0)
    assert (GL.glGetError() == 0)
    return False,None


def gl_draw_elements_wrapper(args: list[Object]):
    if args[0].type!=ObjectType.BUILTIN or args[1].type!=ObjectType.NUM or args[2].type!=ObjectType.BUILTIN or not(args[3].type==ObjectType.REF or args[3].type==ObjectType.NIL):
        error("Invalid value of glDrawElements(args[0],arg[1],arg[2],arg[3]).")
        
    arg0 = args[0].data.value
    arg1 = int(args[1].value)
    arg2 = args[2].data.value
    
    if args[3].type==ObjectType.NIL:
        GL.glDrawElements(arg0,arg1,arg2,None)
    else:
        arg3=Utils.search_object_by_address(args[3].pointer)
        
        rawArg3=[]
        for i in range(0,len(arg3.elements)):
            rawArg3.append(arg3.elements[i].value)
        rawArg3=np.array(rawArg3).astype(np.int32)
        GL.glDrawElements(arg0,arg1,arg2,rawArg3)
    assert (GL.glGetError() == 0)
    return False,None


def register_builtins() -> None:
    gBuiltinManager.register("GL_ARRAY_BUFFER", NumObject(GL.GL_ARRAY_BUFFER))
    gBuiltinManager.register("GL_STATIC_DRAW", NumObject(GL.GL_STATIC_DRAW))
    gBuiltinManager.register("GL_VERTEX_SHADER", NumObject(GL.GL_VERTEX_SHADER))
    gBuiltinManager.register("GL_FRAGMENT_SHADER",NumObject(GL.GL_FRAGMENT_SHADER))
    gBuiltinManager.register("GL_COLOR_BUFFER_BIT",NumObject(GL.GL_COLOR_BUFFER_BIT))
    gBuiltinManager.register("GL_DEPTH_BUFFER_BIT",NumObject(GL.GL_DEPTH_BUFFER_BIT))
    gBuiltinManager.register("GL_ELEMENT_ARRAY_BUFFER",NumObject(GL.GL_ELEMENT_ARRAY_BUFFER))
    gBuiltinManager.register("GL_FLOAT",NumObject(GL.GL_FLOAT))
    gBuiltinManager.register("GL_FALSE",NumObject(GL.GL_FALSE))
    gBuiltinManager.register("GL_TRUE",NumObject(GL.GL_TRUE))
    gBuiltinManager.register("GL_TRIANGLES",NumObject(GL.GL_TRIANGLES))
    gBuiltinManager.register("GL_UNSIGNED_INT",NumObject(GL.GL_UNSIGNED_INT))

    gBuiltinManager.register("gladLoadGL", glad_load_gl_wrapper)
    gBuiltinManager.register("glGenVertexArrays", gl_gen_vertex_arrays_wrapper)
    gBuiltinManager.register("glBindVertexArray", gl_bind_vertex_array_wrapper)
    gBuiltinManager.register("glVertexAttribPointer", gl_vertex_attrib_pointer_wrapper)
    gBuiltinManager.register("glEnableVertexAttribArray",gl_enable_vertex_attrib_array_wrapper)
    gBuiltinManager.register("glGenBuffers", gl_gen_buffers_wrapper)
    gBuiltinManager.register("glBindBuffer", gl_bind_buffer_wrapper)
    gBuiltinManager.register("glBufferData", gl_buffer_data_wrapper)
    gBuiltinManager.register("glCreateShader", gl_create_shader_wrapper)
    gBuiltinManager.register("glShaderSource", gl_shader_source_wrapper)
    gBuiltinManager.register("glCompileShader", gl_compile_shader_wrapper)
    gBuiltinManager.register("glCreateProgram", gl_create_program_wrapper)
    gBuiltinManager.register("glAttachShader", gl_attach_shader_wrapper)
    gBuiltinManager.register("glLinkProgram", gl_link_program_wrapper)
    gBuiltinManager.register("glClearColor", gl_clear_color_wrapper)
    gBuiltinManager.register("glClear", gl_clear_wrapper)
    gBuiltinManager.register("glUseProgram", gl_use_program_wrapper)
    gBuiltinManager.register("glDrawElements", gl_draw_elements_wrapper)
