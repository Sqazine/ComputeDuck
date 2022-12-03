

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
1. C++ compiler(>=17)
2. CMake(>=3.10)
3. clone Microsoft's vcpkg C++ package manager(https://github.com/Microsoft/vcpkg)
4. create environment variables:VCPKG_ROOT=(vcpkg repo's root directory)
5. create environment variables:VCPKG_DEFAULT_TRIPLET=(current target machine's platform and instruction set architecture) (See also:https://vcpkg.io/en/docs/users/triplets.html)
6. install sdl2 via vcpkg(vcpkg install sdl2)

## 1. Run

1. Compile

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
    > a=10;println(a);

linux(C++):
    chmod 777 computeduck
    ./computeduck 
    > a=10;println(a); 
```
3. Run in source code file
```sh
window(C++):
    .\computeduck.exe examples/leetcode-twosum.cd

linux(C++):
    chmod 777 computeduck
    ./computeduck examples/leetcode-twosum.cd
```

## 2. Examples
1. Variable declarations
```sh
a=10;
b=a;
c;
b=20;
println(a);#10
println(b);#20
println(c);#"nil"
```
2. Function
```sh
add=function(x,y){
    return x+y;
};

c=add(1.000000,2.000000);
println(c);#3.000000

#native function
println("hello world!");#The function that outputs to the console

a=[1,2,3];#array
sizeof(a);#get the size of array
```
3. Array
```sh

add=function(vec1,vec2){
    return [vec1[0]+vec2[0],vec1[1]+vec2[1]];
};

sub=function(vec1,vec2){
    return [vec1[0]-vec2[0],vec1[1]-vec2[1]];
};

vec1=[3,3];
vec2=[2,2];

vec3=add(vec1,vec2);

println(vec3);#[5.000000,5.000000]
```

4. If-Statement
```sh
a=10;
b=a;
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
a=0;

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
    x:0,
    y:0
}
struct Vec3
{
    vec2:Vec2,
    z:0
}
struct Vec4
{
    vec3:Vec3,
    w:0
}
a=Vec4;
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
    v:0,
    next:nil,
}

head=Node;

e=head;
i=1;
while(i<10)
{
    e2=Node;
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
nums=[2,7,11,15];
target=9;

twosum=function(nums,target)
{
    i=0;
    j=i+1;
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
};

println(twosum(nums,target));#[0.000000,1.000000]
```
9. Fibonacci numbers
```sh
fib=function(x)
{
    if(x==0) 
        return 0;
    else if(x==1) 
        return 1;
    else 
        return fib(x-1)+fib(x-2);
};
a=fib(10);
println(a);#55
```

10. OOP simulate
```sh
struct ShapeVtbl
{
    area:nil,
}

struct Shape
{
    vptr:ShapeVtbl,
    super,
    x:0,
    y:0;
}

ShapeCtor=function(self,x,y)
{
    self.x=x;
    self.y=y;
    self.vptr.area=lambda(self)
    {
        return 0;
    };
};

ShapeArea=function(self)
{
    if(self.super==nil)
        return self.vptr.area(self);
    return self.super.vptr.area(self);
};

s1=Shape;
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
    super:Shape,
    width,
    height
}

RectangleCtor=function(self,x,y,w,h)
{
    ShapeCtor(self.super,x,y);
    self.width=w;
    self.height=h;
    self.super.vptr.area=lambda(self)
    {
        return self.width*self.height;
    };
};

r1=Rectangle;
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
a={
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


