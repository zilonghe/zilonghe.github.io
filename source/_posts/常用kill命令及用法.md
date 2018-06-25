---
title: 常用kill命令及用法
tags:
  - linux
categories:
  - 编程
date: 2016-09-16 15:06:11
---

当你想要杀掉某个进程 / 服务的时候，你自然而然会想到用`kill`命令结合`ps/grep`找出进程`id`并将其杀之，但很多人也许并不知道`kill`命令还有很多其他选项，而我们又该怎么去选择。今天这一篇主要是介绍常用的`kill`命令及其用法。

![kill bill](http://od3b21nvv.bkt.clouddn.com/hexoimg15f106c985499f555c97316bee98afba.png-960.jpg)

<!-- more -->

## 常用的 KILL 命令
- `-15`/`-TERM`( 即不带任何选项的`kill`命令 ) ：告诉进程停止你所做的所有事，并且结束掉你自己。这种方式结束进程应该是很安全的，但最好还是先试试用`-1`/`-HUP`命令 ( 以安全的角度来讲 )。
- `-1`/`-HUP` ：向进程发送『挂起』的信号。在大多数程序编写的时候，都会对该命令做出相应的操作（正确地杀掉自己）。而当你对一个守护进程（ daemon ）发送这个信号的时候，大多数守护进程都会重新去读取它们的配置信息。总的来说，这种杀掉进程的方式应该是最安全的。
- `-2`/`-SIGINT` ：这个信号就相当于你启动一个程序 / 服务，然后按下`ctrl+c`强行****结束进程的效果一样，大多数进程都会被杀掉，但你有可能会丢失数据。
- `-9`/`-KILL` ：这个信号或许是大家最常用的吧：），当你对某个进程`kill -9`之后，这个进程不会得到任何善后的机会（关闭 socket 连接、清理临时文件、将自己将要被销毁的消息通知给子进程、重置自己的终止状态等等），并且有可能留下一些不完整的文件或者状态，当系统重启的时候，程序将无法理解这些状态。所以这种杀进程的方式是最残暴、最粗鲁同时也是最不安全、不干净的方式，只有在面对无法结束的进程的时候，你才应该考虑使用它。


当你想要杀掉某个进程的时候，如果想要知道一个进程应该接收哪个`kill`信号，你可以对进程用`man`命令进行查看，例如`init(8)`的 man page 里就有如下说明：

```
SIGHUP
Has the same effect as telinit q.
...
Q or q tell init to re-examine the /etc/inittab file.
```

还有更多其他信号见文末附录。

**问：所以到底该怎么有效地杀掉一个进程呢？**

答：如果这个程序编写者有建议使用哪个信号，那么你应该使用他给出的建议信号。如果没有，首先试试用`kill -15`（等价于`kill`），不行的话用`kill -2`，再不行的话用`kill -1`，最后再用`kill -9`。

## 用法
### `ps`命令用于查看进程

`ps -ef`

```
  UID   PID  PPID   C STIME   TTY           TIME CMD
    0     1     0   0 五10下午 ??         8:55.67 /sbin/launchd
    0    46     1   0 五10下午 ??         3:46.70 /usr/sbin/syslogd
    0    47     1   0 五10下午 ??         0:34.02 /usr/libexec/UserEventAgent (System)
    0    49     1   0 五10下午 ??         0:11.47 /usr/libexec/kextd
    0    50     1   0 五10下午 ??         0:45.61 /System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/FSEvents.framework/Versions/A/Support/fseventsd
   55    55     1   0 五10下午 ??         0:01.31 /System/Library/CoreServices/appleeventsd --server
    0    56     1   0 五10下午 ??         1:33.35 /usr/libexec/configd
```

`ps aux`

```
USER              PID  %CPU %MEM      VSZ    RSS   TT  STAT STARTED      TIME COMMAND
...
_coreaudiod       185   0.8  0.1  2499356   6456   ??  Ss   五10下午  33:06.52 /usr/sbin/coreaudiod
...
```

只查询某些进程，结合grep命令：

`ps -ef | grep swcd`

```
5  01 72400     1   0  3:21下午 ??         0:00.15 /usr/libexec/swcd
	
```

我们查询进程的目的是为了杀掉他，所以我们只需要`PID`这一列，于是我们可以只打印出第二列：

`ps -ef | grep swcd | awk '{print $2}'`

```
72400
78907
```

但是这里有个问题，当你用`ps -ef | grep`命令查询某个进程的时候，它所列出来的结果是包含了`grep`这个进程本身的，所以有这个查询并只显示进程id的命令`pgrep`:

`pgrep swcd`

```
72400
```

得到了进程 id 之后，就可以将它传给`kill`命令了：

`kill 72400`/`kill -15 72400`

把两条命令写成一条语句：

`pgrep swcd | xargs kill -15`

或者还有一个命令`pkill`:相当于`pgrep+kill`,只要将你想要杀掉的进程名传给它即可：

`pkill swcd`/`pkill -15 swcd`

或者是`killall`，用法和`pkill`一样，但需要将完整的进程名传入，否则会报错：

`No matching processes belonging to you were found`


（完）

## 附录
```
   Signal     Value     Action   Comment
   -------------------------------------------------------------------------
   SIGHUP        1       Term    Hangup detected on controlling terminal
                                 or death of controlling process
   SIGINT        2       Term    Interrupt from keyboard
   SIGQUIT       3       Core    Quit from keyboard
   SIGILL        4       Core    Illegal Instruction
   SIGABRT       6       Core    Abort signal from abort(3)
   SIGFPE        8       Core    Floating point exception
   SIGKILL       9       Term    Kill signal
   SIGSEGV      11       Core    Invalid memory reference
   SIGPIPE      13       Term    Broken pipe: write to pipe with no readers
   SIGALRM      14       Term    Timer signal from alarm(2)
   SIGTERM      15       Term    Termination signal
   SIGUSR1   30,10,16    Term    User-defined signal 1
   SIGUSR2   31,12,17    Term    User-defined signal 2
   SIGCHLD   20,17,18    Ign     Child stopped or terminated
   SIGCONT   19,18,25    Cont    Continue if stopped
   SIGSTOP   17,19,23    Stop    Stop process
   SIGTSTP   18,20,24    Stop    Stop typed at tty
   SIGTTIN   21,21,26    Stop    tty input for background process
   SIGTTOU   22,22,27    Stop    tty output for background process
```

## 引用文章
- [http://blog.csdn.net/andy572633/article/details/7211546](http://blog.csdn.net/andy572633/article/details/7211546)
- [http://meinit.nl/the-3-most-important-kill-signals-on-the-linux-unix-command-line](http://meinit.nl/the-3-most-important-kill-signals-on-the-linux-unix-command-line)
- [http://unix.stackexchange.com/a/8927](http://unix.stackexchange.com/a/8927)





