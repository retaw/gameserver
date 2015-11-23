#include "map_base.h"
#include "water/componet/xmlparse.h"
#include "water/componet/logger.h"

namespace world{

MapBase& MapBase::me()
{
    static MapBase me;
    return me;
}

void MapBase::loadConfig(const std::string& cfgdir)
{
    using namespace water;
    using componet::XmlParseDoc;
    using componet::XmlParseNode;

    const std::string cfgFile = cfgdir + "/scene.xml";
    LOG_TRACE("读取配置文件 {}", cfgFile);

    XmlParseDoc doc(cfgFile);
    XmlParseNode root = doc.getRoot();
    if(!root)
        EXCEPTION(componet::ExceptionBase, cfgFile + " parse root node failed");

    for(XmlParseNode itemNode = root.getChild("item"); itemNode; ++itemNode)
    {
        MapTpl::Ptr mapTpl = MapTpl::create();
        mapTpl->id = itemNode.getChildNodeText<MapId>("id");
        mapTpl->type = static_cast<CopyMap>(itemNode.getChildNodeText<uint8_t>("type"));
        mapTpl->crime = itemNode.getChildNodeText<uint8_t>("crime");
        mapTpl->mode = static_cast<attack_mode>(itemNode.getChildNodeText<uint8_t>("mode"));
        mapTpl->ride = itemNode.getChildNodeText<uint8_t>("ride");
        mapTpl->dropEquip = itemNode.getChildNodeText<uint8_t>("dropEquip");
        mapTpl->roleMinTurnLife = (TurnLife)itemNode.getChildNodeText<uint8_t>("turnLife");
        mapTpl->roleMinLevel = itemNode.getChildNodeText<uint32_t>("levelLimit");
        mapTpl->reliveyb = itemNode.getChildNodeText<uint16_t>("reliveyb");
        mapTpl->objTplId = itemNode.getChildNodeText<uint32_t>("objTplId");
        mapTpl->objNum = itemNode.getChildNodeText<uint16_t>("objNum");

        m_mapTpls.insert({mapTpl->id, mapTpl});
    }
}

MapTpl::Ptr MapBase::getMapTpl(MapId mapId)
{
    if(m_mapTpls.find(mapId) == m_mapTpls.end())
        return nullptr;
    return m_mapTpls[mapId];
}

}

