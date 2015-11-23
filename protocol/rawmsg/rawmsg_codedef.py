#-*- encoding:UTF-8 -*-  
import os
import sys 

def delBack(s, sub):
    l1 = len(s)
    l2 = len(sub)

    if l1 < l2 :
        return s

    back = s[ -l2 : ] 
    if cmp(back, sub) == 0:
        return s[: -l2]
    return s

def delFront(s, sub):
    l1 = len(s)
    l2 = len(sub)

    if l1 < l2 :
        return s

    front = s[: l2] 
    if cmp(front, sub) == 0 : 
        return s[l2 :]
    return s

search_packet_key = ("namespace ")  #空格不能去
search_message_key = ("extern ")
message_code = 0x80000000 | 0x00000000

#sys.argv[1]	config文件夹路径 
#sys.argv[2]	生成cpp 或 xml
#sys.argv[3]	外部消息或内部消息 public 或 private

config_path = sys.argv[1]
cpp_or_xml = sys.argv[2]
hpp_folder = sys.argv[3]

protobuf_or_rawmsg = ""
public_or_private = ""
if "rawmsg" in os.getcwd():
    protobuf_or_rawmsg = "RAWMSG"
elif "protobuf" in os.getcwd():
    protobuf_or_rawmsg = "PROTOBUF"

if "public" == hpp_folder:
    public_or_private = "PUBLIC"
elif "private" == hpp_folder:
    public_or_private = "PRIVATE"

if "PROTOBUF" == protobuf_or_rawmsg and "PUBLIC" == public_or_private:
    message_code = 0
elif "PROTOBUF" == protobuf_or_rawmsg and "PRIVATE" == public_or_private:
    message_code = 0x80000000 | 0x10000000
elif "RAWMSG" == protobuf_or_rawmsg and "PUBLIC" == public_or_private:
    message_code = 0
elif "RAWMSG" == protobuf_or_rawmsg and "PRIVATE" == public_or_private:
    message_code = 0x80000000 | 0x00000000


#过滤文件 *.codedef.private.h
codedef_files_list = []
def anyTrue(predicate, sequence):
    return True in map(predicate, sequence)

def filterFiles(folder, exts):
    for fileName in os.listdir(folder):
        if os.path.isfile(folder + '/' + fileName) :
            if not anyTrue(fileName.endswith, exts):
                continue
            codedef_files_list.append(fileName)
    codedef_files_list.sort()
    print codedef_files_list

exts = ['.codedef.' + hpp_folder + '.h']
path = os.getcwd() + '/' + hpp_folder
filterFiles(path, exts)


#包含头文件
def getCppHeader(fileName):
    headerName = '#include ' + '"' + fileName + '"' + "\n" 
    return headerName

#获取包含头文件 #include "login.h"
def getCppHeaderStr():
    cpp_header_str=""
    for fileName in codedef_files_list:
        cpp_header_str = cpp_header_str + getCppHeader(fileName)

    cpp_header_str = cpp_header_str + '\n'
    return cpp_header_str

def getCppMsgName(search_line):
    msg_name = delFront(search_line, search_message_key)	 
    msg_name = msg_name.strip(";")  
    msg_name = msg_name.strip()		#去除空格
    return msg_name

def getXmlMsgName(search_line):
    msg_name = delFront(search_line, "extern const uint32_t code")	 
    msg_name = msg_name.strip(";")  
    msg_name = msg_name.strip()		#去除空格
    return msg_name

#获取命名空间名称
def getNamespaceStr():
    for fileName in codedef_files_list:
        fileName = path + '/' + fileName
        read_file_handler = open(fileName, 'r')
        read_text_list = read_file_handler.readlines()  
        for line in read_text_list:
            search_line = line.strip()	#去除前面空格
            if search_line.find(search_packet_key) == 0:   # 查找namespace只有一个，找到就break
                space_str = delFront(search_line, search_packet_key)
                space_str = space_str.strip("{")
                space_str = space_str.strip(";")

                read_file_handler.close()
                return space_str


#***************************** 仅生成一个*.xml 及*.cpp 消息号文件 ********************************
if "cpp" == cpp_or_xml:
    namespace_str = getNamespaceStr()
    cpp_namespace_start_str="namespace " + namespace_str + "\n" + "{" + "\n"
    cpp_namespace_end_str="}"

    codedef_file_name = path + '/' + 'codedef.' + public_or_private.lower() + '.cpp'
    cpp_start_str = getCppHeaderStr()
    write_cpp_handler = open(codedef_file_name, 'w')
    write_cpp_handler.write(cpp_start_str)
    write_cpp_handler.write(cpp_namespace_start_str)

    for fileName in codedef_files_list:
        fileName = path + '/' + fileName
        read_file_handler = open(fileName, 'r')
        read_text_list = read_file_handler.readlines()  

        for line in read_text_list:
            search_line = line.strip()  #去除前面的空格
            if search_line.find(search_message_key) == 0:		# 查找extern
                message_code += 1
                cpp_msg_name = getCppMsgName(search_line)
                cpp_msg = "    " + cpp_msg_name + "\t= " + `message_code` + ";\n"
                write_cpp_handler.write(cpp_msg)

        print "cpp, Parse " + fileName + " OK!"
        read_file_handler.close()	#关闭文件句柄	自定义的原始消息号头文件

    write_cpp_handler.write(cpp_namespace_end_str)
    write_cpp_handler.close()	#关闭文件句柄	写codedef.private.cpp 消息号

elif "xml" == cpp_or_xml:
    xml_start_str = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<root>\n"
    xml_end_str = "</root>"
    xml_file_name = config_path + "/rawmsg.codedef." + public_or_private.lower() + ".xml"	#rawmsg.codedef.public.xml 或 rawmsg.codedef.private.xml 
    write_xml_hanlder = open(xml_file_name, 'w')
    write_xml_hanlder.write(xml_start_str)

    for fileName in codedef_files_list:
        fileName = path + '/' + fileName
        read_file_handler = open(fileName, 'r')
        read_text_list = read_file_handler.readlines()  

        for line in read_text_list:
            search_line = line.strip()  #去除前面的空格
            if search_line.find(search_message_key) == 0:		# 查找extern
                message_code += 1

                xml_msg_name = getXmlMsgName(search_line)
                xml_msg = "\t<item " + "msg_code=\"" + `message_code` +  "\" msg_name=\"" + xml_msg_name + "\"/>\n"  
                write_xml_hanlder.write(xml_msg)

        print "Xml, Parse " + fileName + " OK!"
        read_file_handler.close()	#关闭文件句柄	自定义的原始消息号头文件

    write_xml_hanlder.write(xml_end_str)
    write_xml_hanlder.close()	#关闭文件句柄	写rawmsg.codedef.private.xml消息号
