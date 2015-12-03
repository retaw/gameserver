# gameserver

分布式linux网络服务器框架，以及使用这个框架开发业务的示例。
使用c++11开发，g++4.8及以上版本编译器，-Wall -Werror 编译通过


代码结构概述：


    water:      底层框架代码

            process:        进程框架的最终实现，对tcp连接，高速消息收发缓冲，线程管理，定时器，等进行封装和整合，
                            提供了一个填上业务逻辑就可运行的linux服务器进程代码框架。
                            设计结构请参看process类的定义（process.h）

            net:            linux环境tcp编程接口的面向对象封装，
                            包括socket，listener，connector, connection, epoller，packet等类。
                            即一个高性能面向对象网络库

            componet:       高可复用性基本组件，
                            主要有 无锁队列（基于cas），stl容器序列化，类型安全的字符串格式化，日志，xml解析等

            dbadaptcher:    数据库访问接口

            libs:           第三方库（无需手动安装，makefile会自动管理）

            test:           部分测试代码


    protocol:   通信协议定义，动态消息号管理，支持多种编码方式，例子中有protobuf和raw两种编码格式

    processes:  具体业务进程，（从一个真实项目的初期代码删改得到，仅用于展示，故进程种类和拓扑结构并不自然）

    config:     配置文件，其中processes.xml配置了进程的拓扑结构

    other:      一些相关杂项，其中codestyle.txt为基本编码规范


