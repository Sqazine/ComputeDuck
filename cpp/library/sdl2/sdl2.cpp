#include "sdl2.h"
#include <vector>
#include <SDL2/SDL.h>
#include "../../BuiltinManager.h"
#include "../../Value.h"
void RegisterBuiltins()
{

    BuiltinManager::GetInstance()->RegisterVariable("SDL_WINDOWPOS_CENTERED", Value((double)SDL_WINDOWPOS_CENTERED));
    BuiltinManager::GetInstance()->RegisterVariable("SDL_QUIT", Value((double)SDL_QUIT));

    BuiltinManager::GetInstance()->RegisterFunction("SDL_Init", [&](Value *args, uint8_t argCount, Value &result) -> bool
                                                    {
                                                        auto ret = SDL_Init(SDL_INIT_EVERYTHING);
                                                        result = Value((float)ret);
                                                        return true;
                                                    });

    BuiltinManager::GetInstance()->RegisterFunction("SDL_Quit", [&](Value *args, uint8_t argCount, Value &result) -> bool
                                                    {
                                                        SDL_Quit();
                                                        return false;
                                                    });

    BuiltinManager::GetInstance()->RegisterFunction("SDL_CreateWindow", [&](Value *args, uint8_t argCount, Value &result) -> bool
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

    BuiltinManager::GetInstance()->RegisterFunction("SDL_PollEvent", [&](Value *args, uint8_t argCount, Value &result) -> bool
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

    BuiltinManager::GetInstance()->RegisterFunction("SDL_GetEventType", [&](Value *args, uint8_t argCount, Value &result) -> bool
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

    BuiltinManager::GetInstance()->RegisterFunction("SDL_CreateRenderer", [&](Value *args, uint8_t argCount, Value &result) -> bool
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

    BuiltinManager::GetInstance()->RegisterFunction("SDL_LoadBMP", [&](Value *args, uint8_t argCount, Value &result) -> bool
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

    BuiltinManager::GetInstance()->RegisterFunction("SDL_CreateTextureFromSurface", [&](Value *args, uint8_t argCount, Value &result) -> bool
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

    BuiltinManager::GetInstance()->RegisterFunction("SDL_RenderClear", [&](Value *args, uint8_t argCount, Value &result) -> bool
                                                    {
                                                        if (!IS_BUILTIN_DATA_VALUE(args[0]))
                                                            Assert("Not a valid builtin value of SDL_RenderClear(args[0]).");

                                                        auto renderer = (SDL_Renderer *)(TO_BUILTIN_DATA_VALUE(args[0])->nativeData);
                                                        SDL_RenderClear(renderer);
                                                        return false;
                                                    });

    BuiltinManager::GetInstance()->RegisterFunction("SDL_RenderCopy", [&](Value *args, uint8_t argCount, Value &result) -> bool
                                                    {
                                                        if (!IS_BUILTIN_DATA_VALUE(args[0]) || !IS_BUILTIN_DATA_VALUE(args[1]))
                                                            Assert("Not a valid builtin value of SDL_RenderCopy(args[0] or args[1]).");

                                                        auto renderer = (SDL_Renderer *)(TO_BUILTIN_DATA_VALUE(args[0])->nativeData);
                                                        auto texture = (SDL_Texture *)(TO_BUILTIN_DATA_VALUE(args[1])->nativeData);

                                                        auto ret = SDL_RenderCopy(renderer, texture, nullptr, nullptr);

                                                        result = Value((double)ret);
                                                        return true;
                                                    });

    BuiltinManager::GetInstance()->RegisterFunction("SDL_RenderPresent", [&](Value *args, uint8_t argCount, Value &result) -> bool
                                                    {
                                                        if (!IS_BUILTIN_DATA_VALUE(args[0]))
                                                            Assert("Not a valid builtin value of SDL_RenderPresent(args[0]).");

                                                        auto renderer = (SDL_Renderer *)(TO_BUILTIN_DATA_VALUE(args[0])->nativeData);
                                                        SDL_RenderPresent(renderer);
                                                        return false;
                                                    });
}