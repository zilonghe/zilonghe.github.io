---
title: Alfred workflow初步开发指南（二）
tags:
  - alfred
categories:
  - 工具
date: 2016-09-12 01:29:4
---


有些朋友说，第一期写得有点太不接地气儿了，那么我先在开头讲清楚，这个Alfred Workflow教程到底是干什么的，以及Workflow是什么。
而且我会在接下来的教程中，面向两部分人会有不同的内容：

第一种：普通用户，不需要用到Run Script中的部分功能的（需要自己写代码的），但也不要说对普通用户就没用了，因为你可以直接到文章最后下载我写好的 Workflow! （楼主好人一生平安）

第二种：程序员专用的，比如我现在写的blog就是在本地写好文章，再通过我自己定义的 Workflow 一键上传到服务器的，又或者是你需要启动某些系统服务，跑某些脚本，等等。换句话说就是写 shell 脚本啦~


<!-- more -->

- Workflow 是什么？
> Workflow 工作流就是一系列相互衔接、自动进行的业务活动或任务。
> 简单粗暴地讲，我们这个自定义的 Workflow ，就是一个让你从大量重复性劳动中解放出来的工具。用程序员哥哥的话来讲，Workflow 就是一个自动化脚本。

举个例子：你早上起床想冲一杯豆浆喝，你要做的事情是：

1. 打开冰箱把豆浆粉拿出来。
2. 把拿出来的豆浆粉倒进杯子里。
3. 烧开水。
4. 将开水倒入杯中。

假设现在你有一个机器人或者你伟大的妈妈，在听到你说『我想喝一杯豆浆』后，立即帮你把上述步骤全做了。那么你是不是就会立即得到一杯豆浆。而通过自定义 Workflow 你就可以得到相同效果。


- Afred Workflow 是什么？

答案很显然，就是 Alfred 提供的 Workflow 功能模块。在这个模块下你可以定义属于你自己的 Workflow，只要运行了 Alfred 并被触发，你的 Workflow 就会生效。

## 开始

回顾[上期](http://johnho.cn/2016/07/26/Alfred-workflow-1/)，我们通过演示开发一个简单又好用的 quick search 来熟悉了开发 Workflow 的一些基本流程。那么本期我们要讲解 workflow 的精华: Run Script!

### 热身

首先上几个我最常用的 Workflow 供大家参考：

#### 1. Start Coding

标题已经很明显了，就是开始想要开始打代码，触发这个 workflow ，电脑就会自动帮我打开

- 编辑器 Sublime Text

- 屏蔽其他娱乐网站的工具 Focus

- 翻墙的 shadowsocksX

- Github

最后还会弹出一个提示框：Strat Working NOW!

十分快捷方便，又有工作的仪式感。大家也可以根据平时自己常用软件来进行定制，这里只是提供一个思路。

![Start Coding](http://img.blog.csdn.net/20160825225024762)

#### 2. 一键发表文章


![hexo publish](http://img.blog.csdn.net/20160825233820501)

中间的Terminal Command 如下：同理可以运用到 git 提交代码。

![image](http://img.blog.csdn.net/20160825234145021)


#### 3. 将截图上传到图床并返回一个 markdown 格式的链接

![7niu](http://od3b21nvv.bkt.clouddn.com/hexoimgf833c781f4d6796d82f9eabe29506139.png-960.jpg)


## Run Script & Script Filter
这一部分是 Workflow 的精华，alfred 允许用户通过自定义脚本，再结合 alfred workflow 提供的机制，对你想获取的信息或操作进行配合，就可以让你工作效率无限提高。比如：上述中的把截图上传到图床再返回一个md格式的图片链接，就节省了我成吨的时间，相信写过博客的人都知道。

废话不多说，接下来我会给大家展示一个，通过豆瓣提供的API，搜索电影的信息，并在 alfred 窗口中展示出来，效果图：

搜索广州现在上映了哪些电影：

![上映搜索](http://od3b21nvv.bkt.clouddn.com/hexoimg012bbab0d87310561490e3b9cdbc7b8e.png-960.jpg)

搜索某个演员：

![明星搜索](http://od3b21nvv.bkt.clouddn.com/hexoimgcf216974d897eab9b509d93af64284ff.png-960.jpg)

又或者某部电影：

![电影](http://od3b21nvv.bkt.clouddn.com/hexoimg71d0e9a007e272b8ee82265024d673ff.png-960.jpg)

这里的最近上映和电影搜索是分为两个 workflow ，如图所示我分别定义了两个命令，`dbm`是搜索电影/演员的信息，`dbn`是搜索所在地的上映电影。

我将展示搜索电影的这个 workflow 是如何实现的，剩下的那个原理也是一样，除了对电影搜索之外，还可以对图书信息进行搜索，只要豆瓣API有提供，理论上都可以展示出来。

要实现这个豆瓣电影搜索 workflow 其实很简单，只需要一个 `Script Filter` 和 `Open URL`的组件即可完成：

![实现部件](http://od3b21nvv.bkt.clouddn.com/hexoimgfad9e206f6aa1bde9df5a40ce9d052c2.png-960.jpg)

点开 `Script Filter`:

![Script Filter](http://od3b21nvv.bkt.clouddn.com/hexoimgff2ed183ffcaf870ec4cb011cda160d2.png-960.jpg)

没错，只需要一行即可，因为内部逻辑都在这个`douban_movie.py`的脚本实现了：）

首先要知道的是，这里已经有轮子被实现了，你需要去下载一个 workflow-alfred python版本的库文件，[点击这里](https://github.com/deanishe/alfred-workflow)查看。里面已经教了你如何往 workflow 里导入这个python的库文件，这里就不讲了。

然后，你需要开始写`douban_movie.py`这个脚本了：

```python
#!/usr/bin/python
# -*- coding: UTF-8 -*-
__author__ = 'hzl'
import sys
from workflow import Workflow, ICON_WEB, web

def getMovie(wf):
    baseUrl = 'https://api.douban.com/v2/movie/search'
    params = dict(q=wf.args[0], count=10)
    r = web.get(baseUrl, params)
    r.raise_for_status()
    for post in r.json()['subjects']:
        casts = ''
        for cast in post['casts']:
            casts += cast['name'] + ' '
        #条目的内容
        title = post['title']
        detail = u'主演：' + casts + u'｜年份：' + post['year'] + \
                 u'｜评分：' + str(post['rating']['average'])
        wf.add_item(title, detail, arg=post['id'], uid=post['id'], valid=True, icon=ICON_WEB)
    wf.send_feedback()

if __name__ == '__main__':
    wf = Workflow()
    sys.exit(wf.run(getMovie))  
```

代码很简单，`getMovie`中，通过访问豆瓣电影的API链接，`https://api.douban.com/v2/movie/search`，再通过 `import` 我们上面导入的 python 库文件中的模块`workflow`，里面封装好了访问 url 的操作，我们只需要调用它访问上述链接，得到`json`字符串，解析后，再通过函数传入的`Workflow`实例中的`add_item`方法，将数据传到 alfred 中，即我们展示的页面那样，然后就结束。整个脚本的大概流程就是这样。

写完脚本之后，就通过上面我展示的`Script Filter`中那样，将这个脚本放到这个 Workflow 的目录下，如何打开目录呢，很简单就在你编辑的这个 Workflow 右击，点`Open in Finder`即可，编辑完成后，目录应该如下：

![workflow目录](http://od3b21nvv.bkt.clouddn.com/hexoimg4258b069481e74dd4ba8a4142f1f18a4.png-480.jpg)

最后再将`Open Url`定义如下，就可以实现点击展示出来的结果后，直接跳转到网页：

![openurl](http://od3b21nvv.bkt.clouddn.com/hexoimg99b2c99edd5f16fe96e4542961262d75.png-960.jpg)

大功告成！

## 最后

给出下载链接：

[豆瓣电影搜索](http://od3b21nvv.bkt.clouddn.com/%E8%B1%86%E7%93%A3%E7%94%B5%E5%BD%B1%E6%90%9C%E7%B4%A2.alfredworkflow)

[最近上映搜索](http://od3b21nvv.bkt.clouddn.com/%E8%B1%86%E7%93%A3%E7%94%B5%E5%BD%B1-%E6%9C%80%E8%BF%91%E4%B8%8A%E6%98%A0.alfredworkflow)

等我发现了什么新功能，再回来更新！



