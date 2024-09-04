from BuiltinManager import *
from Object import *
import ctypes
import sdl2


def sdl_init_wrapper(args: list[Object]):
    ret = sdl2.SDL_Init(sdl2.SDL_INIT_EVERYTHING)
    return True, NumObject(ret)


def sdl_quit_wrapper(args: list[Object]):
    sdl2.SDL_Quit()
    return False, None


def sdl_destroy_window_wrapper(data: any):
    sdl2.SDL_DestroyWindow(data)


def sdl_create_window_wrapper(args: list[Object]):
    name = ctypes.string_at(ctypes.c_char_p(args[0].value.encode('utf-8')))
    posX = sdl2.SDL_WindowFlags(args[1].data.value)
    posY = sdl2.SDL_WindowFlags(args[2].data.value)
    width = int(args[3].value)
    height = int(args[4].value)
    flags = int(args[5].value)

    window = sdl2.SDL_CreateWindow(name, posX, posY, width, height, flags)
    builtinData = BuiltinObject(window, sdl_destroy_window_wrapper)
    return True, builtinData


def sdl_poll_event_wrapper(args: list[Object]):
    event = sdl2.SDL_Event()
    sdl2.SDL_PollEvent(ctypes.byref(event))
    builtinData = BuiltinObject(event, None)
    return True, builtinData


def sdl_get_event_type_wrapper(args: list[Object]):
    if args[0].type != ObjectType.BUILTIN:
        error("Invalid builtin data.")

    typeStr = type(args[0].data)().__str__()
    if typeStr.find("sdl2.events.SDL_Event") == -1:
        error("Invalid SDL_Event object.")

    return True, NumObject(args[0].data.type)


def sdl_destroy_renderer_wrapper(data: any):
    sdl2.SDL_DestroyRenderer(data)


def sdl_create_renderer_wrapper(args: list[Object]):
    if args[0].type != ObjectType.BUILTIN:
        error("Invalid builtin data.")

    typeStr = type(args[0].data)().__str__()
    if typeStr.find("sdl2.video.LP_SDL_Window") == -1:
        error("Invalid SDL_Wnidow object.")

    window = args[0].data
    renderer = sdl2.SDL_CreateRenderer(
        window, -1, sdl2.SDL_RENDERER_ACCELERATED | sdl2.SDL_RENDERER_PRESENTVSYNC)
    result = BuiltinObject(renderer, sdl_destroy_renderer_wrapper)
    return True, result


def sdl_free_surface_wrapper(data: any):
    sdl2.SDL_FreeSurface(data)


def sdl_load_bmp_wrapper(args: list[Object]):
    if args[0].type != ObjectType.STR:
        error("Invalid str value.")

    fullPath = gBuiltinManager.to_full_path(args[0].value)
    arg0 = ctypes.string_at(ctypes.c_char_p(fullPath.encode('utf-8')))
    surface = sdl2.SDL_LoadBMP(arg0)
    result = BuiltinObject(surface, sdl_free_surface_wrapper)
    return True, result


def sdlDestroyTextureWrapper(data: any):
    sdl2.SDL_DestroyTexture(data)


def sdlCreateTextureFromSurfaceWrapper(args: list[Object]):
    if args[0].type != ObjectType.BUILTIN or args[1].type != ObjectType.BUILTIN:
        error(
            "Invalid builtin value of SDL_CreateTextureFromSurface(args[0] or args[1])")

    renderer = args[0].data
    surface = args[1].data

    texture = sdl2.SDL_CreateTextureFromSurface(renderer, surface)

    result = BuiltinObject(texture, sdlDestroyTextureWrapper)
    return True, result


def sdl_render_clear_wrapper(args: list[Object]):
    if args[0].type != ObjectType.BUILTIN:
        error("Invalid builtin value of SDL_RenderClear(args[0]).")
    renderer = args[0].data
    sdl2.SDL_RenderClear(renderer)
    return False, None


def sdl_render_copy_wrapper(args: list[Object]):
    if args[0].type != ObjectType.BUILTIN or args[1].type != ObjectType.BUILTIN:
        error("Invalid builtin value of SDL_RenderCopy(args[0] or args[1]).")
    renderer = args[0].data
    texture = args[1].data
    ret = sdl2.SDL_RenderCopy(renderer, texture, None, None)
    return True, NumObject(ret)


def sdl_render_present_wrapper(args: list[Object]):
    if args[0].type != ObjectType.BUILTIN:
        error("Invalid builtin value of SDL_RenderPresent(args[0]).")
    renderer = args[0].data
    sdl2.SDL_RenderPresent(renderer)
    return False, None


def sdl_gl_set_attribute_wrapper(args: list[Object]):
    if args[0].type != ObjectType.BUILTIN:
        error("Invalid builtin value of SDL_GL_SetAttribute(args[0]).")
    if args[1].type != ObjectType.BUILTIN and args[1].type != type != ObjectType.NUM:
        error(
            "Invalid builtin value or num value of SDL_GL_SetAttribute(args[1]).")

    flags0 = args[0].data
    flags1 = 0
    if args[1].type == ObjectType.BUILTIN:
        flags1 = args[1].data.value
    elif args[1].type == ObjectType.NUM:
        flags1 = args[1].value
    result = sdl2.SDL_GL_SetAttribute(flags0, flags1)
    return True, result


def sdl_gl_swap_window_wrapper(args: list[Object]):
    if args[0].type != ObjectType.BUILTIN:
        error("Invalid builtin value of SDL_GL_SwapWindow(args[0]).")
    windowHandle = args[0].data
    sdl2.SDL_GL_SwapWindow(windowHandle)
    return False, None


def sdl_gl_delete_context_wrapper(data: any):
    sdl2.SDL_GL_DeleteContext(data)


def sdl_gl_create_context_wrapper(args: list[Object]):
    if args[0].type != ObjectType.BUILTIN:
        error("Invalid builtin value of SDL_GL_CreateContext(args[0]).")
    windowHandle = args[0].data

    ctx = sdl2.SDL_GL_CreateContext(windowHandle)
    builtinData = BuiltinObject(ctx, sdl_gl_delete_context_wrapper)
    return True, builtinData


def sdl_gl_set_swap_interval_wrapper(args: list[Object]):
    if args[0].type != ObjectType.NUM:
        error("Invalid builtin value of SDL_GL_SetSwapInterval(args[0]).")
    sdl2.SDL_GL_SetSwapInterval(int(args[0].value))
    return False, None


def register_builtins() -> None:
    gBuiltinManager.register("SDL_INIT_AUDIO", NumObject(sdl2.SDL_INIT_AUDIO))
    gBuiltinManager.register("SDL_INIT_VIDEO", NumObject(sdl2.SDL_INIT_VIDEO))
    gBuiltinManager.register(
        "SDL_INIT_JOYSTICK", NumObject(sdl2.SDL_INIT_JOYSTICK))
    gBuiltinManager.register(
        "SDL_INIT_HAPTIC", NumObject(sdl2.SDL_INIT_HAPTIC))
    gBuiltinManager.register("SDL_INIT_GAMECONTROLLER",
                             NumObject(sdl2.SDL_INIT_GAMECONTROLLER))
    gBuiltinManager.register(
        "SDL_INIT_EVENTS", NumObject(sdl2.SDL_INIT_EVENTS))
    gBuiltinManager.register("SDL_INIT_NOPARACHUTE",
                             NumObject(sdl2.SDL_INIT_NOPARACHUTE))
    gBuiltinManager.register("SDL_INIT_EVERYTHING",
                             NumObject(sdl2.SDL_INIT_EVERYTHING))
    gBuiltinManager.register("SDL_QUIT", NumObject(sdl2.SDL_QUIT))
    gBuiltinManager.register("SDL_WINDOWPOS_CENTERED",
                             NumObject(sdl2.SDL_WINDOWPOS_CENTERED))
    gBuiltinManager.register("SDL_WINDOW_FULLSCREEN",
                             NumObject(sdl2.SDL_WINDOW_FULLSCREEN))
    gBuiltinManager.register(
        "SDL_WINDOW_OPENGL", NumObject(sdl2.SDL_WINDOW_OPENGL))
    gBuiltinManager.register(
        "SDL_WINDOW_SHOWN", NumObject(sdl2.SDL_WINDOW_SHOWN))
    gBuiltinManager.register(
        "SDL_WINDOW_HIDDEN", NumObject(sdl2.SDL_WINDOW_HIDDEN))
    gBuiltinManager.register("SDL_WINDOW_BORDERLESS",
                             NumObject(sdl2.SDL_WINDOW_BORDERLESS))
    gBuiltinManager.register("SDL_WINDOW_RESIZABLE",
                             NumObject(sdl2.SDL_WINDOW_RESIZABLE))
    gBuiltinManager.register("SDL_WINDOW_MINIMIZED",
                             NumObject(sdl2.SDL_WINDOW_MINIMIZED))
    gBuiltinManager.register("SDL_WINDOW_MAXIMIZED",
                             NumObject(sdl2.SDL_WINDOW_MAXIMIZED))
    gBuiltinManager.register("SDL_WINDOW_MOUSE_GRABBED",
                             NumObject(sdl2.SDL_WINDOW_MOUSE_GRABBED))
    gBuiltinManager.register("SDL_WINDOW_INPUT_FOCUS",
                             NumObject(sdl2.SDL_WINDOW_INPUT_FOCUS))
    gBuiltinManager.register("SDL_WINDOW_FULLSCREEN_DESKTOP", NumObject(
        sdl2.SDL_WINDOW_FULLSCREEN_DESKTOP))
    gBuiltinManager.register("SDL_WINDOW_ALLOW_HIGHDPI",
                             NumObject(sdl2.SDL_WINDOW_ALLOW_HIGHDPI))
    gBuiltinManager.register("SDL_WINDOW_MOUSE_CAPTURE",
                             NumObject(sdl2.SDL_WINDOW_MOUSE_CAPTURE))
    gBuiltinManager.register("SDL_WINDOW_ALWAYS_ON_TOP",
                             NumObject(sdl2.SDL_WINDOW_ALWAYS_ON_TOP))
    gBuiltinManager.register("SDL_WINDOW_SKIP_TASKBAR",
                             NumObject(sdl2.SDL_WINDOW_SKIP_TASKBAR))
    gBuiltinManager.register("SDL_WINDOW_UTILITY",
                             NumObject(sdl2.SDL_WINDOW_UTILITY))
    gBuiltinManager.register("SDL_WINDOW_TOOLTIP",
                             NumObject(sdl2.SDL_WINDOW_TOOLTIP))
    gBuiltinManager.register("SDL_WINDOW_POPUP_MENU",
                             NumObject(sdl2.SDL_WINDOW_POPUP_MENU))
    gBuiltinManager.register("SDL_WINDOW_KEYBOARD_GRABBED", NumObject(
        sdl2.SDL_WINDOW_KEYBOARD_GRABBED))

    gBuiltinManager.register(
        "SDL_GL_RED_SIZE", NumObject(sdl2.SDL_GL_RED_SIZE))
    gBuiltinManager.register(
        "SDL_GL_GREEN_SIZE", NumObject(sdl2.SDL_GL_GREEN_SIZE))
    gBuiltinManager.register(
        "SDL_GL_BLUE_SIZE", NumObject(sdl2.SDL_GL_BLUE_SIZE))
    gBuiltinManager.register(
        "SDL_GL_ALPHA_SIZE", NumObject(sdl2.SDL_GL_ALPHA_SIZE))
    gBuiltinManager.register("SDL_GL_BUFFER_SIZE",
                             NumObject(sdl2.SDL_GL_BUFFER_SIZE))
    gBuiltinManager.register("SDL_GL_DOUBLEBUFFER",
                             NumObject(sdl2.SDL_GL_DOUBLEBUFFER))
    gBuiltinManager.register(
        "SDL_GL_DEPTH_SIZE", NumObject(sdl2.SDL_GL_DEPTH_SIZE))
    gBuiltinManager.register("SDL_GL_STENCIL_SIZE",
                             NumObject(sdl2.SDL_GL_STENCIL_SIZE))
    gBuiltinManager.register("SDL_GL_ACCUM_RED_SIZE",
                             NumObject(sdl2.SDL_GL_ACCUM_RED_SIZE))
    gBuiltinManager.register("SDL_GL_ACCUM_GREEN_SIZE",
                             NumObject(sdl2.SDL_GL_ACCUM_GREEN_SIZE))
    gBuiltinManager.register("SDL_GL_ACCUM_BLUE_SIZE",
                             NumObject(sdl2.SDL_GL_ACCUM_BLUE_SIZE))
    gBuiltinManager.register("SDL_GL_ACCUM_ALPHA_SIZE",
                             NumObject(sdl2.SDL_GL_ACCUM_ALPHA_SIZE))
    gBuiltinManager.register("SDL_GL_STEREO", NumObject(sdl2.SDL_GL_STEREO))
    gBuiltinManager.register("SDL_GL_MULTISAMPLEBUFFERS",
                             NumObject(sdl2.SDL_GL_MULTISAMPLEBUFFERS))
    gBuiltinManager.register("SDL_GL_MULTISAMPLESAMPLES",
                             NumObject(sdl2.SDL_GL_MULTISAMPLESAMPLES))
    gBuiltinManager.register("SDL_GL_ACCELERATED_VISUAL",
                             NumObject(sdl2.SDL_GL_ACCELERATED_VISUAL))
    gBuiltinManager.register("SDL_GL_RETAINED_BACKING",
                             NumObject(sdl2.SDL_GL_RETAINED_BACKING))
    gBuiltinManager.register("SDL_GL_CONTEXT_MAJOR_VERSION", NumObject(
        sdl2.SDL_GL_CONTEXT_MAJOR_VERSION))
    gBuiltinManager.register("SDL_GL_CONTEXT_MINOR_VERSION", NumObject(
        sdl2.SDL_GL_CONTEXT_MINOR_VERSION))
    gBuiltinManager.register("SDL_GL_CONTEXT_EGL",
                             NumObject(sdl2.SDL_GL_CONTEXT_EGL))
    gBuiltinManager.register("SDL_GL_CONTEXT_FLAGS",
                             NumObject(sdl2.SDL_GL_CONTEXT_FLAGS))
    gBuiltinManager.register("SDL_GL_CONTEXT_PROFILE_MASK", NumObject(
        sdl2.SDL_GL_CONTEXT_PROFILE_MASK))
    gBuiltinManager.register("SDL_GL_SHARE_WITH_CURRENT_CONTEXT", NumObject(
        sdl2.SDL_GL_SHARE_WITH_CURRENT_CONTEXT))
    gBuiltinManager.register("SDL_GL_FRAMEBUFFER_SRGB_CAPABLE", NumObject(
        sdl2.SDL_GL_FRAMEBUFFER_SRGB_CAPABLE))
    gBuiltinManager.register("SDL_GL_CONTEXT_RELEASE_BEHAVIOR", NumObject(
        sdl2.SDL_GL_CONTEXT_RELEASE_BEHAVIOR))
    gBuiltinManager.register("SDL_GL_CONTEXT_RESET_NOTIFICATION", NumObject(
        sdl2.SDL_GL_CONTEXT_RESET_NOTIFICATION))

    gBuiltinManager.register("SDL_GL_CONTEXT_PROFILE_CORE", NumObject(
        sdl2.SDL_GL_CONTEXT_PROFILE_CORE))
    gBuiltinManager.register("SDL_GL_CONTEXT_PROFILE_COMPATIBILITY", NumObject(
        sdl2.SDL_GL_CONTEXT_PROFILE_COMPATIBILITY))
    gBuiltinManager.register("SDL_GL_CONTEXT_PROFILE_ES",
                             NumObject(sdl2.SDL_GL_CONTEXT_PROFILE_ES))

    gBuiltinManager.register("SDL_Init", sdl_init_wrapper)
    gBuiltinManager.register("SDL_Quit", sdl_quit_wrapper)
    gBuiltinManager.register("SDL_CreateWindow", sdl_create_window_wrapper)
    gBuiltinManager.register("SDL_PollEvent", sdl_poll_event_wrapper)
    gBuiltinManager.register("SDL_GetEventType", sdl_get_event_type_wrapper)
    gBuiltinManager.register("SDL_CreateRenderer", sdl_create_renderer_wrapper)
    gBuiltinManager.register("SDL_LoadBMP", sdl_load_bmp_wrapper)
    gBuiltinManager.register(
        "SDL_CreateTextureFromSurface", sdlCreateTextureFromSurfaceWrapper)
    gBuiltinManager.register("SDL_RenderClear", sdl_render_clear_wrapper)
    gBuiltinManager.register("SDL_RenderCopy", sdl_render_copy_wrapper)
    gBuiltinManager.register("SDL_RenderPresent", sdl_render_present_wrapper)
    gBuiltinManager.register("SDL_GL_SetAttribute",
                             sdl_gl_set_attribute_wrapper)
    gBuiltinManager.register("SDL_GL_SwapWindow", sdl_gl_swap_window_wrapper)
    gBuiltinManager.register("SDL_GL_CreateContext",
                             sdl_gl_create_context_wrapper)
    gBuiltinManager.register("SDL_GL_SetSwapInterval",
                             sdl_gl_set_swap_interval_wrapper)
