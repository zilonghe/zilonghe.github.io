---
title: 极简 Git 使用教程
tags:
  - git
categories:
  - 工具
date: 2018-06-26
---



## 本地创建一个新的项目时

**1.1** 进入你想要上传到 git 的目录，运行`git init`

**1.2** 在该文件夹中创建文件，这里用添加一个`.gitignore`文件为例子：`touch .gitignore`

<!-- more -->

**1.3** 当你想要查看当前工作区和 Staging Area ( 下面解释 )中的文件情况时，你可以用`git status`

**1.4** git 分为三个区域，如图：![2016091701](http://od3b21nvv.bkt.clouddn.com/hexoimg79bfbdac37faf4030b8e7cf331063d9d.png-960.jpg)

```
	Working Directory: 存放在这个目录下刚刚 增添修改 过的文件。
	Staging Area: 对所有修改的代码中的某部分进行细分提交（ 或者全部 ），那么你可以单独把他们添加到这个区域中（ 下面讲命令 ）。
	Repository: 那就是你提交到的本地仓库。
```
**1.5** 提交到 Staing Area: `git add .gitignore`或者全部 `git add -A`

**1.6** 如果想要撤销，即从第二个区域将文件返回到第一个区域：`git reset .gitignore`或者全部`git reset`

**1.7** 最后确认提交: `git commit -m "提交代码的说明"`

**1.8** 查看自己提交过的记录: `git log`


## 复制远端仓库到本地
**2.1** 克隆远端代码仓库: `git clone <git url> <想要存储到的本地目录>`

**2.2** 查看远端仓库别名: `git remote -v`（ 查看到仓库别名为 origin 下面例子用该名字来演示 ）, 查看远端仓库分支: `git branch -a`

**2.3** 当你修改了仓库中的代码后，查看修改的地方：`git diff`

**2.4** 提交到本地仓库只需要执行上面的步骤 `1.5 `和 `1.7`

**2.5** 但这样只是提交到本地的仓库，你还要推送到远端的仓库，首先执行`git pull origin <分支>`来更新代码，因为可能不止你一个人对该仓库分支(尤其是 master )进行了修改，先更新再提交，这是常识

**2.6** 更新完，提交：`git push origin <分支>`

**2.7** 你也可以自己创建一个分支：`git branch <分支名>`，这里用`test-branch`来演示

**2.8** 创建完分支后进行修改，首先先切换到该分支: `git checkout test-branch`

**2.9** 修改了文件后，提交到本地仓库，执行上面的步骤 `1.5 `和 `1.7`

**2.10** 将刚修改好的分支 push 到远端仓库: `git push -u origin test-branch`(这里不需要 pull 是因为这是你自己新建的一个分支/ -u 是以后你再对这个分支做修改的时候，可以直接git pull/push 相当于添加一个reference)

**2.11** 将分支合并到 master 分支，首先切换到 master 分支 ：`git checkout master`，然后`git pull origin master`来更新 master

**2.12** 然后进行合并：`git merge test-branch`

**2.13** 最后还要 push 到远端仓库：`git push origin master`

**2.14** 检查是否分支是否的确被合并：`git branch --merged`来查看被合并到 master 的分支

**2.15** 如果在合并完分支后，你想删除掉该分支：`git branch -d test-branch`,但这只是对本地的仓库进行操作，这个时候你查看`git branch -a`，发现该分支仍然存在远端仓库中，所以你需要将删除分支这一操作也 push 到远端：`git push origin --delete test-branch`



## 常见问题

1. 当`git pull`遇到错误：`fatal: refusing to merge unrelated histories`，解决方法：

   `git pull origin branchname --allow-unrelated-histories`

2. 需要在本地修改关联的远程仓库地址时：

   `git remote set-url origin new-remote-git-url`

3. 当`git push`遇到错误：

   ```

   error:failed to push some refs to ...

   Dealing with “non-fast-forward” errors

   From time to time you may encounter this error while pushing:

   $ git push origin master 

   To ../remote/ 

    ! [rejected]        master -> master (non-fast forward) 

   error: failed to push some refs to '../remote/' 

   To prevent you from losing history, non-fast-forward updates were rejected

   Merge the remote changes before pushing again.  See the 'non-fast forward'

   section of 'git push --help' for details.
   ```

   原因：git 仓库中已经有一部分代码，所以它不允许你直接把你的代码覆盖上去。

   解决：

   	1. `git push -f`，利用强覆盖方式用本地的代码替代 git 仓库的内容。
   	2. 先把 git 的代码 fetch 到本地，merge 后再 push：`git pull`

   引用：[http://www.cnblogs.com/renkangke/archive/2013/05/31/conquerAndroid.html](http://www.cnblogs.com/renkangke/archive/2013/05/31/conquerAndroid.html) 

****
