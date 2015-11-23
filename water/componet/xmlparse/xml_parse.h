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

namespace water{
namespace componet{

class XmlParseNode
{
	friend XmlParseDoc;
private:
	XmlParseNode(XmlParseDoc* doc = nullptr, const TiXmlElement* _node = nullptr);
public:
	~XmlParseNode();
	XmlParseNode(const XmlParseNode& other);
    XmlParseNode& operator=(const XmlParseNode& other);

	bool isOk() const;
	operator void *();
	XmlParseNode operator ++();
	XmlParseNode operator ++(int);


	XmlParseNode getChild(const std::string &nodeName) const;

    template<typename T>
    T getText() const
    {
        if(!isOk())
            return T();

        T temp;
        if(getText(&temp))
            return temp;
        return T();
    }

    template<typename T>
    T getChildNodeText(const std::string& nodeName) const
    {
        XmlParseNode childNode = getChild(nodeName);
        if(!childNode)
            return T();

        return childNode.getText<T>();
    }


	template<typename T>
	T getAttr(const std::string& nodeName) const
	{
		if (!isOk())
			return T();

		T temp;
		if(getAttr(nodeName, &temp))
            return temp;
        return T();
	}

private:
	template<typename T>
    bool getAttr(const std::string& nodeName, T* t) const
    {
		return TIXML_SUCCESS == m_curNode->QueryValueAttribute(nodeName, t);
    }

    //避免char和uint8_t混淆, 不提供字符类型, 有符号和无符号char全部解释为整数
    //需要字符的情况都请按字符串来读
    bool getAttr(const std::string& nodeName, unsigned char* t) const
    {
        uint16_t temp = 0;
        if(TIXML_SUCCESS != m_curNode->QueryValueAttribute(nodeName, &temp))
            return false;
        *t = static_cast<unsigned char>(temp);
        return true;
    }
    bool getAttr(const std::string& nodeName, char* t) const
    {
        int16_t temp = 0;
        if(TIXML_SUCCESS != m_curNode->QueryValueAttribute(nodeName, &temp))
            return false;
        *t = static_cast<char>(temp);
        return true;
    }

	bool getAttr(const std::string& nodeName, bool* t) const
	{
		std::string attrStr;
		if(!getAttr(nodeName, &attrStr))
			return false;

		if(attrStr == "true" || attrStr == "TRUE")
		{
			*t = true;
			return true;
		}

		if(attrStr == "false" || attrStr == "FALSE")
		{
			*t = false;
			return true;
		}

		uint16_t temp = 0;
		if(!getAttr(nodeName, &temp))
			return false;

		*t = static_cast<bool>(temp);
		return true;
	}

    template<typename T>
    bool getText(T* t) const
    {
        const char *text = m_curNode->GetText();
        if(!text)
            return false;

        std::stringstream sstream(text);
        sstream >> *t;
        if(sstream.fail())
            return false;

        return true;
    }

    //同getAttr
    bool getText(unsigned char* t) const
    {
        uint16_t temp = 0;
        const char *text = m_curNode->GetText();
        if(!text)
            return false;

        std::stringstream sstream(text);
        sstream >> temp;
        if(sstream.fail())
            return false;

        *t = static_cast<unsigned char>(temp);
        return true;
    }


    bool getText(char* t) const
    {
        int16_t temp = 0;
        const char *text = m_curNode->GetText();
        if(!text)
            return false;

        std::stringstream sstream(text);
        sstream >> temp;
        if(sstream.fail())
            return false;

        *t = static_cast<char>(temp);
        return true;
    }


    bool getText(std::string* out) const
    {
        const char *text = m_curNode->GetText();
        if(!text)
            return false;

        *out = text;
        return true;
    }

	bool getText(bool *t) const
	{
		std::string str;
		if(!getText(&str))
			return false;

		if(str == "true" || str == "TRUE")
		{
			*t = true;
			return true;
		}

		if(str == "false" || str == "FALSE")
		{
			*t = false;
			return true;
		}

		uint16_t temp = 0;
		if(!getText(&temp))
			return false;

		*t = static_cast<bool>(temp);
		return true;
	}

public:
    static XmlParseNode getInvalidNode();

private:
	XmlParseDoc* m_doc;
	const TiXmlElement *m_curNode;
};

}}
#endif
