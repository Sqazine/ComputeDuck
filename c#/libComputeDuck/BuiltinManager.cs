using System;
using System.Collections.Generic;

namespace ComputeDuck
{
    public class BuiltinManager
    {
        private static BuiltinManager instance = null;
        public List<BuiltinFunctionObject> m_BuiltinFunctions;
        public List<string> m_BuiltinFunctionNames;
        public List<BuiltinVariableObject> m_BuiltinVariables;
        public List<string> m_BuiltinVariableNames;

        private BuiltinManager()
        {
            m_BuiltinFunctions = new List<BuiltinFunctionObject>();
            m_BuiltinFunctionNames = new List<string>();
            m_BuiltinVariables = new List<BuiltinVariableObject>();
            m_BuiltinVariableNames = new List<string>();
            Init();
        }

        public static BuiltinManager GetInstance()
        {
            if (instance == null)
                instance = new BuiltinManager();
            return instance;
        }

        public void Init()
        {
            RegisterFunction("print", _Print);
            RegisterFunction("println", _Println);
            RegisterFunction("sizeof", _SizeOf);
            RegisterFunction("insert", _Insert);
            RegisterFunction("erase", _Erase);
            RegisterFunction("clock", _Clock);
        }

        public void Release()
        {
            m_BuiltinFunctions.Clear();
            m_BuiltinFunctionNames.Clear();
            m_BuiltinVariables.Clear();
            m_BuiltinVariableNames.Clear();
        }

        public void RegisterFunction(string name, BuiltinFn fn)
        {
            m_BuiltinFunctions.Add(new BuiltinFunctionObject(name, fn));
            m_BuiltinFunctionNames.Add(name);
        }

        public void RegisterVariable(string name, Object obj)
        {
            m_BuiltinVariables.Add(new BuiltinVariableObject(name, obj));
            m_BuiltinVariableNames.Add(name);
        }

        private bool _Print(List<Object> args, out Object result)
        {
            result = null;
            if (args.Count == 0)
                return false;
            Console.Write(args[0].Stringify());
            return false;
        }

        private bool _Println(List<Object> args, out Object result)
        {
            result = null;
            if (args.Count == 0)
                return false;
            Console.WriteLine(args[0].Stringify());
            return false;
        }

        private bool _SizeOf(List<Object> args, out Object result)
        {
            result = null;
            if (args.Count == 0 || args.Count > 1)
                Utils.Assert("[Native function 'sizeof']:Expect a argument.");

            if (args[0].type == ObjectType.ARRAY)
                result = new NumObject(((ArrayObject)args[0]).elements.Count);
            else if (args[0].type == ObjectType.STR)
                result = new NumObject(((StrObject)args[0]).value.Length);
            else
                Utils.Assert("[Native function 'sizeof']:Expect a array or string argument.");
            return true;
        }

        private bool _Insert(List<Object> args, out Object result)
        {
            result = null;
            if (args.Count == 0 || args.Count != 3)
                Utils.Assert("[Native function 'insert']:Expect 3 arguments,the arg0 must be array or string object.The arg1 is the index object.The arg2 is the value object.");

            if (args[0].type == ObjectType.ARRAY)
            {
                var array = (ArrayObject)(args[0]);
                if (args[1].type != ObjectType.NUM)
                    Utils.Assert("[Native funcrion 'insert']:Arg1 must be integer type while insert to a array.");
                int iIndex = (int)((NumObject)args[1]).value;
                if (iIndex < 0 || iIndex >= array.elements.Count)
                    Utils.Assert("[Native function 'insert']:Index out of array's range");

                array.elements.Insert(iIndex, args[2]);
            }
            else if (args[0].type == ObjectType.STR)
            {
                var str = (StrObject)(args[0]);
                if (args[1].type != ObjectType.NUM)
                    Utils.Assert("[Native funcrion 'insert']:Arg1 must be integer type while insert to a string.");
                int iIndex = (int)((NumObject)args[1]).value;
                if (iIndex < 0 || iIndex >= str.value.Length)
                    Utils.Assert("[Native function 'insert']:Index out of array's range");

                str.value.Insert(iIndex, args[2].Stringify());
            }
            else
                Utils.Assert("[Native function 'insert']:Expect a array or string argument.");
            return false;
        }

        private bool _Erase(List<Object> args, out Object result)
        {
            result = null;
            if (args.Count == 0 || args.Count != 2)
                Utils.Assert("[Native function 'erase']:Expect 2 arguments,the arg0 must be array or string object..The arg1 is the corresponding index object.");

            if (args[0].type == ObjectType.ARRAY)
            {
                var array = (ArrayObject)(args[0]);
                if (args[1].type != ObjectType.NUM)
                    Utils.Assert("[Native funcrion 'erase']:Arg1 must be integer type while deleting array element.");
                int iIndex = (int)((NumObject)args[1]).value;
                if (iIndex < 0 || iIndex >= array.elements.Count)
                    Utils.Assert("[Native function 'erase']:Index out of array's range");

                array.elements.RemoveAt(iIndex);
            }
            else if (args[0].type == ObjectType.STR)
            {
                var str = (StrObject)(args[0]);
                if (args[1].type != ObjectType.NUM)
                    Utils.Assert("[Native funcrion 'erase']:Arg1 must be integer type while deleting string element.");
                 int iIndex = (int)((NumObject)args[1]).value;
                if (iIndex < 0 || iIndex >= str.value.Length)
                    Utils.Assert("[Native function 'erase']:Index out of array's range");

                str.value.Remove(iIndex);
            }
            else
                Utils.Assert("[Native function 'erase']:Expect a array or string argument.");

            return false;
        }

        private bool _Clock(List<Object> args, out Object result)
        {
            long currentTicks = DateTime.Now.Ticks;
            DateTime dtFrom = new DateTime(1970, 1, 1, 0, 0, 0, 0);
            long currentMillis = (currentTicks - dtFrom.Ticks) / 10000;
            result = new NumObject(currentMillis);
            return true;
        }
    }
}