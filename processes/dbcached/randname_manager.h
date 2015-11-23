
#include "water/common/roledef.h"
#include "dbcached.h"
#include "componet/exception.h"
#include "water/process/process_id.h"
#include "componet/random.h"
#include <time.h>
namespace dbcached{

DEFINE_EXCEPTION(LoadNameConfigFailed, componet::ExceptionBase)
DEFINE_EXCEPTION(NameCfgError, componet::ExceptionBase)
class RandNameManager
{
public:
    ~RandNameManager() = default;
    static RandNameManager& me();
    void regMsgHandler();
    void init(const std::string& nameDir);//初始化，加载姓和名
private:
    //for regMsgHandler
    void  servermsg_GetRandName(const uint8_t* msgData, uint32_t msgSize, uint64_t remoteProcessId);
    //
    void initNameVec(const std::string& nameDir);
    //
    std::string getRandName(Sex sex);
private:
    RandNameManager() = default;

    std::vector<std::string> m_familyNameVec;
    std::vector<std::string> m_maleNameVec;
    std::vector<std::string> m_femaleNameVec;

    //std::random_device m_seed;
    std::shared_ptr<water::componet::Random<uint32_t>> m_familyRandPtr;
    std::shared_ptr<water::componet::Random<uint32_t>> m_maleRandPtr;
    std::shared_ptr<water::componet::Random<uint32_t>> m_femaleRandPtr;

};

}
