using System;
using System.Reflection;
using System.Runtime.CompilerServices;
using ComputeDuck;
class Execute
{
    private static PreProcessor mPreProcessor = new PreProcessor();
    private static Parser mParser = new Parser();
    private static Compiler mCompiler = new Compiler();
    private static VM mVm = new VM();

    static void SetBasePath(string path)
    {
        BuiltinManager.GetInstance().SetExecuteFilePath(Path.GetDirectoryName(path)+"/");
    }

    static void Run(string content)
    {
        var tokens = mPreProcessor.PreProcess(content);

        foreach (var token in tokens)
            Console.WriteLine(token.Stringify());

        var stmts = mParser.Parse(tokens);

        foreach (var stmt in stmts)
            Console.WriteLine(stmt.Stringify());

        var fn = mCompiler.Compile(stmts);

       Console.WriteLine(fn.ToStringWithChunk());

        mVm.Run(fn);
    }
    static void Repl(string path)
    {
        SetBasePath(path);

        string line;
        string allLines = "";

        Console.Write("> ");
        line = Console.ReadLine();
        while (line != "")
        {
            if (line == "clear")
                allLines = "";
            else if (line == "exit")
                return;
            else
            {
                allLines += line;
                Run(allLines);
            }

            Console.Write("> ");
            line = Console.ReadLine();
        }
    }

    static void RunFile(string path)
    {
        SetBasePath(path);
        string content = Utils.ReadFile(path);
        Run(content);
    }

    static void PrintUsage()
    {
        Console.WriteLine("Usage: ComputeDuck [option]:");
        Console.WriteLine("-h or --help:show usage info.");
        Console.WriteLine("-f or --file:run source file with a valid file path,like : python3 main.py -f examples/array.cd.");
        Environment.Exit(1);
    }

    static void Main(string[] args)
    {
        string sourceFilePath = "";
        for(var i=0;i<args.Count();++i)
        {
            if (args[i] =="-f" || args[i] == "--file")
            {
                if (i + 1 < args.Count())
                    sourceFilePath = args[i + 1];
                else
                    PrintUsage();
            }
            
            if(args[i] == "-h" || args[i] == "--help")
                PrintUsage();
        }
        if (sourceFilePath.Length!=0)
            RunFile(sourceFilePath);
        else
            Repl(Assembly.GetEntryAssembly().Location);
    }
}