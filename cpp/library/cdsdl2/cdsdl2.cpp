#include "cdsdl2.h"
#include <vector>
#include <SDL.h>
#include "../../BuiltinManager.h"
#include "../../Value.h"
void RegisterBuiltins()
{
    #define REGISTER_SDL_VALUE(x)   BuiltinManager::GetInstance()->RegisterVariable(#x, Value((double)(x)));
   
    REGISTER_SDL_VALUE(SDL_QUIT)

    REGISTER_SDL_VALUE(SDL_WINDOWPOS_CENTERED)
    REGISTER_SDL_VALUE(SDL_WINDOW_FULLSCREEN)
    REGISTER_SDL_VALUE(SDL_WINDOW_OPENGL)
    REGISTER_SDL_VALUE(SDL_WINDOW_SHOWN)
    REGISTER_SDL_VALUE(SDL_WINDOW_HIDDEN)
    REGISTER_SDL_VALUE(SDL_WINDOW_BORDERLESS)
    REGISTER_SDL_VALUE(SDL_WINDOW_RESIZABLE)
    REGISTER_SDL_VALUE(SDL_WINDOW_MINIMIZED)
    REGISTER_SDL_VALUE(SDL_WINDOW_MAXIMIZED)
    REGISTER_SDL_VALUE(SDL_WINDOW_MOUSE_GRABBED)
    REGISTER_SDL_VALUE(SDL_WINDOW_INPUT_FOCUS)
    REGISTER_SDL_VALUE(SDL_WINDOW_FULLSCREEN_DESKTOP)
    REGISTER_SDL_VALUE(SDL_WINDOW_ALLOW_HIGHDPI)
    REGISTER_SDL_VALUE(SDL_WINDOW_MOUSE_CAPTURE)
    REGISTER_SDL_VALUE(SDL_WINDOW_ALWAYS_ON_TOP)
    REGISTER_SDL_VALUE(SDL_WINDOW_SKIP_TASKBAR)
    REGISTER_SDL_VALUE(SDL_WINDOW_UTILITY)
    REGISTER_SDL_VALUE(SDL_WINDOW_TOOLTIP)
    REGISTER_SDL_VALUE(SDL_WINDOW_POPUP_MENU)
    REGISTER_SDL_VALUE(SDL_WINDOW_KEYBOARD_GRABBED)


	REGISTER_SDL_VALUE(SDL_GL_RED_SIZE)
	REGISTER_SDL_VALUE(SDL_GL_GREEN_SIZE)
	REGISTER_SDL_VALUE(SDL_GL_BLUE_SIZE)
	REGISTER_SDL_VALUE(SDL_GL_ALPHA_SIZE)
	REGISTER_SDL_VALUE(SDL_GL_BUFFER_SIZE)
	REGISTER_SDL_VALUE(SDL_GL_DOUBLEBUFFER)
	REGISTER_SDL_VALUE(SDL_GL_DEPTH_SIZE)
	REGISTER_SDL_VALUE(SDL_GL_STENCIL_SIZE)
	REGISTER_SDL_VALUE(SDL_GL_ACCUM_RED_SIZE)
	REGISTER_SDL_VALUE(SDL_GL_ACCUM_GREEN_SIZE)
	REGISTER_SDL_VALUE(SDL_GL_ACCUM_BLUE_SIZE)
	REGISTER_SDL_VALUE(SDL_GL_ACCUM_ALPHA_SIZE)
	REGISTER_SDL_VALUE(SDL_GL_STEREO)
	REGISTER_SDL_VALUE(SDL_GL_MULTISAMPLEBUFFERS)
	REGISTER_SDL_VALUE(SDL_GL_MULTISAMPLESAMPLES)
	REGISTER_SDL_VALUE(SDL_GL_ACCELERATED_VISUAL)
	REGISTER_SDL_VALUE(SDL_GL_RETAINED_BACKING)
	REGISTER_SDL_VALUE(SDL_GL_CONTEXT_MAJOR_VERSION)
	REGISTER_SDL_VALUE(SDL_GL_CONTEXT_MINOR_VERSION)
	REGISTER_SDL_VALUE(SDL_GL_CONTEXT_EGL)
	REGISTER_SDL_VALUE(SDL_GL_CONTEXT_FLAGS)
	REGISTER_SDL_VALUE(SDL_GL_CONTEXT_PROFILE_MASK)
	REGISTER_SDL_VALUE(SDL_GL_SHARE_WITH_CURRENT_CONTEXT)
	REGISTER_SDL_VALUE(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE)
	REGISTER_SDL_VALUE(SDL_GL_CONTEXT_RELEASE_BEHAVIOR)
	REGISTER_SDL_VALUE(SDL_GL_CONTEXT_RESET_NOTIFICATION)

    REGISTER_SDL_VALUE(SDL_GL_CONTEXT_PROFILE_CORE)
    REGISTER_SDL_VALUE(SDL_GL_CONTEXT_PROFILE_COMPATIBILITY)
    REGISTER_SDL_VALUE(SDL_GL_CONTEXT_PROFILE_ES)
    
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
                                                        auto name=TO_STR_VALUE(args[0])->value.c_str();
                                                        auto posX=(int)TO_NUM_VALUE(TO_BUILTIN_VARIABLE_VALUE(args[1])->value);
                                                        auto posY=(int)TO_NUM_VALUE(TO_BUILTIN_VARIABLE_VALUE(args[2])->value);
                                                        auto width=TO_NUM_VALUE(args[3]);
                                                        auto height=TO_NUM_VALUE(args[4]);
                                                        auto flags= (uint64_t)TO_NUM_VALUE(args[5]);

                                                        auto window = SDL_CreateWindow(name,posX,posY,width,height,flags);
                                                        
                                                        BuiltinDataObject *builtinData = new BuiltinDataObject();
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

                                                        auto fullPath = BuiltinManager::GetInstance()->ToFullPath(TO_STR_VALUE(args[0])->value);

                                                        SDL_Surface *surface = SDL_LoadBMP(fullPath.c_str());

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

	BuiltinManager::GetInstance()->RegisterFunction("SDL_GL_SetAttribute", [&](Value* args, uint8_t argCount, Value& result) -> bool
		                                            {
		                                            	if (!IS_BUILTIN_VARIABLE_VALUE(args[0]))
		                                            		Assert("Not a valid builtin value of SDL_GL_SetAttribute(args[0]).");
                                                        
                                                        if(!IS_BUILTIN_VARIABLE_VALUE(args[1]) && !IS_NUM_VALUE(args[1]))
		                                            		Assert("Not a valid builtin value or num value of SDL_GL_SetAttribute(args[1]).");

                                                        auto flags0= (int)TO_NUM_VALUE(TO_BUILTIN_VARIABLE_VALUE(args[0])->value);

                                                        int flags1 = 0;
                                                        if(IS_BUILTIN_VARIABLE_VALUE(args[1]))
                                                               flags1 = (int)TO_NUM_VALUE(TO_BUILTIN_VARIABLE_VALUE(args[1])->value);
                                                        else if(IS_NUM_VALUE(args[1]))
                                                               flags1 = (int)TO_NUM_VALUE(args[1]);

                                                        result = (double)SDL_GL_SetAttribute((SDL_GLattr)flags0, flags1);

                                                        return true;
		                                            });

	BuiltinManager::GetInstance()->RegisterFunction("SDL_GL_SwapWindow", [&](Value* args, uint8_t argCount, Value& result) -> bool
		                                            {
		                                            	if (!IS_BUILTIN_DATA_VALUE(args[0]))
		                                            		Assert("Not a valid builtin value of SDL_GL_SwapWindow(args[0]).");

		                                            	auto windowHandle = (SDL_Window*)(TO_BUILTIN_DATA_VALUE(args[0])->nativeData);
		                                            	SDL_GL_SwapWindow(windowHandle);
		                                            	return false;
		                                            });

	BuiltinManager::GetInstance()->RegisterFunction("SDL_GL_CreateContext", [&](Value* args, uint8_t argCount, Value& result) -> bool
		                                            {
		                                            	if (!IS_BUILTIN_DATA_VALUE(args[0]))
		                                            		Assert("Not a valid builtin value of SDL_GL_CreateContext(args[0]).");

                                                        auto windowHandle = (SDL_Window*)TO_BUILTIN_DATA_VALUE(args[0])->nativeData;

                                                        SDL_GLContext ctx = SDL_GL_CreateContext(windowHandle);

		                                            	BuiltinDataObject* builtinData = new BuiltinDataObject();
		                                            	builtinData->nativeData = (void*)ctx;
		                                            	builtinData->destroyFunc = [](void* nativeData)
		                                            		{
		                                            			SDL_GL_DeleteContext((SDL_GLContext)nativeData);
		                                            		};
		                                            	result = builtinData;
		                                            	return true;
		                                            });

	BuiltinManager::GetInstance()->RegisterFunction("SDL_GL_SetSwapInterval", [&](Value* args, uint8_t argCount, Value& result) -> bool
		                                            {
		                                            	if (!IS_NUM_VALUE(args[0]))
		                                            		Assert("Not a valid builtin value of SDL_GL_SetSwapInterval(args[0]).");

                                                        SDL_GL_SetSwapInterval(TO_NUM_VALUE(args[0]));
		                                            	return false;
		                                            });
}