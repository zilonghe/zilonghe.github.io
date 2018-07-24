---
title: 使用hsql+junit模拟mybatis数据库接口编写UT
tags:
  - Mybatis
  - Java
  - 单元测试
categories:
  - 编程
date: 2018-7-24 11:42:11
---

# 使用hsql+junit模拟mybatis数据库接口编写UT

## 背景

目前UT的框架已经是万花齐放，有mockito、easymock、spock等等优秀的框架，在这篇文章中，主要介绍以hsql内存数据库来模拟数据库接口的方式

<!-- more -->

## mockito

介绍之前，先看看用mockito来写的话是怎么样的吧：

被测试方法：

```java
...

public String queryOntByPppoeNameList(HttpContext context, String pppoeName) throws ServiceException
    {
        List<String> userIds = JsonUtil.jsonToList(pppoeName, String.class);
        Map<String, String> userIdOntRelations = new HashMap<>(userIds.size());

        queryFromSelfLearning(userIds, userIdOntRelations);
        if (userIds.size() == 0)
        {
            return JsonUtil.toJson(userIdOntRelations);
        }

        queryFromMQMC(userIds, userIdOntRelations);
        if (userIds.size() == 0)
        {
            return JsonUtil.toJson(userIdOntRelations);
        }

        queryFromRM(userIds, userIdOntRelations, rmDataDBMgr.getOltByUsers(userIds));
        return JsonUtil.toJson(userIdOntRelations);
    }
```

mock测试方法：

```java
...

public class OntUserResourceDelegateImplTest 
{
    private OntUserResourceDelegateImpl ontUserResourceDelegate;

    @Mock
    private SelfLearningDBMgr selfLearningDBMgr;

    @Mock
    private MQMCSelfLearningDBMgr mqmcSelfLearningDBMgr;

    @Mock
    private RMDataDBMgr rmDataDBMgr;

    @Before
    public void setUp() throws Exception {
        // 初始化测试用例类中由Mockito的注解标注的所有模拟对象
        MockitoAnnotations.initMocks(this);
        // 用模拟对象创建被测类对象
        ontUserResourceDelegate = new OntUserResourceDelegateImpl();
        ontUserResourceDelegate.setMqmcSelfLearningDBMgr(mqmcSelfLearningDBMgr);
        ontUserResourceDelegate.setSelfLearningDBMgr(selfLearningDBMgr);
        ontUserResourceDelegate.setRmDataDBMgr(rmDataDBMgr);
    }

    @Test
    public void testQueryOntByPppoeNameListWithMockito() throws ServiceException
    {
        List<String> userIds = new ArrayList<>(5);
        userIds.add("testnotexist");
        userIds.add("test-0-5-7-0@nceisp0");
        userIds.add("test-0-5-3-21@pppoe");
        userIds.add("test005");
        userIds.add("test004");
        String inputJson = JsonUtil.toJson(userIds);
        String selfRet = "[{\"userID\":\"test004\",\"userMac\":\"xx\",\"neName\":\"OLT\",\"neIp\":\"x.x.x.x\",\"accessPort\":\"GPON 0/4/0\",\"ontID\":\"10\",\"updatetime\":1531879293565,\"accessType\":\"gpon\"},{\"userID\":\"test005\",\"userMac\":\"xx\",\"neName\":\"OLT\",\"neIp\":\"x.x.x.x\",\"accessPort\":\"GPON 0/4/0\",\"ontID\":\"2\",\"updatetime\":1531879293564,\"accessType\":\"gpon\"}]";
        List<UserDeviceRelation> mockSelfLearningRet = JsonUtil.jsonToList(selfRet, UserDeviceRelation.class);
        when(selfLearningDBMgr.queryOntByPppoeNameList(userIds)).thenReturn(mockSelfLearningRet);

        Map<String, Object> filterMap = new HashMap<>(1);
        userIds.remove("test005");
        userIds.remove("test004");
        filterMap.put(SelfLearningConstants.USER_ID_LIST, userIds);
        List<UserDeviceRelation> ontsFromMQMC = mqmcSelfLearningDBMgr.getUserDeviceRelation(filterMap);
        when(mqmcSelfLearningDBMgr.getUserDeviceRelation(filterMap)).thenReturn(Collections.emptyList());

        String rmDataRet = "[{\"bandAccount\":\"test-0-5-3-21@pppoe\",\"city\":\"北京\",\"county\":\"xxx\",\"zone\":\"xxx\",\"olt\":\"BJYD\",\"pon_port\":\"GPON 0/5/3\",\"bandwidth\":\"\",\"oltNeIP\":\"x.x.x.x\",\"ponPortNo\":\"NA-0-5-3\",\"portType\":\"GPON_OLT\",\"loID\":\"\",\"ontID\":\"21\",\"importTime\":1528787997621},{\"bandAccount\":\"test-0-5-7-0@nceisp0\",\"city\":\"北京\",\"county\":\"xxx\",\"zone\":\"xxx\",\"olt\":\"BJYD\",\"pon_port\":\"GPON 0/5/7\",\"bandwidth\":\"\",\"oltNeIP\":\"x.x.x.x\",\"ponPortNo\":\"NA-0-5-7\",\"portType\":\"GPON_OLT\",\"loID\":\"\",\"ontID\":\"0\",\"importTime\":1528787997621}]";
        when(rmDataDBMgr.getOltByUsers(userIds)).thenReturn(JsonUtil.jsonToList(rmDataRet, RMDataBean.class));
    }


    
}

```

上述例子中，被测试类OntUserResourceDelegateImpl依赖三个类SelfLearningDBMgr、RMDataDBMgr、MQMCSelfLearningDBMgr，它们的作用是去不同的数据源中查询数据，也就是DAO层接口，被测试的方法的逻辑是依次调用执行这三个方法，最后将结果返回。

看起来我们的测试方法，还比较简单，对这三个接口的返回分别构造json返回体，然后录制行为结果，最后再验证。

## 内存数据库

用内存数据库来模拟接口的思路很简单，就是将我们的mybatis数据库配置成hsql内存数据库，就可以在每次执行单元测试的时候，在内存中建立数据库，然后所有接口都可以从该库中取数据。

与其说是单元测试，实际上使用这种方法的话，更像是一个spring应用程序而已。

#### 配置spring上下文

```xml
<?xml version="1.0" encoding="UTF-8"?>
<beans xmlns="http://www.springframework.org/schema/beans"
       xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
       xsi:schemaLocation="http://www.springframework.org/schema/beans http://www.springframework.org/schema/beans/spring-beans.xsd">

    <bean id="dataSource"
          class="org.springframework.jdbc.datasource.DriverManagerDataSource">
        <property name="url" value="jdbc:hsqldb:mem:db"></property>
        <property name="driverClassName" value="org.hsqldb.jdbc.JDBCDriver"></property>
        <property name="username" value="root"></property>
        <property name="password" value="admin"></property>
    </bean>

    <bean id="sqlSessionFactoryBean" class="org.mybatis.spring.SqlSessionFactoryBean">
        <property name="dataSource" ref="dataSource"/>
        <property name="configLocation" value="classpath:mybatis/mybatis-config.xml"/>
    </bean>

    <bean id="sqlSessionTemplate" class="org.mybatis.spring.SqlSessionTemplate">
        <constructor-arg index="0" ref="sqlSessionFactoryBean"/>
    </bean>

    <bean id="sqlSessionTemplateBatch" class="org.mybatis.spring.SqlSessionTemplate">
        <constructor-arg index="0" ref="sqlSessionFactoryBean"/>
        <constructor-arg index="1" value="BATCH"/>
    </bean>

    <bean id="mapperManager" class="com.huawei.bsp.mybatis.session.MapperManagerImp">
        <property name="sqlSession" ref="sqlSessionTemplate"/>
        <property name="sqlSessionBatch" ref="sqlSessionTemplateBatch"/>
        <property name="mapperFilesMgr" ref="mapperFilesMgr"/>
    </bean>

    <bean id="mapperFilesMgr" class="com.huawei.bsp.mybatis.mapper.MapperFilesMgr">
        <property name="sqlSessionFactory" ref="sqlSessionFactoryBean"/>
    </bean>

    <!-- 此处添加自己的dao实现类 -->
    <bean id="rMDataDBMgr" class="com.utraffic.metro.applicationcollectorservice.rmdata.RMDataDBMgr">
        <property name="mapper" ref="mapperManager"></property>
    </bean>

    <bean id="selfLearningDBMgr" class="com.utraffic.metro.applicationcollectorservice.userdeviceselflearning.dao.SelfLearningDBMgr">
        <property name="mapper" ref="mapperManager"></property>
    </bean>

    <bean id="mQMCSelfLearningDBMgr" class="com.utraffic.metro.applicationcollectorservice.userdeviceselflearning.dao.MQMCSelfLearningDBMgr">
        <property name="mapper" ref="mapperManager"></property>
    </bean>

    <!-- 此处添加自己的业务逻辑类 -->
    <bean id="ontUserResourceDelegateImpl" class="com.huawei.cloudsop.applicationcollectorservice.impl.OntUserResourceDelegateImpl">
        <property name="rmDataDBMgr" ref="rMDataDBMgr"></property>
        <property name="mqmcSelfLearningDBMgr" ref="mQMCSelfLearningDBMgr"></property>
        <property name="selfLearningDBMgr" ref="selfLearningDBMgr"></property>
    </bean>


</beans>

```

- dataSource这个bean的定义，就是引用了hsqldb，在引入之前，需要在pom文件中加入以下依赖：

  ```xml
  <dependency>
              <groupId>org.hsqldb</groupId>
              <artifactId>hsqldb</artifactId>
              <version>2.3.2</version>
              <scope>test</scope>
  </dependency>
  ```

  ​

- sqlSessionFactoryBean的configLocation是mybatis的配置文件，里面可以注册我们所定义的mapper.xml文件

  ```xml
  <?xml version="1.0" encoding="UTF-8" ?>
  <!DOCTYPE configuration
          PUBLIC "-//mybatis.org//DTD Config 3.0//EN"
          "http://mybatis.org/dtd/mybatis-3-config.dtd">
  <configuration>

      <!-- 此处添加自己的mapper -->
      <mappers>
          <mapper resource="mybatis/mysql/RMDataMapper.xml"/>
          <mapper resource="mybatis/mysql/SelfLearningMapper.xml"/>
          <mapper resource="mybatis/mysql/MQMCSelfLearningMapper.xml"/>
      </mappers>

  </configuration>
  ```

  ​

- 由于在项目的代码中，使用的都是平台封装好的mybatis管理类，所以需要注入它所需要的bean，最后将id为mapperManager的bean注入到我们的dao实现类即可。

#### 编写UT测试类

```java
...

public class MockDBUtil
{
    protected static ApplicationContext ctx;

    /**
     * 在所有测试方法前会执行一次，用于初始化模拟内存数据库
     */
    @BeforeClass
    public static void dbSetUp() throws Exception
    {
        ctx = new ClassPathXmlApplicationContext("test-applicationcxt.xml");
        DataSource ds = (DataSource) ctx.getBean("dataSource");

        try (Connection conn = ds.getConnection();
             Reader reader = Resources.getResourceAsReader("mybatis/schema.sql"))
        {
            ScriptRunner runner = new ScriptRunner(conn);
            runner.setLogWriter(null);
            runner.runScript(reader);
        }
    }
}

```

这个类的作用就是获取上个步骤定义的spring上下文，并且初始化数据库——加载我们自己定义的sql文件。在这个行为封装到一个工具类，并且在该方法使用注解@BeforeClass，实现的效果是：每个继承了该类的子类，在执行其所有用@Test注解的测试方法前都会执行一次。这样的目的是为了让数据库只建立一次就可以跑完一个类全部测试方法。

#### 测试dao层

```java
...

public class RMDataDBMgrTest extends MockDBUtil
{
    @Test
    public void getOltByUsers()
    {
        RMDataDBMgr dao = (RMDataDBMgr) ctx.getBean("rMDataDBMgr");
        List<String> inputs = new ArrayList<>();
        inputs.add("test-0-5-3-21@pppoe");
        inputs.add("test-0-5-7-0@nceisp0");
        inputs.add("testnotexist");

        List<RMDataBean> olts = dao.getOltByUsers(inputs);
        System.out.println(JsonUtil.toJson(olts));
        assert olts.size() == 2;
    }
}

```

使用这种方式，测试方法的编写和应用程序代码，几乎没有区别，只需要调用执行被测试方法，验证结果即可。



#### 测试业务逻辑service层

```java
...

public class OntUserResourceDelegateImplTest extends MockDBUtil
{
    @Test
    public void queryOntByPppoeNameList() throws ServiceException
    {
        OntUserResourceDelegateImpl bussiness = (OntUserResourceDelegateImpl) ctx.getBean("ontUserResourceDelegateImpl");
        List<String> userIds = new ArrayList<>(5);
        userIds.add("testnotexist");
        userIds.add("test-0-5-7-0@nceisp0");
        userIds.add("test-0-5-3-21@pppoe");
        userIds.add("test005");
        userIds.add("test004");
        String inputJson = JsonUtil.toJson(userIds);
        System.out.println(bussiness.queryOntByPppoeNameList(null, inputJson));
        ...
    }
}

```

测试业务逻辑的代码也同样简单。

## 对比

使用hsql内存数据库方式的好处是，只需要前期将spring配置与mybatis配置好，后续需要新增测试方法，都只需要在sql文件中，将依赖的数据写入（将现网的数据进行处理后导入），然后调用被测试方法直接验证结果即可，无需为所有依赖的数据一一进行mock，简单直接。而且可以对mybatis的dao层接口进行测试，例如有时候想对写的复杂sql进行测试覆盖，防止后续的人改错；对mybatis的高级特性（类型转换等）进行测试覆盖等等。

缺点是：无法构造数据库接口超时等异常场景，数据库过于庞大时，由于消耗内存，可能会影响执行速度。

使用mockito等mock框架的好处是：灵活、无需配置，这种框架一般使用的是动态代理，在录制模拟结果的时候，对被测试方法进行打桩，后续调用即可直接返回。三方库的轮子完善，可以构造各种异常场景。

缺点是：当接口依赖的数据变多，每个接口都需要自己构造json返回体（给变量起名已经那么难，更何况...），麻烦。



## 总结

在项目中，可以将两者结合，当接口实现简单的时候，可以直接使用mock方式来编写，当觉得构造数据多且麻烦时，可以使用内存数据库方式。

