#ifndef WARTER_NET_HTTP_PACKET_H
#define WARTER_NET_HTTP_PACKET_H
#include <string>
#include "net/packet.h"

namespace water{
namespace process{

class HttpPacket : public net::Packet
{
public:
	explicit HttpPacket();
    explicit HttpPacket(SizeType size);
    explicit HttpPacket(const void* data, SizeType size);
	~HttpPacket();

public: 
	TYPEDEF_PTR(HttpPacket);
	CREATE_FUN_MAKE(HttpPacket);

	//接口
public:
    virtual void addCursor(SizeType add);

private:
	const int32_t getHeaderLen(const char* buf, const int buflen) const;
	const int32_t getBodyLength() const;
	std::string searchHeaderInfo(const std::string& srcStr, const std::string &serachKey, const std::string &endKey) const;

private:
	enum class ReadState
	{
		read_header = 0,
		read_body = 1,
		read_ok = 2
	};
	ReadState state = ReadState::read_header;
	
	const static int MAX_REQUEST_SIZE = 8192;
	std::string m_headerInfo;     //total http head
	uint32_t m_headerLen = 0;
	uint32_t m_bodyLen = 0;
};

}}

#endif
