#include "process_config.h"

#include "componet/logger.h"
#include "componet/xmlparse.h"
#include "componet/format.h"
#include "componet/string_kit.h"

namespace water{
namespace process{


ProcessConfig::ProcessConfig(const std::string& processName, int16_t processNum)
:m_processName(processName), m_processNum(processNum)
{
}

void ProcessConfig::load(const std::string& cfgDir)
{
    using componet::XmlParseDoc;
    using componet::XmlParseNode;

    const std::string configFile = cfgDir + "/process.xml";

    LOG_TRACE("读取主配置文件 {}", configFile);

    XmlParseDoc doc(configFile);
    XmlParseNode root = doc.getRoot();
    if(!root)
        EXCEPTION(LoadProcessConfigFailed, configFile + " parse root node failed");

    {//通用配置
        XmlParseNode commonNode = root.getChild("common");
        if(!commonNode)
            EXCEPTION(LoadProcessConfigFailed, "common node not exisit");

        auto platformValue = commonNode.getAttr<uint16_t>("platform");

        m_platform = static_cast<Platform>(platformValue);

        m_zoneId = commonNode.getAttr<ZoneId>("zone");
        m_processId.zoneId() = m_zoneId;
        m_opentime = componet::stringToTimePoint(commonNode.getAttr<std::string>("opentime"));
        m_mergeFlag = commonNode.getAttr<bool>("mergeflag");

        ProcessIdentity::s_zoneId = m_zoneId;

        LOG_TRACE("platform = {}, zoneId = {}", m_platform, m_zoneId);
    }

    {//进程拓扑结构
        XmlParseNode allProcessesNode = root.getChild("allProcesses");
        if(!allProcessesNode)
            EXCEPTION(LoadProcessConfigFailed, "allProcesses node not exisit");

        {//进程类型声明, 生成并建立processType和processName的映射, 记录在ProcessIdendity中

            const std::string processTypeListStr = allProcessesNode.getAttr<std::string>("typeList");
            std::vector<std::string> allProcessTypeNames = componet::splitString(processTypeListStr, " ");

            for(auto& name : allProcessTypeNames)
            {
                //ProcessIdendity::type2Name
                ProcessIdentity::s_type2Name.push_back(name);

                //ProcessIdendity::name2Type
                ProcessType type = ProcessIdentity::s_type2Name.size() - 1;
                ProcessIdentity::s_name2Type[name] = type;
            }
            ProcessIdentity::s_type2Name.shrink_to_fit();
        }

        
        {   //对每个zone节点下的processType节点做一次遍历，
            //做好processId 到privateListen的Endpoint映射
            //并找到当前进程类型对应的配置结点
            XmlParseNode theProcessNode = XmlParseNode::getInvalidNode();
            for(XmlParseNode zoneNode = allProcessesNode.getChild("zone"); zoneNode; ++zoneNode)
            {
                ZoneId zoneId = m_zoneId;
                const auto zoneIdStr = zoneNode.getAttr<std::string>("id");
                if(zoneIdStr != "default")
                    zoneId = zoneNode.getAttr<ZoneId>("id");

                for(XmlParseNode processTypeNode = zoneNode.getChild("processType"); processTypeNode; ++processTypeNode)
                {
                    auto name = processTypeNode.getAttr<std::string>("name");
                    ProcessType type = ProcessIdentity::stringToType(name);
                    if(type == INVALID_PROCESS_TYPE)
                        EXCEPTION(ProcessCfgNotExisit, "{} is not exisit in allProcesses.typeList", name);

                    //processType.process.private.listen
                    for(XmlParseNode processNode = processTypeNode.getChild("process"); processNode; ++processNode)
                    {
                        auto num = processNode.getAttr<int32_t>("num");
                        ProcessIdentity processId(zoneId, name, num);
                        XmlParseNode privateNode = processNode.getChild("private");
                        if(!privateNode)
                            EXCEPTION(ProcessCfgNotExisit, "process cfg {} do not has {} node", processId, "private");

                        auto endPointStr = privateNode.getAttr<std::string>("listen");
                        std::shared_ptr<net::Endpoint> privateListen;
                        if(!endPointStr.empty())
                        {
                            privateListen.reset(new net::Endpoint(endPointStr));
                            m_processIdPrivateListenEps[processId] = *privateListen;
                        }

                        //process self node
                        if(zoneId == m_zoneId && name == m_processName && num == m_processNum)
                        {
                            m_processId.type() = type;
                            m_processId.num() = m_processNum;

                            theProcessNode = processNode;
                            m_processInfo.privateNet.listen = privateListen;
                        }
                    }
                }
            }

            if(!theProcessNode)
            {
                //出错，这此时的m_processId不可靠，日志中的processFullName要自己拼
                auto processFullName = componet::format("[{}:{}]", m_processName, m_processNum);
                EXCEPTION(ProcessCfgNotExisit, "进程{}在配置文件中不存在", processFullName);
            }

            ProcessInfo& info = m_processInfo;

            //开始开始解析私网除listen外的部分
            XmlParseNode privateNode = theProcessNode.getChild("private");
            if(!privateNode) //私网配置节点必须存在
                EXCEPTION(ProcessCfgNotExisit, "进程 {} 下缺少 private 配置", m_processId);
            //私网接出
            if(!parsePrivateEndpoint(&info.privateNet.connect, privateNode.getAttr<std::string>("connectProcess")))
                EXCEPTION(LoadProcessConfigFailed, "进程 {} 属性 connectProcess 解析失败", m_processId);

            //解析公网
            XmlParseNode publicNode = theProcessNode.getChild("public");
            if(publicNode)
                parseEndpointList(&info.publicNet.listen, publicNode.getAttr<std::string>("listen"));

            //解析Flash Sandbox
            XmlParseNode flashSandboxNode = theProcessNode.getChild("flashSandbox");
            if(flashSandboxNode)
                parseEndpointList(&info.flashSandbox.listen, flashSandboxNode.getAttr<std::string>("listen")); 

            //解析http
            XmlParseNode httpNode = theProcessNode.getChild("http");
            if (httpNode)
                parseEndpointList(&info.httpNet.listen, httpNode.getAttr<std::string>("listen"));
        }
    }
}

bool ProcessConfig::parsePrivateEndpoint(std::set<net::Endpoint>* ret, const std::string& str)
{
    ret->clear();

    std::set<ProcessIdentity> processIds;
    if(!parseProcessList(&processIds, str))
        return false;

    for(const auto& processId : processIds)
    {
        auto it = m_processIdPrivateListenEps.find(processId);
        if(it == m_processIdPrivateListenEps.end())
            return false;

        ret->insert(it->second);
    }

    return true;
}

bool ProcessConfig::parseProcessList(std::set<ProcessIdentity>* ret, const std::string& str)
{
    ret->clear();
    std::vector<std::string> items;
    componet::splitString(&items, str, " ");

    for(const std::string& item : items)
    {
        ProcessIdentity id;
        std::vector<std::string> processStr = componet::splitString(item, ":");
        if(processStr.size() ==  2)
        {
            id.zoneId() = m_zoneId;
            id.type() = ProcessIdentity::stringToType(processStr[0]);
            if(id.type() == INVALID_PROCESS_TYPE)
                return false;

            auto processNums = componet::fromString<std::vector<int16_t>>(processStr[1], ",");

            for(int32_t num : processNums)
            {
                id.num() = num;
                ret->insert(id);
            }
        }
        else if(processStr.size() == 3)
        {
            id.zoneId() = componet::fromString<ZoneId>(processStr[0]);
            id.type() = ProcessIdentity::stringToType(processStr[1]);
            if(id.type() == INVALID_PROCESS_TYPE)
                continue;
            auto processNums = componet::fromString<std::vector<int16_t>>(processStr[2], ",");

            for(int32_t num : processNums)
            {
                id.num() = num;
                ret->insert(id);
            }
        }
        else
        {
            return false;
        }
    }

    return true;
}

void ProcessConfig::parseEndpointList(std::set<net::Endpoint>* ret, const std::string& str)
{
    ret->clear();
    std::vector<std::string> endpointStrs = componet::splitString(str, " ");
    for(const auto& endpointStr : endpointStrs)
    {
        net::Endpoint ep(endpointStr);
        ret->insert(ep);
    }
}

const ProcessConfig::ProcessInfo& ProcessConfig::getInfo()
{
    return m_processInfo;
}

ProcessIdentity ProcessConfig::getProcessId() const
{
    return m_processId;
}

ZoneId ProcessConfig::getZoneId() const
{
    return m_zoneId;
}

Platform ProcessConfig::getPlatform() const
{
    return m_platform;
}

componet::TimePoint ProcessConfig::opentime() const
{
    return m_opentime;
}

bool ProcessConfig::mergeFlag() const
{
    return m_mergeFlag;
}

}}

