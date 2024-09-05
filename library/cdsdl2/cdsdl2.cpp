#include "cdsdl2.h"
#include <vector>
#include <SDL.h>
#include "BuiltinManager.h"
#include "Value.h"
#include "Object.h"
#include "Config.h"
#include "Allocator.h"

extern "C" COMPUTE_DUCK_API bool BUILTIN_FN(SDL_Init)(Value *args, uint8_t argCount, Value &result)
{
    result = SDL_Init(TO_NUM_VALUE(args[0]));
    return true;
}

extern "C" COMPUTE_DUCK_API bool BUILTIN_FN(SDL_Quit)(Value *args, uint8_t argCount, Value &result)
{
    SDL_Quit();
    return false;
}

extern "C" COMPUTE_DUCK_API bool BUILTIN_FN(SDL_CreateWindow)(Value *args, uint8_t argCount, Value &result)
{
    auto name = TO_STR_VALUE(args[0])->value;
    auto posX = (int32_t)TO_NUM_VALUE(TO_BUILTIN_VALUE(args[1])->Get<Value>());
    auto posY = (int32_t)TO_NUM_VALUE(TO_BUILTIN_VALUE(args[2])->Get<Value>());
    int32_t width = (int32_t)TO_NUM_VALUE(args[3]);
    int32_t height = (int32_t)TO_NUM_VALUE(args[4]);
    uint32_t flags = (uint32_t)TO_NUM_VALUE(args[5]);

    auto window = SDL_CreateWindow(name, posX, posY, width, height, flags);

    BuiltinObject *builtinData = Allocator::GetInstance()->CreateObject<BuiltinObject>(window, [](void *nativeData)
        { SDL_DestroyWindow((SDL_Window *)nativeData); });

    result = builtinData;
    return true;
}

extern "C" COMPUTE_DUCK_API bool BUILTIN_FN(SDL_PollEvent)(Value *args, uint8_t argCount, Value &result)
{
    SDL_Event *event = new SDL_Event();
    SDL_PollEvent(event);
    BuiltinObject *builtinData = Allocator::GetInstance()->CreateObject<BuiltinObject>(event, [](void *nativeData)
        {
            delete (SDL_Event *)nativeData;
        });

    result = Value(builtinData);
    return true;
}

extern "C" COMPUTE_DUCK_API bool BUILTIN_FN(SDL_GetEventType)(Value *args, uint8_t argCount, Value &result)
{
    if (!IS_BUILTIN_VALUE(args[0]))
        ASSERT("Invalid builtin data.");
    auto builtinData = TO_BUILTIN_VALUE(args[0]);
    if (builtinData->Get<NativeData>().IsSameTypeAs<SDL_Event>())
        ASSERT("Invalid SDL_Event object.");
    SDL_Event *event = builtinData->Get<NativeData>().As<SDL_Event>();
    result = (double)(event->type);
    return true;
}

extern "C" COMPUTE_DUCK_API bool BUILTIN_FN(SDL_CreateRenderer)(Value *args, uint8_t argCount, Value &result)
{
    if (!IS_BUILTIN_VALUE(args[0]))
        ASSERT("Invalid builtin data.");
    auto builtinData = TO_BUILTIN_VALUE(args[0]);
    if (builtinData->Get<NativeData>().IsSameTypeAs<SDL_Window>())
        ASSERT("Invalid SDL_Window object.");
    SDL_Window *window = builtinData->Get<NativeData>().As<SDL_Window>();
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    BuiltinObject *resultBuiltin = Allocator::GetInstance()->CreateObject<BuiltinObject>(renderer, [](void *nativeData)
        { SDL_DestroyRenderer((SDL_Renderer *)nativeData); });
    if (renderer)
        result = Value(resultBuiltin);
    return true;
}

extern "C" COMPUTE_DUCK_API bool BUILTIN_FN(SDL_LoadBMP)(Value *args, uint8_t argCount, Value &result)
{
    if (!IS_STR_VALUE(args[0]))
        ASSERT("Invalid str value.");

    auto fullPath = Config::GetInstance()->ToFullPath(TO_STR_VALUE(args[0])->value);

    SDL_Surface *surface = SDL_LoadBMP(fullPath.c_str());

    BuiltinObject *resultBuiltinData = Allocator::GetInstance()->CreateObject<BuiltinObject>(surface, [](void *nativeData)
        { SDL_FreeSurface((SDL_Surface *)nativeData); });

    if (surface)
        result = resultBuiltinData;
    return true;
}

extern "C" COMPUTE_DUCK_API bool BUILTIN_FN(SDL_CreateTextureFromSurface)(Value *args, uint8_t argCount, Value &result)
{
    if (!IS_BUILTIN_VALUE(args[0]) || !IS_BUILTIN_VALUE(args[1]))
        ASSERT("Invalid builtin value of SDL_CreateTextureFromSurface(args[0] or args[1]).");

    auto renderer = TO_BUILTIN_VALUE(args[0])->Get<NativeData>().As<SDL_Renderer>();
    auto surface = TO_BUILTIN_VALUE(args[1])->Get<NativeData>().As<SDL_Surface>();

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    BuiltinObject *resultBuiltin = Allocator::GetInstance()->CreateObject<BuiltinObject>(texture, [](void *nativeData)
        { SDL_DestroyTexture((SDL_Texture *)nativeData); });

    result = resultBuiltin;
    return true;
}

extern "C" COMPUTE_DUCK_API bool BUILTIN_FN(SDL_RenderClear)(Value *args, uint8_t argCount, Value &result)
{
    if (!IS_BUILTIN_VALUE(args[0]))
        ASSERT("Invalid builtin value of SDL_RenderClear(args[0]).");

    auto renderer = TO_BUILTIN_VALUE(args[0])->Get<NativeData>().As<SDL_Renderer>();
    SDL_RenderClear(renderer);
    return false;
}

extern "C" COMPUTE_DUCK_API bool BUILTIN_FN(SDL_RenderCopy)(Value *args, uint8_t argCount, Value &result)
{
    if (!IS_BUILTIN_VALUE(args[0]) || !IS_BUILTIN_VALUE(args[1]))
        ASSERT("Invalid builtin value of SDL_RenderCopy(args[0] or args[1]).");

    auto renderer = TO_BUILTIN_VALUE(args[0])->Get<NativeData>().As<SDL_Renderer>();
    auto texture = TO_BUILTIN_VALUE(args[1])->Get<NativeData>().As<SDL_Texture>();

   result = SDL_RenderCopy(renderer, texture, nullptr, nullptr);

    return true;
}

extern "C" COMPUTE_DUCK_API bool BUILTIN_FN(SDL_RenderPresent)(Value *args, uint8_t argCount, Value &result)
{
    if (!IS_BUILTIN_VALUE(args[0]))
        ASSERT("Invalid builtin value of SDL_RenderPresent(args[0]).");

    auto renderer = TO_BUILTIN_VALUE(args[0])->Get<NativeData>().As<SDL_Renderer>();
    SDL_RenderPresent(renderer);
    return false;
}

extern "C" COMPUTE_DUCK_API bool BUILTIN_FN(SDL_GL_SetAttribute)(Value *args, uint8_t argCount, Value &result)
{
    if (!IS_BUILTIN_VALUE(args[0]))
        ASSERT("Invalid builtin value of SDL_GL_SetAttribute(args[0]).");

    if (!IS_BUILTIN_VALUE(args[1]) && !IS_NUM_VALUE(args[1]))
        ASSERT("Invalid builtin value or num value of SDL_GL_SetAttribute(args[1]).");

    auto flags0 = (int)TO_NUM_VALUE(TO_BUILTIN_VALUE(args[0])->Get<Value>());

    int flags1 = 0;
    if (IS_BUILTIN_VALUE(args[1]))
        flags1 = (int)TO_NUM_VALUE(TO_BUILTIN_VALUE(args[1])->Get<Value>());
    else if (IS_NUM_VALUE(args[1]))
        flags1 = (int)TO_NUM_VALUE(args[1]);

    result = (double)SDL_GL_SetAttribute((SDL_GLattr)flags0, flags1);
    return true;
}

extern "C" COMPUTE_DUCK_API bool BUILTIN_FN(SDL_GL_SwapWindow)(Value *args, uint8_t argCount, Value &result)
{
    if (!IS_BUILTIN_VALUE(args[0]))
        ASSERT("Invalid builtin value of SDL_GL_SwapWindow(args[0]).");

    auto windowHandle = TO_BUILTIN_VALUE(args[0])->Get<NativeData>().As<SDL_Window>();
    SDL_GL_SwapWindow(windowHandle);
    return false;
}

extern "C" COMPUTE_DUCK_API bool BUILTIN_FN(SDL_GL_CreateContext)(Value *args, uint8_t argCount, Value &result)
{
    if (!IS_BUILTIN_VALUE(args[0]))
        ASSERT("Invalid builtin value of SDL_GL_CreateContext(args[0]).");

    auto windowHandle = TO_BUILTIN_VALUE(args[0])->Get<NativeData>().As<SDL_Window>();

    SDL_GLContext ctx = SDL_GL_CreateContext(windowHandle);

    BuiltinObject *builtinData = Allocator::GetInstance()->CreateObject<BuiltinObject>(ctx, [](void *nativeData)
        { SDL_GL_DeleteContext((SDL_GLContext)nativeData); });
    result = builtinData;
    return true;
}

extern "C" COMPUTE_DUCK_API bool BUILTIN_FN(SDL_GL_SetSwapInterval)(Value *args, uint8_t argCount, Value &result)
{
    if (!IS_NUM_VALUE(args[0]))
        ASSERT("Invalid builtin value of SDL_GL_SetSwapInterval(args[0]).");

    SDL_GL_SetSwapInterval((int32_t)TO_NUM_VALUE(args[0]));
    return false;
}

void RegisterBuiltins()
{
    REGISTER_BUILTIN_VALUE(SDL_INIT_AUDIO);
    REGISTER_BUILTIN_VALUE(SDL_INIT_VIDEO);
    REGISTER_BUILTIN_VALUE(SDL_INIT_JOYSTICK);
    REGISTER_BUILTIN_VALUE(SDL_INIT_HAPTIC);
    REGISTER_BUILTIN_VALUE(SDL_INIT_GAMECONTROLLER);
    REGISTER_BUILTIN_VALUE(SDL_INIT_EVENTS);
    REGISTER_BUILTIN_VALUE(SDL_INIT_NOPARACHUTE);
    REGISTER_BUILTIN_VALUE(SDL_INIT_EVERYTHING);

    REGISTER_BUILTIN_VALUE(SDL_QUIT);

    REGISTER_BUILTIN_VALUE(SDL_WINDOWPOS_CENTERED);
    REGISTER_BUILTIN_VALUE(SDL_WINDOW_FULLSCREEN);
    REGISTER_BUILTIN_VALUE(SDL_WINDOW_OPENGL);
    REGISTER_BUILTIN_VALUE(SDL_WINDOW_SHOWN);
    REGISTER_BUILTIN_VALUE(SDL_WINDOW_HIDDEN);
    REGISTER_BUILTIN_VALUE(SDL_WINDOW_BORDERLESS);
    REGISTER_BUILTIN_VALUE(SDL_WINDOW_RESIZABLE);
    REGISTER_BUILTIN_VALUE(SDL_WINDOW_MINIMIZED);
    REGISTER_BUILTIN_VALUE(SDL_WINDOW_MAXIMIZED);
    REGISTER_BUILTIN_VALUE(SDL_WINDOW_MOUSE_GRABBED);
    REGISTER_BUILTIN_VALUE(SDL_WINDOW_INPUT_FOCUS);
    REGISTER_BUILTIN_VALUE(SDL_WINDOW_FULLSCREEN_DESKTOP);
    REGISTER_BUILTIN_VALUE(SDL_WINDOW_ALLOW_HIGHDPI);
    REGISTER_BUILTIN_VALUE(SDL_WINDOW_MOUSE_CAPTURE);
    REGISTER_BUILTIN_VALUE(SDL_WINDOW_ALWAYS_ON_TOP);
    REGISTER_BUILTIN_VALUE(SDL_WINDOW_SKIP_TASKBAR);
    REGISTER_BUILTIN_VALUE(SDL_WINDOW_UTILITY);
    REGISTER_BUILTIN_VALUE(SDL_WINDOW_TOOLTIP);
    REGISTER_BUILTIN_VALUE(SDL_WINDOW_POPUP_MENU);
    REGISTER_BUILTIN_VALUE(SDL_WINDOW_KEYBOARD_GRABBED);

    REGISTER_BUILTIN_VALUE(SDL_GL_RED_SIZE);
    REGISTER_BUILTIN_VALUE(SDL_GL_GREEN_SIZE);
    REGISTER_BUILTIN_VALUE(SDL_GL_BLUE_SIZE);
    REGISTER_BUILTIN_VALUE(SDL_GL_ALPHA_SIZE);
    REGISTER_BUILTIN_VALUE(SDL_GL_BUFFER_SIZE);
    REGISTER_BUILTIN_VALUE(SDL_GL_DOUBLEBUFFER);
    REGISTER_BUILTIN_VALUE(SDL_GL_DEPTH_SIZE);
    REGISTER_BUILTIN_VALUE(SDL_GL_STENCIL_SIZE);
    REGISTER_BUILTIN_VALUE(SDL_GL_ACCUM_RED_SIZE);
    REGISTER_BUILTIN_VALUE(SDL_GL_ACCUM_GREEN_SIZE);
    REGISTER_BUILTIN_VALUE(SDL_GL_ACCUM_BLUE_SIZE);
    REGISTER_BUILTIN_VALUE(SDL_GL_ACCUM_ALPHA_SIZE);
    REGISTER_BUILTIN_VALUE(SDL_GL_STEREO);
    REGISTER_BUILTIN_VALUE(SDL_GL_MULTISAMPLEBUFFERS);
    REGISTER_BUILTIN_VALUE(SDL_GL_MULTISAMPLESAMPLES);
    REGISTER_BUILTIN_VALUE(SDL_GL_ACCELERATED_VISUAL);
    REGISTER_BUILTIN_VALUE(SDL_GL_RETAINED_BACKING);
    REGISTER_BUILTIN_VALUE(SDL_GL_CONTEXT_MAJOR_VERSION);
    REGISTER_BUILTIN_VALUE(SDL_GL_CONTEXT_MINOR_VERSION);
    REGISTER_BUILTIN_VALUE(SDL_GL_CONTEXT_EGL);
    REGISTER_BUILTIN_VALUE(SDL_GL_CONTEXT_FLAGS);
    REGISTER_BUILTIN_VALUE(SDL_GL_CONTEXT_PROFILE_MASK);
    REGISTER_BUILTIN_VALUE(SDL_GL_SHARE_WITH_CURRENT_CONTEXT);
    REGISTER_BUILTIN_VALUE(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE);
    REGISTER_BUILTIN_VALUE(SDL_GL_CONTEXT_RELEASE_BEHAVIOR);
    REGISTER_BUILTIN_VALUE(SDL_GL_CONTEXT_RESET_NOTIFICATION);

    REGISTER_BUILTIN_VALUE(SDL_GL_CONTEXT_PROFILE_CORE);
    REGISTER_BUILTIN_VALUE(SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    REGISTER_BUILTIN_VALUE(SDL_GL_CONTEXT_PROFILE_ES);

    REGISTER_BUILTIN_FN(SDL_Init);
    REGISTER_BUILTIN_FN(SDL_Quit);
    REGISTER_BUILTIN_FN(SDL_CreateWindow);
    REGISTER_BUILTIN_FN(SDL_PollEvent);
    REGISTER_BUILTIN_FN(SDL_GetEventType);
    REGISTER_BUILTIN_FN(SDL_CreateRenderer);
    REGISTER_BUILTIN_FN(SDL_LoadBMP);
    REGISTER_BUILTIN_FN(SDL_CreateTextureFromSurface);
    REGISTER_BUILTIN_FN(SDL_RenderClear);
    REGISTER_BUILTIN_FN(SDL_RenderCopy);
    REGISTER_BUILTIN_FN(SDL_RenderPresent);
    REGISTER_BUILTIN_FN(SDL_GL_SetAttribute);
    REGISTER_BUILTIN_FN(SDL_GL_SwapWindow);
    REGISTER_BUILTIN_FN(SDL_GL_CreateContext);
    REGISTER_BUILTIN_FN(SDL_GL_SetSwapInterval);
}