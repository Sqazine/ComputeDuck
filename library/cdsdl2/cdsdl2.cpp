#include "cdsdl2.h"
#include <vector>
#include <SDL.h>
#include "BuiltinManager.h"
#include "Value.h"
void RegisterBuiltins()
{
#define REGISTER_VALUE(x) BuiltinManager::GetInstance()->Register(#x, Value(x))

	REGISTER_VALUE(SDL_QUIT);

	REGISTER_VALUE(SDL_WINDOWPOS_CENTERED);
	REGISTER_VALUE(SDL_WINDOW_FULLSCREEN);
	REGISTER_VALUE(SDL_WINDOW_OPENGL);
	REGISTER_VALUE(SDL_WINDOW_SHOWN);
	REGISTER_VALUE(SDL_WINDOW_HIDDEN);
	REGISTER_VALUE(SDL_WINDOW_BORDERLESS);
	REGISTER_VALUE(SDL_WINDOW_RESIZABLE);
	REGISTER_VALUE(SDL_WINDOW_MINIMIZED);
	REGISTER_VALUE(SDL_WINDOW_MAXIMIZED);
	REGISTER_VALUE(SDL_WINDOW_MOUSE_GRABBED);
	REGISTER_VALUE(SDL_WINDOW_INPUT_FOCUS);
	REGISTER_VALUE(SDL_WINDOW_FULLSCREEN_DESKTOP);
	REGISTER_VALUE(SDL_WINDOW_ALLOW_HIGHDPI);
	REGISTER_VALUE(SDL_WINDOW_MOUSE_CAPTURE);
	REGISTER_VALUE(SDL_WINDOW_ALWAYS_ON_TOP);
	REGISTER_VALUE(SDL_WINDOW_SKIP_TASKBAR);
	REGISTER_VALUE(SDL_WINDOW_UTILITY);
	REGISTER_VALUE(SDL_WINDOW_TOOLTIP);
	REGISTER_VALUE(SDL_WINDOW_POPUP_MENU);
	REGISTER_VALUE(SDL_WINDOW_KEYBOARD_GRABBED);

	REGISTER_VALUE(SDL_GL_RED_SIZE);
	REGISTER_VALUE(SDL_GL_GREEN_SIZE);
	REGISTER_VALUE(SDL_GL_BLUE_SIZE);
	REGISTER_VALUE(SDL_GL_ALPHA_SIZE);
	REGISTER_VALUE(SDL_GL_BUFFER_SIZE);
	REGISTER_VALUE(SDL_GL_DOUBLEBUFFER);
	REGISTER_VALUE(SDL_GL_DEPTH_SIZE);
	REGISTER_VALUE(SDL_GL_STENCIL_SIZE);
	REGISTER_VALUE(SDL_GL_ACCUM_RED_SIZE);
	REGISTER_VALUE(SDL_GL_ACCUM_GREEN_SIZE);
	REGISTER_VALUE(SDL_GL_ACCUM_BLUE_SIZE);
	REGISTER_VALUE(SDL_GL_ACCUM_ALPHA_SIZE);
	REGISTER_VALUE(SDL_GL_STEREO);
	REGISTER_VALUE(SDL_GL_MULTISAMPLEBUFFERS);
	REGISTER_VALUE(SDL_GL_MULTISAMPLESAMPLES);
	REGISTER_VALUE(SDL_GL_ACCELERATED_VISUAL);
	REGISTER_VALUE(SDL_GL_RETAINED_BACKING);
	REGISTER_VALUE(SDL_GL_CONTEXT_MAJOR_VERSION);
	REGISTER_VALUE(SDL_GL_CONTEXT_MINOR_VERSION);
	REGISTER_VALUE(SDL_GL_CONTEXT_EGL);
	REGISTER_VALUE(SDL_GL_CONTEXT_FLAGS);
	REGISTER_VALUE(SDL_GL_CONTEXT_PROFILE_MASK);
	REGISTER_VALUE(SDL_GL_SHARE_WITH_CURRENT_CONTEXT);
	REGISTER_VALUE(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE);
	REGISTER_VALUE(SDL_GL_CONTEXT_RELEASE_BEHAVIOR);
	REGISTER_VALUE(SDL_GL_CONTEXT_RESET_NOTIFICATION);

	REGISTER_VALUE(SDL_GL_CONTEXT_PROFILE_CORE);
	REGISTER_VALUE(SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	REGISTER_VALUE(SDL_GL_CONTEXT_PROFILE_ES);

#undef REGISTER_VALUE

	BuiltinManager::GetInstance()->Register<BuiltinFn>("SDL_Init", [&](Value *args, uint8_t argCount, Value &result) -> bool
													   {
															auto ret = SDL_Init(SDL_INIT_EVERYTHING);
															result = Value(ret);
															return true; });

	BuiltinManager::GetInstance()->Register<BuiltinFn>("SDL_Quit", [&](Value *args, uint8_t argCount, Value &result) -> bool
													   {
															SDL_Quit();
															return false; });

	BuiltinManager::GetInstance()->Register<BuiltinFn>("SDL_CreateWindow", [&](Value *args, uint8_t argCount, Value &result) -> bool
													   {
															auto name = TO_STR_VALUE(args[0])->value.c_str();
															auto posX = (int32_t)TO_NUM_VALUE(TO_BUILTIN_VALUE(args[1])->GetBuiltinValue());
															auto posY = (int32_t)TO_NUM_VALUE(TO_BUILTIN_VALUE(args[2])->GetBuiltinValue());
															int32_t width = (int32_t)TO_NUM_VALUE(args[3]);
															int32_t height = (int32_t)TO_NUM_VALUE(args[4]);
															uint32_t flags = (uint32_t)TO_NUM_VALUE(args[5]);

															auto window = SDL_CreateWindow(name, posX, posY, width, height, flags);

															BuiltinObject* builtinData = new BuiltinObject(window, [](void* nativeData)
																{ SDL_DestroyWindow((SDL_Window*)nativeData); });

															result = builtinData;
															return true; });

	BuiltinManager::GetInstance()->Register<BuiltinFn>("SDL_PollEvent", [&](Value *args, uint8_t argCount, Value &result) -> bool
													   {
															SDL_Event* event = new SDL_Event();
															SDL_PollEvent(event);
															BuiltinObject* builtinData = new BuiltinObject(event, [](void* nativeData)
																{ delete (SDL_Event*)nativeData; });

															result = Value(builtinData);
															return true; });

	BuiltinManager::GetInstance()->Register<BuiltinFn>("SDL_GetEventType", [&](Value *args, uint8_t argCount, Value &result) -> bool
													   {
															if (!IS_BUILTIN_VALUE(args[0]))
																ASSERT("Not a valid builtin data.");
															auto builtinData = TO_BUILTIN_VALUE(args[0]);
															if (builtinData->GetNativeData().IsSameTypeAs<SDL_Event>())
																ASSERT("Not a valid SDL_Event object.");
															SDL_Event* event = builtinData->GetNativeData().As<SDL_Event>();
															result = (double)(event->type);
															return true; });

	BuiltinManager::GetInstance()->Register<BuiltinFn>("SDL_CreateRenderer", [&](Value *args, uint8_t argCount, Value &result) -> bool
													   {
															if (!IS_BUILTIN_VALUE(args[0]))
																ASSERT("Not a valid builtin data.");
															auto builtinData = TO_BUILTIN_VALUE(args[0]);
															if (builtinData->GetNativeData().IsSameTypeAs<SDL_Window>())
																ASSERT("Not a valid SDL_Window object.");
															SDL_Window* window = builtinData->GetNativeData().As<SDL_Window>();
															SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
															BuiltinObject* resultBuiltin = new BuiltinObject(renderer, [](void* nativeData)
																{ SDL_DestroyRenderer((SDL_Renderer*)nativeData); });
															if (renderer)
																result = Value(resultBuiltin);
															return true; });

	BuiltinManager::GetInstance()->Register<BuiltinFn>("SDL_LoadBMP", [&](Value *args, uint8_t argCount, Value &result) -> bool
													   {
															if (!IS_STR_VALUE(args[0]))
																ASSERT("Not a valid str value.");

															auto fullPath = BuiltinManager::GetInstance()->ToFullPath(TO_STR_VALUE(args[0])->value);

															SDL_Surface* surface = SDL_LoadBMP(fullPath.c_str());

															BuiltinObject* resultBuiltinData = new BuiltinObject(surface, [](void* nativeData)
																{ SDL_FreeSurface((SDL_Surface*)nativeData); });

															if (surface)
																result = Value(resultBuiltinData);
															else
																result = Value();
															return true; });

	BuiltinManager::GetInstance()->Register<BuiltinFn>("SDL_CreateTextureFromSurface", [&](Value *args, uint8_t argCount, Value &result) -> bool
													   {
															if (!IS_BUILTIN_VALUE(args[0]) || !IS_BUILTIN_VALUE(args[1]))
																ASSERT("Not a valid builtin value of SDL_CreateTextureFromSurface(args[0] or args[1]).");

															auto renderer = TO_BUILTIN_VALUE(args[0])->GetNativeData().As<SDL_Renderer>();
															auto surface = TO_BUILTIN_VALUE(args[1])->GetNativeData().As<SDL_Surface>();

															SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

															BuiltinObject* resultBuiltin = new BuiltinObject(texture, [](void* nativeData)
																{ SDL_DestroyTexture((SDL_Texture*)nativeData); });

															result = Value(resultBuiltin);
															return true; });

	BuiltinManager::GetInstance()->Register<BuiltinFn>("SDL_RenderClear", [&](Value *args, uint8_t argCount, Value &result) -> bool
													   {
															if (!IS_BUILTIN_VALUE(args[0]))
																ASSERT("Not a valid builtin value of SDL_RenderClear(args[0]).");

															auto renderer = TO_BUILTIN_VALUE(args[0])->GetNativeData().As<SDL_Renderer>();
															SDL_RenderClear(renderer);
															return false; });

	BuiltinManager::GetInstance()->Register<BuiltinFn>("SDL_RenderCopy", [&](Value *args, uint8_t argCount, Value &result) -> bool
													   {
															if (!IS_BUILTIN_VALUE(args[0]) || !IS_BUILTIN_VALUE(args[1]))
																ASSERT("Not a valid builtin value of SDL_RenderCopy(args[0] or args[1]).");

															auto renderer = TO_BUILTIN_VALUE(args[0])->GetNativeData().As<SDL_Renderer>();
															auto texture = TO_BUILTIN_VALUE(args[1])->GetNativeData().As<SDL_Texture>();

															auto ret = SDL_RenderCopy(renderer, texture, nullptr, nullptr);

															result = Value(ret);
															return true; });

	BuiltinManager::GetInstance()->Register<BuiltinFn>("SDL_RenderPresent", [&](Value *args, uint8_t argCount, Value &result) -> bool
													   {
															if (!IS_BUILTIN_VALUE(args[0]))
																ASSERT("Not a valid builtin value of SDL_RenderPresent(args[0]).");

															auto renderer = TO_BUILTIN_VALUE(args[0])->GetNativeData().As<SDL_Renderer>();
															SDL_RenderPresent(renderer);
															return false; });

	BuiltinManager::GetInstance()->Register<BuiltinFn>("SDL_GL_SetAttribute", [&](Value *args, uint8_t argCount, Value &result) -> bool
													   {
															if (!IS_BUILTIN_VALUE(args[0]))
																ASSERT("Not a valid builtin value of SDL_GL_SetAttribute(args[0]).");

															if (!IS_BUILTIN_VALUE(args[1]) && !IS_NUM_VALUE(args[1]))
																ASSERT("Not a valid builtin value or num value of SDL_GL_SetAttribute(args[1]).");

															auto flags0 = (int)TO_NUM_VALUE(TO_BUILTIN_VALUE(args[0])->GetBuiltinValue());

															int flags1 = 0;
															if (IS_BUILTIN_VALUE(args[1]))
																flags1 = (int)TO_NUM_VALUE(TO_BUILTIN_VALUE(args[1])->GetBuiltinValue());
															else if (IS_NUM_VALUE(args[1]))
																flags1 = (int)TO_NUM_VALUE(args[1]);

															result = (double)SDL_GL_SetAttribute((SDL_GLattr)flags0, flags1);
															return true; });

	BuiltinManager::GetInstance()->Register<BuiltinFn>("SDL_GL_SwapWindow", [&](Value *args, uint8_t argCount, Value &result) -> bool
													   {
															if (!IS_BUILTIN_VALUE(args[0]))
																ASSERT("Not a valid builtin value of SDL_GL_SwapWindow(args[0]).");

															auto windowHandle = TO_BUILTIN_VALUE(args[0])->GetNativeData().As<SDL_Window>();
															SDL_GL_SwapWindow(windowHandle);
															return false; });

	BuiltinManager::GetInstance()->Register<BuiltinFn>("SDL_GL_CreateContext", [&](Value *args, uint8_t argCount, Value &result) -> bool
													   {
															if (!IS_BUILTIN_VALUE(args[0]))
																ASSERT("Not a valid builtin value of SDL_GL_CreateContext(args[0]).");

															auto windowHandle = TO_BUILTIN_VALUE(args[0])->GetNativeData().As<SDL_Window>();

															SDL_GLContext ctx = SDL_GL_CreateContext(windowHandle);

															BuiltinObject* builtinData = new BuiltinObject(ctx, [](void* nativeData)
																{ SDL_GL_DeleteContext((SDL_GLContext)nativeData); });
															result = builtinData;
															return true; });

	BuiltinManager::GetInstance()->Register<BuiltinFn>("SDL_GL_SetSwapInterval", [&](Value *args, uint8_t argCount, Value &result) -> bool
													   {
															if (!IS_NUM_VALUE(args[0]))
																ASSERT("Not a valid builtin value of SDL_GL_SetSwapInterval(args[0]).");

															SDL_GL_SetSwapInterval((int32_t)TO_NUM_VALUE(args[0]));
															return false; });
}