---
title: 浅析python中的yield机制
tags:
  - python
categories:
  - 编程
date: 2016-04-16 12:59:21
---

最近在看python中关于协程的部分，看到[廖雪峰的官方网站](http://www.liaoxuefeng.com/wiki/0014316089557264a6b348958f449949df42a6d3a2e542c000/001432090171191d05dae6e129940518d1d6cf6eeaaa969000)中关于消费者和生产者那一部分有些困惑，于是自己研究了一下，有了以下了解：

<!-- more -->
代码：

```python
def consumer():
    r = 'a'
    while True:
        print('there')
        n = yield r
        print('here')
        if not n:
            return
        print('[CONSUMER] Consuming %s...' % n)
        r = '200 OK'


def produce(c):
    c.send(None)
    n = 0
    while n < 5:
        n = n + 1
        print('[PRODUCER] Producing %s...' % n)
        r = c.send(n)
        print('[PRODUCER] Consumer return: %s' % r)
    c.close()
```

首先我们先初始化一个consumer：
`c = consumer()`

注意：由于consumer()中包含了yield，那么consumer()就会自动变成一个迭代器generator，所以首先我们要启动这个迭代器：
`c.send(None)`

注意：这里的send(None)相当于next()，但是你不能通过send(1)，传入参数来启动迭代器，会报错：
```python
c.send(1)
TypeError: can't send non-None value to a just-started generator
```
接下来看第一种情况：
```python
c = consumer()
c.send(None)

#输出
there

#说明：初始化迭代器后，执行完yield语句后终止
n = yield r
```
那么换个方式将c.send(None)打印出来会是什么值呢？
```python
print(c.send(None))

#输出
there
a

#说明：初始化迭代器，c.send(None)自动返回yield r的值，即当前迭代器迭代r的值，由于一开始赋值r为'a'，所以当前迭代器迭代到r的值是'a'。
```
接下来继续：
```python
c = consumer()
c.send(None)
print(c.send(1))

#输出
there
here
[CONSUMER] Consuming 1...
there
200 OK

#说明:
1.初始化迭代器后，再通过c.send(1)来调用迭代器时，代码会从上一次迭代停止的地方继续执行，即n = yield r。换句话说，初始化迭代器意味着，迭代器将起点设置在yield r所在的语句，以后每一次调用迭代器都会从这里开始。

2.n = yield r这一句意味着，当调用迭代器时，将当前迭代的值r赋给n, 而c.send(1)返回的的值是r的值，因为r是迭代器所迭代的变量，我们在循环最后执行了r= '200 ok'，那么c.send(1)返回的就是200 OK了。

3.那为什么会输出there呢？那是因为while循环的原因，这是个无限循环，除非你再调用一次c.send(None)来终止迭代器迭代引发StopIteration，或者调用c.close()，否则会一直循环，又回到了yield r所在的语句，等待下一次的调用迭代。
```
继续：(假如你能猜到将会输出什么结果，那恭喜呢你就掌握了yield的运作机制了！)
```python
c = consumer()
c.send(None)
print(c.send(c.send(1)))

#输出
there
here
[CONSUMER] Consuming 1...
there
here
[CONSUMER] Consuming 200 OK...
there
200 OK
```
掌握了yield的用法，那么现在就讲consumer和produce结合起来看：
```python
c = consumer()
produce(c)

#输出
there
[PRODUCER] Producing 1...
here
[CONSUMER] Consuming 1...
there
[PRODUCER] Consumer return: 200 OK
[PRODUCER] Producing 2...
here
[CONSUMER] Consuming 2...
there
[PRODUCER] Consumer return: 200 OK
[PRODUCER] Producing 3...
here
[CONSUMER] Consuming 3...
there
[PRODUCER] Consumer return: 200 OK
[PRODUCER] Producing 4...
here
[CONSUMER] Consuming 4...
there
[PRODUCER] Consumer return: 200 OK
[PRODUCER] Producing 5...
here
[CONSUMER] Consuming 5...
there
[PRODUCER] Consumer return: 200 OK
```
总结：
之所以通过yield来实现协程，就是因为当produce调用c.send(n)时可以触发consumer执行它自身的流程，待consumer执行完后又返回，produce继续执行，最后根据判断结束流程。
与传统的生产者消费者方案相比，整个过程中，没有锁，单线程，共享资源，不用担心死锁的问题，而且效率更高。

假若有什么意见，请欢迎留言~
