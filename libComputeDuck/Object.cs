using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace ComputeDuck
{
    using OpCodes = List<int>;
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
        BUILTIN_FUNCTION,
        BUILTIN_DATA,
        BUILTIN_VARIABLE
    };

    public abstract class Object
    {
        public Object(ObjectType type)
        {
            this.type = type;
        }
        public abstract string Stringify();
        public abstract bool IsEqualTo(Object other);

        public IntPtr GetAddress()
        {
            GCHandle h = GCHandle.Alloc(this, GCHandleType.WeakTrackResurrection);
            IntPtr addr = GCHandle.ToIntPtr(h);
            return addr;
        }

        public ObjectType type;
    }

    public class NilObject : Object
    {
        public NilObject()
        : base(ObjectType.NIL)
        {
        }

        public override string Stringify()
        {
            return "nil";
        }

        public override bool IsEqualTo(Object other)
        {
            if (other.type != ObjectType.NIL)
                return false;
            return true;
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

        public override string Stringify()
        {
            return value.ToString();
        }

        public override bool IsEqualTo(Object other)
        {
            if (other.type != ObjectType.NUM)
                return false;
            return this.value == ((NumObject)other).value;
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

        public override string Stringify()
        {
            return value.ToString();
        }

        public override bool IsEqualTo(Object other)
        {
            if (other.type != ObjectType.BOOL)
                return false;
            return this.value == ((BoolObject)other).value;
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

        public override string Stringify()
        {
            return value.ToString();
        }

        public override bool IsEqualTo(Object other)
        {
            if (other.type != ObjectType.STR)
                return false;
            return this.value == ((StrObject)other).value;
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

        public override string Stringify()
        {
            string result = "[";
            if (elements.Count != 0)
            {
                foreach (var e in elements)
                    result += e.Stringify() + ",";
                result = result.Substring(0, result.Length - 1);
            }
            result += "]";
            return result;
        }


        public override bool IsEqualTo(Object other)
        {
            if (other.type != ObjectType.ARRAY)
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

        public override string Stringify()
        {
            return pointer.ToString();
        }

        public override bool IsEqualTo(Object other)
        {
            if (other.type != ObjectType.REF)
                return false;
            return this.pointer == ((RefObject)other).pointer;
        }
        public IntPtr pointer;
    }

    public class FunctionObject : Object
    {
        public FunctionObject(OpCodes opCodes, int localVarCount = 0, int parameterCount = 0) : base(ObjectType.FUNCTION)
        {
            this.opCodes = opCodes;
            this.localVarCount = localVarCount;
            this.parameterCount = parameterCount;
        }

        public override string Stringify()
        {
            GCHandle h = GCHandle.Alloc(this, GCHandleType.WeakTrackResurrection);
            IntPtr addr = GCHandle.ToIntPtr(h);
            return "function:(0x" + addr.ToString() + ")";
        }

        public override bool IsEqualTo(Object other)
        {
            if (other.type != ObjectType.REF)
                return false;

            var otherOpCodes = ((FunctionObject)other).opCodes;
            if (opCodes.Count != otherOpCodes.Count)
                return false;

            for (int i = 0; i < opCodes.Count; ++i)
                if (opCodes[i] != otherOpCodes[i])
                    return false;
            return true;
        }

        public OpCodes opCodes;
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

        public override string Stringify()
        {
            GCHandle h = GCHandle.Alloc(this, GCHandleType.WeakTrackResurrection);
            IntPtr addr = GCHandle.ToIntPtr(h);
            string result = "struct instance(0x:" + addr.ToString() + "):\n{\n";
            foreach (var member in members)
                result += member.Key + ":" + member.Value.Stringify() + "\n";
            result = result.Substring(0, result.Length - 1);
            result += "\n}\n";
            return result;
        }

        public override bool IsEqualTo(Object other)
        {
            if (other.type != ObjectType.STRUCT)
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

    public delegate bool BuiltinFn(List<Object> args, out Object result);

    public class BuiltinFunctionObject : Object
    {
        public BuiltinFunctionObject(string name, BuiltinFn fn)
        : base(ObjectType.BUILTIN_FUNCTION)
        {
            this.name = name;
            this.fn = fn;
        }

        public override string Stringify()
        {
            GCHandle h = GCHandle.Alloc(this, GCHandleType.WeakTrackResurrection);
            IntPtr addr = GCHandle.ToIntPtr(h);
            return "Builtin Function:(0x" + addr.ToString() + ")";
        }

        public override bool IsEqualTo(Object other)
        {
            if (other.type != ObjectType.BUILTIN_FUNCTION)
                return false;
            return name == ((BuiltinFunctionObject)other).name;
        }

        public string name;
        public BuiltinFn fn;
    }


    public delegate void DestroyFunc(object nativeData);
    public class BuiltinDataObject : Object
    {
        public BuiltinDataObject()
        : base(ObjectType.BUILTIN_DATA)
        {
        }

        ~BuiltinDataObject()
        {
            if (destroyFunc != null)
                destroyFunc(nativeData);
        }

        public override string Stringify()
        {
            GCHandle h = GCHandle.Alloc(this, GCHandleType.WeakTrackResurrection);
            IntPtr addr = GCHandle.ToIntPtr(h);
            return "Builtin Data:(0x" + addr.ToString() + ")";
        }

        public override bool IsEqualTo(Object other)
        {
            if (other.type != ObjectType.BUILTIN_DATA)
                return false;
            GCHandle h1 = GCHandle.Alloc(this, GCHandleType.WeakTrackResurrection);
            IntPtr addr1 = GCHandle.ToIntPtr(h1);

            GCHandle h2 = GCHandle.Alloc(this, GCHandleType.WeakTrackResurrection);
            IntPtr addr2 = GCHandle.ToIntPtr(h2);
            return addr1 == addr2;
        }

        public object? nativeData;
        public DestroyFunc? destroyFunc;
    }

    public class BuiltinVariableObject : Object
    {
        public BuiltinVariableObject(string name, Object obj)
         : base(ObjectType.BUILTIN_VARIABLE)
        {
            this.name = name;
            this.obj = obj;
        }

        public override bool IsEqualTo(Object other)
        {
            if (other.type != ObjectType.BUILTIN_VARIABLE)
                return false;
            return name == ((BuiltinVariableObject)other).name && obj == ((BuiltinVariableObject)other).obj;
        }

        public override string Stringify()
        {
            return "Builtin Variable:(" + name + ":" + obj.Stringify() + ")";
        }

        public string name;
        public Object obj;
    }
}