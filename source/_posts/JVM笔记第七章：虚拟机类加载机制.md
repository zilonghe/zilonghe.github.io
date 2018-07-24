---
title: JVM读书笔记之第七章
tags:
  - Java
categories:
  - 读书笔记
date: 2018-7-5 19:31:33
---
# 虚拟机类加载机制

## 概述

虚拟机的类加载机制：虚拟机把描述类的数据从Class文件加载到内存，并对数据进行校验、转换解析和初始化，最终形成可以被虚拟机直接使用的java类型。

在Java语言中，类型的加载、连接和初始化过程都是在程序运行期间完成的，这种策略虽然会令类加载时稍微增加一些开销，但会为java应用程序提供高度的灵活性。

<!-- more -->

## 类加载的时机

类加载的生命周期包括**加载、验证、准备、解析、初始化、使用和卸载**7个阶段。其中**验证、准备、解析**部分统称为连接。**加载、验证、准备、初始化和卸载**这5个阶段的顺序是确定的，而解析阶段则不一定：它在某些情况下可以在初始化阶段之后再开始，这是为了支持java语言的运行时绑定（也称动态绑定或晚期绑定）。

虚拟机规范严格规定了**有且只有**下列5种情况必须立即对类进行“初始化”（加载、验证、准备要在此之前开始）：

1. 遇到new、getstatic、putstatic或invokestatic这4条字节码指令时，**如果类没有进行过初始化**，则需要先触发其初始化。生成这4条指令的最常见java代码场景是：
   - 使用new关键字实例化对象时候
   - 读取或设置一个类的静态字段（被final修饰、已在编译期把结果放入常量池的静态字段除外）的时候，以及调用一个类的静态方法的时候。
2. 使用java.lang.reflect包的方法对类进行反射调用的时候，如果类没有进行过初始化，则需要先触发其初始化。
3. 当初始化一个类的时候，如果发现其父类还没有进行过初始化，则需要先触发其父类的初始化。
4. 当虚拟机启动时，用户需要指定一个要执行的主类（包含main()方法的那个类），虚拟机会先初始化这个主类。
5. 当使用jdk1.7的动态语言支持时，如果一个java.lang.invoke.MethodHandle实例最后的解析结果（是）REF_getStatic、REF_putStatic、REF_invokeStatic的方法句柄，并且这个方法句柄所对应的类没有进行初始化，则需要先触发其初始化。

上述5种场景中的行为称为对一个类的主动引用，除此之外所有引用类的方式都不会触发初始化，称为被动引用。

例子一：通过子类引用父类的静态字段，不会导致子类初始化

```java
public class SuperClass {
    static {
        System.out.println("SuperClass init!");
    }
    public static int value = 123;
}

public class SubClass extends SuperClass {
    static {
        System.out.println("SubClass init!");
    }
}

public class NotInitialization {
    public static void main(String[] args) {
        System.out.println(SubClass.value); 
    }
}
```

运行结果是：

```java
SuperClass init!
123
```

解释：对于静态字段，只有直接定义这个字段的类才会被初始化，因此通过其子类来引用父类中定义的静态字段，只会触发父类的初始化而不会触发子类的初始化，至于是否要触发子类的加载和验证，取决于虚拟机的具体实现。对于Sun HotSpot虚拟机来说，会触发子类的加载。

例子二：通过数组定义来引用类，不会触发此类的初始化

```java
public class NotInitialization {
    public static void main() {
        SuperClass[] sca = new SuperClass[10];
    }
}
```

此处引用了上个例子中的SuperClass。运行后发现没有输出SuperClass init!，说明并没有触发SuperClass的初始化阶段。但是这段代码里触发了另外一个名为“[Lorg.xxx.SuperClass]”(xxx为忽略的包名)的类的初始化阶段。对于用户代码来说，这并不是一个合法的类的名称，它是一个由虚拟机自动生成的、直接继承于java.lang.Object的子类，创建动作由字节码指令newarray触发。这个类代表了一个元素类型为SuperClass的一维数组，Java语言中对数组的访问比C/C++相对安全的原因是这个类封装了数组元素的访问方法，而C/C++直接翻译为对数组指针的移动。当检查到发生数组越界时会抛出java.lang.ArrayIndexOutOfBoundsException异常。

例子三：常量在编译阶段会存入调用类的常量池中，本质上并没有直接引用到定义常量的类，因此不会触发定义常量的类的初始化。

```java
public class ConstClass {
    static {
        System.out.println("ConstClass init");
    }
    
    public static final String A = "hello";
}

public class NotInitialization {
    public static void main(String[] args) {
        System.out.println(ConstClass.A);
    }
}
```

运行后，没有输出"ConstClass init"，这是因为虽然在Java源码中引用了ConstClass类中的常量A，但其实在编译阶段通过**常量编译优化**，已经将此常量的值存储到了NotInitialization类的常量池中，以后该类对常量A的引用，都被传化为NotInitialization类的自身常量池的引用，这两个类在编译成Class之后就不存在联系了。

接口与类的区别：

上述的5种有且仅有需要开始初始化的场景中的第三种，当一个类的初始化时，要去其父类全部都已经初始化了，但是一个接口在初始化时，并不要求其父接口类全部都完成了初始化，只有在真正使用到父接口的时候（如引用接口中定义的常量）才会初始化。



## 类加载的过程

也就是加载、验证、准备、解析和初始化这5个阶段的具体动作。

### 加载

加载是类加载过程的一个阶段，在加载阶段，虚拟机需要完成以下3件事情：

1. 通过一个类的全限定名来获取定义此类的二进制字节流。
2. 将这个字节流所代表的静态存储结构转行成方法区的运行时数据结构。
3. 在内存中生成一个代表这个类的java.lang.Class对象，作为方法区这个类的各种数据访问入口。

一个非数组类的加载阶段，可以通过用户自定义的类加载器去完成，或者通过系统提供的引导类加载器。但数组类就有所不同，数组类本身不通过类加载器创建，它是由java虚拟机直接创建的，但数组类的元素类型最终还是要靠类加载器去加载的，有以下的规则：

1. 如果数组的组件类型是引用类型，那就递归采用上述的加载过程去加载这个组件类型。
2. 如果数组的组件类型不是引用类型，java虚拟机将会把数组标记为与引导类加载器关联。
3. 数组类的可见性与它的组件类型的可见性一致，如果组件类型不是引用类型，那数组类的可见性默认为public。

加载阶段完成之后，虚拟机外部的二进制字节流就按照虚拟机所需的格式存储在方法区之中，方法区中的数据存储格式由虚拟机实现自行定义，然后在内存中实例化一个java.lang.Class类的对象（Class对象比较特殊，虽然是对象，但是存放在方法区中），这个对象将作为程序访问方法区中的这些类型数据的外部接口。

### 验证

验证是连接阶段的第一步，目的是为了确保Class文件中的字节流中包含的信息符合当前虚拟机的要求，并且不会危害虚拟机自身的安全。从整体上看，验证阶段大致上会完成下面4个阶段的检验动作：

1. 文件格式验证：验证字节流是否符合Class文件格式的规范，并且能被当前版本的虚拟机处理。

2. 元数据验证：对字节码描述的信息进行语义分析，以保证其描述的信息符合Java语言规范的要求，验证点包括：这个类是否有父类、这个类是否继承了不允许被继承的类（被final修饰的类）、非抽象类是否实现了其父类或接口中要求实现的所有方法、类中的字段、方法是否与父类产生矛盾（如覆盖了父类的final字段，或者出现不符合规则的方法重载等）。

3. 字节码验证：是整个验证过程中最复杂的一个阶段，主要目的是通过数据流和控制流分析，确定程序语义是合法的、符合逻辑的。如：保证任意时刻操作数栈的数据类型与指令代码序列都能配合工作、保证跳转指令不会跳转到方法体以外的字节码指令上、保证方法体中的类型转换是有效的。

4. 符号引用验证：发生在虚拟机将符号引用转化为直接引用的时候，这个转化动作将在连接的第三个阶段——解析阶段中发生。目的是确保解析动作能正常执行。符号引用验证可以看做是的类自身以外（常量池中的各种符号引用）的信息进行匹配性校验。通常校验一下内容：符号引用中通过字符串描述的全限定名是否能找到对应的类、在指定类中是否存在符合方法的字段描述符以及简单名称所描述的方法和字段、符号引用中的类、字段、方法的访问性是否可以被当前类访问。非常重要但非必要的阶段，如果运行的代码可信，则可以通过-Xverify:none参数来关闭大部分的类验证措施，以缩短虚拟机类加载的时间。

   

### 准备

准备阶段是正式为类变量分配内存并设置类变量初始值的阶段，这些变量所使用的内存都将在方法区中进行分配。这时候进行的内存分配的仅包括类变量（被static修饰的变量），而不包括实例变量，实例变量将会在对象实例化时随着对象一起分配在java堆中。

`public static int value = 123 `

这个类变量value将在准备阶段中被赋值0，把123赋值给value的动作是在类构造器clint方法中，即在初始化阶段中才会执行。

`public static final int value = 123 `

该value在准备阶段就会被赋值123，原因是这个变量将会存在类字段的字段属性表中的ConstantValue属性。



### 解析

解析阶段是虚拟机将常量池内的符号引用替换为直接引用的过程。符号引用在class文件中以CONSTANT_Class_info、CONSTANT_Fieldref_info、CONSTANT_Methodref_info等类型的常量出现

- 符号引用：以一组符号来描述所引用的目标，符号可以是任何形式的字面量，只要使用时能无歧义地定位到目标即可。符号引用与虚拟机实现的内存布局无关，引用的目标并不一定已经加载到内存中。各虚拟机所接受的符号引用必须都是一致的，因为符号引用的字面量形式明确定义在Java虚拟机规范的Class文件格式中。
- 直接引用：直接引用可以是直接指向目标的指针、相对偏移量或是一个能间接定位的目标的句柄。直接引用和虚拟机实现的内存布局相关，同一个符号引用在不同虚拟机实例上翻译出来的直接引用一般不会相同。如果有了直接引用，那引用目标必定已经在内存中存在。



### 初始化

类初始化阶段是类加载过程的最后一步，前面的类加载过程中，除了在加载阶段用户应用程序可以通过自定义类加载器参与之外，其余动作完全由虚拟机主导和控制。到了初始化阶段，才真正开始执行类中定义的java程序代码（或者说是字节码）。

初始化阶段是执行类构造器`<clint>()`方法的过程：

- clint()方法是由编译器自动收集类中的所有类变量的赋值动作和静态语句块中的语句合并产生的，静态语句块中只能访问到定义在静态语句块之前的变量，定义在它之后的变量，在前面的静态语句块中可以赋值，但是不能访问。例如：

  ```java
  public class Test {
      static {
          i = 0; // 给变量赋值可以正常编译通过
          System.out.print(i); // 这句编译器会提示“非法向前引用”
      }
      static int i = 1;
  }
  ```

  

- `<clint>()`方法与类的构造函数(或者说实例构造器<init>()）方法不同，它不需要显式地调用父类构造器，虚拟机会保证在子类的clint方法执行之前，父类的clinit方法已经执行完毕。因此在虚拟机中第一个被执行的clint方法的类肯定是object类。
- 由于父类的clinit方法先执行，也就意味着父类中定义的静态语句块要优先于子类的变量赋值操作，例如
```java
static class Parent {
    public static int A = 1;
    static {
        A = 2;
    }
}

static class Sub extends Parent {
    public static int B = A;
}

public staic void main(String[] args) {
    System.out.println(Sub.B); // 2
}
```
- clinit方法对于类或接口来说并不是必需的，如果一个类中没有静态语句块，也没有对类变量的赋值操作，那么编译器可以不为这个类生成clint方法。
- 接口中不能使用静态语句块，但仍然有变量初始化的赋值操作，因此接口与类一样都会生成clint方法。只有当父接口中定义的变量使用时，父接口才会初始化，另外，接口的实现类在初始化时也一样不会执行接口的clint方法。
- 虚拟机会保证一个类的clint方法在多线程环境中被正确地加锁、同步，如果多个线程同时去初始化一个类，那么只会有一个线程去执行这个类的clint方法，其他线程都需要阻塞等待，知道活动线程执行clint完毕。如果一个类的clint方法中有耗时很长的操作，就可能造成多个进程阻塞。



## 类加载器

虚拟机设计团队把类加载阶段中的“通过一个类的全限定名来获取描述此类的二进制字节流”这个动作放到Java虚拟机外部去实现，以便让应用程序自己决定如何去获取所需要的类，实现这个动作的代码模块称为“类加载器”。

### 类与类加载器

比较两个类是否相等，只有在这两个类是由同一个类加载器加载的前提下才有意义，否则，即使这两个类来源于同一个class文件，被同一个虚拟机加载，只要加载它们的类加载器不同，那着两个类必定不相等。如：

```java
public class ClassLoaderTest {
    public static void main(String[] args) {
        ClassLoader myLoader = new ClassLoader() {
            @Override
            public Class<?> loadClass(String name) throws ClassNotFoundException {
                try {
                    String filename = name.substring(name.lastIndexOf(".") + 1) + ".class";
                    
                    InputStream is = getClass().getResourceAsStream(filename);
                    if (is = null) {
                        return super.loadClass(name);
                    }
                    byte[] b = new byte[is.available()];
                    is.read(b);
                    return defineClass(name, b, 0, b.length);
                } catch (IOException e) {
                    //...
                }
            }
        };
        
        Object obj = myLoader.loadClass("xxx.ClassLoaderTest").newInstance();
        
        System.out.println(obj.getClass()); // xxx.ClassLoaderTest
        System.out.println(obj instanceof xxx.ClassLoaderTest); // false
    }
}
```

虚拟机中存在两个ClassLoaderTest类，一个是由系统应用程序类加载器加载的，另外一个是由我们自定义的类加载器加载的，虽然都来自同一个Class文件，但依然是两个独立的类。

### 双亲委派模型

从Java虚拟机的角度来讲，只存在两种不同的类加载器：一种是启动类加载器（Bootstrap ClassLoader）,这个类加载器使用C++语言实现，是虚拟机自身的一部分；另一种就是所有其他的类加载器，这些类加载器都由java语言实现，独立于虚拟机外部，并且全都继承自抽象类java.lang.ClassLoader。

从java开发人员的角度来看，绝大部分应用程序都会使用的3种类加载器：

1. 启动类加载器：负责加载 `<JAVA_HOME>\lib`目录中的、或者被-Xbootclasspath参数所指定的路径中的、并且是虚拟机识别的类库加载到虚拟机内存中。启动类加载器无法被Java程序直接引用，用户在编写自定义类加载器时，如果需要把加载请求委派到引导类加载器，那直接使用null引用即可，如:

   ```java
   public ClassLoader getClassLoader() {
       ClassLoader cl = getClassLoader();
       if (cl = null)
       	return null;
       ... 
   }
   ```

   

2. 扩展类加载器（Extension ClassLoader）:负责加载 `<JAVA_HOME>\lib\ext`目录中的，或者被java.ext.dirs系统变量所指定的路径中的所有类库，开发者可以直接使用该加载器。

3. 应用程序类加载器(Application ClassLoader)：这个类加载器是ClassLoader中的getSystemClassLoader()方法的返回值，所以一般也称为系统类加载器。负责加载用户类路径（ClassPath）上所指定的类库，开发者可以直接使用这个类加载器，如果应用程序中没有自定义过自己的类加载器，一般情况下这个就是默认的类加载器。

双亲委派模型的工作过程是：如果一个类加载器收到了类加载的请求，它首先不会自己去尝试加载这个类，而是把这个请求委派给父类加载器去完成，每一个层次的类加载器都是如此，因此所有的加载请求最终都应该传送到顶层的启动类加载器中，只有当父加载器反馈自己无法完成这个加载请求时（它的搜索范围中没有找到所需的类），子加载器才会尝试自己加载。

实现逻辑:先检查是否已经加载过，若没有加载则调用父加载器的loadClass()方法，若父加载器为空则默认使用启动类加载器作为父加载器。如果父加载器加载失败，抛出ClassNotFoundException异常后，再调用自己的findClass()方法进行加载。

#### 破坏双亲委派模型

线程上下文加载器（Thread Context ClassLoader），这个类加载器可以通过java.lang.Thread类的setContextClassLoader()方法进行设置，如果创建线程时还未设置，它将会从父线程中继承一个，如果在应用程序的全局范围内都没有设置过的话，那这个类加载器默认就是应用程序类加载器。

OSGi实现模块化热部署的关键则是它自定义的类加载器机制的实现。每一个程序模块（OSGI中称为Bundle）都有一个自己的类加载器，当需要更换一个Bundle时，就把Bundle连同类加载器一起换掉以实现代码的热替换。

在OSGi环境下，类加载器不再是双亲委托模型中的树状结构，而是进一步发展为更加复杂的网状结构，当收到类加载请求时，OSGi将按照下面的顺序进行类搜索：

1. 将以java.*开头的类委派给父类加载器加载。
2. 否则，将委派列表名单内的类委派给父类加载器加载。
3. 否则，将Import列表中的类委派给Export这个类的Bundle的类加载器加载。
4. 否则，查找当前Bundle的ClassPath，使用自己的类加载器加载。
5. 否则，查找类是否存在自己的Fragment Bundle中，如果在，则委派给Fragment Bundle的类加载器加载。
6. 否则，查找Dynamic Import列表的Bundle，委派给对应Bundle的类加载器加载。
7. 否则，类查找失败。

上面的查找顺序中只有开头两点仍然符合双亲委派规则，其余的类查找都是在平级的类加载器中进行的。

