---
title: Java基础笔记（二）：JMM内存模型
tags:
  - Java
categories:
  - 面试
date: 2019-7-10 16:12:21
---

# Java基础笔记（二）：JMM内存模型

先抛出问题：JMM内存模型是什么，cas实现原理( CPU Lock前缀指令),它是如何保证其他cpu core的cache失效的，然后会问你volatile的实现原理，要结合java内存模型来讲，可见性是如何实现的(内存屏障)，synchronized锁和reentrantlock的区别以及内部怎么实现的:常用的gc算法及优缺点，如何判断对象的存活性

<!-- more -->



