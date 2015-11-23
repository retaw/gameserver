#include "randname_manager.h"
#include "role_manager.h"
#include "water/componet/logger.h"
#include "protocol/rawmsg/rawmsg_manager.h"
#include "protocol/rawmsg/private/login.h"
#include "protocol/rawmsg/private/login.codedef.private.h"
#include "water/componet/xmlparse.h"
#include <vector>


namespace dbcached{

RandNameManager& RandNameManager::me()
{
    static RandNameManager me;
    return me;
}

void RandNameManager::init(const std::string& nameDir)
{
    initNameVec(nameDir);
}

void RandNameManager::initNameVec(const std::string& nameDir)
{
    using componet::XmlParseDoc;
    using componet::XmlParseNode;
    LOG_DEBUG("开始读取姓名库,{}",nameDir);
    XmlParseDoc doc(nameDir);
    XmlParseNode items = doc.getRoot();
    if(!items)
        EXCEPTION(LoadNameConfigFailed,nameDir + "parse error");
    for(XmlParseNode itemNode = items.getChild("item"); itemNode; ++itemNode)
    {
        std::string undefined("undefined");
        std::string familyName(itemNode.getChild("familyName").getText<std::string>());
        std::string maleName(itemNode.getChild("maleName").getText<std::string>());
        std::string femaleName(itemNode.getChild("femaleName").getText<std::string>());
        if(familyName.compare(undefined) != 0)
            m_familyNameVec.emplace_back(familyName);
        if(maleName.compare(undefined) != 0)
            m_maleNameVec.emplace_back(maleName);
        if(femaleName.compare(undefined) != 0)
            m_femaleNameVec.emplace_back(femaleName); 
    }
    //初始化随机种子
    m_familyRandPtr = std::make_shared<water::componet::Random<uint32_t>>(0,m_familyNameVec.size()-1);
    m_maleRandPtr = std::make_shared<water::componet::Random<uint32_t>>(0,m_maleNameVec.size()-1);
    m_femaleRandPtr = std::make_shared<water::componet::Random<uint32_t>>(0,m_femaleNameVec.size()-1);
    LOG_DEBUG("得到姓名库:m_familyNameVec={},m_maleNameVec={},m_femaleNameVec={}",m_familyNameVec.size(),m_maleNameVec.size(),m_femaleNameVec.size());

}

std::string RandNameManager::getRandName(Sex sex)
{
    if(sex == Sex::male)
    {
        std::string randName = m_familyNameVec[m_familyRandPtr->get()] + m_maleNameVec[m_maleRandPtr->get()];
        return randName;
    }
    else
    {
        std::uniform_int_distribution<uint32_t> disfemale(0,m_femaleNameVec.size()-1);
        std::string randName = m_familyNameVec[m_familyRandPtr->get()] + m_femaleNameVec[m_femaleRandPtr->get()];
        return randName;
    }
}

void  RandNameManager::servermsg_GetRandName(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId)
{
    auto rev = reinterpret_cast<const PrivateRaw::GetRandName*>(msgData);
    ProcessIdentity pid(remoteProcessId);
    LOG_DEBUG("收到随机名请求,loginId={},remoteId={}",rev->loginId,pid);

    std::string randName = getRandName(rev->sex);
    while (RoleManager::me().isExisitName(randName))
    {
        randName = getRandName(rev->sex);
    }
    PrivateRaw::RetRandName send;
    send.loginId = rev->loginId;
    std::memset(send.name,0,sizeof(send.name));
    randName.copy(send.name,sizeof(send.name) - 1);
    DbCached::me().sendToPrivate(pid, RAWMSG_CODE_PRIVATE(RetRandName), &send, sizeof(send));
    LOG_DEBUG("返回随机名请求,name={}",randName);
}

void RandNameManager::regMsgHandler()
{
    using namespace std::placeholders;
    REG_RAWMSG_PRIVATE(GetRandName, std::bind(&RandNameManager::servermsg_GetRandName, this, _1, _2, _3));
}

}
