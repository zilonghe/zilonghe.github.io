---
title: Java基础笔记（一）：Collection、Map
tags:
  - Java
categories:
  - 面试
date: 2018-11-19 13:31:51
---

# Java基础笔记（一）：Collection、Map

先抛出问题：collection接口下面有哪些集合，hashmap的实现原理，要把1.7和1.8的区别(红黑树)讲出来，map有哪些实现类以及使用场景，hashmap, hashtable, linkedhashmap,weakHashMap, treemap, concurrentmap，linkedhashmap和treemap排序的区别，concurrenthashmap如何实现线程安全，这里也要把1.7和1.8实现差异说出来(分段加锁和cas技术)，说到这里以后就会问你cas实现原理( CPU Lock前缀指令),它是如何保证其他cpu core的cache失效的，然后会问你volatile的实现原理，要结合java内存模型来讲，可见性是如何实现的(内存屏障)，synchronized锁和reentrantlock的区别以及内部怎么实现的:常用的gc算法及优缺点，如何判断对象的存活性

<!-- more -->

## Collection集合

Collection接口是最基本的集合接口，代表了一组对象。以下是继承了Collection接口的几个集合接口：

- Set接口：

  代表了一组不可重复的对象，并且无序。

- List接口：

  代表了一组有序、可重复的对象

- Queue接口：

  和List接口类似，除了基本的集合操作之外，还提供了额外的插入、提取、检查的操作。

Map接口不是继承于Collection接口，它描述了键值对的映射，不允许重复key值，每个key只能最多只能映射到一个值。常见的实现类是HashMap和TreeMap。

## HashMap的实现原理（1.8）

### 特征

- 继承Map接口
- 允许空的键值对
- 非同步容器，需要同步可以使用`ConcurrentHashMap`或者使用`Collections.synchronizedMap(new HashMap())`方法封装。在遍历HashMap的同时进行增删改的操作，会抛出`ConcurrentModificationException`异常，即`fail-fast`
- 与HashTable类似，只是HashTable不允许空键值对，并且是同步容器

### 关键参数

- 初始容量大小：初始化HashMap的桶（buckets）个数，必须是2的平方，默认16
- 负载因子：描述这个Map的饱和程度，当哈希表的表项达到当前桶个数与负载因子的乘积时，会进行重哈希，桶个数扩容至当前容量大小的两倍；默认的负载因子是0.75

- 树化阈值：当一个桶的元素多于该阈值，则会将当前桶的元素列表转化成树，该阈值默认是8

- 去树化阈值：当一个桶的元素树化后的个数小于该值，则会将树转换成列表。该阈值默认是6

### 关键操作

- `putVal`

```java
	/**
     * Implements Map.put and related methods
     *
     * @param hash hash for key
     * @param key the key
     * @param value the value to put
     * @param onlyIfAbsent if true, don't change existing value
     * @param evict if false, the table is in creation mode.
     * @return previous value, or null if none
     */
    final V putVal(int hash, K key, V value, boolean onlyIfAbsent,
                   boolean evict) {
        Node<K,V>[] tab; Node<K,V> p; int n, i;
        // 如果map为空，或者长度为0，则初始化map，并且赋值给tab
        if ((tab = table) == null || (n = tab.length) == 0)
            n = (tab = resize()).length;
        // 若将要插入的桶为空，则新建一个节点插入该位置，(n-1)&hash的含义是将插入的键值对尽可能的分布均匀，由于n是2的平方，n-1则会获得二进制全是1的二进制数，再与hash值与操作，使得该键值对所属的位置完全取决于hash值的最低几位，这样有利于分布均匀，性能更好。
        if ((p = tab[i = (n - 1) & hash]) == null)
            tab[i] = newNode(hash, key, value, null);
        // 若插入的桶已有元素
        else {
            Node<K,V> e; K k;
            // 若该节点的key值与插入的key值一样，将原节点赋值给e
            if (p.hash == hash &&
                ((k = p.key) == key || (key != null && key.equals(k))))
                e = p;
            // 若不相等，且该节点是树形节点，为该树插入节点
            else if (p instanceof TreeNode)
                e = ((TreeNode<K,V>)p).putTreeVal(this, tab, hash, key, value);
            // 若不相等，且还未树化，则在该列表的尾部插入新节点
            else {
                for (int binCount = 0; ; ++binCount) {
                    // 尾部
                    if ((e = p.next) == null) {
                        p.next = newNode(hash, key, value, null);
                        // 若列表的节点数量达到了树化阈值，则转化成树
                        if (binCount >= TREEIFY_THRESHOLD - 1) // -1 for 1st
                            treeifyBin(tab, hash);
                        break;
                    }
                    // 遍历比较列表的节点的key值是否和插入的key值相等，若相等跳出循环，注意在上面的if条件中，将p.next节点赋值给e了
                    if (e.hash == hash &&
                        ((k = e.key) == key || (key != null && key.equals(k))))
                        break;
                    // 将当前指针向前一位
                    p = e;
                }
            }
            // 如果e不为空，表明插入的key和原有key相同，则直接替换value，并且返回旧value
            if (e != null) { // existing mapping for key
                V oldValue = e.value;
                // 如果不是putIfAbsent，即不存在该key的键值对时才插入的话，或者原value是空，替换旧值
                if (!onlyIfAbsent || oldValue == null)
                    e.value = value;
                // 提供给ConcurrentHashMap的后续操作
                afterNodeAccess(e);
                return oldValue;
            }
        }
        // 修改标志位加一，表示增加了一次操作
        ++modCount;
        // 如果键值对的数目达到了阈值，对桶的个数进行2倍扩容。注意区分，键值对的个数是size，table.length是桶的个数
        if (++size > threshold)
            resize();
        // 提供给ConcurrentHashMap的后续操作
        afterNodeInsertion(evict);
        return null;
    }
```

- `resize`

  ```java
  /**
   * Initializes or doubles table size.  If null, allocates in
   * accord with initial capacity target held in field threshold.
   * Otherwise, because we are using power-of-two expansion, the
   * elements from each bin must either stay at same index, or move
   * with a power of two offset in the new table.
   *
   * @return the table
   */
  final Node<K,V>[] resize() {
      Node<K,V>[] oldTab = table;
      // 旧容量，即桶的个数，非键值对的个数，一个桶可以有很多键值对
      int oldCap = (oldTab == null) ? 0 : oldTab.length;
      // 旧阈值
      int oldThr = threshold;
      int newCap, newThr = 0;
      // 如果旧的容量大于0
      if (oldCap > 0) {
          // 如果已经达到了上限，将阈值赋值为最大整形，直接返回
          if (oldCap >= MAXIMUM_CAPACITY) {
              threshold = Integer.MAX_VALUE;
              return oldTab;
          }
          // 如果当前桶个数的2倍小于桶上限，并且旧的桶个数大于等于默认值16，将阈值乘以2
          else if ((newCap = oldCap << 1) < MAXIMUM_CAPACITY &&
                   oldCap >= DEFAULT_INITIAL_CAPACITY)
              newThr = oldThr << 1; // double threshold
      }
      // 如果旧的容量为0，阈值不为0，就新的桶个数上限设置为旧的阈值
      else if (oldThr > 0) // initial capacity was placed in threshold
          newCap = oldThr;
      // 如果阈值和容量都为0，则将容量设置为默认值16，阈值设置为默认值0.75*16
      else {               // zero initial threshold signifies using defaults
          newCap = DEFAULT_INITIAL_CAPACITY;
          newThr = (int)(DEFAULT_LOAD_FACTOR * DEFAULT_INITIAL_CAPACITY);
      }
      if (newThr == 0) {
          float ft = (float)newCap * loadFactor;
          newThr = (newCap < MAXIMUM_CAPACITY && ft < (float)MAXIMUM_CAPACITY ?
                    (int)ft : Integer.MAX_VALUE);
      }
      threshold = newThr;
      @SuppressWarnings({"rawtypes","unchecked"})
          Node<K,V>[] newTab = (Node<K,V>[])new Node[newCap];
      table = newTab;
      if (oldTab != null) {
          for (int j = 0; j < oldCap; ++j) {
              Node<K,V> e;
              if ((e = oldTab[j]) != null) {
                  oldTab[j] = null;
                  // 若该桶里只有一个节点，直接重hash放置
                  if (e.next == null)
                      newTab[e.hash & (newCap - 1)] = e;
                  else if (e instanceof TreeNode)
                      ((TreeNode<K,V>)e).split(this, newTab, j, oldCap);
                  else { // preserve order
                      // 存放不需要移动的节点链表
                      Node<K,V> loHead = null, loTail = null;
                      // 存放需要'翻倍'移动的节点链表
                      Node<K,V> hiHead = null, hiTail = null;
                      Node<K,V> next;
                      do {
                          next = e.next;
                          // 由于每次的桶个数都会翻倍，即原有的桶个数左移一位，由于原有的hash方法是(n-1)&hash,那么在这里只需要判断新增的一位与hash值的值，如果是0，即和原来的不变，若非0，则将其移动到当前位置加oldCap的位置，即'翻倍'。
                          if ((e.hash & oldCap) == 0) {
                              if (loTail == null)
                                  loHead = e;
                              else
                                  loTail.next = e;
                              loTail = e;
                          }
                          else {
                              if (hiTail == null)
                                  hiHead = e;
                              else
                                  hiTail.next = e;
                              hiTail = e;
                          }
                      } while ((e = next) != null);
                      // 不需要移动的节点链表，表头直接放置到原有位置
                      if (loTail != null) {
                          loTail.next = null;
                          newTab[j] = loHead;
                      }
                      // 需要翻倍移动的节点链表，表头放置到当前位置+oldCap的位置上
                      if (hiTail != null) {
                          hiTail.next = null;
                          newTab[j + oldCap] = hiHead;
                      }
                  }
              }
          }
      }
      return newTab;
  }
  ```

### 总结

HashMap的实现原理是，通过传入的键值对，对键进行hash，并存放在哈希表中的具体某个桶中，当多个键值的hash值相同时，即哈希冲突时，桶中的节点起初以链表形式存储，在1.8中，若链表节点的数目大于树化的阈值8并且桶的个数大于等于64时，会将链表转换成红黑树实现，否则（链表节点数目大于树化阈值但桶个数小于64）会进行resize扩容，将当前的桶个数扩大一倍，并重hash分配位置。当获取指定的键值的时候，会将键的hash值与哈希表的长度-1进行逻辑与操作获得具体存放的桶位置，若该桶的节点数不唯一，则通过遍历该桶的节点，通过equals匹配获取目标键值。



## ConcurrentHashMap

线程安全的高并发hashmap，不允许null键值对，jdk1.8中锁定每个桶的链表头或者红黑树的根节点来保证线程安全，只要hash不冲突，就不会有竞争，效率更高。

### 1.7与1.8的区别

1.7是采用分段加锁，1.8是对每个桶做加锁，并且通过cas来更新、插入节点。(关于CAS的总结请看第二篇基础笔记)

### 关键操作

```java
final V putVal(K key, V value, boolean onlyIfAbsent) {
    	// 不允许空的key或者value
        if (key == null || value == null) throw new NullPointerException();
    	// 计算传入的key值的hash值
        int hash = spread(key.hashCode());
        int binCount = 0;
    	// for循环是为了配合cas，不断重试
        for (Node<K,V>[] tab = table;;) {
            Node<K,V> f; int n, i, fh;
            // 如果桶数组还没初始化，则初始化桶数组
            if (tab == null || (n = tab.length) == 0)
                tab = initTable();
            // 如果索引到的桶为空，则cas设置该桶的头结点为当前插入键值对
            else if ((f = tabAt(tab, i = (n - 1) & hash)) == null) {
                // 如果设置成功，则直接break；如果有竞争，其他地方把该桶初始化了，则下一次循环不会走到这里
                if (casTabAt(tab, i, null,
                             new Node<K,V>(hash, key, value, null)))
                    break;                   // no lock when adding to empty bin
            }
            // 如果当前桶数组正在扩容，则帮助它扩容
            else if ((fh = f.hash) == MOVED)
                tab = helpTransfer(tab, f);
            else {
                V oldVal = null;
                // 对桶的头结点加锁
                synchronized (f) {
                    // 确保在此过程中，map没有进行扩容等操作，还是修改同一个桶
                    if (tabAt(tab, i) == f) {
                        // 当前节点是链表头结点
                        if (fh >= 0) {
                            binCount = 1;
                            for (Node<K,V> e = f;; ++binCount) {
                                K ek;
                                // 如果hash值相等，则更新
                                if (e.hash == hash &&
                                    ((ek = e.key) == key ||
                                     (ek != null && key.equals(ek)))) {
                                    oldVal = e.val;
                                    if (!onlyIfAbsent)
                                        e.val = value;
                                    break;
                                }
                                Node<K,V> pred = e;
                                // hash值不相等，遍历链表，将新节点插入到末尾
                                if ((e = e.next) == null) {
                                    pred.next = new Node<K,V>(hash, key,
                                                              value, null);
                                    break;
                                }
                            }
                        }
                        // 如果是红黑树
                        else if (f instanceof TreeBin) {
                            Node<K,V> p;
                            binCount = 2;
                            // 插入新红黑树节点
                            if ((p = ((TreeBin<K,V>)f).putTreeVal(hash, key,
                                                           value)) != null) {
                                oldVal = p.val;
                                if (!onlyIfAbsent)
                                    p.val = value;
                            }
                        }
                    }
                }
                if (binCount != 0) {
                    // 如果当前桶的节点个数大于树化阈值，则转化成红黑树
                    if (binCount >= TREEIFY_THRESHOLD)
                        treeifyBin(tab, i);
                    if (oldVal != null)
                        return oldVal;
                    break;
                }
            }
        }
        addCount(1L, binCount);
        return null;
    }
```




## LinkedHashMap

LinkedHashMap实现有序key值的关键就是根据插入顺序另外维护了一个按照插入顺序作为标记的双向循环列表，这样在获取所有数据进行循环获取时获取到的数据就是有序的数据

```java
Node<K,V> newNode(int hash, K key, V value, Node<K,V> e) {
    LinkedHashMap.Entry<K,V> p =
        new LinkedHashMap.Entry<K,V>(hash, key, value, e);
    linkNodeLast(p);
    return p;
}
```

每次调用继承自基类HashMap的putVal方法插入键值对的时候，都会调用newNode，在双向链表尾部插入节点。



```java
void afterNodeInsertion(boolean evict) { // possibly remove eldest
    LinkedHashMap.Entry<K,V> first;
    if (evict && (first = head) != null && removeEldestEntry(first)) {
        K key = first.key;
        removeNode(hash(key), key, null, false, true);
    }
}

protected boolean removeEldestEntry(Map.Entry<K,V> eldest) {
        return false;
}
```

可能你会注意到，上面putVal方法中，若新插入了一个键值对（即不是覆盖原有的键值对），会调用`afterNodeInsertion`回调方法，`evict`参数表示是否执行删除最旧的元素，注意在删除最旧元素之前，还会判断`removeEldestEntry`方法的返回值，默认实现是返回false，通常我们需要覆盖该方法，自定义淘汰策略。



## Treemap

TreeMap是另外一种有序的map，与LinkedHashMap根据键值对插入的顺序来保证顺序的方式不一样，treeMap是根据key值的自然顺序，或者传入的comparator来排序。TreeMap非同步，可以使用`Collections.synchronizedSortedMap(new TreeMap(...))`获取同步的TreeMap，同样，treeMap的也是具有fail-fast行为的。treeMap的底层实现是红黑树。



```java
public V put(K key, V value) {
    Entry<K,V> t = root;
    if (t == null) {
        compare(key, key); // type (and possibly null) check

        root = new Entry<>(key, value, null);
        size = 1;
        modCount++;
        return null;
    }
    int cmp;
    Entry<K,V> parent;
    // split comparator and comparable paths
    Comparator<? super K> cpr = comparator;
    // 若有comparator，则使用comparator来进行key值的比较，否则根据key值的自然顺序比较，下面两个分支的操作都一样，就是找到应该插入的位置
    if (cpr != null) {
        do {
            parent = t;
            cmp = cpr.compare(key, t.key);
            if (cmp < 0)
                t = t.left;
            else if (cmp > 0)
                t = t.right;
            else
                return t.setValue(value);
        } while (t != null);
    }
    else {
        if (key == null)
            throw new NullPointerException();
        @SuppressWarnings("unchecked")
            Comparable<? super K> k = (Comparable<? super K>) key;
        do {
            parent = t;
            cmp = k.compareTo(t.key);
            if (cmp < 0)
                t = t.left;
            else if (cmp > 0)
                t = t.right;
            else
                return t.setValue(value);
        } while (t != null);
    }
    Entry<K,V> e = new Entry<>(key, value, parent);
    if (cmp < 0)
        parent.left = e;
    else
        parent.right = e;
    // 插入后需要对红黑树进行调整，关键步骤
    fixAfterInsertion(e);
    size++;
    modCount++;
    return null;
}
```

put函数的操作比较简单，比较key值，找到插入的位置，插入节点，调整红黑树，这里大概讲讲红黑树的定义及规则，还有左旋右旋操作，由于记住也没什么太大意义，具体调整的技巧则不讨论了。

#### 红黑树

红黑树又称红-黑二叉树，它首先是一颗二叉树，它具体二叉树所有的特性。同时红黑树更是一颗自平衡的排序二叉树。

二叉树满足一个基本性质–即树中的任何节点的值大于它的左子节点，且小于它的右子节点。按照这个基本性质使得树的检索效率大大提高。我们知道在生成二叉树的过程是非常容易失衡的，最坏的情况就是一边倒（只有右/左子树），这样势必会导致二叉树的检索效率大大降低（O(n)），所以为了维持二叉树的平衡，大牛们提出了各种实现的算法，如：[AVL](http://baike.baidu.com/view/414610.htm)，[SBT](http://baike.baidu.com/view/2957252.htm)，[伸展树](http://baike.baidu.com/view/1118088.htm)，[TREAP](http://baike.baidu.com/view/956602.htm) ，[红黑树](http://baike.baidu.com/view/133754.htm?fr=aladdin#1_1)等等。

平衡二叉树必须具备如下特性：它是一棵空树或它的左右两个子树的高度差的绝对值不超过 1，并且左右两个子树都是一棵平衡二叉树。

##### 定义

1. 节点是红色或黑色

2. 根是黑色

3. 所有叶子都是黑色（叶子是NULL节点）

4. 如果一个节点是红的，则它的两个儿子都是黑的

5. 从任一节点到其叶子的所有路径都包含相同数目的黑色节点。

写到一半，发现讲得更好的[博客](https://github.com/CarpenterLee/JCFInternals/blob/master/markdown/5-TreeSet%20and%20TreeMap.md)...



