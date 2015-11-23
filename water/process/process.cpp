#include "process.h"

#include "componet/logger.h"
#include "componet/scope_guard.h"
#include "componet/string_kit.h"
#include "signal_handler.h"

#include <iostream>
#include <thread>
#include <map>


#include <unistd.h>
namespace water{
namespace process{

Process::Process(const std::string& name, int16_t num, const std::string& configDir, const std::string& logDir)
: m_processName(name), m_cfgDir(configDir), m_cfg(name, num), m_logDir(logDir)
{
}

Process::~Process()
{
    m_threads.clear();
}

void Process::lanchThreads()
{
    if(m_privateNetServer)
    {
        m_privateNetServer->run();
        const std::string name = "private server";
        m_threads.insert({name, m_privateNetServer.get()});
        LOG_DEBUG("{} thread start ok", name);
    }
    if(m_privateNetClient)
    {
        m_privateNetClient->run();
        const std::string name = "private client";
        m_threads.insert({name, m_privateNetClient.get()});
        LOG_DEBUG("{} thread start ok", name);
    }
    if(m_publicNetServer)
    {
        m_publicNetServer->run();
        const std::string name =  "public server";
        m_threads.insert({name, m_publicNetServer.get()});
        LOG_DEBUG("{} thread start ok", name);
    }
    if (m_httpServer)
    {
        const std::string serverName = "http server";
        m_httpServer->run();
        m_threads.insert({serverName, m_httpServer.get()});
        LOG_DEBUG("{} thread start ok", serverName);

        const std::string connName = "http conns";
        m_httpcons.run();
        m_threads.insert({connName, &m_timer});
        LOG_DEBUG("{} thread start ok", connName);
    }
    if(m_privateConnChecker)
    {
        m_privateConnChecker->run();
        const std::string name = "connection checker";
        m_threads.insert({name, m_privateConnChecker.get()});
        LOG_DEBUG("{} thread start ok", name);
    }
    if(m_flashSandboxServer && m_flashSandboxHandler)
    {
        m_flashSandboxServer->run();
        const std::string fsServerName = "flash sandbox server";
        m_threads.insert({fsServerName, m_flashSandboxServer.get()});
        LOG_DEBUG("{} thread start ok", fsServerName);

        m_flashSandboxHandler->run();
        const std::string fsHandlerName = "flash sandbox handler";
        m_threads.insert({fsHandlerName, m_flashSandboxHandler.get()});
        LOG_DEBUG("{} thread start ok", fsHandlerName);
    }
    {
        m_conns.run();
        const std::string name = "tcp conns";
        m_threads.insert({name, &m_conns});
        LOG_DEBUG("{} thread start ok", name);
    }
    {
        m_timer.suspend(); //设为挂起状态
        m_timer.run();
        const std::string name = "main timer";
        m_threads.insert({name, &m_timer});
        LOG_DEBUG("{} thread start ok", name);
    }
}

void Process::joinThreads()
{
    while(!m_threads.empty())
    {
        if(m_timer.isSuspend())
        {
            const ProcessConfig::ProcessInfo& cfg = m_cfg.getInfo();
            //发起私网连接全部成功，主定时器开始执行
            if(m_conns.totalPrivateConnNum() == cfg.privateNet.connect.size())
            {
                LOG_DEBUG("{} lanuch successful", getFullName());

                //启动成功，关掉标准输出日志
                LOG_CLEAR_STD;

                //touch启动好的进程
               // std::string shell_cmd = "touch " + m_logDir + "/touchfile/" + getFullName();
               // system(shell_cmd.c_str());

                //主定时器恢复
                m_timer.resume();
            }
        }

        for(auto& threadInfo : m_threads)
        {
            const std::string& name = threadInfo.first;
            ProcessThread* thread = threadInfo.second;

            bool threadRetValue;
            const auto waitRet = thread->wait(&threadRetValue, std::chrono::milliseconds(0));
            if(waitRet == ProcessThread::WaitRet::timeout)
                continue;

            if(threadRetValue)
            {
                LOG_TRACE("thread {}  stoped", name, getFullName());
            }
            else
            {
                LOG_ERROR("thread {}  abort", name, getFullName());
                stop();
            }
            m_threads.erase(name);
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void Process::start()
{
    try
    {
        //init()为虚函数， 不能在constructor中调用
        init();

        ON_EXIT_SCOPE_DO(SignalHandler::resetSignalHandle({SIGINT, SIGTERM}));

        lanchThreads();

        joinThreads();
    }
    catch (const componet::ExceptionBase& ex)
    {
        LOG_ERROR("process {} start, fatal error: [{}]", getFullName(), ex.what());
        stop();
    }
    catch (const std::exception& ex)
    {
        LOG_ERROR("process {} start, fatal error: [{}]", getFullName(), ex.what());
        stop();
    }
    catch (...)
    {
        LOG_ERROR("process {} start, unkonw error", getFullName());
    }
    LOG_TRACE("{} exited", getFullName());
    LOG_CLEAR_FILE;
}

void Process::stop()
{
    LOG_TRACE("exiting ...");
    for(auto& item : m_threads)
	{
        item.second->stop();
	}
    LOG_TRACE("stoped");
}

const std::string& Process::getName() const
{
    return m_processName;
}

std::string Process::getFullName() const
{
    return componet::toString(getId());
}

ProcessIdentity Process::getId() const
{
    return m_cfg.getProcessId();
}

Platform Process::platform() const
{
    return m_cfg.getPlatform();
}

const std::string& Process::cfgDir() const
{
    return m_cfgDir;
}

ZoneId Process::zoneId() const
{
    return m_cfg.getZoneId();
}

componet::TimePoint Process::opentime() const
{
    return m_cfg.opentime();
}

bool Process::mergeFlag() const
{
    return m_cfg.mergeFlag();
}

void Process::regTimer(std::chrono::milliseconds interval, const ProcessTimer::EventHandler& handler)
{
    m_timer.regEventHandler(interval, handler);
    
}

void Process::init()
{
    {//处理linux信号
        SignalHandler::setSignalHandle({SIGINT, SIGTERM}, std::bind(&Process::stop, this));
    }

    //配置解析
    m_cfg.load(m_cfgDir);
    m_conns.setSelfZoneId(m_cfg.getZoneId());

    //指定日志文件
    LOG_ADD_FILE(m_logDir + "/" + getFullName() + ".log");

    {//check db

    }

    {//初始化各个组件

        const ProcessConfig::ProcessInfo& cfg = m_cfg.getInfo();
        const auto& privateNet = cfg.privateNet;

        //私网连接检查
        m_privateConnChecker = PrivateConnectionChecker::create(getId());
        //私网监听
        if(privateNet.listen != nullptr)
        {
            m_privateNetServer = TcpServer::create();
            m_privateNetServer->addLocalEndpoint(*privateNet.listen);
        }

        //私网连出
        if(!privateNet.connect.empty())
        {
            m_privateNetClient = TcpClient::create();
            for(const net::Endpoint& ep : privateNet.connect)
                m_privateNetClient->addRemoteEndpoint(ep, std::chrono::seconds(5));
        }

        //公网监听
        if(!cfg.publicNet.listen.empty())
        {
            m_publicNetServer = TcpServer::create();
            for(const net::Endpoint& ep : cfg.publicNet.listen)
                m_publicNetServer->addLocalEndpoint(ep);
        }

        //flash sandbox
        if(!cfg.flashSandbox.listen.empty())
        {
            m_flashSandboxServer = TcpServer::create();
            for(const net::Endpoint& ep : cfg.flashSandbox.listen)
                m_flashSandboxServer->addLocalEndpoint(ep);

            m_flashSandboxHandler = FlashSandboxHandler::create();
        }

        //http监听
        const auto& httpNet = cfg.httpNet;
        if (!httpNet.listen.empty())
        {
            m_httpServer = TcpServer::create();
            for(const net::Endpoint& ep : cfg.httpNet.listen)
                m_httpServer->addLocalEndpoint(ep);
        }
    }

    {//绑定各种核心事件的处理函数
        using namespace std::placeholders;
        //私网的新连接, 放入连接检查器
        if(m_privateNetServer)
        {
            auto checker = std::bind(&ConnectionChecker::addUncheckedConnection
                                     , m_privateConnChecker
                                     , _1
                                     , ConnectionChecker::ConnType::in);
            m_privateNetServer->e_newConn.reg(checker);
        }
        if(m_privateNetClient)
        {
            auto checker = std::bind(&ConnectionChecker::addUncheckedConnection
                                     , m_privateConnChecker
                                     , _1
                                     , ConnectionChecker::ConnType::out);
            m_privateNetClient->e_newConn.reg(checker);
        }

        if(m_publicNetServer)
        {
            //当有外部连接接入时的处理，这里级直接把新conn加入了connManager，
            //应该要改，给这里加一个checker，像privaterConnectionChecker一样，
            //checker会和clinet做通信，确认client的合法性，并确定client的id
            //然后checker再把验证过的conn加入connManager
        }

        if(m_flashSandboxServer && m_flashSandboxHandler)
        {
            auto handler = std::bind(&FlashSandboxHandler::addFlashSandboxConn,
                                     m_flashSandboxHandler, _1);

            m_flashSandboxServer->e_newConn.reg(handler);
        }

        if (m_httpServer)
        {
            m_httpServer->e_newConn.reg(std::bind(&HttpConnectionManager::addPrivateConnection, 
                                                  &m_httpcons, _1));
        }
        //通过检查的连接加入连接管理器
        m_privateConnChecker->e_connConfirmed.reg(std::bind(&TcpConnectionManager::addPrivateConnection, 
                                                            &m_conns, _1, _2));

        //处理消息接收队列
        regTimer(std::chrono::milliseconds(20), std::bind(&Process::dealTcpPackets, this, _1));
    }
}

void Process::dealTcpPackets(const componet::TimePoint& now)
{
    TcpConnectionManager::ConnectionHolder::Ptr conn;
    net::Packet::Ptr packet;
    while(m_conns.getPacket(&conn, &packet))
    {
        TcpPacket::Ptr tcpPacket = std::static_pointer_cast<TcpPacket>(packet);

        tcpPacketHandle(tcpPacket, conn, now);
    }
}


}}

