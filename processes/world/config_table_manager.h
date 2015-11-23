/*
 * Description: 配置表加载文件
 */


#ifndef WORLD_CONFIG_TABLE_MGR_HPP
#define WORLD_CONFIG_TABLE_MGR_HPP

#include "config_table.h"
#include "water/componet/logger.h"
#include "water/componet/exception.h"

#include <unordered_map>
#include <functional>

namespace world{

using namespace water;
using componet::XmlParseDoc;
using componet::XmlParseNode;

template<typename DataStructure>
class ConfigTable
{
public:
    ConfigTable() = default;
    ~ConfigTable() = default;


public:
    static ConfigTable& me()
    {
        static ConfigTable me;
        return me;   
    }

    typedef std::function<void (std::shared_ptr<DataStructure>)> CfgExecFunc;

public:
    void loadCfgTable(std::string filename)
    {
        XmlParseDoc doc(filename);
        XmlParseNode root = doc.getRoot();
        if(!root)
        {
            EXCEPTION(componet::ExceptionBase, filename + " 配置表 parse root node failed");
            return;
        }

        for(XmlParseNode itemNode = root.getChild("item"); itemNode; ++itemNode)
        {
            std::shared_ptr<DataStructure> pdata = std::make_shared<DataStructure>();
            uint32_t id = pdata->parse(itemNode);

            //LOG_DEBUG("{}配置表 id={}", filename, id);
            if(id)
                datas.insert(std::make_pair(id, pdata));
        }
        return;
    }



public:
    std::shared_ptr<DataStructure> get(uint32_t id)
    {
        if(datas.find(id) == datas.end())
            return nullptr;

        return datas[id];
    }


    void execAll(CfgExecFunc func)
    {
        for(const auto& iter : datas)
            func(iter.second);
    }

private:
    std::unordered_map<uint32_t, std::shared_ptr<DataStructure> > datas;
};


class ConfigTableMgr
{
public:
    ConfigTableMgr() = default;
    ~ConfigTableMgr() = default;

public:
    static ConfigTableMgr& me();

    void loadAllCfgTable(const std::string& cfgdir);
};


extern ConfigTable<SkillBase> skillCT;
extern ConfigTable<SkillEffectBase> skillEffectCT;
extern ConfigTable<SkillStrengthenBase> skillStrengthenCT;
extern ConfigTable<BuffBase> buffCT;
extern ConfigTable<LevelPropsBase> levelPropCT;
extern ConfigTable<KValueBase> kCT;

}

#endif

