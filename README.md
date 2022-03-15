

# ComputeDuck

玩具级类C语法的脚本语言

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

## 环境
C++:
1. C++ 编译器(>=17)
2. CMake(>=3.10)

Python:
1. 安装python开发环境(作者使用3.9.7版本,其他版本未测试)
## 1. 运行

1. 编译(仅C++)

```sh
git clone https://github.com/Sqazine/ComputeDuck.git
cd computeduck
mkdir build
cd build 
cmake ..
cmake -build .
```

2. 命令行运行
```sh
window(C++):
    .\computeduck.exe  
    > var a=10;println(a);

linux(C++):
    chmod 777 computeduck
    ./computeduck 
    > var a=10;println(a); 

window或者linux(Python):
    cd py
    python main.py
```
3. 源码文件运行
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

## 2. 例子
1. 变量声明
```sh
var a=10;
var b=a;
var c;
b=20;
println(a);#10
println(b);#20
println(c);#"nil"
```
2. 函数
```sh
fn add(x,y){
    return x+y;
}

var c=add(1.000000,2.000000);
println(c);#3.000000

#原生函数
println("hello world!");#输出到控制台的函数

var a=[1,2,3];#数组
sizeof(a);#获取数组个数
```
3. 数组
```sh

fn add(vec1,vec2){
    return [vec1[0]+vec2[0],vec1[1]+vec2[1]];
}

fn sub(vec1,vec2){
    return [vec1[0]-vec2[0],vec1[1]-vec2[1]];
}

var vec1=[3,3];
var vec2=[2,2];

var vec3=add(vec1,vec2);

println(vec3);#[5.000000,5.000000]
```

4. 条件语句
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

5. 循环
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

6. 结构体
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

7. 链表模拟
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

8. leetcode 两数之和
```sh
var nums=[2,7,11,15];
var target=9;

fn twosum(nums,target)
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
9. 斐波那契数列
```sh
fn fib(x)
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
10. lambda函数
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

11. 面向对象模拟
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

fn ShapeCtor(self,x,y)
{
    self.x=x;
    self.y=y;
    self.vptr.area=lambda(self)
    {
        return 0;
    };
}

fn ShapeArea(self)
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

fn RectangleCtor(self,x,y,w,h)
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

## 3. 特性
1. 基本语法

``` sh
1. 每个语句后跟一个分号';',与C语言类似:
var a=10;

2. 使用 '#'表示单行注释,不支持多行注释:
#var a=10;

3. 标识符以字母 A-Z 或 a-z 或下划线 _ 开始,后跟零个或多个字母,下划线和数字(0-9),不允许出现标点字符,比如 @,$和%,也不允许以字母为开头的标识符:
var _a=10;
var Aa=10;
var 9a=10;#非法,不能以数字作变量名开头
var a;#a默认赋nil值

4.关键字:
var : 声明变量
fn : 声明函数
struct : 声明结构体
if : 条件语句
else : 条件语句否定分支(与if一起用,不可单独使用)
while : 循环语句
return : 返回语句(可以带一个或零个参数)
true : 表示真值
false : 表示假值
nil : 表示空
and : 逻辑与
or : 逻辑或
```
2. 变量类型
```sh
  var a=10; 数值类型(使用double的C++类型)
  var a="string"; 字符串类型
  var a=true/false; 布尔类型
  var a=[1,2,3]; 数组类型
  struct Vec2{var x=0;var y=0;} var a=Vec2;结构体类型
```

3. 运算符
```sh
1. 算术运算符:
    a+b;
    a-b;
    a*b;
    a/b;
2. 关系运算符:
    a==b;
    a!=b;
    a>b;
    a>=b;
    a<b;
    a<=b;
3. 赋值运算符:
    a=b;
```

4. 条件语句(与C语言类似)

```sh
1. 单if语句:
    if(a<b)
        return a;
   或者
   if(a<b)
   {
       return a;
   }

2. if-else语句:
    if(a<b)
        return a;
    else return b;
    或者
    if(a<b)
    {
        return a;
    }
    else 
    {
        return b;
    }

3. if-else if-else语句:
    if(a>10)
        return 10;
    else if(a>5)
        return 5;
    else return 0;

4. dangling-else(else与最接近的if语句匹配):
    if(a<b)
        if(a<10)
            return 10;
        else 
            return a;
    else 
        return b;
```

5. 作用域({}内为一个局部作用域)
```sh
var a=10;#全局变量

{
    var a=100;#局部变量
    println(a);#100
}

println(a);#10
```

6. 循环(仅支持while循环)
```sh
while(true)
{
    #多个执行语句
}
或则和
while(true)
    #单条执行语句
```

7. 函数
```sh
fn add(x,y)
{
    return x+y;
}

var a=add(1,2);
```

8. 数组
```sh
var a=[1,2,3];#以[]表示一个数组,内部元素以','分隔
println(a[0]);#以 数组变量+'['+数值变量+']'表示数组的索引
println(a[10]);#非法的索引会报Index out of array range错误
```


9. 结构体
```sh
struct Vec2
{
    var x=0;
    var y=0;
}

struct Vec3
{
    var vec2=Vec2;#支持结构体嵌套
    var z=0;
}

var a=Vec3;
a.vec2.x=1000;#支持结构体成员赋值
println(a.vec2.x);#支持结构体成员获取值
```

10. 引用
```sh
var a=10;
var b=ref a;
println(b);# 10
b=100;#通过引用修改原对象值
println(a);#100

a=[1,2,3,4,5];
println(b);#[1,2,3,4,5]

var c=ref a[0];#错误,仅能引用显式对象,不能引用数组内对象

struct Vec2
{
    var x=0;
    var y=0;
}

a=Vec2;
println(b);#struct instance Vec2: x=0.0 y=0.0 可引用结构体对象

fn resetVec2(v)
{
    #在函数中修改引用对象实参的值
    v.x=100;
    v.y=100;
}

resetVec2(ref a);
println(b);#struct instance Vec2: x=100.0 y=100.0 
```

11. lambda函数
```sh
var lam=lambda()#将函数赋值给变量lam
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
var b=a.super.length(10,8);#支持调用结构体中的lambda
println(b);    #164
```

## 4. 版权说明

该项目签署了 Apache-2.0 License 授权许可,详情请参阅 [LICENSE](https://github.com/Sqazine/ComputeDuck/blob/main/LICENSE)

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


