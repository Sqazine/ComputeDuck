using System;
using System.Collections.Generic;
using System.IO;
using System.Reflection;

namespace ComputeDuck
{
    public class Utils
    {
        public static void Assert(string msg)
        {
            Console.WriteLine(msg);
            System.Environment.Exit(1);
        }

        public static string ReadFile(string path)
        {
            if (!File.Exists(path))
                Assert("Failed to open file:" + path);
            return System.IO.File.ReadAllText(path);
        }

        //thanks for:
        //https://www.cnblogs.com/jiceberg420/p/12648624.html
        //http://theraneman.blogspot.com/2010/04/creating-instance-of-type-outside.html
        public static void LoadAssembly(AssemblyName[] arr,string dir)
        {
            Assembly[] loadedAssemblies = AppDomain.CurrentDomain.GetAssemblies();
            List<string> names = new List<string>();
            foreach (Assembly assem in loadedAssemblies)
            {
                names.Add(assem.FullName);
            }

            foreach (AssemblyName aname in arr)
            {
                if (!names.Contains(aname.FullName))
                {
                    try
                    {
                        Assembly loadedAssembly = Assembly.LoadFrom(dir+aname.Name+".dll");
                        AssemblyName[] referencedAssemblies = loadedAssembly.GetReferencedAssemblies();
                        LoadAssembly(referencedAssemblies,dir);
                    }
                    catch (Exception ex)
                    {
                        continue;
                    }
                }
            }
        }
    }
}