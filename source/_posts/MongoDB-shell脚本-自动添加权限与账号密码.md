---
title: MongoDB shell脚本 自动添加权限与账号密码
tags:
  - MongoDB
categories:
  - 编程
date: 2016-04-09 13:55:16
---

## 需求 ##
将无需账号密码访问的MongoDB数据库，添加上账号密码，并且适配兼容旧的无权限访问的环境，即自动判断有无权限，无则自动加上。

<!-- more -->

## 分析 ##
MongoDB开启权限访问需要两步：

 1. 为数据库添加账号密码，即添加用户，默认是无用户的。
 2. 将mongod服务开启时，加上auth参数，即开启权限认证。（或者修改MongoDB的mongod.conf配置文件，将#auth = true的注释去掉即可）

## 实现 ##

话不多说，直接看脚本:
fit_mongo.sh

```shell
#!bin/bash

#以无权限控制的方式启动服务（--fork -f并不是必须的，是我自己的需求在用，有需要自行查询其他参数，）
mongod --fork -f /path/to/mongod.conf

#使用mongo [options] [db address] [file names (ending in .js)]命令执行js脚本操作数据库，并写入日志
mongo --host 127.0.0.1:27017 testdb fit_mongo.js >> fit_mongo.log

#安全地杀掉mongod进程
pkill mongod

#开启权限控制
mongo --auth --fork -f /path/to/mongod.conf
```
然后就是fit_mongo.js
```javascript
try{
	var db = connect('testdb');
	if(db.system.users.find({'user':'username'}).count() == 0){
		print('Ready to addUser...');
		db.addUser('username','pwd');
	}else{
		print('Already exist user.')
	}
}catch(err){
	print('Error occured:' + err);
}
```
最后就是选择在合适的地方和时间执行这个脚本了，可以在整个系统服务跑起来的时候执行。
在实现这个功能的过程中遇到了几个问题：

 1. 一开始为了修改MongoDB以需要权限访问的时候，老想着去用命令改mongod.conf文件，要用正则啊各种语句找到auth还要取消掉注释，殊不知MongoDB已经提供了auth这个参数，浪费了许多功夫和时间，所以还是要好好了解清楚官方提供了什么再动手。
 2. 在fit_mongo.js中，由于mongo shell的原因，在shell中执行js代码，比如：

	`db.system.users.find({user: 'UserName'}) `

	返回的是json对象，但是在shell上确取不到这个json里面的值，后来发现多此一举，直接用count()判断是否为0就可以了。

 3. 其中还有一些零零碎碎的小问题也记不太清除了，但是过程中却学到了不少，获益良多！

如果对以上有什么问题，欢迎提出~