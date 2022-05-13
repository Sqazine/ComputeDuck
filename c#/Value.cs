namespace ComputeDuck
{
    public enum ValueType
    {
        NIL,
        NUM,
        BOOL,
        OBJECT,
        UNKNOWN
    }

    public class Value
    {
        public Value()
        {
            this.type = ValueType.NIL;
        }

        public Value(double number)
        {
            this.number = number;
            this.type = ValueType.NUM;
        }

        public Value(bool boolean)
        {
            this.boolean = boolean;
            this.type = ValueType.BOOL;
        }

        public Value(Object obj)
        {
            this.obj = obj;
            this.type = ValueType.OBJECT;
        }

        public Value(ValueType type)
        {
            this.type = type;
        }

        public ValueType Type()
        {
            return type;
        }

        public string Stringify()
        {
            switch (type)
            {
                case ValueType.NIL:
                    return "nil";
                case ValueType.NUM:
                    return number.ToString();
                case ValueType.BOOL:
                    return boolean ? "true" : "false";
                case ValueType.OBJECT:
                    return obj.Stringify();
                default:
                    return "nil";
            }
        }

        public bool IsEqualTo(Value other)
        {
            switch (type)
            {
                case ValueType.NIL:
                    return other.Type() == ValueType.NIL;
                case ValueType.UNKNOWN:
                    {
                        if (other.type == ValueType.UNKNOWN)
                            return true;
                        return false;
                    }
                case ValueType.NUM:
                    {
                        if (other.Type() == ValueType.NUM)
                            return number == other.number;
                        else
                            return false;
                    }
                case ValueType.BOOL:
                    {
                        if (other.Type() == ValueType.BOOL)
                            return boolean == other.boolean;
                        return false;
                    }
                case ValueType.OBJECT:
                    {
                        if (other.Type() == ValueType.OBJECT)
                            return obj.IsEqualTo(other.obj);
                        return false;
                    }
                default:
                    return other.Type() == ValueType.NIL;
            }
        }

        public ValueType type;

        public double number;
        public bool boolean;
        public Object obj;
        public static readonly Value g_UnknownValue = new Value(ValueType.UNKNOWN);
    }

}