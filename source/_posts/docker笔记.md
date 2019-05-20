# docker笔记

## 基本概念

`image`：是镜像，就像是java中的一个类，静态资源

`container`：是容器，就像是java中的类实例，根据image运行起来的容器实例



## 基本命令

`docker image ls`：展示已有image

`docker run -it <imageName>`：以交互形式在一个新的container里面运行一个image

`docker run -d`:后台执行

`docker container ls`：显示已有container

`docker build -t <tag>`：讲一个dockerfile编译成一个image

`docker ps`:显示正在运行的container

`docker exec`：Run a command in a running container

`docker stop`：停止一个正在运行的container

`docker start`：启动一个container

`docker rm`：删除一个container

`docker inspect`:Return low-level information on Docker objects

`docker network ls`:展示已有网络

`docker network create -d <driver> <name>`：创建一个网络

`docker network inspect <name>`：查看一个网络的详情，比如包含了什么容器

## docker网络

### 网络类型

- bridge：网桥模式，最常用
- host：共享host机器的网络命名空间
- none：孤立的网络命名空间，只有本地通过docker exec才能访问



### bridge模式

`—-link`：可以将一个容器连接到另外一个容器，然后可以通过容器名称访问，例子：`docker run -d --name test1 --link test2 busybox`，创建一个后台运行的busybox容器，名称为test1，与test2容器连接。

`--network`:

```
# 创建一个driver为bridge的网络my-bridge
docker network create -d bridge my-bridge
# 启动一个容器，加入my-bridge
docker run -d --name test3 --network my-bridge busybox
```

如用户自己创建了一个network（默认创建的容器都是加入docker0 bridge 网络），那么默认加入该网络的容器都可以通过容器名称访问。



### 端口映射

`docker run -p <容器端口>:<需要映射到本地的端口>`



设置环境变量：`docker run -e <env_key>=<env_value>`



## docker数据持久化

1. Data Valume：将container中运行产生的数据映射到本地路径。

   `docker volume ls`：列出当前所有volume

   `docker volume inspect <id>`：查看指定volume信息

   `docker run -v <volumeName>:local/path`：指定需要映射的路径和volume的名字，可以复用

2. Bind Mounting：运行容器的时候指定本地路径和容器里面的路径，这两个路径将会同步共享

   `docker run -v local/path:container/path`

