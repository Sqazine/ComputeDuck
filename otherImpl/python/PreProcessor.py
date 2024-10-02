from Token import *
from Lexer import *
from Utils import *
from operator import attrgetter
from Config import gConfig


class TokenBlockTable:
    refCount: int = 0
    filePath: str = None
    importedFilePaths: list[str] = []
    tokens: list[Token] = []


class PreProcessor:
    __lexer: Lexer = Lexer()

    def pre_process(self, src: str) -> list[Token]:
        tokens = self.__lexer.generate_tokens(src)

        tables: list[TokenBlockTable] = []

        loc = self.__search_import_token(tokens)

        if loc == -1:
            tokens.append(Token(TokenType.END, "END", 0, "RootFile"))
            return tokens

        tables.append(self.__find_block_table(tokens))

        for t in tables:
            for path in t.importedFilePaths:
                absPath = gConfig.to_full_path(path)
                toks = self.__lexer.generate_tokens(read_file(absPath), path)

                alreadyExists = False
                for j in range(0, len(tables)):
                    if tables[j].filePath == path:
                        tables[j].refCount += 1
                        alreadyExists = True
                        break

                if alreadyExists != True:
                    blockTable = self.__find_block_table(toks)
                    blockTable.filePath = path
                    blockTable.refCount += 1
                    tables.append(blockTable)

        tables.sort(key=attrgetter('refCount'), reverse=True)

        result: list[Token] = []

        for t in tables:
            for i in range(0, len(t.tokens)):
                result.append(t.tokens[i])

        result.append(Token(TokenType.END, "END", 0, "RootFile"))
        return result

    def __find_block_table(self, tokens: list[Token]) -> TokenBlockTable:
        loc = self.__search_import_token(tokens)
        if loc == -1:
            result = TokenBlockTable()
            result.tokens = tokens
            return result

        importedFilePaths: list[str] = []

        while loc != -1:
            if tokens[loc+1].type != TokenType.LPAREN:
                error("[line "+str(tokens[loc+1].line) + "]:Expect '(' after import keyword.")

            if tokens[loc+2].type != TokenType.STRING:
                error("[line "+str(tokens[loc+2].line) + "]:Expect file path after import stmt's '('.")

            if tokens[loc+3].type != TokenType.RPAREN:
                error("[line "+str(tokens[loc+3].line) + "]:Expect ')' after import stmt's file path.")

            if tokens[loc+4].type != TokenType.SEMICOLON:
                error("[line "+str(tokens[loc+4].line) + "]:Expect ';' after the end of import stmt.")

            importedFilePaths.append(tokens[loc+2].literal)
            del tokens[loc:loc+5]
            loc = self.__search_import_token(tokens)

        result = TokenBlockTable()
        result.importedFilePaths = importedFilePaths
        result.tokens = tokens
        return result

    def __search_import_token(self, tokens: list[Token]) -> int:
        for i in range(0, len(tokens)):
            if tokens[i].type == TokenType.IMPORT:
                return i
        return -1
