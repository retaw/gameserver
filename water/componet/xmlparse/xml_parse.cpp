//================================================
//作者:yechun
//时间:2012年12月29日 星期六 20时55分19秒
//文件名:XmlParse.cpp
//描述:
//==============================================

#include "xml_parse.h"

namespace water{
namespace componet{

XmlParseNode XmlParseNode::getInvalidNode()
{
    return XmlParseNode();
}

XmlParseNode::~XmlParseNode()
{
}

XmlParseNode::XmlParseNode(XmlParseDoc* doc, const TiXmlElement* node)
: m_doc(doc), m_curNode(node)
{
}

XmlParseNode::XmlParseNode(const XmlParseNode& other)
{
    *this = other;
}

XmlParseNode::operator void* ()
{
	return isOk() ? reinterpret_cast<void*>(1) : nullptr;
}

XmlParseNode XmlParseNode::operator ++()
{
	if (!isOk())
		return XmlParseNode(m_doc, nullptr);
	m_curNode = m_curNode->NextSiblingElement(m_curNode->Value());
	return *this;
}

XmlParseNode XmlParseNode::operator ++(int)
{
	if (!isOk())
		return XmlParseNode(m_doc, nullptr);

	XmlParseNode temp(m_doc, m_curNode);
	m_curNode = m_curNode->NextSiblingElement(m_curNode->Value());
	return temp;
}

XmlParseNode& XmlParseNode::operator=(const XmlParseNode& other)
{
    m_doc = other.m_doc;
    m_curNode = other.m_curNode;

    return *this;
}

bool XmlParseNode::isOk() const
{
	return m_doc != nullptr && m_curNode != nullptr;
}

XmlParseNode XmlParseNode::getChild(const std::string &nodeName) const
{
	if (!isOk())
		return XmlParseNode(m_doc, nullptr);
	const TiXmlElement * temp_curNode = m_curNode->FirstChildElement(nodeName);
	return XmlParseNode(m_doc, temp_curNode);
}


}}
