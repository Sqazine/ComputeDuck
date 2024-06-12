using ComputeDuck;
using SDL2;
using Object = ComputeDuck.Object;
public class cdsdl2
{
    //set as exe to get the "Sayers.SDL2.Core.dll" library
    static void Main(string[] args)
    {
        return;
    }
    public static void RegisterBuiltins()
    {
        BuiltinManager.GetInstance().Register("SDL_INIT_AUDIO", new NumObject(SDL.SDL_INIT_AUDIO));
        BuiltinManager.GetInstance().Register("SDL_INIT_VIDEO", new NumObject(SDL.SDL_INIT_VIDEO));
        BuiltinManager.GetInstance().Register("SDL_INIT_JOYSTICK", new NumObject(SDL.SDL_INIT_JOYSTICK));
        BuiltinManager.GetInstance().Register("SDL_INIT_HAPTIC", new NumObject(SDL.SDL_INIT_HAPTIC));
        BuiltinManager.GetInstance().Register("SDL_INIT_GAMECONTROLLER", new NumObject(SDL.SDL_INIT_GAMECONTROLLER));
        BuiltinManager.GetInstance().Register("SDL_INIT_EVENTS", new NumObject(SDL.SDL_INIT_EVENTS));
        BuiltinManager.GetInstance().Register("SDL_INIT_NOPARACHUTE", new NumObject(SDL.SDL_INIT_NOPARACHUTE));
        BuiltinManager.GetInstance().Register("SDL_INIT_EVERYTHING", new NumObject(SDL.SDL_INIT_EVERYTHING));
        BuiltinManager.GetInstance().Register("SDL_QUIT", new NumObject((double)SDL.SDL_EventType.SDL_QUIT));
        BuiltinManager.GetInstance().Register("SDL_WINDOWPOS_CENTERED", new NumObject(SDL.SDL_WINDOWPOS_CENTERED));
        BuiltinManager.GetInstance().Register("SDL_WINDOW_FULLSCREEN", new NumObject((uint)SDL.SDL_WindowFlags.SDL_WINDOW_FULLSCREEN));
        BuiltinManager.GetInstance().Register("SDL_WINDOW_OPENGL", new NumObject((uint)SDL.SDL_WindowFlags.SDL_WINDOW_OPENGL));
        BuiltinManager.GetInstance().Register("SDL_WINDOW_SHOWN", new NumObject((uint)SDL.SDL_WindowFlags.SDL_WINDOW_SHOWN));
        BuiltinManager.GetInstance().Register("SDL_WINDOW_HIDDEN", new NumObject((uint)SDL.SDL_WindowFlags.SDL_WINDOW_HIDDEN));
        BuiltinManager.GetInstance().Register("SDL_WINDOW_BORDERLESS", new NumObject((uint)SDL.SDL_WindowFlags.SDL_WINDOW_BORDERLESS));
        BuiltinManager.GetInstance().Register("SDL_WINDOW_RESIZABLE", new NumObject((uint)SDL.SDL_WindowFlags.SDL_WINDOW_RESIZABLE));
        BuiltinManager.GetInstance().Register("SDL_WINDOW_MINIMIZED", new NumObject((uint)SDL.SDL_WindowFlags.SDL_WINDOW_MINIMIZED));
        BuiltinManager.GetInstance().Register("SDL_WINDOW_MAXIMIZED", new NumObject((uint)SDL.SDL_WindowFlags.SDL_WINDOW_MAXIMIZED));
        BuiltinManager.GetInstance().Register("SDL_WINDOW_MOUSE_GRABBED", new NumObject((uint)SDL.SDL_WindowFlags.SDL_WINDOW_MOUSE_GRABBED));
        BuiltinManager.GetInstance().Register("SDL_WINDOW_INPUT_FOCUS", new NumObject((uint)SDL.SDL_WindowFlags.SDL_WINDOW_INPUT_FOCUS));
        BuiltinManager.GetInstance().Register("SDL_WINDOW_FULLSCREEN_DESKTOP", new NumObject((uint)SDL.SDL_WindowFlags.SDL_WINDOW_FULLSCREEN_DESKTOP));
        BuiltinManager.GetInstance().Register("SDL_WINDOW_ALLOW_HIGHDPI", new NumObject((uint)SDL.SDL_WindowFlags.SDL_WINDOW_ALLOW_HIGHDPI));
        BuiltinManager.GetInstance().Register("SDL_WINDOW_MOUSE_CAPTURE", new NumObject((uint)SDL.SDL_WindowFlags.SDL_WINDOW_MOUSE_CAPTURE));
        BuiltinManager.GetInstance().Register("SDL_WINDOW_ALWAYS_ON_TOP", new NumObject((uint)SDL.SDL_WindowFlags.SDL_WINDOW_ALWAYS_ON_TOP));
        BuiltinManager.GetInstance().Register("SDL_WINDOW_SKIP_TASKBAR", new NumObject((uint)SDL.SDL_WindowFlags.SDL_WINDOW_SKIP_TASKBAR));
        BuiltinManager.GetInstance().Register("SDL_WINDOW_UTILITY", new NumObject((uint)SDL.SDL_WindowFlags.SDL_WINDOW_UTILITY));
        BuiltinManager.GetInstance().Register("SDL_WINDOW_TOOLTIP", new NumObject((uint)SDL.SDL_WindowFlags.SDL_WINDOW_TOOLTIP));
        BuiltinManager.GetInstance().Register("SDL_WINDOW_POPUP_MENU", new NumObject((uint)SDL.SDL_WindowFlags.SDL_WINDOW_POPUP_MENU));
        BuiltinManager.GetInstance().Register("SDL_WINDOW_KEYBOARD_GRABBED", new NumObject((uint)SDL.SDL_WindowFlags.SDL_WINDOW_KEYBOARD_GRABBED));

        BuiltinManager.GetInstance().Register("SDL_GL_RED_SIZE", new NumObject((uint)SDL.SDL_GLattr.SDL_GL_RED_SIZE));
        BuiltinManager.GetInstance().Register("SDL_GL_GREEN_SIZE", new NumObject((uint)SDL.SDL_GLattr.SDL_GL_GREEN_SIZE));
        BuiltinManager.GetInstance().Register("SDL_GL_BLUE_SIZE", new NumObject((uint)SDL.SDL_GLattr.SDL_GL_BLUE_SIZE));
        BuiltinManager.GetInstance().Register("SDL_GL_ALPHA_SIZE", new NumObject((uint)SDL.SDL_GLattr.SDL_GL_ALPHA_SIZE));
        BuiltinManager.GetInstance().Register("SDL_GL_BUFFER_SIZE", new NumObject((uint)SDL.SDL_GLattr.SDL_GL_BUFFER_SIZE));
        BuiltinManager.GetInstance().Register("SDL_GL_DOUBLEBUFFER", new NumObject((uint)SDL.SDL_GLattr.SDL_GL_DOUBLEBUFFER));
        BuiltinManager.GetInstance().Register("SDL_GL_DEPTH_SIZE", new NumObject((uint)SDL.SDL_GLattr.SDL_GL_DEPTH_SIZE));
        BuiltinManager.GetInstance().Register("SDL_GL_STENCIL_SIZE", new NumObject((uint)SDL.SDL_GLattr.SDL_GL_STENCIL_SIZE));
        BuiltinManager.GetInstance().Register("SDL_GL_ACCUM_RED_SIZE", new NumObject((uint)SDL.SDL_GLattr.SDL_GL_ACCUM_RED_SIZE));
        BuiltinManager.GetInstance().Register("SDL_GL_ACCUM_GREEN_SIZE", new NumObject((uint)SDL.SDL_GLattr.SDL_GL_ACCUM_GREEN_SIZE));
        BuiltinManager.GetInstance().Register("SDL_GL_ACCUM_BLUE_SIZE", new NumObject((uint)SDL.SDL_GLattr.SDL_GL_ACCUM_BLUE_SIZE));
        BuiltinManager.GetInstance().Register("SDL_GL_ACCUM_ALPHA_SIZE", new NumObject((uint)SDL.SDL_GLattr.SDL_GL_ACCUM_ALPHA_SIZE));
        BuiltinManager.GetInstance().Register("SDL_GL_STEREO", new NumObject((uint)SDL.SDL_GLattr.SDL_GL_STEREO));
        BuiltinManager.GetInstance().Register("SDL_GL_MULTISAMPLEBUFFERS", new NumObject((uint)SDL.SDL_GLattr.SDL_GL_MULTISAMPLEBUFFERS));
        BuiltinManager.GetInstance().Register("SDL_GL_MULTISAMPLESAMPLES", new NumObject((uint)SDL.SDL_GLattr.SDL_GL_MULTISAMPLESAMPLES));
        BuiltinManager.GetInstance().Register("SDL_GL_ACCELERATED_VISUAL", new NumObject((uint)SDL.SDL_GLattr.SDL_GL_ACCELERATED_VISUAL));
        BuiltinManager.GetInstance().Register("SDL_GL_RETAINED_BACKING", new NumObject((uint)SDL.SDL_GLattr.SDL_GL_RETAINED_BACKING));
        BuiltinManager.GetInstance().Register("SDL_GL_CONTEXT_MAJOR_VERSION", new NumObject((uint)SDL.SDL_GLattr.SDL_GL_CONTEXT_MAJOR_VERSION));
        BuiltinManager.GetInstance().Register("SDL_GL_CONTEXT_MINOR_VERSION", new NumObject((uint)SDL.SDL_GLattr.SDL_GL_CONTEXT_MINOR_VERSION));
        BuiltinManager.GetInstance().Register("SDL_GL_CONTEXT_EGL", new NumObject((uint)SDL.SDL_GLattr.SDL_GL_CONTEXT_EGL));
        BuiltinManager.GetInstance().Register("SDL_GL_CONTEXT_FLAGS", new NumObject((uint)SDL.SDL_GLattr.SDL_GL_CONTEXT_FLAGS));
        BuiltinManager.GetInstance().Register("SDL_GL_CONTEXT_PROFILE_MASK", new NumObject((uint)SDL.SDL_GLattr.SDL_GL_CONTEXT_PROFILE_MASK));
        BuiltinManager.GetInstance().Register("SDL_GL_SHARE_WITH_CURRENT_CONTEXT", new NumObject((uint)SDL.SDL_GLattr.SDL_GL_SHARE_WITH_CURRENT_CONTEXT));
        BuiltinManager.GetInstance().Register("SDL_GL_FRAMEBUFFER_SRGB_CAPABLE", new NumObject((uint)SDL.SDL_GLattr.SDL_GL_FRAMEBUFFER_SRGB_CAPABLE));
        BuiltinManager.GetInstance().Register("SDL_GL_CONTEXT_RELEASE_BEHAVIOR", new NumObject((uint)SDL.SDL_GLattr.SDL_GL_CONTEXT_RELEASE_BEHAVIOR));
        BuiltinManager.GetInstance().Register("SDL_GL_CONTEXT_RESET_NOTIFICATION", new NumObject((uint)SDL.SDL_GLattr.SDL_GL_CONTEXT_RESET_NOTIFICATION));

        BuiltinManager.GetInstance().Register("SDL_GL_CONTEXT_PROFILE_CORE", new NumObject((uint)SDL.SDL_GLprofile.SDL_GL_CONTEXT_PROFILE_CORE));
        BuiltinManager.GetInstance().Register("SDL_GL_CONTEXT_PROFILE_COMPATIBILITY", new NumObject((uint)SDL.SDL_GLprofile.SDL_GL_CONTEXT_PROFILE_COMPATIBILITY));
        BuiltinManager.GetInstance().Register("SDL_GL_CONTEXT_PROFILE_ES", new NumObject((uint)SDL.SDL_GLprofile.SDL_GL_CONTEXT_PROFILE_ES));

        BuiltinManager.GetInstance().Register("SDL_Init", (List<Object> args) =>
        {
            var ret = SDL.SDL_Init(SDL.SDL_INIT_EVERYTHING);
            var result = new NumObject(ret);
            return (true, result);
        });

        BuiltinManager.GetInstance().Register("SDL_Quit", (List<Object> args) =>
        {
            SDL.SDL_Quit();
            return (false, null);
        });

        BuiltinManager.GetInstance().Register("SDL_CreateWindow", (List<Object> args) =>
        {
            var window = SDL.SDL_CreateWindow(
                 ((StrObject)args[0]).value,
                 (int)((NumObject)((BuiltinObject)args[1]).GetBuiltinVar()).value,
                 (int)((NumObject)((BuiltinObject)args[2]).GetBuiltinVar()).value,
                 (int)((NumObject)args[3]).value,
                 (int)((NumObject)args[4]).value,
                 (SDL.SDL_WindowFlags)((NumObject)args[5]).value
                 );

            var builtinData = new BuiltinObject(new NativeData(window, (object? nativeData) =>
            {
                SDL.SDL_DestroyWindow((nint)nativeData);
            }));

            return (true, builtinData);
        });

        BuiltinManager.GetInstance().Register("SDL_PollEvent", (List<Object> args) =>
        {
            SDL.SDL_Event e = new SDL.SDL_Event();
            SDL.SDL_PollEvent(out e);
            BuiltinObject builtinData = new BuiltinObject(new NativeData(e, null));
            return (true, builtinData);
        });

        BuiltinManager.GetInstance().Register("SDL_GetEventType", (List<Object> args) =>
        {
            if (args[0].type != ObjectType.BUILTIN)
                Utils.Assert("Invalid builtin object.");
            var builtinData = (BuiltinObject)args[0];

            if (!(builtinData.GetNativeData().nativeData is SDL.SDL_Event))
                Utils.Assert("Invalid SDL_Event object.");

            SDL.SDL_Event e = (SDL.SDL_Event)builtinData.GetNativeData().nativeData;

            return (true, new NumObject((double)e.type));
        });

        BuiltinManager.GetInstance().Register("SDL_CreateRenderer", (List<Object> args) =>
       {
           if (args[0].type != ObjectType.BUILTIN)
               Utils.Assert("Invalid builtin data.");
           var builtinData = (BuiltinObject)args[0];

           if (!(builtinData.GetNativeData().nativeData is nint))
               Utils.Assert("Invalid SDL_Window object.");

           var window = builtinData.GetNativeData().nativeData;
           var renderer = SDL.SDL_CreateRenderer((nint)window, -1, SDL.SDL_RendererFlags.SDL_RENDERER_ACCELERATED | SDL.SDL_RendererFlags.SDL_RENDERER_PRESENTVSYNC);
           var r = new BuiltinObject(new NativeData(renderer, (object? nativeData) =>
           {
               SDL.SDL_DestroyRenderer((nint)nativeData);
           }));

           return (true, r);
       });

        BuiltinManager.GetInstance().Register("SDL_LoadBMP", (List<Object> args) =>
        {
            if (args[0].type != ObjectType.STR)
                Utils.Assert("Invalid str data.");

            var fullPath = BuiltinManager.GetInstance().ToFullPath(((StrObject)args[0]).value);

            var surface = SDL.SDL_LoadBMP(fullPath);

            var r = new BuiltinObject(new NativeData(surface, (object? nativeData) =>
            {
                SDL.SDL_FreeSurface((nint)nativeData);
            }));
           
            return (true, r);
        });

        BuiltinManager.GetInstance().Register("SDL_CreateTextureFromSurface", (List<Object> args) =>
       {
           if (args[0].type != ObjectType.BUILTIN || args[1].type != ObjectType.BUILTIN)
               Utils.Assert("Invalid builtin value of SDL_CreateTextureFromSurface(args[0] or args[1]).");
           var renderer = (nint)((BuiltinObject)args[0]).GetNativeData().nativeData;
           var surface = (nint)((BuiltinObject)args[1]).GetNativeData().nativeData;

           var texture = SDL.SDL_CreateTextureFromSurface(renderer, surface);

           var r = new BuiltinObject(new NativeData(texture, (object? nativeData) =>
           {
               SDL.SDL_DestroyTexture((nint)nativeData);
           }));
         
           return (true, r);
       });

        BuiltinManager.GetInstance().Register("SDL_RenderClear", (List<Object> args) =>
              {
                  if (args[0].type != ObjectType.BUILTIN)
                      Utils.Assert("Invalid builtin value of SDL_RenderClear(args[0]).");
                  var renderer = (nint)((BuiltinObject)args[0]).GetNativeData().nativeData;
                  SDL.SDL_RenderClear(renderer);

                  return (false, null);
              });

        BuiltinManager.GetInstance().Register("SDL_RenderCopy", (List<Object> args) =>
        {
            if (args[0].type != ObjectType.BUILTIN || args[1].type != ObjectType.BUILTIN)
                Utils.Assert("Invalid builtin value of SDL_RenderCopy(args[0] or args[1]).");
            var renderer = (nint)((BuiltinObject)args[0]).GetNativeData().nativeData;
            var texture = (nint)((BuiltinObject)args[1]).GetNativeData().nativeData;

            nint src = IntPtr.Zero;
            nint dst = src;

            var ret = SDL.SDL_RenderCopy(renderer, texture, src, dst);
            return (true, new NumObject(ret));
        });

        BuiltinManager.GetInstance().Register("SDL_RenderPresent", (List<Object> args) =>
              {
                  if (args[0].type != ObjectType.BUILTIN)
                      Utils.Assert("Invalid builtin value of SDL_RenderClear(args[0]).");
                  var renderer = (nint)((BuiltinObject)args[0]).GetNativeData().nativeData;
                  SDL.SDL_RenderPresent(renderer);

                  return (false, null);
              });

    }
}
