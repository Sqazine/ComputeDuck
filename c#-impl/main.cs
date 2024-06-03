using System;
using ComputeDuck;
class Execute
{
    static void Repl()
    {
        string line = "";
        PreProcessor preProcessor = new PreProcessor();
        Parser parser = new Parser();
        Compiler compiler = new Compiler();
        VM vm = new VM();
        Console.Write("> ");
        line = Console.ReadLine();
        while (line != "")
        {
            var tokens = preProcessor.PreProcess(line);

            foreach (var token in tokens)
                Console.WriteLine(token.Stringify());

            var stmts = parser.Parse(tokens);

            foreach (var stmt in stmts)
                Console.WriteLine(stmt.Stringify());

            var chunk = compiler.Compile(stmts);

            chunk.Stringify();

            vm.ResetStatus();
            vm.Run(chunk);

            Console.Write("> ");
            line = Console.ReadLine();
        }
    }

    static void RunFile(string path)
    {
        string content = Utils.ReadFile(path);
        PreProcessor preProcessor = new PreProcessor();
        Parser parser = new Parser();
        Compiler compiler = new Compiler();
        VM vm = new VM();
        var tokens = preProcessor.PreProcess(content);

        foreach (var token in tokens)
            Console.WriteLine(token.Stringify());

        var stmts = parser.Parse(tokens);

        foreach (var stmt in stmts)
            Console.WriteLine(stmt.Stringify());

        var chunk = compiler.Compile(stmts);

        chunk.Stringify();

        vm.Run(chunk);

    }
    static void Main(string[] args)
    {
        if (args.Length == 1)
            RunFile(args[0]);
        else if (args.Length == 0)
            Repl();
        else
            Console.WriteLine("Usage: ComputeDuck [filepath]");
    }
}