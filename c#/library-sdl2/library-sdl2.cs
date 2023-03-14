using ComputeDuck;
using SDL2;
using Object = ComputeDuck.Object;
public class sdl2
{
    public static void RegisterBuiltins()
    {
        BuiltinManager.GetInstance().RegisterVariable("SDL_WINDOWPOS_CENTERED", new NumObject(SDL.SDL_WINDOWPOS_CENTERED));
        BuiltinManager.GetInstance().RegisterVariable("SDL_QUIT", new NumObject((double)SDL.SDL_EventType.SDL_QUIT));

        BuiltinManager.GetInstance().RegisterFunction("SDL_Init", (List<Object> args, out Object result) =>
        {
            var ret = SDL.SDL_Init(SDL.SDL_INIT_EVERYTHING);
            result = new NumObject(ret);
            return true;
        });

        BuiltinManager.GetInstance().RegisterFunction("SDL_Quit", (List<Object> args, out Object result) =>
        {
            SDL.SDL_Quit();
            result = new NilObject();
            return false;
        });

        BuiltinManager.GetInstance().RegisterFunction("SDL_CreateWindow", (List<Object> args, out Object result) =>
        {
            var builtinData = new BuiltinDataObject();
            var window = SDL.SDL_CreateWindow(
                 ((StrObject)args[0]).value,
                 (int)((NumObject)((BuiltinVariableObject)args[1]).obj).value,
                 (int)((NumObject)((BuiltinVariableObject)args[2]).obj).value,
                 (int)((NumObject)args[3]).value,
                 (int)((NumObject)args[4]).value,
                 0
                 );

            result = builtinData;
            return true;
        });

        BuiltinManager.GetInstance().RegisterFunction("SDL_PollEvent", (List<Object> args, out Object result) =>
        {
            SDL.SDL_Event e = new SDL.SDL_Event();
            SDL.SDL_PollEvent(out e);
            BuiltinDataObject builtinData = new BuiltinDataObject();
            builtinData.nativeData = e;
            builtinData.destroyFunc = null;
            result = builtinData;
            return true;
        });

        BuiltinManager.GetInstance().RegisterFunction("SDL_GetEventType", (List<Object> args, out Object result) =>
        {
            if (args[0].type != ObjectType.BUILTIN_DATA)
                Utils.Assert("Not a valid builtin data.");
            var builtinData = (BuiltinDataObject)args[0];

            if (!(builtinData.nativeData is SDL.SDL_Event))
                Utils.Assert("Not a valid SDL_Event object.");

            SDL.SDL_Event e = (SDL.SDL_Event)builtinData.nativeData;
            result = new NumObject((double)e.type);

            return true;
        });

        BuiltinManager.GetInstance().RegisterFunction("SDL_CreateRenderer", (List<Object> args, out Object result) =>
       {
           if (args[0].type != ObjectType.BUILTIN_DATA)
               Utils.Assert("Not a valid builtin data.");
           var builtinData = (BuiltinDataObject)args[0];

           if (!(builtinData.nativeData is nint))
               Utils.Assert("Not a valid SDL_Window object.");

           var window = builtinData.nativeData;
           var renderer = SDL.SDL_CreateRenderer((nint)window, -1, SDL.SDL_RendererFlags.SDL_RENDERER_ACCELERATED | SDL.SDL_RendererFlags.SDL_RENDERER_PRESENTVSYNC);
           var r = new BuiltinDataObject();
           r.nativeData = renderer;
           r.destroyFunc = (object nativeData) =>
           {
               SDL.SDL_DestroyRenderer((nint)nativeData);
           };

           result = r;
           return true;
       });

        BuiltinManager.GetInstance().RegisterFunction("SDL_LoadBMP", (List<Object> args, out Object result) =>
        {
            if (args[0].type != ObjectType.STR)
                Utils.Assert("Not a valid str data.");

            var surface = SDL.SDL_LoadBMP(((StrObject)args[0]).value);

            var r = new BuiltinDataObject();
            r.nativeData = surface;
            r.destroyFunc = (object nativeData) =>
                {
                    SDL.SDL_FreeSurface((nint)nativeData);
                };

            result = r;
            return true;
        });

        BuiltinManager.GetInstance().RegisterFunction("SDL_CreateTextureFromSurface", (List<Object> args, out Object result) =>
       {
           if (args[0].type != ObjectType.BUILTIN_DATA || args[1].type != ObjectType.BUILTIN_DATA)
               Utils.Assert("Not a valid builtin value of SDL_CreateTextureFromSurface(args[0] or args[1]).");
           var renderer = (nint)((BuiltinDataObject)args[0]).nativeData;
           var surface = (nint)((BuiltinDataObject)args[1]).nativeData;

           var texture = SDL.SDL_CreateTextureFromSurface(renderer, surface);

           var r = new BuiltinDataObject();
           r.nativeData = texture;
           r.destroyFunc = (object nativeData) =>
           {
               SDL.SDL_DestroyTexture((nint)nativeData);
           };

           result = r;
           return true;
       });

        BuiltinManager.GetInstance().RegisterFunction("SDL_RenderClear", (List<Object> args, out Object result) =>
              {
                  if (args[0].type != ObjectType.BUILTIN_DATA)
                      Utils.Assert("Not a valid builtin value of SDL_RenderClear(args[0]).");
                  var renderer = (nint)((BuiltinDataObject)args[0]).nativeData;
                  SDL.SDL_RenderClear(renderer);

                  result = new NilObject();
                  return true;
              });

        BuiltinManager.GetInstance().RegisterFunction("SDL_RenderCopy", (List<Object> args, out Object result) =>
        {
            if (args[0].type != ObjectType.BUILTIN_DATA || args[1].type != ObjectType.BUILTIN_DATA)
                Utils.Assert("Not a valid builtin value of SDL_RenderCopy(args[0] or args[1]).");
            var renderer = (nint)((BuiltinDataObject)args[0]).nativeData;
            var texture = (nint)((BuiltinDataObject)args[0]).nativeData;

            uint format = 0;
            int access = 0;
            int width = 0;
            int height = 0;

            SDL.SDL_QueryTexture(texture, out format, out access, out width, out height);

            SDL.SDL_Rect dstrect;
            dstrect.x = 0;
            dstrect.y = 0;
            dstrect.w = width;
            dstrect.h = height;
            SDL.SDL_Rect srcrect = dstrect;

            var ret = SDL.SDL_RenderCopy(renderer, texture,ref srcrect,ref dstrect);
            result = new NumObject(ret);
            return true;
        });

        BuiltinManager.GetInstance().RegisterFunction("SDL_RenderPresent", (List<Object> args, out Object result) =>
              {
                  if (args[0].type != ObjectType.BUILTIN_DATA)
                      Utils.Assert("Not a valid builtin value of SDL_RenderClear(args[0]).");
                  var renderer = (nint)((BuiltinDataObject)args[0]).nativeData;
                  SDL.SDL_RenderPresent(renderer);

                  result = new NilObject();
                  return true;
              });

    }
}
