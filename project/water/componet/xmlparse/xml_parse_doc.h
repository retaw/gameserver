/*
 * Author: YeChun 
 *
 * Last modified: 2014-11-18 14:32 +0800
 *
 * Description: 
 */

#ifndef WATER_COMPONET_XML_PARSE_DOC_H
#define WATER_COMPONET_XML_PARSE_DOC_H

#include "tinyxml.h"
#include <memory>

namespace water{
namespace componet{

class XmlParseNode;

class XmlParseDoc
{
public:
	XmlParseDoc(const std::string &file_name);
	~XmlParseDoc();
public:
	XmlParseNode getRoot();
private:
	std::shared_ptr<TiXmlDocument> document;
};

}}

#endif
