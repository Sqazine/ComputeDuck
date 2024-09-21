using ComputeDuck;
using System.Collections.Generic;

namespace ComputeDuck
{
    class TokenBlockTable
    {
        public int refCount = 0;
        public string filePath;
        public List<string> importedFilePaths = new List<string>();

        public List<Token> tokens = new List<Token>();
    }

    public class PreProcessor
    {
        private Lexer m_Lexer = new Lexer();

        public List<Token> PreProcess(string src)
        {
            var tokens = m_Lexer.GenerateTokens(src);
            var tables = new List<TokenBlockTable>();

            var loc = SearchImportToken(tokens);

            if (loc == -1)
            {
                tokens.Add(new Token(TokenType.END, "END", 0, 0, "RootFile"));
                return tokens;
            }

            tables.Add(FindBlockTable(tokens));

            for (int i = 0; i < tables.Count; ++i)
            {
                foreach (var path in tables[i].importedFilePaths)
                {
                    var fullPath = Config.GetInstance().ToFullPath(path);
                    var toks = m_Lexer.GenerateTokens(Utils.ReadFile(fullPath), path);

                    var alreadyExists = false;
                    for (int j = 0; j < tables.Count; ++j)
                    {
                        if (tables[j].filePath == path)
                        {
                            tables[j].refCount += 1;
                            alreadyExists = true;
                            break;
                        }
                    }

                    if (alreadyExists != true)
                    {
                        var blockTable = FindBlockTable(toks);
                        blockTable.filePath = path;
                        blockTable.refCount++;
                        tables.Add(blockTable);
                    }
                }
            }
            tables.Sort((TokenBlockTable left, TokenBlockTable right) =>
            {
                return left.refCount < right.refCount ? 1 : -1;
            });

            var result = new List<Token>();

            foreach (var t in tables)
                for (int i = 0; i < t.tokens.Count; ++i)
                    result.Add(t.tokens[i]);
            result.Add(new Token(TokenType.END, "END", 0, 0, "RootFile"));
            return result;
        }

        private TokenBlockTable FindBlockTable(List<Token> tokens)
        {
            var result = new TokenBlockTable();

            var loc = SearchImportToken(tokens);
            if (loc == -1)
            {
                result.tokens = tokens;
                return result;
            }

            var importedFilePaths = new List<string>();

            while (loc != -1)
            {
                if (tokens[loc + 1].type != TokenType.LPAREN)
                    Utils.Assert("[line " + tokens[loc + 1].line.ToString() + "]:Expect '(' after import keyword.");

                if (tokens[loc + 2].type != TokenType.STRING)
                    Utils.Assert("[line " + tokens[loc + 2].literal.ToString() + "]:Expect file path after import stmt's '('.");

                if (tokens[loc + 3].type != TokenType.RPAREN)
                    Utils.Assert("[line " + tokens[loc + 3].literal.ToString() + "]:Expect ')' after import stmt's file path.");

                if (tokens[loc + 4].type != TokenType.SEMICOLON)
                    Utils.Assert("[line " + tokens[loc + 4].literal.ToString() + "]:Expect ';' after the end of import stmt.");

                importedFilePaths.Add(tokens[loc + 2].literal);
                tokens.RemoveRange(loc, 5);
                loc = SearchImportToken(tokens);
            }

            result.importedFilePaths = importedFilePaths;
            result.tokens = tokens;
            return result;
        }

        private int SearchImportToken(List<Token> tokens)
        {
            for (int i = 0; i < tokens.Count; ++i)
                if (tokens[i].type == TokenType.IMPORT)
                    return i;
            return -1;
        }
    }
}