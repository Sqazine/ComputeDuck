from BuiltinManager import *
from Object import *
import ctypes
import sdl2


def sdlInitWrapper(args: list[Object]):
    ret = sdl2.SDL_Init(sdl2.SDL_INIT_EVERYTHING)
    return True, NumObject(ret)


def sdlQuitWrapper(args: list[Object]):
    sdl2.SDL_Quit()
    return False, None


def sdlDestroyWindowWrapper(nativeData: any):
    sdl2.SDL_DestroyWindow(nativeData)


def sdlCreateWindowWrapper(args: list[Object]):

    arg0 = ctypes.string_at(ctypes.c_char_p(args[0].value.encode('utf-8')))
    arg1 = sdl2.SDL_WindowFlags(args[1].obj.value)
    arg2 = sdl2.SDL_WindowFlags(args[2].obj.value)
    arg3 = int(args[3].value)
    arg4 = int(args[4].value)

    window = sdl2.SDL_CreateWindow(
        arg0, arg1, arg2, arg3, arg4, sdl2.SDL_WINDOW_ALLOW_HIGHDPI)
    builtinData = BuiltinDataObject(window, sdlDestroyWindowWrapper)
    return True, builtinData


def sdlPollEventWrapper(args: list[Object]):
    event = sdl2.SDL_Event()
    sdl2.SDL_PollEvent(ctypes.byref(event))
    builtinData = BuiltinDataObject(event, None)
    return True, builtinData


def sdlGetEventTypeWrapper(args: list[Object]):
    if args[0].type != ObjectType.BUILTIN_DATA:
        Assert("Not a valid builtin data.")

    typeStr = type(args[0].nativeData)().__str__()
    if typeStr.find("sdl2.events.SDL_Event") == -1:
        Assert("Not a valid SDL_Event object.")

    return True, NumObject(args[0].nativeData.type)


def sdlDestroyRendererWrapper(nativeData: any):
    sdl2.SDL_DestroyRenderer(nativeData)


def sdlCreateRendererWrapper(args: list[Object]):
    if args[0].type != ObjectType.BUILTIN_DATA:
        Assert("Not a valid builtin data.")

    typeStr = type(args[0].nativeData)().__str__()
    if typeStr.find("sdl2.video.LP_SDL_Window") == -1:
        Assert("Not a valid SDL_Wnidow object.")

    window = args[0].nativeData
    renderer = sdl2.SDL_CreateRenderer(
        window, -1, sdl2.SDL_RENDERER_ACCELERATED | sdl2.SDL_RENDERER_PRESENTVSYNC)
    result = BuiltinDataObject(renderer, sdlDestroyRendererWrapper)
    return True, result


def sdlFreeSurfaceWrapper(nativeData: any):
    sdl2.SDL_FreeSurface(nativeData)


def sdlLoadBmpWrapper(args: list[Object]):
    if args[0].type != ObjectType.STR:
        Assert("Not a valid str value.")

    arg0 = ctypes.string_at(ctypes.c_char_p(args[0].value.encode('utf-8')))

    surface = sdl2.SDL_LoadBMP(arg0)
    result = BuiltinDataObject(surface, sdlFreeSurfaceWrapper)
    return True, result


def sdlDestroyTextureWrapper(nativeData: any):
    sdl2.SDL_DestroyTexture(nativeData)


def sdlCreateTextureFromSurfaceWrapper(args: list[Object]):
    if args[0].type != ObjectType.BUILTIN_DATA or args[1].type != ObjectType.BUILTIN_DATA:
        Assert(
            "Not a valid builtin value of SDL_CreateTextureFromSurface(args[0] or args[1])")

    renderer = args[0].nativeData
    surface = args[1].nativeData

    texture = sdl2.SDL_CreateTextureFromSurface(renderer, surface)

    result = BuiltinDataObject(texture, sdlDestroyTextureWrapper)
    return True, result


def sdlRenderClearWrapper(args: list[Object]):
    if args[0].type != ObjectType.BUILTIN_DATA:
        Assert("Not a valid builtin value of SDL_RenderClear(args[0]).")
    renderer = args[0].nativeData
    sdl2.SDL_RenderClear(renderer)
    return False, None


def sdlRenderCopyWrapper(args: list[Object]):
    if args[0].type != ObjectType.BUILTIN_DATA or args[1].type != ObjectType.BUILTIN_DATA:
        Assert(
            "Not a valid builtin value of SDL_RenderCopy(args[0] or args[1]).")
    renderer = args[0].nativeData
    texture = args[1].nativeData
    ret = sdl2.SDL_RenderCopy(renderer, texture, None, None)
    return True, NumObject(ret)


def sdlRenderPresentWrapper(args: list[Object]):
    if args[0].type != ObjectType.BUILTIN_DATA:
        Assert("Not a valid builtin value of SDL_RenderPresent(args[0]).")
    renderer = args[0].nativeData
    sdl2.SDL_RenderPresent(renderer)
    return False, None


def RegisterBuiltins() -> None:
    gBuiltinManager.RegisterVariable(
        "SDL_WINDOWPOS_CENTERED", NumObject(sdl2.SDL_WINDOWPOS_CENTERED))
    gBuiltinManager.RegisterVariable("SDL_QUIT", NumObject(sdl2.SDL_QUIT))

    gBuiltinManager.RegisterFunction("SDL_Init", sdlInitWrapper)
    gBuiltinManager.RegisterFunction("SDL_Quit", sdlQuitWrapper)
    gBuiltinManager.RegisterFunction(
        "SDL_CreateWindow", sdlCreateWindowWrapper)
    gBuiltinManager.RegisterFunction("SDL_PollEvent", sdlPollEventWrapper)
    gBuiltinManager.RegisterFunction(
        "SDL_GetEventType", sdlGetEventTypeWrapper)
    gBuiltinManager.RegisterFunction(
        "SDL_CreateRenderer", sdlCreateRendererWrapper)
    gBuiltinManager.RegisterFunction("SDL_LoadBMP", sdlLoadBmpWrapper)
    gBuiltinManager.RegisterFunction(
        "SDL_CreateTextureFromSurface", sdlCreateTextureFromSurfaceWrapper)
    gBuiltinManager.RegisterFunction("SDL_RenderClear", sdlRenderClearWrapper)
    gBuiltinManager.RegisterFunction("SDL_RenderCopy", sdlRenderCopyWrapper)
    gBuiltinManager.RegisterFunction(
        "SDL_RenderPresent", sdlRenderPresentWrapper)
