#pragma once
#include <vector>
#include <map>
#include <algorithm>
#include "Token.h"
#include "Lexer.h"
#include "Config.h"
#include "Utils.h"

struct TokenBlockTable
{
    int32_t refCount = 0;
    std::string filePath;
    std::vector<std::string> importedFilePaths;
    std::vector<Token> tokens;
};

class COMPUTE_DUCK_API PreProcessor
{
public:
    PreProcessor() {}
    ~PreProcessor() {}

    std::vector<Token> PreProcess(std::vector<Token> tokens)
    {
        std::vector<TokenBlockTable> tables;

        auto loc = SearchImportToken(tokens);
        if (loc == -1)
            return tokens;

        //root token block,refCount=0,filePath=""
        tables.emplace_back(FindBlockTable(tokens));

        for (int32_t i = 0; i < tables.size(); ++i)
        {
            for (const auto &path : tables[i].importedFilePaths)
            {
                auto toks = m_Lexer.GenerateTokens(ReadFile(path));

                bool alreadyExists = false;
                for (int32_t j = 0; j < tables.size(); ++j)
                {
                    if (tables[j].filePath == path)
                    {
                        tables[j].refCount++;
                        alreadyExists = true;
                        break;
                    }
                }

                if (!alreadyExists)
                {
                    auto blockTable = FindBlockTable(toks);
                    blockTable.filePath = path;
                    blockTable.refCount++;
                    tables.emplace_back(blockTable);
                }
            }
        }

        std::sort(tables.begin(), tables.end(), [](const TokenBlockTable &left, const TokenBlockTable &right)
                  { return left.refCount > right.refCount; });

        std::vector<Token> result;
        for (const auto &t : tables)
            result.insert(result.end(), t.tokens.begin(), t.tokens.end());

        result.emplace_back(TokenType::END, "END", 0);
        return result;
    }

private:
    Lexer m_Lexer;

    TokenBlockTable FindBlockTable(std::vector<Token> tokens)
    {
        auto loc = SearchImportToken(tokens);
        if (loc == -1)
        {
            TokenBlockTable result;
            result.tokens = tokens;
            return result;
        }

        //import("path");
        std::vector<std::string> importFilePaths;
        while (loc != -1)
        {
            if (tokens[loc + 1].type != TokenType::LPAREN)
                Assert("[line " + std::to_string(tokens[loc + 1].line) + "]:" + "Expect '(' after import keyword.");

            if (tokens[loc + 2].type != TokenType::STRING)
                Assert("[line " + std::to_string(tokens[loc + 2].line) + "]:" + "Expect file path after import stmt's '('.");

            if (tokens[loc + 3].type != TokenType::RPAREN)
                Assert("[line " + std::to_string(tokens[loc + 3].line) + "]:" + "Expect ')' after import stmt's file path.");

            if (tokens[loc + 4].type != TokenType::SEMICOLON)
                Assert("[line " + std::to_string(tokens[loc + 4].line) + "]:" + "Expect ';' at the end of import stmt.");

            importFilePaths.emplace_back(tokens[loc + 2].literal);
            tokens.erase(tokens.begin() + loc, tokens.begin() + loc + 5);
            loc = SearchImportToken(tokens);
        }

        TokenBlockTable result;
        result.importedFilePaths = importFilePaths;
        result.tokens = tokens;
        return result;
    }

    int32_t SearchImportToken(const std::vector<Token> &tokens)
    {
        for (int32_t i = 0; i < tokens.size(); ++i)
            if (tokens[i].type == TokenType::IMPORT)
                return i;
        return -1;
    }
};