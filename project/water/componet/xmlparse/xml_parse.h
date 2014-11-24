//================================================
//作者:yechun
//时间:2014年11月15日 星期六 15时43分35秒
//文件名:xmlparse/xml_parse.h
//描述: XML 解析器，当前版本只支持文件分析
//==============================================
#ifndef WATER_COMPONET_XML_PARSE_H
#define WATER_COMPONET_XML_PARSE_H

#include "tinyxml.h"
#include "xml_parse_doc.h"

#include <string>
#include <type_traits>

namespace water{
namespace componet{

class XmlParseNode
{
	friend XmlParseDoc;
private:
	XmlParseNode(XmlParseDoc &_m_doc, const TiXmlElement *_node = nullptr);
public:
	~XmlParseNode();
	bool isOk() const;
	operator void *();
	XmlParseNode operator ++();
	XmlParseNode operator ++(int);
	XmlParseNode getChild(const std::string &nodeName) ;

    std::string getText() const;

	template<typename T>
	T getAttr(const std::string &nodeName) const
	{
		if (!isOk())
			return 0;

		T temp;
		curNode->QueryValueAttribute(nodeName, &temp);
		return temp;
	}

private:
	XmlParseDoc& m_doc;
	const TiXmlElement *curNode;
};

}}
#endif
