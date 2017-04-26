RAWMSG_ROOT_DIR = $(shell pwd)


#对外协议定义文件存放目录
PUBLIC_PROTOCOL_DEFINITION_DIR = $(RAWMSG_ROOT_DIR)/public

#内部协议定义文件存放目录
PRIVATE_PROTOCOL_DEFINITION_DIR = $(RAWMSG_ROOT_DIR)/private

#外部协议定义文件
PUBLIC_RAWMSG_SOURCE_FILE  = $(wildcard $(PUBLIC_PROTOCOL_DEFINITION_DIR)/*.h)

#内部协议定义文件
PRIVATE_RAWMSG_SOURCE_FILE = $(wildcard $(PRIVATE_PROTOCOL_DEFINITION_DIR)/*.h)

objects = \
	rawmsg_manager.o\
	public/codedef.public.o\
	private/codedef.private.o\

public_msgcodedef_list = \
	public/login.codedef.public.h\

private_msgcodedef_list = \
	private/login.codedef.private.h\
	private/relay.codedef.private.h\

CC = g++
cc = g++

cc_flags = -std=c++11 -ggdb -Wall -Werror\
		   $(optimization_flag)\
		   -I$(ROOT_DIR)

target_lib = librawmsg


.PHONY:
all: $(target_lib).a public/rawmsg.codedef.public.xml private/rawmsg.codedef.private.xml

#rawmsg_codedef.py 参数说明
#argv[1]: $(CONF_DIR) config路径
#argv[2]: 生成消息号文件 *.cpp 或 *.xml (cpp 或 xml) 
#argc[3]: 生成外部消息号 或 内部消息号 (public 或 private)

#外部的*.codedef.public.h文件生成消息号文件codedef.public.cpp
public/codedef.public.cpp : $(public_msgcodedef_list)
	@curdir=$$(pwd);\
	cd $(RAWMSG_ROOT_DIR); \
	python rawmsg_codedef.py $(CONF_DIR) cpp public || exit 1; \

#内部的*.codedef.private.h文件生成消息号文件codedef.private.cpp
private/codedef.private.cpp : $(private_msgcodedef_list)
	@curdir=$$(pwd);\
	cd $(RAWMSG_ROOT_DIR);\
	python rawmsg_codedef.py $(CONF_DIR) cpp private || exit 1; \


#外部的*.codedef.private.h文件生成序列号xml rawmsg.codedef.public.xml
public/rawmsg.codedef.public.xml : $(public_msgcodedef_list)
	@curdir=$$(pwd);\
	cd $(RAWMSG_ROOT_DIR);\
	python rawmsg_codedef.py $(PUBLIC_PROTOCOL_DEFINITION_DIR) xml public || exit 1; \

#内部的*.codedef.private.h文件生成序列号xml rawmsg.codedef.private.xml
private/rawmsg.codedef.private.xml : $(private_msgcodedef_list)
	@curdir=$$(pwd);\
	cd $(RAWMSG_ROOT_DIR);\
	python rawmsg_codedef.py $(PRIVATE_PROTOCOL_DEFINITION_DIR) xml private || exit 1; \


#根据 *.h 生成 *.codedef.public.h
$(public_msgcodedef_list): %.codedef.public.h: %.h
	python codedef_hpp.py  $< $@ 

#根据 *.h 生成 *.codedef.private.h
$(private_msgcodedef_list): %.codedef.private.h: %.h
	python codedef_hpp.py  $< $@


$(target_lib).a : $(objects)
	@echo $(objects)
	ar rs $(target_lib).a $(objects) 

$(target_lib).so : $(objects)
	$(cc) -shared $(objects) -o $(target_lib).so

$(objects): %.o: %.cpp
	$(cc) $(cc_flags) -c $< -o $@

%.d: %.cpp
#此行用于调试	@set -e -x; \ #此行用于调试
	@rm -f $@; \
	$(cc) $(cc_flags) -MM $< > $@.$$$$;  \
	sed -r 's,^(.*)\.o:,$@ $*\.o:,' $@.$$$$ > $@;  \
	rm -f $@.$$$$

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)
-include $(objects:.o=.d)
endif
endif

clean:
	-rm -rf *.temp
	-rm -f *.o *.d *.d.*
	-rm -f $(target_lib).a
	-rm -f $(PUBLIC_PROTOCOL_DEFINITION_DIR)/codedef.* 
	-rm -f $(PUBLIC_PROTOCOL_DEFINITION_DIR)/*.codedef.*.h 
	-rm -f $(PUBLIC_PROTOCOL_DEFINITION_DIR)/rawmsg.codedef.*.xml
	-rm -f $(PRIVATE_PROTOCOL_DEFINITION_DIR)/codedef.* 
	-rm -f $(PRIVATE_PROTOCOL_DEFINITION_DIR)/*.codedef.*.h 
	-rm -f $(PRIVATE_PROTOCOL_DEFINITION_DIR)/rawmsg.codedef.*.xml

distclean: clean

