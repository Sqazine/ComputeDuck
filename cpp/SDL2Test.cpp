#include <SDL2/SDL.h>
#include <string>
#include <string_view>
#include "Lexer.h"
#include "Parser.h"
#include "Compiler.h"
#include "VM.h"
#include "BuiltinFunctionManager.h"

#undef main
int main(int argc, char **argv)
{
    BuiltinFunctionManager::Init();

    BuiltinFunctionManager::Register("SDL_Init", [&](const std::vector<Value> &args, Value &result) -> bool
                                     {
                                         auto ret = SDL_Init(SDL_INIT_EVERYTHING);

                                         result = Value((float)ret);

                                         return true;
                                     });

    BuiltinFunctionManager::Register("SDL_CreateWindow", [&](const std::vector<Value> &args, Value &result) -> bool
                                     {
                                         BuiltinDataObject *builtinData = new BuiltinDataObject();

                                         auto window = SDL_CreateWindow(TO_STR_VALUE(args[0])->value.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, TO_NUM_VALUE(args[1]), TO_NUM_VALUE(args[2]), SDL_WINDOW_ALLOW_HIGHDPI);

                                         builtinData->nativeData = (void *)window;

                                         builtinData->destroyFunc = [](void *nativeData)
                                         {
                                             SDL_DestroyWindow((SDL_Window *)nativeData);
                                         };

                                         result = builtinData;

                                         return true;
                                     });

    std::string content = {"var ok=SDL_Init();\n"
                           "if(ok<0)\n"
                           "println(\"Failed to init sdl2!\");\n"
                           "var window=SDL_CreateWindow(\"First Window\",800,600);"
                           "while(true)\n"
                           "{\n"
                           "}\n"};

    Lexer lexer;
    Parser parser;
    Compiler compiler;
    VM vm;

    auto tokens = lexer.GenerateTokens(content);

    for (const auto &token : tokens)
        std::cout << token << std::endl;

    auto stmt = parser.Parse(tokens);

    auto stmts = parser.Parse(tokens);

    for (const auto &stmt : stmts)
        std::cout << stmt->Stringify() << std::endl;

    auto chunk = compiler.Compile(stmts);

    chunk.Stringify();

    vm.Run(chunk);

    BuiltinFunctionManager::Release();

    return 0;
}
