

# ComputeDuck

A toy-level C-like syntax Scripting language  

<!-- PROJECT SHIELDS -->

[![Contributors][contributors-shield]][contributors-url]
[![Forks][forks-shield]][forks-url]
[![Stargazers][stars-shield]][stars-url]
[![Issues][issues-shield]][issues-url]
[![Apache-2.0 License][license-shield]][license-url]

<!-- PROJECT LOGO -->
<br />

[English](https://github.com/Sqazine/ComputeDuck/blob/master/README-EN.md)
[中文](https://github.com/Sqazine/ComputeDuck/blob/master/README.md)
## Environment
C++:
1. C++ compiler(>=17)
2. CMake(>=3.10)

Python:
1. Install python development environment(The authors use version 3.9.7, other versions are not tested)
## 1. Run

1. Compile(C++ only)

```sh
git clone https://github.com/Sqazine/ComputeDuck.git
cd computeduck
mkdir build
cd build 
cmake ..
cmake -build .
```

2. Run in command-line
```sh
window(C++):
    .\computeduck.exe  
    > var a=10;println(a);

linux(C++):
    chmod 777 computeduck
    ./computeduck 
    > var a=10;println(a); 

window or linux(Python):
    cd py
    python main.py
```
3. Run in source code file
```sh
window(C++):
    .\computeduck.exe examples/leetcode-twosum.cd


linux(C++):
    chmod 777 computeduck
    ./computeduck examples/leetcode-twosum.cd

window或者linux(Python):
    cd py
    python main.py examples/leetcode-twosum.cd
```

## 2. Examples
1. Variable declarations
```sh
var a=10;
var b=a;
var c;
b=20;
println(a);#10
println(b);#20
println(c);#"nil"
```
2. Function
```sh
function add(x,y){
    return x+y;
}

var c=add(1.000000,2.000000);
println(c);#3.000000

#native function
println("hello world!");#The function that outputs to the console

var a=[1,2,3];#array
sizeof(a);#get the size of array
```
3. Array
```sh

function add(vec1,vec2){
    return [vec1[0]+vec2[0],vec1[1]+vec2[1]];
}

function sub(vec1,vec2){
    return [vec1[0]-vec2[0],vec1[1]-vec2[1]];
}

var vec1=[3,3];
var vec2=[2,2];

var vec3=add(vec1,vec2);

println(vec3);#[5.000000,5.000000]
```

4. If-Statement
```sh
var a=10;
var b=a;
b=20;
println(a);#10.000000
println(b);#20.000000

if(b<a)
    b=a;
else 
{
    a=b;
}

println(a);#20.000000
println(b);#20.000000


if(a>100)
    a=100;
else if(a>50)
    a=50;
else if(a>30)
    a=30;
else a=5; 

println(a);#5.000000

if(b>a)
   if(a==100)
        a=1000;
    else 
        a=500;
else 
    a=300;

println(a);#500.000000
```

5. Loop
```sh
var a=0;

while(a<100)
{
    println(a);
    a=a+1;
}

# 0.000000
#...
#...
#...
# 99.000000
```

6. Struct
```sh
struct Vec2
{
    var x=0;
    var y=0;
}
struct Vec3
{
    var vec2=Vec2;
    var z=0;
}
struct Vec4
{
    var vec3=Vec3;
    var w=0;
}
var a=Vec4;
a.vec3.vec2.x=1000;
println(a);
#struct
#{
#    vec3=struct
#         {
#            z=0.000000
#           vec2=struct
#                 {
#                    x=1000.000000
#                    y=0.000000
#                 }
#         }
#   w=0.000000
#}
println(a.vec3.vec2.x);# 1000.000000
```

7. LinkedList
```sh
struct Node
{
    var v=0;
    var next=nil;
}

var head=Node;

var e=head;
var i=1;
while(i<10)
{
    var e2=Node;
    e2.v=i;

    e.next=e2;

    e=e.next;

    i=i+1;
}

println(head);

#struct{
#    v=0.000000
#    next=struct
#        {
#            v=1.000000
#            next=struct
#                {
#                    v=2.000000
#                    next=struct
#                        {
#                            v=3.000000
#                            next=struct
#                                {
#                                    v=4.000000
#                                    next=struct
#                                        {
#                                            v=5.000000
#                                            next=struct
#                                                {
#                                                    v=6.000000
#                                                    next=struct
#                                                        {
#                                                            v=7.000000
#                                                            next=struct
#                                                                {
#                                                                    v=8.000000
#                                                                    next=struct
#                                                                        {
#                                                                            v=9.000000
#                                                                            next=nil
#                                                                        }
#                                                                }
#                                                        }
#                                                }
#                                        }
#                                }
#                        }
#                }
#        }
#}
```

8. leetcode's two-sum example
```sh
var nums=[2,7,11,15];
var target=9;

function twosum(nums,target)
{
    var i=0;
    var j=i+1;
    println(j);

    while(i<sizeof(nums)-1)
    {
        j=i+1;
        while(j<sizeof(nums))
        {
            println(nums[i]);
            println(nums[j]);
            if(nums[i]+nums[j]==target)
                return [i,j];
            j=j+1;
        }
        i=i+1;
    }
}

println(twosum(nums,target));#[0.000000,1.000000]
```
9. Fibonacci numbers
```sh
function fib(x)
{
    if(x==0) 
        return 0;
    else if(x==1) 
        return 1;
    else 
        return fib(x-1)+fib(x-2);
}
var a=fib(10);
println(a);#55
```

10. lambda
```sh
var lam=lambda()
{
    return 10;
};

var x=lam();

println(x);#10

struct Vec2
{
    var length=lambda(x,y)
    {
        return x*x+y*y;
    };
} 
struct Vec3
{
    var super=Vec2;
} 
var a=Vec3;
var b=a.super.length(10,8);
println(b);    #164
```

11. OOP simulate
```sh
struct ShapeVtbl
{
    var area=nil;
}

struct Shape
{
    var vptr=ShapeVtbl;
    var super=nil;
    var x=0;
    var y=0;
}

function ShapeCtor(self,x,y)
{
    self.x=x;
    self.y=y;
    self.vptr.area=lambda(self)
    {
        return 0;
    };
}

function ShapeArea(self)
{
    if(self.super==nil)
        return self.vptr.area(self);
    return self.super.vptr.area(self);
}

var s1=Shape;
ShapeCtor(ref s1,10,10);
println(s1);#struct instance Shape:
            #   vptr=struct instance ShapeVtbl:
            #       area=lambda:0
            #super=nil
            #x=10.0
            #y=10.0
println(ShapeArea(s1));#0.0


struct Rectangle
{
    var super=Shape;
    var width;
    var height;
}

function RectangleCtor(self,x,y,w,h)
{
    ShapeCtor(self.super,x,y);
    self.width=w;
    self.height=h;
    self.super.vptr.area=lambda(self)
    {
        return self.width*self.height;
    };
}

var r1=Rectangle;
RectangleCtor(ref r1,10,10,3,5);
println(r1);#struct instance Rectangle:
            #super=struct instance Shape:
            #        vptr=struct instance ShapeVtbl:
            #                area=lambda:1
            #        super=nil
            #        x=10.0
            #        y=10.0
            #width=3.0
            #height=5.0

println(ShapeArea(r1));#15
```

12. AnonymousStruct(similar to javascript's Object)
```sh
var a={
    x:10,
    y:20
};

println(a);
#struct instance:
#{
#y:20.000000
#x:10.000000
#}

```

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


