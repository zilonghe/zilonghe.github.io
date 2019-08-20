# Operating Systems: three easy pieces

## virtualization

### CPU virtualization

fork、join、exec

trap

用户态和内核态

schedule

​	single cpu

​	multi cpu

​		cpu cache

​		work stealing

总结，cpu虚拟化是对程序调度、权限控制的虚拟化，在此基础上需要确保安全性和高效性，一系列的技术由此衍生出来。

### memory virtualization

Adress space