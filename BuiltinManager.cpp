#include "BuiltinManager.h"
#include <ctime>
#include <SDL2/SDL.h>
std::vector<BuiltinFunctionObject *> BuiltinManager::m_BuiltinFunctions;
std::vector<std::string> BuiltinManager::m_BuiltinFunctionNames;

std::vector<BuiltinVariableObject *> BuiltinManager::m_BuiltinVariables;
std::vector<std::string> BuiltinManager::m_BuiltinVariableNames;

void BuiltinManager::Init()
{
    RegisterFunction("print", [&](const std::vector<Value> &args, Value &result) -> bool
                     {
                         if (args.empty())
                             return false;

                         std::cout << args[0].Stringify();
                         return false;
                     });

    RegisterFunction("println", [&](const std::vector<Value> &args, Value &result) -> bool
                     {
                         if (args.empty())
                             return false;

                         std::cout << args[0].Stringify() << std::endl;
                         return false;
                     });

    RegisterFunction("sizeof", [&](const std::vector<Value> &args, Value &result) -> bool
                     {
                         if (args.empty() || args.size() > 1)
                             Assert("[Native function 'sizeof']:Expect a argument.");

                         if (IS_ARRAY_VALUE(args[0]))
                             result = Value((double)TO_ARRAY_VALUE(args[0])->elements.size());
                         else if (IS_STR_VALUE(args[0]))
                             result = Value((double)TO_STR_VALUE(args[0])->value.size());
                         else
                             Assert("[Native function 'sizeof']:Expect a array or string argument.");
                         return true;
                     });

    RegisterFunction("insert", [&](const std::vector<Value> &args, Value &result) -> bool
                     {
                         if (args.empty() || args.size() != 3)
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

    RegisterFunction("erase", [&](const std::vector<Value> &args, Value &result) -> bool
                     {
                         if (args.empty() || args.size() != 2)
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

    RegisterFunction("clock", [&](const std::vector<Value> &args, Value &result) -> bool
                     {
                         result = Value((double)clock() / CLOCKS_PER_SEC);
                         return true;
                     });

    BuiltinManager::RegisterVariable("SDL_WINDOWPOS_CENTERED", Value((double)SDL_WINDOWPOS_CENTERED));
    BuiltinManager::RegisterVariable("SDL_QUIT", Value((double)SDL_QUIT));

    BuiltinManager::RegisterFunction("SDL_Init", [&](const std::vector<Value> &args, Value &result) -> bool
                                     {
                                         auto ret = SDL_Init(SDL_INIT_EVERYTHING);
                                         result = Value((float)ret);
                                         return true;
                                     });

    BuiltinManager::RegisterFunction("SDL_Quit", [&](const std::vector<Value> &args, Value &result) -> bool
                                     {
                                         SDL_Quit();
                                         return false;
                                     });

    BuiltinManager::RegisterFunction("SDL_CreateWindow", [&](const std::vector<Value> &args, Value &result) -> bool
                                     {
                                         BuiltinDataObject *builtinData = new BuiltinDataObject();
                                         auto window = SDL_CreateWindow(TO_STR_VALUE(args[0])->value.c_str(), (int)TO_NUM_VALUE(TO_BUILTIN_VARIABLE_VALUE(args[1])->value), (int)TO_NUM_VALUE(TO_BUILTIN_VARIABLE_VALUE(args[2])->value), TO_NUM_VALUE(args[3]), TO_NUM_VALUE(args[4]), SDL_WINDOW_ALLOW_HIGHDPI);
                                         builtinData->nativeData = (void *)window;
                                         result = builtinData;
                                         return true;
                                     });

    BuiltinManager::RegisterFunction("SDL_DestroyWindow", [&](const std::vector<Value> &args, Value &result) -> bool
                                     {
                                         if (!IS_BUILTIN_DATA_VALUE(args[0]))
                                             Assert("Not a valid builtin data.");
                                         auto builtinData = TO_BUILTIN_DATA_VALUE(args[0]);
                                         if (reinterpret_cast<SDL_Window *>(builtinData->nativeData) == nullptr)
                                             Assert("Not a valid SDL_Window object.");
                                         SDL_DestroyWindow((SDL_Window *)builtinData->nativeData);
                                         return false;
                                     });

    BuiltinManager::RegisterFunction("SDL_PollEvent", [&](const std::vector<Value> &args, Value &result) -> bool
                                     {
                                         SDL_Event *event = new SDL_Event();
                                         auto retV = SDL_PollEvent(event);
                                         if (retV == 0)
                                             return true;
                                         BuiltinDataObject *builtinData = new BuiltinDataObject();
                                         builtinData->nativeData = event;
                                         result = Value(builtinData);
                                         return true;
                                     });

    BuiltinManager::RegisterFunction("SDL_GetEventType", [&](const std::vector<Value> &args, Value &result) -> bool
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

    BuiltinManager::RegisterFunction("SDL_CreateRenderer", [&](const std::vector<Value> &args, Value &result) -> bool
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
                                         result = Value(resultBuiltinData);
                                         return true;
                                     });

    BuiltinManager::RegisterFunction("SDL_DestroyRenderer", [&](const std::vector<Value> &args, Value &result) -> bool
                                     {
                                         if (!IS_BUILTIN_DATA_VALUE(args[0]))
                                             Assert("Not a valid builtin value of SDL_DestroyRenderer(args[0]).");

                                         auto renderer = (SDL_Renderer *)(TO_BUILTIN_DATA_VALUE(args[0])->nativeData);
                                         SDL_DestroyRenderer(renderer);
                                         return false;
                                     });

    BuiltinManager::RegisterFunction("SDL_LoadBMP", [&](const std::vector<Value> &args, Value &result) -> bool
                                     {
                                         if (!IS_STR_VALUE(args[0]))
                                             Assert("Not a valid str value.");

                                         SDL_Surface *surface = SDL_LoadBMP(TO_STR_VALUE(args[0])->value.c_str());

                                         BuiltinDataObject *resultBuiltinData = new BuiltinDataObject();
                                         resultBuiltinData->nativeData = surface;
                                         result = Value(resultBuiltinData);
                                         return true;
                                     });

    BuiltinManager::RegisterFunction("SDL_FreeSurface", [&](const std::vector<Value> &args, Value &result) -> bool
                                     {
                                         if (!IS_BUILTIN_DATA_VALUE(args[0]))
                                             Assert("Not a valid builtin value of SDL_FreeSurface(args[0]).");

                                         auto surface = (SDL_Surface *)(TO_BUILTIN_DATA_VALUE(args[0])->nativeData);
                                         SDL_FreeSurface(surface);

                                         return false;
                                     });

    BuiltinManager::RegisterFunction("SDL_CreateTextureFromSurface", [&](const std::vector<Value> &args, Value &result) -> bool
                                     {
                                         if (!IS_BUILTIN_DATA_VALUE(args[0]) || !IS_BUILTIN_DATA_VALUE(args[1]))
                                             Assert("Not a valid builtin value of SDL_CreateTextureFromSurface(args[0] or args[1]).");

                                         auto renderer = (SDL_Renderer *)(TO_BUILTIN_DATA_VALUE(args[0])->nativeData);
                                         auto surface = (SDL_Surface *)(TO_BUILTIN_DATA_VALUE(args[1])->nativeData);

                                         SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

                                         BuiltinDataObject *resultBuiltinData = new BuiltinDataObject();
                                         resultBuiltinData->nativeData = texture;
                                         result = Value(resultBuiltinData);
                                         return true;
                                     });

    BuiltinManager::RegisterFunction("SDL_DestroyTexture", [&](const std::vector<Value> &args, Value &result) -> bool
                                     {
                                         if (!IS_BUILTIN_DATA_VALUE(args[0]))
                                             Assert("Not a valid builtin value of SDL_CreateTextureFromSurface(args[0]).");

                                         auto texture = (SDL_Texture *)(TO_BUILTIN_DATA_VALUE(args[0])->nativeData);
                                         SDL_DestroyTexture(texture);
                                         return false;
                                     });

    BuiltinManager::RegisterFunction("SDL_RenderClear", [&](const std::vector<Value> &args, Value &result) -> bool
                                     {
                                         if (!IS_BUILTIN_DATA_VALUE(args[0]))
                                             Assert("Not a valid builtin value of SDL_RenderClear(args[0]).");

                                         auto renderer = (SDL_Renderer *)(TO_BUILTIN_DATA_VALUE(args[0])->nativeData);
                                         SDL_RenderClear(renderer);
                                         return false;
                                     });

    BuiltinManager::RegisterFunction("SDL_RenderCopy", [&](const std::vector<Value> &args, Value &result) -> bool
                                     {
                                         if (!IS_BUILTIN_DATA_VALUE(args[0]) || !IS_BUILTIN_DATA_VALUE(args[1]))
                                             Assert("Not a valid builtin value of SDL_RenderCopy(args[0] or args[1]).");

                                         auto renderer = (SDL_Renderer *)(TO_BUILTIN_DATA_VALUE(args[0])->nativeData);
                                         auto texture = (SDL_Texture *)(TO_BUILTIN_DATA_VALUE(args[1])->nativeData);

                                         auto ret = SDL_RenderCopy(renderer, texture, nullptr, nullptr);

                                         result = Value((double)ret);
                                         return true;
                                     });

    BuiltinManager::RegisterFunction("SDL_RenderPresent", [&](const std::vector<Value> &args, Value &result) -> bool
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