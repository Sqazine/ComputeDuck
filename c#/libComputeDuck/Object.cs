using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace ComputeDuck
{
    using OpCodes = List<int>;

    public delegate (bool, Object?) BuiltinFn(List<Object> args);

    public delegate void DestroyFunc(object nativeData);

    public class NativeData
    {

        public object? nativeData;
        public DestroyFunc? destroyFunc;

        public NativeData(object? nativeData, DestroyFunc? fn)
        {
            this.nativeData = nativeData;
            this.destroyFunc = fn;
        }

        ~NativeData()
        {
            if (destroyFunc != null)
            {
                Console.WriteLine("Deleting Object!");
                destroyFunc(nativeData);
            }
        }
    };

    public enum ObjectType
    {
        NIL,
        NUM,
        BOOL,
        STR,
        ARRAY,
        STRUCT,
        REF,
        FUNCTION,
        BUILTIN
    };

    public abstract class Object
    {
        public Object(ObjectType type)
        {
            gcHandle = GCHandle.Alloc(this, GCHandleType.WeakTrackResurrection);
            this.type = type;
        }
        public virtual bool IsEqualTo(Object other)
        {
            if (other != null && other.type != this.type)
                return false;
            return true;
        }

        public IntPtr GetAddress()
        {
            IntPtr addr = GCHandle.ToIntPtr(gcHandle);
            return addr;
        }

        public ObjectType type;
        private GCHandle gcHandle;
    }

    public class NilObject : Object
    {
        public NilObject()
        : base(ObjectType.NIL)
        {
        }

        public override string ToString()
        {
            return "nil";
        }
    }

    public class NumObject : Object
    {
        public NumObject()
        : base(ObjectType.NUM)
        {
            this.value = 0.0;
        }
        public NumObject(double value)
        : base(ObjectType.NUM)
        {
            this.value = value;
        }

        public NumObject(int value)
        : base(ObjectType.NUM)
        {
            this.value = value;
        }

        public NumObject(uint value)
            : base(ObjectType.NUM)
        {
            this.value = value;
        }

        public override string ToString()
        {
            return value.ToString();
        }

        public override bool IsEqualTo(Object other)
        {
            return base.IsEqualTo(other) && this.value == ((NumObject)other).value;
        }
        public double value;
    }


    public class BoolObject : Object
    {
        public BoolObject()
        : base(ObjectType.BOOL)
        {
            this.value = false;
        }
        public BoolObject(bool value)
        : base(ObjectType.BOOL)
        {
            this.value = value;
        }

        public override string ToString()
        {
            return value.ToString();
        }

        public override bool IsEqualTo(Object other)
        {
            return base.IsEqualTo(other) && this.value == ((BoolObject)other).value;
        }
        public bool value;
    }

    public class StrObject : Object
    {
        public StrObject()
        : base(ObjectType.STR)
        {
            this.value = "";
        }
        public StrObject(string value)
        : base(ObjectType.STR)
        {
            this.value = value;
        }

        public override string ToString()
        {
            return value.ToString();
        }

        public override bool IsEqualTo(Object other)
        {
            return base.IsEqualTo(other) && this.value == ((StrObject)other).value;
        }
        public string value;
    }

    public class ArrayObject : Object
    {
        public ArrayObject(List<Object> elements)
        : base(ObjectType.ARRAY)
        {
            this.elements = elements;
        }

        public override string ToString()
        {
            string result = "[";
            if (elements.Count != 0)
            {
                foreach (var e in elements)
                    result += e.ToString() + ",";
                result = result.Substring(0, result.Length - 1);
            }
            result += "]";
            return result;
        }


        public override bool IsEqualTo(Object other)
        {
            if (base.IsEqualTo(other) == false)
                return false;

            ArrayObject arrayOther = (ArrayObject)other;

            if (arrayOther.elements.Count != this.elements.Count)
                return false;

            for (var i = 0; i < this.elements.Count; ++i)
                if (!this.elements[i].IsEqualTo(arrayOther.elements[i]))
                    return false;
            return true;
        }

        public List<Object> elements;
    }

    public class RefObject : Object
    {
        public RefObject()
         : base(ObjectType.REF)
        {
            this.pointer = new IntPtr(-1);
        }
        public RefObject(IntPtr pointer)
          : base(ObjectType.REF)
        {
            this.pointer = pointer;
        }

        public override string ToString()
        {
            return GCHandle.FromIntPtr(pointer).Target.ToString();
        }

        public override bool IsEqualTo(Object other)
        {
            return base.IsEqualTo(other) && this.pointer == ((RefObject)other).pointer;
        }
        public IntPtr pointer;
    }

    public class FunctionObject : Object
    {
        public FunctionObject(Chunk chunk, int localVarCount = 0, int parameterCount = 0) : base(ObjectType.FUNCTION)
        {
            this.chunk = chunk;
            this.localVarCount = localVarCount;
            this.parameterCount = parameterCount;
        }

        public override string ToString()
        {
            return "function:(0x" + GetAddress() + ")";
        }

        public string ToStringWithChunk()
        {
            string result=this.ToString();
            result += ":\n" + chunk.ToString();
            return result; 
        }

        public override bool IsEqualTo(Object other)
        {
            if (base.IsEqualTo(other) == false)
                return false;

            var otherOpCodes = ((FunctionObject)other).chunk.opCodes;
            if (chunk.opCodes.Count != otherOpCodes.Count)
                return false;

            for (int i = 0; i < chunk.opCodes.Count; ++i)
                if (chunk.opCodes[i] != otherOpCodes[i])
                    return false;
            return true;
        }

        public Chunk chunk;
        public int localVarCount;
        public int parameterCount;
    }

    public class StructObject : Object
    {
        public StructObject()
         : base(ObjectType.STRUCT)
        {
            this.members = new Dictionary<string, Object>();
        }

        public StructObject(Dictionary<string, Object> members)
          : base(ObjectType.STRUCT)
        {
            this.members = members;
        }

        public override string ToString()
        {
            string result = "struct instance(0x:" + GetAddress().ToString() + "):\n{\n";
            foreach (var member in members)
                result += member.Key + ":" + member.Value.ToString() + "\n";
            result = result.Substring(0, result.Length - 1);
            result += "\n}\n";
            return result;
        }

        public override bool IsEqualTo(Object other)
        {
            if (base.IsEqualTo(other) == false)
                return false;

            var structOther = (StructObject)other;

            foreach (var entry in this.members)
                foreach (var entry2 in structOther.members)
                    if (entry.Key == entry2.Key)
                        if (!entry.Value.IsEqualTo(entry2.Value))
                            return false;
            return true;
        }

        public Dictionary<string, Object> members;
    }

    public class BuiltinObject : Object
    {
        private Object? var = null;
        private BuiltinFn? fn = null;
        private NativeData? data = null;

        public BuiltinObject(Object var)
          : base(ObjectType.BUILTIN)
        {
            this.var = var;
        }

        public BuiltinObject(BuiltinFn fn)
          : base(ObjectType.BUILTIN)
        {
            this.fn = fn;
        }

        public BuiltinObject(NativeData data)
         : base(ObjectType.BUILTIN)
        {
            this.data = data;
        }

        public bool IsNativeData()
        {
            return data != null;
        }

        public bool IsBuiltinFn()
        {
            return fn != null;
        }

        public bool IsBuiltinVar()
        {
            return var != null;
        }

        public NativeData? GetNativeData()
        {
            return data;
        }

        public BuiltinFn? GetBuiltinFn()
        {
            return fn;
        }

        public Object? GetBuiltinVar()
        {
            return var;
        }

        public override bool IsEqualTo(Object other)
        {
            if (base.IsEqualTo(other) == false)
                return false;

            if (IsNativeData())
                return data.Equals(((BuiltinObject)other)?.data);

            if (IsBuiltinFn())
                return fn.Equals(((BuiltinObject)other)?.fn);

            if (IsBuiltinVar())
                return var.IsEqualTo(((BuiltinObject)other).var);

            return false;
        }

        public override string ToString()
        {
            string result = "Builtin:";
            if (IsNativeData())
            {
                GCHandle h = GCHandle.Alloc(this.data, GCHandleType.WeakTrackResurrection);
                IntPtr addr = GCHandle.ToIntPtr(h);
                result += "(0x" + addr.ToString() + ")";
            }
            else if (IsBuiltinFn())
            {
                GCHandle h = GCHandle.Alloc(this.fn, GCHandleType.WeakTrackResurrection);
                IntPtr addr = GCHandle.ToIntPtr(h);
                result += "(0x" + addr.ToString() + ")";
            }
            else if (IsBuiltinVar())
                result += "(0x" + this.var.GetAddress().ToString() + ")";
            else
                return "";

            return result;
        }
    }
}