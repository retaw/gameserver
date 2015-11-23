#include "fenjie_manager.h"
#include "role_manager.h"

#include "protocol/rawmsg/public/fenjie.h"
#include "protocol/rawmsg/public/fenjie.codedef.public.h"

#include "protocol/rawmsg/rawmsg_manager.h"


namespace world{

using namespace std::placeholders;

FenjieManager FenjieManager::m_me;

FenjieManager& FenjieManager::me()
{
	return m_me;
}


void FenjieManager::regMsgHandler()
{
	REG_RAWMSG_PUBLIC(RequestFenjie, std::bind(&FenjieManager::clientmsg_RequestFenjie, this, _1, _2, _3));

}

void FenjieManager::clientmsg_RequestFenjie(const uint8_t* msgData, uint32_t msgSize, RoleId roleId)
{
	Role::Ptr role = RoleManager::me().getById(roleId);
	if(role == nullptr)
		return;

	auto rev = reinterpret_cast<const PublicRaw::RequestFenjie*>(msgData);
	if(!rev)
		return;

	std::vector<uint16_t> cellVec;
	for(ArraySize i = 0; i < rev->size; ++i)
	{
		cellVec.push_back(rev->cell[i]);
	}

	role->m_fenjie.requestFenjie(cellVec);
	return;
}


}
