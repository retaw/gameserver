//================================================
//作者:yechun
//时间:2012年12月29日 星期六 20时55分19秒
//文件名:XmlParse.cpp
//描述:
//==============================================

#include "xml_parse.h"
#include <unistd.h>

namespace water{
namespace componet{

XmlParseNode::~XmlParseNode()
{
}

XmlParseNode::XmlParseNode(XmlParseDoc &_m_doc, const TiXmlElement *_node):m_doc(_m_doc)
{
	curNode = _node;
}

XmlParseNode::operator void *()
{
	return (void*)this->isOk();
}

XmlParseNode XmlParseNode::operator ++()
{
	if (!isOk())
		return XmlParseNode(m_doc, nullptr);
	curNode = curNode->NextSiblingElement(curNode->Value());
	return *this;
}

XmlParseNode XmlParseNode::operator ++(int)
{
	if (!isOk())
		return XmlParseNode(m_doc, nullptr);

	XmlParseNode temp(this->m_doc, curNode);
	curNode = curNode->NextSiblingElement(curNode->Value());
	return temp;
}

bool XmlParseNode::isOk() const
{
	return curNode != nullptr;
}

XmlParseNode XmlParseNode::getChild(const std::string &nodeName)
{
	if (!isOk())
		return XmlParseNode(m_doc, nullptr);
	const TiXmlElement * temp_curNode = curNode->FirstChildElement(nodeName);
	return XmlParseNode(m_doc, temp_curNode);
}

std::string XmlParseNode::getText() const
{
    return curNode->GetText();
}


}}
