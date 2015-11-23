#include "xml_parse_doc.h"
#include "xml_parse.h"
#include <unistd.h>


namespace water{
namespace componet{

XmlParseDoc::XmlParseDoc(const std::string& file_name)
: document(std::make_shared<TiXmlDocument>())
{
    if (::access(file_name.c_str(), 0) != 0)  //文件不存在
    {
        return ;
    }
    if (!document->LoadFile(file_name))
    {
        return ;
    }
}

XmlParseDoc::~XmlParseDoc()
{
}

XmlParseNode XmlParseDoc::getRoot()
{
    if (document)
    {
        const TiXmlElement *temp_curNode = document->RootElement();
        return XmlParseNode(this, temp_curNode);
    }
    return XmlParseNode(this, nullptr);
}

}}
