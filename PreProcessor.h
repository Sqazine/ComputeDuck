#pragma once
#include <vector>
#include <map>
#include <algorithm>
#include "Token.h"
#include "Lexer.h"
#include "Utils.h"
#include "Config.h"

class COMPUTEDUCK_API PreProcessor
{
public:
    PreProcessor();
    ~PreProcessor();

    std::vector<Token> PreProcess(std::string_view src);

private:
    struct TokenBlockTable
    {
        int32_t refCount = 0;
        std::string filePath;
        std::vector<std::string> importedFilePaths;
        std::vector<Token> tokens;
    };

    TokenBlockTable FindBlockTable(std::vector<Token> tokens);
    int32_t SearchImportToken(const std::vector<Token> &tokens);

    Lexer m_Lexer;
};