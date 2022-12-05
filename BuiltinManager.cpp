#include "BuiltinManager.h"
#include <ctime>
#include <SDL2/SDL.h>
std::vector<BuiltinFunctionObject *> BuiltinManager::m_BuiltinFunctions;
std::vector<std::string> BuiltinManager::m_BuiltinFunctionNames;

std::vector<BuiltinVariableObject *> BuiltinManager::m_BuiltinVariables;
std::vector<std::string> BuiltinManager::m_BuiltinVariableNames;

void BuiltinManager::Init()
{
    RegisterFunction("print", [&](Value *args, uint8_t argCount, Value &result) -> bool
                     {
                         if (argCount == 0)
                             return false;

                         std::cout << args[0].Stringify();
                         return false;
                     });

    RegisterFunction("println", [&](Value *args, uint8_t argCount, Value &result) -> bool
                     {
                         if (argCount == 0)
                             return false;

                         std::cout << args[0].Stringify() << std::endl;
                         return false;
                     });

    RegisterFunction("sizeof", [&](Value *args, uint8_t argCount, Value &result) -> bool
                     {
                         if (argCount == 0 || argCount > 1)
                             Assert("[Native function 'sizeof']:Expect a argument.");

                         if (IS_ARRAY_VALUE(args[0]))
                             result = Value((double)TO_ARRAY_VALUE(args[0])->elements.size());
                         else if (IS_STR_VALUE(args[0]))
                             result = Value((double)TO_STR_VALUE(args[0])->value.size());
                         else
                             Assert("[Native function 'sizeof']:Expect a array or string argument.");
                         return true;
                     });

    RegisterFunction("insert", [&](Value *args, uint8_t argCount, Value &result) -> bool
                     {
                         if (argCount == 0 || argCount != 3)
                             Assert("[Native function 'insert']:Expect 3 arguments,the arg0 must be array,table or string object.The arg1 is the index object.The arg2 is the value object.");

                         if (IS_ARRAY_VALUE(args[0]))
                         {
                             ArrayObject *array = TO_ARRAY_VALUE(args[0]);
                             if (!IS_NUM_VALUE(args[1]))
                                 Assert("[Native function 'insert']:Arg1 must be integer type while insert to a array");

                             size_t iIndex = (size_t)TO_NUM_VALUE(args[1]);

                             if (iIndex < 0 || iIndex >= array->elements.size())
                                 Assert("[Native function 'insert']:Index out of array's range");

                             array->elements.insert(array->elements.begin() + iIndex, 1, args[2]);
                         }
                         else if (IS_STR_VALUE(args[0]))
                         {
                             StrObject *string = TO_STR_VALUE(args[0]);
                             if (!IS_NUM_VALUE(args[1]))
                                 Assert("[Native function 'insert']:Arg1 must be integer type while insert to a array");

                             size_t iIndex = (size_t)TO_NUM_VALUE(args[1]);

                             if (iIndex < 0 || iIndex >= string->value.size())
                                 Assert("[Native function 'insert']:Index out of array's range");

                             string->value.insert(iIndex, args[2].Stringify());
                         }
                         else
                             Assert("[Native function 'insert']:Expect a array,table ot string argument.");
                         return false;
                     });

    RegisterFunction("erase", [&](Value *args, uint8_t argCount, Value &result) -> bool
                     {
                         if (argCount == 0 || argCount != 2)
                             Assert("[Native function 'erase']:Expect 2 arguments,the arg0 must be array,table or string object.The arg1 is the corresponding index object.");

                         if (IS_ARRAY_VALUE(args[0]))
                         {
                             ArrayObject *array = TO_ARRAY_VALUE(args[0]);
                             if (!IS_NUM_VALUE(args[1]))
                                 Assert("[Native function 'erase']:Arg1 must be integer type while insert to a array");

                             size_t iIndex = (size_t)TO_NUM_VALUE(args[1]);

                             if (iIndex < 0 || iIndex >= array->elements.size())
                                 Assert("[Native function 'erase']:Index out of array's range");

                             array->elements.erase(array->elements.begin() + iIndex);
                         }
                         else if (IS_STR_VALUE(args[0]))
                         {
                             StrObject *string = TO_STR_VALUE(args[0]);
                             if (!IS_NUM_VALUE(args[1]))
                                 Assert("[Native function 'erase']:Arg1 must be integer type while insert to a array");

                             size_t iIndex = (size_t)TO_NUM_VALUE(args[1]);

                             if (iIndex < 0 || iIndex >= string->value.size())
                                 Assert("[Native function 'erase']:Index out of array's range");

                             string->value.erase(string->value.begin() + iIndex);
                         }
                         else
                             Assert("[Native function 'erase']:Expect a array,table ot string argument.");
                         return false;
                     });

    RegisterFunction("clock", [&](Value *args, uint8_t argCount, Value &result) -> bool
                     {
                         result = Value((double)clock() / CLOCKS_PER_SEC);
                         return true;
                     });

    BuiltinManager::RegisterVariable("SDL_WINDOWPOS_CENTERED", Value((double)SDL_WINDOWPOS_CENTERED));
    BuiltinManager::RegisterVariable("SDL_QUIT", Value((double)SDL_QUIT));

    BuiltinManager::RegisterFunction("SDL_Init", [&](Value *args, uint8_t argCount, Value &result) -> bool
                                     {
                                         auto ret = SDL_Init(SDL_INIT_EVERYTHING);
                                         result = Value((float)ret);
                                         return true;
                                     });

    BuiltinManager::RegisterFunction("SDL_Quit", [&](Value *args, uint8_t argCount, Value &result) -> bool
                                     {
                                         SDL_Quit();
                                         return false;
                                     });

    BuiltinManager::RegisterFunction("SDL_CreateWindow", [&](Value *args, uint8_t argCount, Value &result) -> bool
                                     {
                                         BuiltinDataObject *builtinData = new BuiltinDataObject();
                                         auto window = SDL_CreateWindow(TO_STR_VALUE(args[0])->value.c_str(), (int)TO_NUM_VALUE(TO_BUILTIN_VARIABLE_VALUE(args[1])->value), (int)TO_NUM_VALUE(TO_BUILTIN_VARIABLE_VALUE(args[2])->value), TO_NUM_VALUE(args[3]), TO_NUM_VALUE(args[4]), SDL_WINDOW_ALLOW_HIGHDPI);
                                         builtinData->nativeData = (void *)window;
                                         builtinData->destroyFunc = [](void *nativeData)
                                         {
                                             SDL_DestroyWindow((SDL_Window *)nativeData);
                                         };
                                         result = builtinData;
                                         return true;
                                     });

    BuiltinManager::RegisterFunction("SDL_PollEvent", [&](Value *args, uint8_t argCount, Value &result) -> bool
                                     {
                                         SDL_Event *event = new SDL_Event();
                                         SDL_PollEvent(event);
                                         BuiltinDataObject *builtinData = new BuiltinDataObject();
                                         builtinData->nativeData = event;
                                         builtinData->destroyFunc = [](void *nativeData)
                                         {
                                             delete (SDL_Event *)nativeData;
                                         };
                                         result = Value(builtinData);
                                         return true;
                                     });

    BuiltinManager::RegisterFunction("SDL_GetEventType", [&](Value *args, uint8_t argCount, Value &result) -> bool
                                     {
                                         if (!IS_BUILTIN_DATA_VALUE(args[0]))
                                             Assert("Not a valid builtin data.");
                                         auto builtinData = TO_BUILTIN_DATA_VALUE(args[0]);
                                         if (reinterpret_cast<SDL_Event *>(builtinData->nativeData) == nullptr)
                                             Assert("Not a valid SDL_Event object.");
                                         SDL_Event *event = ((SDL_Event *)builtinData->nativeData);
                                         result = (double)((SDL_Event *)builtinData->nativeData)->type;
                                         return true;
                                     });

    BuiltinManager::RegisterFunction("SDL_CreateRenderer", [&](Value *args, uint8_t argCount, Value &result) -> bool
                                     {
                                         if (!IS_BUILTIN_DATA_VALUE(args[0]))
                                             Assert("Not a valid builtin data.");
                                         auto builtinData = TO_BUILTIN_DATA_VALUE(args[0]);
                                         if (reinterpret_cast<SDL_Window *>(builtinData->nativeData) == nullptr)
                                             Assert("Not a valid SDL_Window object.");
                                         SDL_Window *window = ((SDL_Window *)builtinData->nativeData);
                                         SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
                                         BuiltinDataObject *resultBuiltinData = new BuiltinDataObject();
                                         resultBuiltinData->nativeData = renderer;
                                         resultBuiltinData->destroyFunc = [](void *nativeData)
                                         {
                                             SDL_DestroyRenderer((SDL_Renderer *)nativeData);
                                         };
                                         result = Value(resultBuiltinData);
                                         return true;
                                     });

    BuiltinManager::RegisterFunction("SDL_LoadBMP", [&](Value *args, uint8_t argCount, Value &result) -> bool
                                     {
                                         if (!IS_STR_VALUE(args[0]))
                                             Assert("Not a valid str value.");

                                         SDL_Surface *surface = SDL_LoadBMP(TO_STR_VALUE(args[0])->value.c_str());

                                         BuiltinDataObject *resultBuiltinData = new BuiltinDataObject();
                                         resultBuiltinData->nativeData = surface;
                                         resultBuiltinData->destroyFunc = [](void *nativeData)
                                         {
                                             SDL_FreeSurface((SDL_Surface *)nativeData);
                                         };
                                         result = Value(resultBuiltinData);
                                         return true;
                                     });

    BuiltinManager::RegisterFunction("SDL_CreateTextureFromSurface", [&](Value *args, uint8_t argCount, Value &result) -> bool
                                     {
                                         if (!IS_BUILTIN_DATA_VALUE(args[0]) || !IS_BUILTIN_DATA_VALUE(args[1]))
                                             Assert("Not a valid builtin value of SDL_CreateTextureFromSurface(args[0] or args[1]).");

                                         auto renderer = (SDL_Renderer *)(TO_BUILTIN_DATA_VALUE(args[0])->nativeData);
                                         auto surface = (SDL_Surface *)(TO_BUILTIN_DATA_VALUE(args[1])->nativeData);

                                         SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

                                         BuiltinDataObject *resultBuiltinData = new BuiltinDataObject();
                                         resultBuiltinData->nativeData = texture;
                                         resultBuiltinData->destroyFunc = [](void *nativeData)
                                         {
                                             SDL_DestroyTexture((SDL_Texture *)nativeData);
                                         };
                                         result = Value(resultBuiltinData);
                                         return true;
                                     });

    BuiltinManager::RegisterFunction("SDL_RenderClear", [&](Value *args, uint8_t argCount, Value &result) -> bool
                                     {
                                         if (!IS_BUILTIN_DATA_VALUE(args[0]))
                                             Assert("Not a valid builtin value of SDL_RenderClear(args[0]).");

                                         auto renderer = (SDL_Renderer *)(TO_BUILTIN_DATA_VALUE(args[0])->nativeData);
                                         SDL_RenderClear(renderer);
                                         return false;
                                     });

    BuiltinManager::RegisterFunction("SDL_RenderCopy", [&](Value *args, uint8_t argCount, Value &result) -> bool
                                     {
                                         if (!IS_BUILTIN_DATA_VALUE(args[0]) || !IS_BUILTIN_DATA_VALUE(args[1]))
                                             Assert("Not a valid builtin value of SDL_RenderCopy(args[0] or args[1]).");

                                         auto renderer = (SDL_Renderer *)(TO_BUILTIN_DATA_VALUE(args[0])->nativeData);
                                         auto texture = (SDL_Texture *)(TO_BUILTIN_DATA_VALUE(args[1])->nativeData);

                                         auto ret = SDL_RenderCopy(renderer, texture, nullptr, nullptr);

                                         result = Value((double)ret);
                                         return true;
                                     });

    BuiltinManager::RegisterFunction("SDL_RenderPresent", [&](Value *args, uint8_t argCount, Value &result) -> bool
                                     {
                                         if (!IS_BUILTIN_DATA_VALUE(args[0]))
                                             Assert("Not a valid builtin value of SDL_RenderPresent(args[0]).");

                                         auto renderer = (SDL_Renderer *)(TO_BUILTIN_DATA_VALUE(args[0])->nativeData);
                                         SDL_RenderPresent(renderer);
                                         return false;
                                     });
}
void BuiltinManager::Release()
{
    std::vector<BuiltinFunctionObject *>().swap(m_BuiltinFunctions);
    std::vector<std::string>().swap(m_BuiltinFunctionNames);

    std::vector<BuiltinVariableObject *>().swap(m_BuiltinVariables);
    std::vector<std::string>().swap(m_BuiltinVariableNames);
}

void BuiltinManager::RegisterFunction(std::string_view name, const BuiltinFn &fn)
{
    m_BuiltinFunctions.emplace_back(new BuiltinFunctionObject(name, fn));
    m_BuiltinFunctionNames.emplace_back(name);
}

void BuiltinManager::RegisterVariable(std::string_view name, const Value &value)
{
    m_BuiltinVariables.emplace_back(new BuiltinVariableObject(name, value));
    m_BuiltinVariableNames.emplace_back(name);
}