#include "process_config.h"

#include "componet/xmlparse.h"
#include "componet/format.h"
#include "componet/string_kit.h"

namespace water{


ProcessConfig::ProcessConfig(const std::string& filePath, ProcessIdentity processId)
:m_filePath(filePath), m_processId(processId)
{
    
}

void ProcessConfig::load()
{
    using componet::XmlParseDoc;
    using componet::XmlParseNode;

    XmlParseDoc test1(m_filePath);
    XmlParseNode root = test1.getRoot();
    if(!root)
        EXCEPTION(LoadProcessConfigFailed, m_filePath + " parse error");

    //读endpoint list
    for(XmlParseNode endpointNode = root.getChild("endpoint"); endpointNode; ++endpointNode)
    {
        int32_t id = endpointNode.getAttr<int32_t>("id");
        if(m_eps.size() != id)
            EXCEPTION(LoadProcessConfigFailed, "invalid endpoint id, [{}]", id);

        net::Endpoint ep(endpointNode.getAttr<std::string>("str"));
        m_eps.push_back(ep);
    }
    m_eps.shrink_to_fit();

    //读对应的process 结点
    const std::string typeStr = processTypeToString(m_processId.type);
    const int32_t num = m_processId.num;
    const std::string processName = componet::format("[{}:{}]", typeStr, num);

    XmlParseNode processNode = root.getChild("process");
    for(; processNode; ++processNode)
    {
        if(processNode.getAttr<std::string>("type") == typeStr &&
           processNode.getAttr<int32_t>("num") == num)
            break;
    }

    //对应结点未找到
    if(!processNode)
        EXCEPTION(LoadProcessConfigFailed, "process {} not exist", processName);

    //开始开始解析私网
    ProcessInfo& info = m_processInfo;
    XmlParseNode privateNetNode = processNode.getChild("privateNet");
    if(!privateNetNode)
        EXCEPTION(LoadProcessConfigFailed, "privateNet not exist, {}", processName);
    //私网监听
    if(!parseEndpointListStr(&info.privateNet.listen, privateNetNode.getAttr<std::string>("listenEps")))
        EXCEPTION(LoadProcessConfigFailed, "invalid private listenEps, {}", processName);
    //私网接入过滤
    if(!parseProcessList(&info.privateNet.acceptWhiteList, 
                                privateNetNode.getAttr<std::string>("acceptWhiteList")))
        EXCEPTION(LoadProcessConfigFailed, "invalid private acceptWhiteList, {}", processName);
    //私网接出
    if(!parseEndpointListStr(&info.privateNet.connect, privateNetNode.getAttr<std::string>("connectEps")))
        EXCEPTION(LoadProcessConfigFailed, "invalid private connectEps, {}", processName);

    //开始解析公网
    XmlParseNode publicNetNode = processNode.getChild("publicNet");
    if(!publicNetNode)
        EXCEPTION(LoadProcessConfigFailed, "publicNet not exist, {}", processName);
    //公网监听
    if(!parseEndpointListStr(&info.publicNet.listen, publicNetNode.getAttr<std::string>("listenEps")))
        EXCEPTION(LoadProcessConfigFailed, "invalid public listenEps, {}", processName);

}

bool ProcessConfig::parseEndpointListStr(std::set<net::Endpoint>* ret, const std::string& epsStr)
{
    ret->clear();
    auto connEpIDs = componet::fromString<std::vector<int32_t>>(epsStr, ",");
    for(int32_t epID : connEpIDs)
    {
        if(epID >= m_eps.size())
            return false;

        ret->insert(m_eps[epID]);
    }

    return true;
}

bool parseProcessList(std::set<ProcessIdentity>* ret, const std::string& epsStr)
{
    ret->clear();
    std::vector<std::string> itmes = componet::splitString(epsStr, " ");
    for(const std::string& item : itmes)
    {
        ProcessIdentity id;
        std::vector<std::string> typeAndNumsStr = componet::splitString(item, ":");
        if(typeAndNumsStr.size() != 2)
            return false;
        id.type = stringToProcessType(typeAndNumsStr[0]);
        if(id.type == ProcessType::none)
            return false;
        auto processNums = componet::fromString<std::vector<int32_t>>(typeAndNumsStr[1], ",");
        for(int32_t num : processNums)
        {
            id.num = num;
            ret->insert(id);
        }
    }

    return true;
}

const ProcessConfig::ProcessInfo& ProcessConfig::getInfo()
{
    return m_processInfo;
}


}

