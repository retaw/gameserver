/*
   Test program for TinyXML.
*/

#include "test.h"

#include <iostream>
#include "../xmlparse/xml_parse.h"

using namespace water;
using namespace componet;

void foo(XmlParseNode rootNode)
{
	XmlParseNode node = rootNode.getChild("WOCAO");
	if (node)
	{   
		std::string str;
		str = node.getAttr<std::string>("str");
		printf("%s\n",str.c_str());
	}   
}

int main()
{
	XmlParseDoc test1("test1.xml"); 

	XmlParseNode rootTest1 = test1.getRoot();
	if (!rootTest1)
	{
		printf("file no exist\n");
	}

	XmlParseDoc xmlparse("test.xml"); 

	XmlParseNode rootNode = xmlparse.getRoot();
	if (!rootNode)
		return 0;

	for (XmlParseNode node = rootNode.getChild("Person"); node; ++node)
	{   
		int id, go; 
		id = go = 0;
		id = node.getAttr<int>("ID");
		go = node.getAttr<int>("GOGO");
		printf("test id=%u, go=%u\n", id, go);
		for (XmlParseNode subNode = node.getChild("age"); subNode; ++subNode)
		{   
			int age;
			age = subNode.getAttr<int>("Age");
			printf("age=%u\n", age);
		}   
	}
	foo(rootNode);

    XmlParseNode node = rootNode.getChild("haha");
    if(!node)
        return 0;
    std::cout << node.getText() << std::endl;
	return 0;  
}

