

# ComputeDuck

A C-like syntax Scripting toy language  

<!-- PROJECT SHIELDS -->
[![Contributors][contributors-shield]][contributors-url]
[![Forks][forks-shield]][forks-url]
[![Stargazers][stars-shield]][stars-url]
[![Issues][issues-shield]][issues-url]
[![Apache-2.0 License][license-shield]][license-url]
<!-- PROJECT LOGO -->
<br />

## Environment
1. C++ compiler(>=17)
2. CMake(>=3.10)
3. Python(>=3.10)
4. C#(>=.NetCore 9.0)

### How to Build

#### C++ build:
```sh
#on my ubuntu wsl2 environment,i need to install this(opengl and x11) to run sdl2 and opengl example
sudo apt-get install build-essential
sudo apt-get install libgl1-mesa-dev
sudo apt-get install xorg
sudo apt-get install xauth
sudo apt-get install openbox
sudo apt-get install xserver-xorg-legacy
sudo apt install x11-apps -y
apt install libx11-dev libxext-dev libxtst-dev libxrender-dev libxmu-dev libxmuu-dev

# C++ build:
git clone https://github.com/Sqazine/ComputeDuck.git
mkdir build
cd build 
cmake ..
cmake -build .
```

If you want to build opengl example,you need to open COMPUTEDUCK_BUILD_WITHOPENGL:
```sh
cmake -DCOMPUTEDUCK_BUILD_WITHOPENGL=ON ..
```

If you want to build cdsdl2(SDL2 C++ binding library),you need to:
1. Download SDL2.28.2 from [github](https://github.com/libsdl-org/SDL/archive/refs/tags/release-2.28.2.zip)
2. Extract SDL-release-2.28.2.zip
3. cmake variable COMPUTEDUCK_BUILD_WITH_SDL2=ON,SDL2_ROOT_DIR=yourpath/to/SDL-release-2.28.2,like:

```sh
cmake -DCOMPUTEDUCK_BUILD_WITH_SDL2=ON -DSDL2_ROOT_DIR=yourpath/to/SDL-release-2.28.2
```

If you want to build JIT(LLVM implementation):

1. download llvm 14.x from [github](https://github.com/llvm/llvm-project/archive/refs/heads/release/14.x.zip) or [gitee(zh-cn)](https://gitee.com/mirrors/LLVM/repository/archive/release/14.x.zip)
2. Extract llvm-release-14.x.zip
3. cmake variable COMPUTEDUCK_BUILD_WITH_LLVM=ON,LLVM_ROOT_DIR=yourpath/to/llvm-release-14.x,like:

```sh
cmake -DCOMPUTEDUCK_BUILD_WITH_LLVM=ON -DLLVM_ROOT_DIR=yourpath/to/llvm-release-14.x
```

(MY OPINION:Why did I choose such a cumbersome third-party library reference method, because I think LLVM and SDL are both options for ComputeDuck projects and should not be forced to be submodules)

#### Python build:
```sh
#dependencies
# for SDL2 and opengl external libraries
pip install pysdl2-dll==2.30.2
pip install PySDL2==0.9.16
pip install PyOpenGL==3.1.6
pip install PyOpenGL_accelerate==3.1.6

# or using requirements.txt
pip install -r otherImpl/python/requirements.txt

#in terminal:
#execute source file
python3 main.py -f examples/array.cd.
#repl mode
python3 main.py
```

#### C#(.NetCore 9.0) build:
```sh
#install .NetCore 9.0
#just open otherImpl/c#/ComputeDuck.sln
# all external dependencies will be downloaded automatically.
```

## Features

Variable declaration

Compound Statement

Reference

Function

Upvalue

Array

If-else

Loop

Struct

Anonymous Struct(similar to javascript's Object)

File import

C++ dynamic library import

Jit(LLVM implementation)

## Examples

#### [Create window and show an image using SDL2](examples/sdl2.cd)
```sh
dllimport("sdl2");

ok=SDL_Init(SDL_INIT_VIDEO);
if(ok<0)
    println("Failed to init sdl2!");
window=SDL_CreateWindow("First Window",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,800,600);
isRunning=true;

renderer=SDL_CreateRenderer(window);

if(renderer==nil)
    println("Failed to create renderer.");

surface=SDL_LoadBMP("examples/hello.bmp");

if(surface==nil)
    println("Failed to load bmp image.");

texture=SDL_CreateTextureFromSurface(renderer,surface);

SDL_RenderClear(renderer);

SDL_RenderCopy(renderer,texture);

SDL_RenderPresent(renderer);

while(isRunning)
{
  event=SDL_PollEvent();
  if(event!=nil)
  {
      if(SDL_GetEventType(event)==SDL_QUIT)
      {
          isRunning=false;
      }
  }
}

SDL_Quit();
```

![](screenshots/Image.png)

#### [SDL2 and opengl example](examples/sdl2-opengl.cd)
```sh
dllimport("cdsdl2");
dllimport("cdopengl");

ok=SDL_Init(SDL_INIT_VIDEO);
if(ok<0)
    println("Failed to init sdl2!");
window=SDL_CreateWindow("sdl2-opengl",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,800,600,SDL_WINDOW_SHOWN|SDL_WINDOW_RESIZABLE|SDL_WINDOW_ALLOW_HIGHDPI|SDL_WINDOW_OPENGL);
isRunning=true;

positions=[-0.5,-0.5,0.0,
            0.5,-0.5,0.0,
            0.0,0.5,0.0];

colors=[1.0,0.0,0.0,
        0.0,1.0,0.0,
        0.0,0.0,1.0];

indices=[0,1,2];

vertShaderSrc="#version 330 core
            layout (location = 0) in vec3 position;
            layout (location = 1) in vec3 vColor;
            out vec3 color;
            void main()
            {
                gl_Position = vec4(position.x, position.y, position.z, 1.0);
                color=vColor;
            }";

fragShaderSrc="#version 330 core
            out vec4 FragColor;
            in vec3 color;
            void main()
            {
                FragColor = vec4(color.xyz,1.0);
            }";

SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
#not work on WSL2 linux subsystem,need to comment the follow two lines
SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 1);

sdlGLCTX = SDL_GL_CreateContext(window);
success = SDL_GL_SetSwapInterval(1);

if (gladLoadGL()!=1)
	println("failed to load opengl");

vao=0;
glGenVertexArrays(1,ref vao);
glBindVertexArray(vao);

vbo=0;
glGenBuffers(1,ref vbo);
glBindBuffer(GL_ARRAY_BUFFER, vbo);  
glBufferData(GL_ARRAY_BUFFER, sizeof(positions)*4, positions, GL_STATIC_DRAW);
glBindBuffer(GL_ARRAY_BUFFER, 0);  

color_vbo=0;
glGenBuffers(1,ref color_vbo);
glBindBuffer(GL_ARRAY_BUFFER, color_vbo);  
glBufferData(GL_ARRAY_BUFFER, sizeof(colors)*4, colors, GL_STATIC_DRAW);
glBindBuffer(GL_ARRAY_BUFFER, 0);  

ebo=0;
glGenBuffers(1,ref ebo);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);  
glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices)*4, indices, GL_STATIC_DRAW);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);  

glBindBuffer(GL_ARRAY_BUFFER, vbo);  
glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*4,nil);
glEnableVertexAttribArray(0);

glBindBuffer(GL_ARRAY_BUFFER, color_vbo);  
glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,3*4,nil);
glEnableVertexAttribArray(1);

glBindBuffer(GL_ARRAY_BUFFER, 0); 
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); 
glBindVertexArray(0);

vertShader=glCreateShader(GL_VERTEX_SHADER);
glShaderSource(vertShader, 1, ref vertShaderSrc, nil);
glCompileShader(vertShader);

fragShader=glCreateShader(GL_FRAGMENT_SHADER);
glShaderSource(fragShader, 1, ref fragShaderSrc, nil);
glCompileShader(fragShader);

shaderProgram = glCreateProgram();
glAttachShader(shaderProgram, vertShader);
glAttachShader(shaderProgram, fragShader);
glLinkProgram(shaderProgram);

while(isRunning)
{
  event=SDL_PollEvent();
  if(event!=nil)
  {
      if(SDL_GetEventType(event)==SDL_QUIT)
      {
          isRunning=false;
      }
  }

  glClearColor(0.2, 0.3, 0.5, 1.0);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  glUseProgram(shaderProgram);
  glBindVertexArray(vao);
  glDrawElements(GL_TRIANGLES, sizeof(indices), GL_UNSIGNED_INT, ref indices);

  SDL_GL_SwapWindow(window);
}

SDL_Quit();
```

![](screenshots/Image2.png)

## 4. License

This project is licensed under the Apache-2.0 License, see the details[LICENSE](https://github.com/Sqazine/ComputeDuck/blob/main/LICENSE)

<!-- links -->
[contributors-shield]: https://img.shields.io/github/contributors/Sqazine/ComputeDuck.svg?style=flat-square
[contributors-url]: https://github.com/Sqazine/ComputeDuck/graphs/contributors
[forks-shield]: https://img.shields.io/github/forks/Sqazine/ComputeDuck.svg?style=flat-square
[forks-url]: https://github.com/Sqazine/ComputeDuck/network/members
[stars-shield]: https://img.shields.io/github/stars/Sqazine/ComputeDuck.svg?style=flat-square
[stars-url]: https://github.com/Sqazine/ComputeDuck/stargazers
[issues-shield]: https://img.shields.io/github/issues/Sqazine/ComputeDuck.svg?style=flat-square
[issues-url]: https://img.shields.io/github/issues/Sqazine/ComputeDuck.svg
[license-shield]: https://img.shields.io/github/license/Sqazine/ComputeDuck.svg?style=flat-square
[license-url]: https://github.com/Sqazine/ComputeDuck/blob/master/LICENSE