---
title: Netty源码学习笔记——Netty服务端启动
tags:
  - 计算机知识
categories:
  - 编程
date: 2019-05-20 22:34:20
---


## 问题

1. 服务端的socket在哪里初始化？

   答：在io.netty.bootstrap.ServerBootstrap#init中初始化服务端的socket。

   ```java
   @Override
       void init(Channel channel) throws Exception {
           final Map<ChannelOption<?>, Object> options = options0();
           synchronized (options) {
               channel.config().setOptions(options);
           }
   
           final Map<AttributeKey<?>, Object> attrs = attrs0();
           synchronized (attrs) {
               for (Entry<AttributeKey<?>, Object> e: attrs.entrySet()) {
                   @SuppressWarnings("unchecked")
                   AttributeKey<Object> key = (AttributeKey<Object>) e.getKey();
                   channel.attr(key).set(e.getValue());
               }
           }
   
           ChannelPipeline p = channel.pipeline();
   
           final EventLoopGroup currentChildGroup = childGroup;
           final ChannelHandler currentChildHandler = childHandler;
           final Entry<ChannelOption<?>, Object>[] currentChildOptions;
           final Entry<AttributeKey<?>, Object>[] currentChildAttrs;
           synchronized (childOptions) {
               currentChildOptions = childOptions.entrySet().toArray(newOptionArray(childOptions.size()));
           }
           synchronized (childAttrs) {
               currentChildAttrs = childAttrs.entrySet().toArray(newAttrArray(childAttrs.size()));
           }
   
           p.addLast(new ChannelInitializer<Channel>() {
               @Override
               public void initChannel(Channel ch) throws Exception {
                   final ChannelPipeline pipeline = ch.pipeline();
                   ChannelHandler handler = config.handler();
                   if (handler != null) {
                       pipeline.addLast(handler);
                   }
   
                   // We add this handler via the EventLoop as the user may have used a ChannelInitializer as handler.
                   // In this case the initChannel(...) method will only be called after this method returns. Because
                   // of this we need to ensure we add our handler in a delayed fashion so all the users handler are
                   // placed in front of the ServerBootstrapAcceptor.
                   ch.eventLoop().execute(new Runnable() {
                       @Override
                       public void run() {
                           pipeline.addLast(new ServerBootstrapAcceptor(
                                   currentChildGroup, currentChildHandler, currentChildOptions, currentChildAttrs));
                       }
                   });
               }
           });
       }
   ```

   在这里会将我们实际使用的代码中对serverbootstrap的配置初始化到服务端的channel上

   ```java
   ServerBootstrap b = new ServerBootstrap();
               b.group(bossGroup, workerGroup)
                       .channel(NioServerSocketChannel.class)
                       .childOption(ChannelOption.TCP_NODELAY, true)
                       .childAttr(AttributeKey.newInstance("childAttr"), "childAttrValue")
                       .handler(new ServerHandler())
                       .childHandler(new ChannelInitializer<SocketChannel>() {
                           @Override
                           public void initChannel(SocketChannel ch) {
                               ch.pipeline().addLast(new AuthHandler());
                               //..
                           }
                       });
   ```

2. 在哪里accept连接？

io.netty.channel.nio.AbstractNioChannel#doBeginRead

```java
@Override
    protected void doBeginRead() throws Exception {
        // Channel.read() or ChannelHandlerContext.read() was called
        final SelectionKey selectionKey = this.selectionKey;
        if (!selectionKey.isValid()) {
            return;
        }

        readPending = true;

        final int interestOps = selectionKey.interestOps();
        if ((interestOps & readInterestOp) == 0) {
            selectionKey.interestOps(interestOps | readInterestOp);
        }
    }
```

