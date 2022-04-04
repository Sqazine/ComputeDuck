using System;

namespace ComputeDuck
{
    class ComputeDuck
    {
        static void Repl()
        {
            string line ="";
            Lexer lexer = new Lexer();
            Parser parser = new Parser();
            Compiler compiler = new Compiler();
            Console.Write("> ");
            line = Console.ReadLine();
            while (line != "")
            {
                var tokens = lexer.GenerateTokens(line);

                foreach (var token in tokens)
                    Console.WriteLine(token.Stringify());

                var stmts = parser.Parse(tokens);

                foreach (var stmt in stmts)
                    Console.WriteLine(stmt.Stringify());

                Frame frame = compiler.Compile(stmts);

                Console.WriteLine(frame.Stringify());

                Console.Write("> ");
                line = Console.ReadLine();
            }
        }

        static void RunFile(string path)
        {
            string content = Utils.ReadFile(path);
            Lexer lexer = new Lexer();
            Parser parser = new Parser();
            Compiler compiler = new Compiler();

            var tokens = lexer.GenerateTokens(content);

            foreach (var token in tokens)
                Console.WriteLine(token.Stringify());

            var stmts = parser.Parse(tokens);

            foreach (var stmt in stmts)
                Console.WriteLine(stmt.Stringify());


            Frame frame = compiler.Compile(stmts);

            Console.WriteLine(frame.Stringify());

        }
        static void Main(string[] args)
        {
            if (args.Length == 1)
                RunFile(args[1]);
            else if (args.Length == 0)
                Repl();
            else
                Console.WriteLine("Usage: ComputeDuck [filepath]");
        }
    }
}