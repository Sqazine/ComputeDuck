
using ComputeDuck;
using OpenTK;
using OpenTK.Graphics.OpenGL;
using OpenTK.Windowing.GraphicsLibraryFramework;
using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using Object = ComputeDuck.Object;

public class cdopengl
{
    //set as exe to get the "OpenTK.Graphics.dll" library
    static void Main(string[] args)
    {
        return;
    }

    public class SDL2BindingsContext : IBindingsContext
    {
        public IntPtr GetProcAddress(string procName)
        {
            [DllImport("SDL2")]
            extern static IntPtr SDL_GL_GetProcAddress([MarshalAs(UnmanagedType.LPStr)] string procName);

            return SDL_GL_GetProcAddress(procName);
        }
    }

    private static (bool, Object?) GladLoadGLWrapper(List<Object> args)
    {
        GL.LoadBindings(new SDL2BindingsContext());
        return (true, new NumObject(1));
    }

    private static (bool, Object?) GLGenVertexArraysWrapper(List<Object> args)
    {
        if (args[0].type != ObjectType.NUM || args[1].type != ObjectType.REF)
            Utils.Assert("Invalid value of glGenVertexArrays(args[0],args[1]).");

        var count = (int)((NumObject)args[0]).value;
        var ptr = ((RefObject)args[1]).pointer;
        var refer = Utils.SearchObjectByAddress(ptr);
        if (refer.type == ObjectType.ARRAY)
        {
            var array = (ArrayObject)refer;
            uint[] vaos = new uint[count];

            GL.GenVertexArrays(count, vaos);

            array.elements = new List<Object>(count);
            for (int i = 0; i < array.elements.Count; ++i)
                array.elements[i] = new NumObject((double)vaos[i]);

            Debug.Assert(GL.GetError() == 0);
        }
        else if (refer.type == ObjectType.NUM)
        {
            var id = GL.GenVertexArray();
            ((NumObject)refer).value = id;
            Debug.Assert(GL.GetError() == 0);
        }
        else
            Utils.Assert("Invalid value of glGenVertexArrays:args[1].only number array or number is available");
        return (false, null);
    }

    private static (bool, Object?) GLBindVertexArrayWrapper(List<Object> args)
    {
        if (args[0].type != ObjectType.NUM)
            Utils.Assert("Invalid value of glBindVertexArray(args[0]).");
        var vao = ((NumObject)args[0]).value;
        GL.BindVertexArray((int)vao);
        Debug.Assert(GL.GetError() == 0);
        return (false, null);
    }

    private static (bool, Object?) GLVertexAttribPointerWrapper(List<Object> args)
    {
        if (args[0].type != ObjectType.NUM || args[1].type != ObjectType.NUM || args[2].type != ObjectType.BUILTIN || args[3].type != ObjectType.BUILTIN || args[4].type != ObjectType.NUM || !(args[5].type == ObjectType.REF || args[5].type == ObjectType.NIL))
            Utils.Assert("Invalid value of glVertexAttribPointer(args[0],args[1],args[2],args[3],args[4],args[5]).");

        var arg0 = (int)((NumObject)(args[0])).value;
        var arg1 = (int)((NumObject)(args[1])).value;
        var arg2 = (int)((NumObject)(((BuiltinObject)(args[2])).GetBuiltinVar())).value;
        var arg3 = ((NumObject)((BuiltinObject)(args[3])).GetBuiltinVar()).value;
        var arg4 = (int)((NumObject)(args[4])).value;

        if (args[5].type == ObjectType.NIL)
            GL.VertexAttribPointer(arg0, arg1, (VertexAttribPointerType)arg2, arg3 == 0 ? false : true, arg4, 0);
        else
        {
            if (arg2 == (double)VertexAttribPointerType.Float)
            {
                var arg5 = Utils.SearchObjectByAddress(((RefObject)args[5]).pointer);
                var arrArg5 = (ArrayObject)arg5;

                float[] rawArg5 = new float[arrArg5.elements.Count];
                for (int i = 0; i < arrArg5.elements.Count; i++)
                    rawArg5[i] = (float)((NumObject)arrArg5.elements[i]).value;
                GL.VertexAttribPointer(arg0, arg1, (VertexAttribPointerType)arg2, arg3 == 0 ? false : true, arg4, rawArg5);
            }
            else if (arg2 == (double)VertexAttribPointerType.UnsignedInt)
            {
                var arg5 = Utils.SearchObjectByAddress(((RefObject)args[5]).pointer);
                var arrArg5 = (ArrayObject)arg5;

                uint[] rawArg5 = new uint[arrArg5.elements.Count];
                for (int i = 0; i < arrArg5.elements.Count; i++)
                    rawArg5[i] = (uint)((NumObject)arrArg5.elements[i]).value;
                GL.VertexAttribPointer(arg0, arg1, (VertexAttribPointerType)arg2, arg3 == 0 ? false : true, arg4, rawArg5);
            }
            else if (arg2 == (double)VertexAttribPointerType.Int)
            {
                var arg5 = Utils.SearchObjectByAddress(((RefObject)args[5]).pointer);
                var arrArg5 = (ArrayObject)arg5;

                int[] rawArg5 = new int[arrArg5.elements.Count];
                for (int i = 0; i < arrArg5.elements.Count; i++)
                    rawArg5[i] = (int)((NumObject)arrArg5.elements[i]).value;
                GL.VertexAttribPointer(arg0, arg1, (VertexAttribPointerType)arg2, arg3 == 0 ? false : true, arg4, rawArg5);
            }
        }
        Debug.Assert(GL.GetError() == 0);
        return (false, null);
    }

    private static (bool, Object?) GLEnableVertexAttribArrayWrapper(List<Object> args)
    {
        if (args[0].type != ObjectType.NUM)
            Utils.Assert("Invalid value of glEnableVertexAttribArray(args[0]).");
        var arg0 = (int)((NumObject)args[0]).value;
        GL.EnableVertexAttribArray(arg0);
        Debug.Assert(GL.GetError() == 0);
        return (false, null);
    }

    private static (bool, Object?) GlGenBuffersWrapper(List<Object> args)
    {
        if (args[0].type != ObjectType.NUM || args[1].type != ObjectType.REF)
            Utils.Assert("Invalid value of glGenBuffers(args[0],args[1]).");

        var count = (int)((NumObject)args[0]).value;
        var refer = Utils.SearchObjectByAddress(((RefObject)args[1]).pointer);

        if (refer.type == ObjectType.ARRAY)
        {
            var array = (ArrayObject)refer;
            uint[] vaos = new uint[count];

            GL.GenBuffers(count, vaos);

            array.elements = new List<Object>(count);

            for (int i = 0; i < count; ++i)
                array.elements[i] = new NumObject((double)vaos[i]);
            Debug.Assert(GL.GetError() == 0);
        }
        else if (refer.type == ObjectType.NUM)
        {
            var id = GL.GenBuffer();
            ((NumObject)refer).value = id;
            Debug.Assert(GL.GetError() == 0);
        }
        else
            Utils.Assert("Invalid value of glGenBuffers:args[1].only number array or number is available");

        return (false, null);
    }

    private static (bool, Object?) GlBindBufferWrapper(List<Object> args)
    {
        if (args[0].type != ObjectType.BUILTIN || args[1].type != ObjectType.NUM)
            Utils.Assert("Invalid value of glBindBuffer(args[0],args[1]).");

        var flag = (BufferTarget)((NumObject)(((BuiltinObject)args[0]).GetBuiltinVar())).value;
        var obj = (int)((NumObject)args[1]).value;

        GL.BindBuffer(flag, obj);
        Debug.Assert(GL.GetError() == 0);
        return (false, null);
    }

    private static (bool, Object?) GlBufferDataWrapper(List<Object> args)
    {
        if (args[0].type != ObjectType.BUILTIN || args[1].type != ObjectType.NUM || args[2].type != ObjectType.ARRAY || args[3].type != ObjectType.BUILTIN)
            Utils.Assert("Invalid value of glBufferData(args[0],args[1],args[2],args[3]).");

        var arg0 = (BufferTarget)((NumObject)(((BuiltinObject)args[0]).GetBuiltinVar())).value;
        var arg1 = (int)((NumObject)args[1]).value;
        var arg2 = ((ArrayObject)args[2]);
        var arg3 = (BufferUsageHint)((NumObject)(((BuiltinObject)args[3]).GetBuiltinVar())).value;

        if (arg0 == BufferTarget.ElementArrayBuffer)
        {
            uint[] rawArg2 = new uint[arg2.elements.Count];
            for (int i = 0; i < arg2.elements.Count; i++)
                rawArg2[i] = (uint)((NumObject)arg2.elements[i]).value;
            GL.BufferData(arg0, arg1, rawArg2, arg3);
        }
        else
        {
            float[] rawArg2 = new float[arg2.elements.Count];
            for (int i = 0; i < arg2.elements.Count; i++)
                rawArg2[i] = (float)((NumObject)arg2.elements[i]).value;
            GL.BufferData(arg0, arg1, rawArg2, arg3);
        }

        Debug.Assert(GL.GetError() == 0);
        return (false, null);
    }

    private static (bool, Object?) GlCreateShaderWrapper(List<Object> args)
    {
        if (args[0].type != ObjectType.BUILTIN)
            Utils.Assert("Invalid value of glCreateShader(args[0]).");

        var arg0 = (ShaderType)((NumObject)(((BuiltinObject)args[0]).GetBuiltinVar())).value;
        var result = GL.CreateShader(arg0);
        Debug.Assert(GL.GetError() == 0);
        return (true, new NumObject(result));
    }

    private static (bool, Object?) GlShaderSourceWrapper(List<Object> args)
    {
        if (args[0].type != ObjectType.NUM || args[1].type != ObjectType.NUM || args[2].type != ObjectType.REF || !(args[3].type == ObjectType.REF || args[3].type == ObjectType.NIL))
            Utils.Assert("Invalid value of glShaderSource(args[0],args[1],args[2],args[3]).");

        var arg0 = (int)((NumObject)(args[0])).value;
        var arg1 = (int)((NumObject)(args[1])).value;
        var arg2 = ((StrObject)Utils.SearchObjectByAddress(((RefObject)(args[2])).pointer)).value;
        int arg3 = 0;
        if (args[3].type == ObjectType.REF)
            arg3 = (int)((NumObject)(Utils.SearchObjectByAddress(((RefObject)args[3]).pointer))).value;

        var arg2Arr = new string[arg1];
        for (int i = 0; i < arg1; ++i)
            arg2Arr[i] = arg2;

        var arg3Arr = new int[arg1];
        for (int i = 0; i < arg1; ++i)
            arg3Arr[i] = arg2Arr[i].Length;

        GL.ShaderSource(arg0, arg1, arg2Arr, arg3Arr);
        Debug.Assert(GL.GetError() == 0);
        return (false, null);
    }

    private static (bool, Object?) GlCompileShaderWrapper(List<Object> args)
    {
        if (args[0].type != ObjectType.NUM)
            Utils.Assert("Invalid value of glCompileShader(args[0]).");

        var arg0 = (int)((NumObject)args[0]).value;
        GL.CompileShader(arg0);

        int[] isSuccess = new int[1];
        GL.GetShader(arg0, ShaderParameter.CompileStatus, isSuccess);
        if (isSuccess[0] == 0)
        {
            var str = GL.GetShaderInfoLog(arg0);
            Console.WriteLine("ERROR::SHADER::COMPILATION_FAILED\n" + str);
        }

        Debug.Assert(GL.GetError() == 0);
        return (false, null);
    }

    private static (bool, Object?) GlCreateProgramWrapper(List<Object> args)
    {
        var result = GL.CreateProgram();
        Debug.Assert(GL.GetError() == 0);
        return (true, new NumObject(result));
    }

    private static (bool, Object?) GlAttachShaderWrapper(List<Object> args)
    {
        if (args[0].type != ObjectType.NUM || args[1].type != ObjectType.NUM)
            Utils.Assert("Invalid value of glAttachShader(args[0],args[1]).");

        var arg0 = (int)((NumObject)args[0]).value;
        var arg1 = (int)((NumObject)args[1]).value;

        GL.AttachShader(arg0, arg1);
        Debug.Assert(GL.GetError() == 0);
        return (false, null);
    }

    private static (bool, Object?) GlLinkProgramWrapper(List<Object> args)
    {
        if (args[0].type != ObjectType.NUM)
            Utils.Assert("Invalid value of glLinkProgram(args[0]).");

        var arg0 = (int)((NumObject)args[0]).value;

        GL.LinkProgram(arg0);

        int[] isSuccess = new int[1];
        GL.GetProgram(arg0, GetProgramParameterName.LinkStatus, isSuccess);
        if (isSuccess[0] == 0)
        {
            var str = GL.GetProgramInfoLog(arg0);
            Console.WriteLine("ERROR::SHADER::PROGRAM::LINKING_FAILED\n" + str);
        }
        Debug.Assert(GL.GetError() == 0);
        return (false, null);
    }

    private static (bool, Object?) GlClearColorWrapper(List<Object> args)
    {
        if (args[0].type != ObjectType.NUM || args[1].type != ObjectType.NUM || args[2].type != ObjectType.NUM || args[3].type != ObjectType.NUM)
            Utils.Assert("Invalid value of glClearColor(args[0],arg[1],arg[2],arg[3]).");

        var arg0 = (float)((NumObject)args[0]).value;
        var arg1 = (float)((NumObject)args[1]).value;
        var arg2 = (float)((NumObject)args[2]).value;
        var arg3 = (float)((NumObject)args[3]).value;

        GL.ClearColor(arg0, arg1, arg2, arg3);
        Debug.Assert(GL.GetError() == 0);
        return (false, null);
    }

    private static (bool, Object?) GlClearWrapper(List<Object> args)
    {
        if (args[0].type == ObjectType.BUILTIN)
        {
            var arg0 = (ClearBufferMask)((NumObject)((BuiltinObject)args[0]).GetBuiltinVar()).value;
            GL.Clear(arg0);
        }
        else if (args[0].type == ObjectType.NUM)
        {
            var arg0 = (ClearBufferMask)((NumObject)(args[0])).value;
            GL.Clear(arg0);
        }
        else
            Utils.Assert("Invalid value of glClear(args[0]).");
        Debug.Assert(GL.GetError() == 0);
        return (false, null);
    }

    private static (bool, Object?) GlUseProgramWrapper(List<Object> args)
    {
        if (args[0].type != ObjectType.NUM)
            Utils.Assert("Invalid value of glUseProgram(args[0]).");
        var arg0 = (int)((NumObject)args[0]).value;
        GL.UseProgram(arg0);
        Debug.Assert(GL.GetError() == 0);
        return (false, null);
    }

    private static (bool, Object?) GlDrawElementsWrapper(List<Object> args)
    {
        if (args[0].type != ObjectType.BUILTIN || args[1].type != ObjectType.NUM || args[2].type != ObjectType.BUILTIN || !(args[3].type == ObjectType.REF || args[3].type == ObjectType.NIL))
            Utils.Assert("Invalid value of glDrawElements(args[0],arg[1],arg[2],arg[3]).");

        var arg0 = (BeginMode)((NumObject)(((BuiltinObject)args[0]).GetBuiltinVar())).value;
        var arg1 = (int)((NumObject)(args[1])).value;
        var arg2 = (DrawElementsType)((NumObject)(((BuiltinObject)args[2]).GetBuiltinVar())).value;

        if (args[3].type == ObjectType.NIL)
            GL.DrawElements(arg0, arg1, arg2, 0);
        else
        {
            var arg3 = Utils.SearchObjectByAddress(((RefObject)args[3]).pointer);
            var arrArg3 = (ArrayObject)arg3;

            uint[] rawArgs3= new uint[arrArg3.elements.Count];
            for(int i = 0;i<arrArg3.elements.Count;++i)
                rawArgs3[i]=(uint)((NumObject)arrArg3.elements[i]).value;

            GL.DrawElements(arg0, arg1, arg2, rawArgs3);
        }
        return (false, null);
    }

    public static void RegisterBuiltins()
    {
        BuiltinManager.GetInstance().Register("GL_ARRAY_BUFFER", new NumObject((double)BufferTarget.ArrayBuffer));
        BuiltinManager.GetInstance().Register("GL_STATIC_DRAW", new NumObject((double)BufferUsageHint.StaticDraw));
        BuiltinManager.GetInstance().Register("GL_VERTEX_SHADER", new NumObject((double)ShaderType.VertexShader));
        BuiltinManager.GetInstance().Register("GL_FRAGMENT_SHADER", new NumObject((double)ShaderType.FragmentShader));
        BuiltinManager.GetInstance().Register("GL_COLOR_BUFFER_BIT", new NumObject((double)ClearBufferMask.ColorBufferBit));
        BuiltinManager.GetInstance().Register("GL_DEPTH_BUFFER_BIT", new NumObject((double)ClearBufferMask.DepthBufferBit));
        BuiltinManager.GetInstance().Register("GL_ELEMENT_ARRAY_BUFFER", new NumObject((double)BufferTarget.ElementArrayBuffer));
        BuiltinManager.GetInstance().Register("GL_FLOAT", new NumObject((double)VertexAttribPointerType.Float));
        BuiltinManager.GetInstance().Register("GL_FALSE", new NumObject((double)0));
        BuiltinManager.GetInstance().Register("GL_TRUE", new NumObject((double)1));
        BuiltinManager.GetInstance().Register("GL_TRIANGLES", new NumObject((double)PrimitiveType.Triangles));
        BuiltinManager.GetInstance().Register("GL_UNSIGNED_INT", new NumObject((double)VertexAttribPointerType.UnsignedInt));

        BuiltinManager.GetInstance().Register("gladLoadGL", GladLoadGLWrapper);
        BuiltinManager.GetInstance().Register("glGenVertexArrays", GLGenVertexArraysWrapper);
        BuiltinManager.GetInstance().Register("glBindVertexArray", GLBindVertexArrayWrapper);
        BuiltinManager.GetInstance().Register("glVertexAttribPointer", GLVertexAttribPointerWrapper);
        BuiltinManager.GetInstance().Register("glEnableVertexAttribArray", GLEnableVertexAttribArrayWrapper);
        BuiltinManager.GetInstance().Register("glGenBuffers", GlGenBuffersWrapper);
        BuiltinManager.GetInstance().Register("glBindBuffer", GlBindBufferWrapper);
        BuiltinManager.GetInstance().Register("glBufferData", GlBufferDataWrapper);
        BuiltinManager.GetInstance().Register("glCreateShader", GlCreateShaderWrapper);
        BuiltinManager.GetInstance().Register("glShaderSource", GlShaderSourceWrapper);
        BuiltinManager.GetInstance().Register("glCompileShader", GlCompileShaderWrapper);
        BuiltinManager.GetInstance().Register("glCreateProgram", GlCreateProgramWrapper);
        BuiltinManager.GetInstance().Register("glAttachShader", GlAttachShaderWrapper);
        BuiltinManager.GetInstance().Register("glLinkProgram", GlLinkProgramWrapper);
        BuiltinManager.GetInstance().Register("glClearColor", GlClearColorWrapper);
        BuiltinManager.GetInstance().Register("glClear", GlClearWrapper);
        BuiltinManager.GetInstance().Register("glUseProgram", GlUseProgramWrapper);
        BuiltinManager.GetInstance().Register("glDrawElements", GlDrawElementsWrapper);
    }
}
