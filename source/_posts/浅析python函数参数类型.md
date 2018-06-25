---
title: 浅析python函数参数类型
tags:
  - python
categories:
  - 编程
date: 2016-05-03 22:53:07
---

在python函数定义时，截止目前到3.x的版本，一共有五种类型的参数：(注：该文章基于python 3.4.4)

<!-- more -->

 1. POSITIONAL_OR_KEYWORD（位置或关键字参数）
 2. POSITIONAL_ONLY（仅位置参数）
 3. KEYWORD_ONLY（仅关键字参数）
 4. VAR_POSITIONAL（任意数量的位置参数）
 5. VAR_KEYWORD（任意数量的关键字参数）

第一种：位置或关键字参数：
先上文档的定义：
>  * Parameter.POSITIONAL_OR_KEYWORD - value may be supplied as either a keyword or positional argument (this is the standard binding behaviour for functions implemented in Python.)

意思就是该参数，运行两种方式的传递：位置和关键字。
```python
def foo(arg1, arg2):
    print(arg1, arg2)
# 以位置参数传递
foo('one', 'two')
# output:
one two

# 以关键字参数传递：
foo(arg1='one', arg2='two')
foo(arg2='two', arg1='one')
# output:
one two
one two

# 混合方式：
foo('one', arg2='two')
# output:
one two

# 注意：关键字参数不能先于位置参数被传递
foo(arg1='one', 'two')
# output:
	foo(arg1='one', 'two')
SyntaxError: non-keyword arg after keyword arg
```

第二种：仅位置参数：

>    * Parameter.POSITIONAL_ONLY - value must be supplied as a positional argument.
>    Python has no explicit syntax for defining positional-only parameters, but many built-in and extension module functions (especially those that accept only one or two parameters) accept them.

意思就是：这类仅位置参数只允许接收位置参数传递，目前还没有能够让开发者自行定义接收该参数的函数，简而言之，这一般会在python内建函数中出现，如：
``` 
abs(-1)# 1
abs(a='-1')# TypeError: abs() takes no keyword arguments
```

第三种：仅关键字参数：
>    * Parameter.KEYWORD_ONLY - value must be supplied as a keyword argument. Keyword only parameters are those which appear after a "*" or "*args" entry in a Python function definition.

意思就是：该参数只能通过关键字方式传递，且定义在 " * " 或 " *args "后面。例如：
```python
def foo(arg1, *, arg2):
	print(arg1, arg2)

foo('one', arg2='two')# one two

foo('one', 'two')# TypeError: foo() takes 1 positional argument but 2 were given

foo('one')# TypeError: foo() missing 1 required keyword-only argument: 'arg2'

# 综上：当你定义了一个仅关键字参数时，必须传入该参数并且只能用关键字方式传递，即键值对方式。

```

第四种：任意数量的位置参数
>    * Parameter.VAR_POSITIONAL - a tuple of positional arguments that aren't bound to any other parameter. This corresponds to a "*args" parameter in a Python function definition.

这种参数就是一个星号" * "的参数了，就是可以接受任意数目的参数，这些参数将会被放在元组内提供给函数。
``` python
def foo(arg1, arg2, *arg3):
	print(arg1, arg2, arg3)

foo('doc', 'txt', 'png', 'exe', 'jpeg')# doc txt ('png', 'exe', 'jpeg')

# 若传入一个元祖提供给 *arg3 呢？
foo('doc', 'txt', ('png', 'exe', 'jpeg'))# doc txt (('png', 'exe', 'jpeg'),)

# 所以要用*tuple的方式传入元祖给 *arg3
foo('doc', 'txt', *('png', 'exe', 'jpeg'))# doc txt ('png', 'exe', 'jpeg')
```

第五种：任意数量的关键字参数：
>    * Parameter.VAR_KEYWORD - a dict of keyword arguments that aren't bound to any other parameter. This corresponds to a "**kwargs" parameter in a Python function definition.

这种参数就是两个星号"**"的参数了，意思就是把上面那种单个星星参数可以接受位置参数的方式，变化成只能接受键值对的方式即关键字方式传入，并放在字典内提供给函数：
```python
def foo(arg1, arg2, **arg3):
	print(arg1, arg2, arg3)

foo('doc', 'txt', a='png', b='jpeg')# doc txt {'a': 'png', 'b': 'jpeg'}

# 同单个星号一样，若需要传入字典呢？
foo('doc', 'txt', {'a':'png', 'b'='jpeg'})# TypeError: foo() takes 2 positional arguments but 3 were given

# 用*试试？
foo('doc', 'txt', *{'a':'png', 'b'='jpeg'})# TypeError: foo2() takes 2 positional arguments but 4 were given

# 用**试试
foo('doc', 'txt', **{'a':'png', 'b'='jpeg'})# doc txt {'a': 'png', 'b': 'jpeg'}

# 综上，当需要将字典传入任意数量的关键字参数时，需要用**dict的方式传入。
```

总结：部分引用自该博主的[博客](http://blog.useasp.net/archive/2014/06/23/the-python-function-or-method-parameter-types.aspx)
>总结：五种参数形式中仅有第二种Python没有提供定义的方法，其他四种在定义的时候也需要注意，定义的时候应该根据Python的解析规律进行定义，其中：

>位置或关键字参数（POSITIONAL_OR_KEYWORD）应该在最前面，其中，没有默认值的应该在有默认值的参数前面

>仅关键字参数（KEYWORD_ONLY）应放在任意数量位置参数（VAR_POSITIONAL）后面	

>任意数量位置参数（VAR_POSITIONAL）应该放在所有位置或关键字参数的后面

>任意数量关键字参数（VAR_KEYWORD）应该放在任意数量位置参数的后面

>*顺序如下（POSITIONAL_OR_KEYWORD, VAR_POSITIONAL,   KEYWORD_ONLY,  VAR_KEYWORD）

>**注意：任意数量位置参数和任意数量关键字参数只能在定义中定义一次。

```python
def func(arg1, arg2='default', *args, **kwargs):
    out= "arg1=%s, arg2=%s, args=%s, kwargs=%s" % (arg1, arg2, args, kwargs)
    print(out)
 
 
func(1) # correct
func(1,2) # correct
func(1,2,3,4) # correct
func(1,2,3,4,x=1,y=2) # correct
func(1,2,x=1) # correct
 
func(x=1) # wrong
func(arg1=1) # correct
func(1,x=1) # correct

# output
arg1=1, arg2=default, args=(), kwargs={}
arg1=1, arg2=2, args=(), kwargs={}
arg1=1, arg2=2, args=(3, 4), kwargs={}
arg1=1, arg2=2, args=(3, 4), kwargs={'y': 2, 'x': 1}
arg1=1, arg2=2, args=(), kwargs={'x': 1}
arg1=1, arg2=default, args=(), kwargs={}
wrong
arg1=1, arg2=default, args=(), kwargs={'x': 1}

# 而加上仅位置参数就简单了，那就是每次都必须传入仅位置参数，否则会报错：
def func(arg1, arg2='default', *args, arg3, **kwargs):
    out= "arg1=%s, arg2=%s, args=%s, arg3=%s, kwargs=%s" % (arg1, arg2, args, arg3, kwargs)
    print(out)

func(1)# wrong
func(1, arg3=2)# right
func(1, 2, 3, 4, arg3=5, x=1,y=2)# right
# output
wrong
arg1=1, arg2=default, args=(), arg3=2, kwargs={}
arg1=1, arg2=2, args=(3, 4), arg3=5, kwargs={'y': 2, 'x': 1}
```
