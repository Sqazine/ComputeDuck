using System;

namespace ComputeDuck
{
    class ComputeDuck
    {
        static void Repl()
        {
            string line;
            Lexer lexer = new Lexer();
            Console.Write("> ");
            line = Console.ReadLine();
            while (line != "")
            {
                var tokens = lexer.GenerateTokens(line);

                foreach (var token in tokens)
                    Console.WriteLine(token.Stringify());



                Console.Write("> ");
                line = Console.ReadLine();
            }
        }

        static void RunFile(string path)
        {
            string content = Utils.ReadFile(path);
            Lexer lexer = new Lexer();
            var tokens = lexer.GenerateTokens(content);

            foreach (var token in tokens)
                Console.WriteLine(token.Stringify());

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