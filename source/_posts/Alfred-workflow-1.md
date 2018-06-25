---
title: Alfred workflow初步开发指南（一）
tags:
  - alfred
categories:
  - 工具
date: 2016-07-26 23:23:02
---

OS X用户都知道一款神器：[Alfred](https://www.alfredapp.com/)
但是大部分人都只是在用它的快捷搜索的功能，但是它最强大的功能是它的workflow，今天我来抛砖引玉，写一下如何开发属于自己的workflow。

<!-- more -->

------

打开Alfred，点击上方栏目的workflows：

![栏目](http://img.blog.csdn.net/20160726223537726)

建议大家可以先点击左边侧边栏下面的加号，试用几个examples，来熟悉workflow的大概操作流程：

![侧边栏](http://img.blog.csdn.net/20160726223857915)

-----

这里我演示开发一个简单又好用的workflow : quick search

![quick search](http://img.blog.csdn.net/20160726224320386)

## 触发器Triggers
定义触发你的workflow的动作Triggers，点击右上角的加号，这里我选择了hotkey，也就是以快捷键的形式来触发你的workflow：

![trigger](http://img.blog.csdn.net/20160726224752138)

第一栏Hotkey：就是键入你想要定义的快捷键，这里我选择了command+2
第二栏Action：就是当你按下你定义的快捷键，将要触发的动作，第一个选项就是传递到workflow的下一步，第二个就是show alfred，显然我们应该选择第一个，不然也就没什么意义了：）
第三栏Argument：这个就是传递的参数了，这里我们选择Selection in OS X，也就是当前光标选中的文本。

ok,到这里已经把触发器定义好了，Save保存。

## Actions动作
有了触发器之后，我们当然就要定义触发的动作了，猛击加号，Actions，里面的动作都描述地很清楚了，这里我们选择Open URL

![actions](http://img.blog.csdn.net/20160726230431765)

这里的{query}就是贯穿整个workflow传递的一个参数，trigger和action就是靠这个变量来通信。当我选中文本，并按下快捷键时，选中的文本就会传递以{query}传递到action处，那么这里我定义的是百度搜索，同样你可以定义其他搜索。

> 注：获得其他搜索引擎url的方法：
> 打开其他网站的搜索，将地址栏的url里你输入搜索框的内容替换成{query}即可获得。

最后点击Save。

## 大功告成
最后将两个模块连接起来，你的第一个workflow就这么开发完了！

![这里写图片描述](http://img.blog.csdn.net/20160726231159917)

试试选中文本再猛击command+2，是不是很好用！这和最快的普通操作：
1. 打开浏览器
2. 直接在浏览器的地址栏输入搜索的内容
3. Enter
相比不知道有多高效！

## 未完待续
但是在使用的过程中，你会发现，当选中的文本是一个网址时，是不能够直接打开网址的，那这个时候你就可以再定义多一个workflow专门打开网址啦，这个就留给读者们自己开发吧~

下一期将讲述Alfred中的精华workflow中的精华：Run Script！
