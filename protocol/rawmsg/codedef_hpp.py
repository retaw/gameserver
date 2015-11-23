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
search_message_key = ("struct ")

file_src = sys.argv[1]
file_dest = sys.argv[2]

protobuf_or_rawmsg = ""
if "rawmsg" in os.getcwd():
    protobuf_or_rawmsg = "RAWMSG"
elif "protobuf" in os.getcwd():
    protobuf_or_rawmsg = "PROTOBUF"

public_or_private = ""
if "public" in file_src:
    public_or_private = "PUBLIC"
elif "private" in file_src:
    public_or_private = "PRIVATE"


#获取原始头文件名login.h
def getHppFileName():
    fileName = delFront(file_src, public_or_private.lower())
    fileName = fileName.strip("/")
    fileName = fileName.strip()
    return fileName

#ifndef PROTOCOL_RAWMSG_PRIVATE_CODE_LOGIN_HPP
def getIfndefDefineStr():
    fileName = getHppFileName()
    fileName = delBack(fileName, ".h")
    strName =  "#ifndef PROTOCOL_" + protobuf_or_rawmsg + "_" + public_or_private + "_CODE_" + fileName.upper() + "_HPP" + "\n"
    strName += "#define PROTOCOL_" + protobuf_or_rawmsg + "_" + public_or_private + "_CODE_" + fileName.upper() + "_HPP" + "\n" 
    return strName

#获取inlude包含文件
def getIncludeHpp():
    strInclude = getHppFileName()
    strInclude = "\n" + "#include " + '"' + strInclude + '"' + "\n"
    return strInclude


namespace_start_str=""
namespace_end_str="}" + "\n"
endif_str = "\n" + "#endif"

#################################### 生成*.codedef.*.h ################################

ifndef_define_str = getIfndefDefineStr()
hpp_include_str = getIncludeHpp()
write_codedef_handler = open(file_dest, 'w')
write_codedef_handler.write(ifndef_define_str)
write_codedef_handler.write(hpp_include_str)

read_file_handler = open(file_src, 'r')
read_text_list = read_file_handler.readlines()  
for line in read_text_list:
    search_line = line.strip()	#去除前面的空格
    if search_line.find(search_packet_key) == 0:   # 查找namespace只有一个，找到就break
        namespace_start_str = search_line.strip("{")
        namespace_start_str = namespace_start_str.strip(";")
        namespace_start_str = "\n" + namespace_start_str + "\n" + "{" + "\n"
        break;

write_codedef_handler.write(namespace_start_str)
for line in read_text_list:
    search_line = line.strip()  #去除前面的空格
    if search_line.find(search_message_key) == 0:		# 查找struct
        msg_name = delFront(search_line, search_message_key)
        msg_name = msg_name.strip()
        msg_name = msg_name.split(' ')
        msg_name =  msg_name[0]
        msg_name = "    extern const uint32_t code" + msg_name + ";" + "\n"
        write_codedef_handler.write(msg_name)

write_codedef_handler.write(namespace_end_str)
write_codedef_handler.write(endif_str)
print "Parse " + file_src + " OK!"

read_file_handler.close()		#关闭文件句柄	自定义的原始消息号头文件
write_codedef_handler.close()	#关闭文件句柄		*.codedef.*.h

