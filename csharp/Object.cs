namespace ComputeDuck
{
    public enum ObjectType
    {
        NUM,
        STR,
        BOOL,
        NIL,
        ARRAY,
        STRUCT,
        REF,
        LAMBDA
    };

    public abstract class Object
    {
        public abstract string Stringify();
        public abstract ObjectType Type();
        public abstract bool IsEqualTo(Object other);
    }

    public class NumObject : Object
    {
        public NumObject() { this.value = 0.0; }
        public NumObject(double value) { this.value = value; }

        public override string Stringify()
        {
            return value.ToString();
        }
        public override ObjectType Type()
        {
            return ObjectType.NUM;
        }
        public override bool IsEqualTo(Object other)
        {
            if (other.Type() != ObjectType.NUM)
                return false;
            return this.value == ((NumObject)other).value;
        }

        public double value;
    }

    public class StrObject : Object
    {
        public StrObject() { this.value = ""; }
        public StrObject(string value) { this.value = value; }

        public override string Stringify()
        {
            return value.ToString();
        }
        public override ObjectType Type()
        {
            return ObjectType.STR;
        }
        public override bool IsEqualTo(Object other)
        {
            if (other.Type() != ObjectType.STR)
                return false;
            return this.value == ((StrObject)other).value;
        }
        public string value;
    }

    public class BoolObject : Object
    {
        public BoolObject() { this.value = false; }
        public BoolObject(bool value) { this.value = value; }

        public override string Stringify()
        {
            return value ? "true" : "false";
        }
        public override ObjectType Type()
        {
            return ObjectType.BOOL;
        }
        public override bool IsEqualTo(Object other)
        {
            if (other.Type() != ObjectType.BOOL)
                return false;
            return this.value == ((BoolObject)other).value;
        }
        public bool value;
    }


    public class NilObject : Object
    {
        public NilObject() { }
        public override string Stringify()
        {
            return "nil";
        }
        public override ObjectType Type()
        {
            return ObjectType.NIL;
        }
        public override bool IsEqualTo(Object other)
        {
            if (other.Type() != ObjectType.NIL)
                return false;
            return true;
        }
    }

    public class ArrayObject : Object
    {
        public ArrayObject(List<Object> elements) { this.elements = elements; }

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

        public override ObjectType Type()
        {
            return ObjectType.ARRAY;
        }

        public override bool IsEqualTo(Object other)
        {
            if (other.Type() != ObjectType.ARRAY)
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
        public RefObject() { this.name = ""; }
        public RefObject(string name) { this.name = name; }

        public override string Stringify()
        {
            return name;
        }
        public override ObjectType Type()
        {
            return ObjectType.REF;
        }
        public override bool IsEqualTo(Object other)
        {
            if (other.Type() != ObjectType.REF)
                return false;
            return this.name == ((RefObject)other).name;
        }
        public string name;
    }

    public class LambdaObject : Object
    {
        public LambdaObject() { this.idx = 0; }
        public LambdaObject(int idx) { this.idx = idx; }

        public override string Stringify()
        {
            return "lambda:" + idx.ToString();
        }
        public override ObjectType Type()
        {
            return ObjectType.LAMBDA;
        }
        public override bool IsEqualTo(Object other)
        {
            if (other.Type() != ObjectType.LAMBDA)
                return false;
            return this.idx == ((LambdaObject)other).idx;
        }

        public int idx;
    }


    public class StructObject : Object
    {
        public StructObject()
        {
            this.name = "";
            this.members = new Dictionary<string, Object>();
        }

        public StructObject(string name, Dictionary<string, Object> members)
        {
            this.name = name;
            this.members = members;
        }

        public override string Stringify()
        {
            string result = "struct instance " + name;
            if (members.Count > 0)
            {
                result += ":\n";
                foreach (var entry in members)
                    result += entry.Key + "=" + entry.Value + "\n";
                result = result.Substring(0, result.Length - 1);
            }
            return result;
        }

        public override bool IsEqualTo(Object other)
        {
            if (other.Type() != ObjectType.STRUCT)
                return false;

            var structOther = (StructObject)other;

            if (this.name != structOther.name)
                return false;

            foreach (var entry in this.members)
                foreach (var entry2 in structOther.members)
                    if (entry.Key == entry2.Key)
                        if (!entry.Value.IsEqualTo(entry2.Value))
                            return false;
            return true;
        }

        public override ObjectType Type()
        {
            return ObjectType.STRUCT;
        }


        public void AssignMember(string name, Object value)
        {
            if (this.members.ContainsKey(name))
                this.members[name] = value;
            else Utils.Assert("Undefine struct member:" + name);
        }

        public Object GetMember(string name)
        {
            if (this.members.ContainsKey(name))
                return this.members[name];
            return null;
        }

       public string name;
       public Dictionary<string, Object> members;
    }
}