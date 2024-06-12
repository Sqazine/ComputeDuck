using System;
using System.Collections.Generic;

namespace ComputeDuck
{
    public class BuiltinManager
    {
        private static BuiltinManager? instance = null;
        public Dictionary<string,BuiltinObject> m_Builtins;
        private string m_CurExecuteFilePath;


        private BuiltinManager()
        {
            m_Builtins = new Dictionary<string, BuiltinObject>();

            Register("print", _Print);
            Register("println", _Println);
            Register("sizeof", _SizeOf);
            Register("insert", _Insert);
            Register("erase", _Erase);
            Register("clock", _Clock);
        }

        public static BuiltinManager GetInstance()
        {
            if (instance == null)
                instance = new BuiltinManager();
            return instance;
        }

        public void Register(string name, BuiltinFn fn)
        {
            if (m_Builtins.ContainsKey(name))
                Utils.Assert("Redefined builtin:"+ name);
            m_Builtins[name] = new BuiltinObject(fn);
        }

        public void Register(string name, Object obj)
        {
            if (m_Builtins.ContainsKey(name))
                Utils.Assert("Redefined builtin:" + name);
            m_Builtins[name] = new BuiltinObject(obj);
        }

        public void SetExecuteFilePath(string path)
        {
            m_CurExecuteFilePath = path;
        }

        public string ToFullPath(string path)
        {
            return m_CurExecuteFilePath + path;
        }

        private (bool,Object?) _Print(List<Object> args)
        {
            if (args.Count == 0)
                return (false,null);
            Console.Write(args[0].ToString());
            return (false, null);
        }

        private (bool, Object?) _Println(List<Object> args)
        {
            if (args.Count == 0)
                return (false, null);
            Console.WriteLine(args[0].ToString());
            return (false, null);
        }

        private (bool, Object?) _SizeOf(List<Object> args)
        {
            Object? result = null;
            if (args.Count == 0 || args.Count > 1)
                Utils.Assert("[Native function 'sizeof']:Expect a argument.");

            if (args[0].type == ObjectType.ARRAY)
                result = new NumObject(((ArrayObject)args[0]).elements.Count);
            else if (args[0].type == ObjectType.STR)
                result = new NumObject(((StrObject)args[0]).value.Length);
            else
                Utils.Assert("[Native function 'sizeof']:Expect a array or string argument.");
            return (true,result);
        }

        private (bool, Object?) _Insert(List<Object> args)
        {
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

                str.value.Insert(iIndex, args[2].ToString());
            }
            else
                Utils.Assert("[Native function 'insert']:Expect a array or string argument.");
            return (false, null);
        }

        private (bool, Object?) _Erase(List<Object> args)
        {
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

            return (false, null);
        }

        private (bool, Object?) _Clock(List<Object> args)
        {
            long currentTicks = DateTime.Now.Ticks;
            DateTime dtFrom = new DateTime(1970, 1, 1, 0, 0, 0, 0);
            long currentMillis = (currentTicks - dtFrom.Ticks) / 10000;
            Object? result = new NumObject(currentMillis/1000.0);
            return (true,result);
        }
    }
}