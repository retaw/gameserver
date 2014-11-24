#include "process_config.h"

#include "componet/xmlparse.h"
#include "componet/format.h"

namespace water{

ProcessConfig::ProcessConfig(const std::string& filePath)
:m_filePath(filePath)
{
}

bool ProcessConfig::load()
{
    using componet::XmlParseDoc;
    using componet::XmlParseNode;

    XmlParseDoc test1(m_filePath);
    XmlParseNode root = test1.getRoot();
    if(!root)
        EXCEPTION(LoadProcessConfigFailed, m_filePath + " parse error");

    std::array<std::string, 2> processTypeStr = {"router", "function"};
    for(int32_t i = 0; i < processTypeStr.size(); ++i)
    {
        const std::string processNodeName = processTypeStr[i];

        for(XmlParseNode processNode = root.getChild(processNodeName); processNode; ++processNode)
        {
            int32_t id = processNode.getAttr<uint32_t>("id");
            auto& infos = m_data[static_cast<ProcessType>(i)];
            if(infos.size() != id)
                EXCEPTION(LoadProcessConfigFailed, componet::format("invald id, [{},{}]", infos.size(), id));

            NetInfo info;
            for(XmlParseNode listen = processNode.getChild("listen"); listen; ++listen)
            {
                net::Endpoint ep;
                ep.fromString(listen.getText());
                info.listen.insert(ep);
            }
            for(XmlParseNode connect = processNode.getChild("connect"); connect; ++connect)
            {
                net::Endpoint ep;
                ep.fromString(connect.getText());
                info.connect.insert(ep);
            }

            infos.push_back(info);
            infos.shrink_to_fit();
        }
    }
}

const ProcessConfig::NetInfo& ProcessConfig::getInfo(ProcessType type, int const id)
{
    auto it = m_data.find(type);
    if(it == m_data.end())
        EXCEPTION(ProcessConfigNotExist, componet::format("type {} no exist", type));

    if(it->second.size() <= id)
        EXCEPTION(ProcessConfigNotExist, componet::format("id {} no exist", id));

    return it->second[id];
}


}
