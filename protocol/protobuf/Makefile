PROTO_ROOT_DIR = $(shell pwd)

#google protobuf安装路径
PROTOBUF_INSTALLED_DIR = $(LIBS_DIR)/google_protobuf/installed

#对外协议定义文件存放目录
PUBLIC_PROTOCOL_DEFINITION_DIR = $(PROTO_ROOT_DIR)/public

#内部协议定义文件存放目录
PRIVATE_PROTOCOL_DEFINITION_DIR = $(PROTO_ROOT_DIR)/private

#安装好的protobuf编译器路径
PROTOBUF_COMPILER = $(PROTOBUF_INSTALLED_DIR)/bin/protoc

objects = \
	proto_manager.o \
	public/codedef.o \
	private/codedef.o \

public_protodef_objects = \
	public/client.pb.o\
	public/account.pb.o\

private_protodef_objects =\
	private/login.pb.o\
	private/super.pb.o\

protodef_objects = $(private_protodef_objects) $(public_protodef_objects)


public_protobuf_codedef_list = \
	public/client.codedef.h\
	public/account.codedef.h\


private_protobuf_codedef_list = \
	private/login.codedef.h\
	private/super.codedef.h\



CC = g++
cc = g++

cc_flags = -std=c++11 -ggdb -Wall -Werror\
		   $(optimization_flag)\
		   -I$(PROTOBUF_INSTALLED_DIR)/include -I$(ROOT_DIR)

target_lib = libprotobuf_message

.PHONY: all
all: $(target_lib).a $(CONF_DIR)/protobuf.codedef.public.xml $(CONF_DIR)/protobuf.codedef.private.xml

#.a和.so依赖于objs
$(target_lib).a: $(protodef_objects) $(objects) 
	@echo $(objects) $(protodef_objects)
	ar rs $(target_lib).a $(objects) $(protodef_objects)

$(target_lib).so : $(objects)
	$(cc) -shared $(objects) -o $(target_lib).so

#objs 依赖于cpp 和 .pb.cc
$(objects): %.o: %.cpp
	$(cc) $(cc_flags) -c $< -o $@

$(protodef_objects): %.pb.o: %.pb.cc
	$(cc) $(cc_flags) -c $< -o $@

#proto文件翻译成c++源文件
public/%.pb.cc: public/%.proto
	$(PROTOBUF_COMPILER) --proto_path=$(PUBLIC_PROTOCOL_DEFINITION_DIR) $< --cpp_out=$(PUBLIC_PROTOCOL_DEFINITION_DIR)
#	$(PROTOBUF_COMPILER)  $< --proto_path=. --cpp_out=.

private/%.pb.cc: private/%.proto
	$(PROTOBUF_COMPILER) --proto_path=$(PRIVATE_PROTOCOL_DEFINITION_DIR) $< --cpp_out=$(PRIVATE_PROTOCOL_DEFINITION_DIR)
#	$(PROTOBUF_COMPILER)  $< --proto_path=. --cpp_out=.

#从proto生成 消息号定义的 .h文件
$(public_protobuf_codedef_list): %.codedef.h: %.proto
	python codedef_hpp.py $< $@

$(private_protobuf_codedef_list): %.codedef.h: %.proto
	python codedef_hpp.py $< $@

#protobuf_codedef.py 参数说明
#argv[1]: $(CONF_DIR) config路径
#argv[2]: 生成消息号文件 *.cpp 或 *.xml (cpp 或 xml) 
#argc[3]: 生成外部消息号 或 内部消息号 (public 或 private)

#外部的*.codedef.h文件生成消息号文件codedef.cpp
public/codedef.cpp : $(public_protobuf_codedef_list)
	@curdir=$$(pwd);\
	cd $(PROTO_ROOT_DIR); \
	python protobuf_codedef.py $(PUBLIC_PROTOCOL_DEFINITION_DIR) cpp public || exit 1; \


#内部的*.codedef.private.h文件生成消息号文件codedef.private.cpp
private/codedef.cpp : $(private_protobuf_codedef_list)
	@curdir=$$(pwd);\
	cd $(PROTO_ROOT_DIR);\
	python protobuf_codedef.py $(PRIVATE_PROTOCOL_DEFINITION_DIR) cpp private || exit 1; \


#外部的*.codedef.private.h文件生成序列号xml protobuf.codedef.public.xml
$(CONF_DIR)/protobuf.codedef.public.xml : $(public_protobuf_codedef_list)
	@curdir=$$(pwd);\
	cd $(PROTO_ROOT_DIR);\
	python protobuf_codedef.py $(PUBLIC_PROTOCOL_DEFINITION_DIR) xml public || exit 1; \
	cp $(PUBLIC_PROTOCOL_DEFINITION_DIR)/protobuf.codedef.xml $(CONF_DIR)/protobuf.codedef.public.xml 
 

#内部的*.codedef.private.h文件生成序列号xml protobuf.codedef.private.xml
$(CONF_DIR)/protobuf.codedef.private.xml : $(private_protobuf_codedef_list)
	@curdir=$$(pwd);\
	cd $(PROTO_ROOT_DIR);\
	python protobuf_codedef.py $(PRIVATE_PROTOCOL_DEFINITION_DIR) xml private || exit 1; \
	cp $(PRIVATE_PROTOCOL_DEFINITION_DIR)/protobuf.codedef.xml $(CONF_DIR)/protobuf.codedef.private.xml 

%.d: %.cpp
#此行用于调试	@set -e -x; \ #此行用于调试
	@rm -f $@; \
	$(cc) $(cc_flags) -MM $< > $@.$$$$;  \
	sed -r 's,^(.*)\.o:,$@ $*\.o:,' $@.$$$$ > $@;  \
	rm -f $@.$$$$

%.pb.d: %.pb.cc
#此行用于调试	@set -e -x; \ #此行用于调试
	@rm -f $@; \
	$(cc) $(cc_flags) -MM $< > $@.$$$$;  \
	sed -r 's,^(.*)\.o:,$@ $*\.o:,' $@.$$$$ > $@;  \
	rm -f $@.$$$$

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)
-include $(objects:.o=.d)
-include $(protodef_objects:.pb.o=.pb.d)
endif
endif

.PHONY: clean
clean:
	rm -rf *.temp
	rm -f *.o *.d *.d.*
	rm -f $(target_lib).a
	rm -f $(CONF_DIR)/protobuf.codedef.private.xml
	rm -f $(CONF_DIR)/protobuf.codedef.public.xml
	rm -f $(PUBLIC_PROTOCOL_DEFINITION_DIR)/*.d
	rm -f $(PUBLIC_PROTOCOL_DEFINITION_DIR)/*.dh
	rm -f $(PUBLIC_PROTOCOL_DEFINITION_DIR)/*.cpp
	rm -f $(PUBLIC_PROTOCOL_DEFINITION_DIR)/*.cc
	rm -f $(PUBLIC_PROTOCOL_DEFINITION_DIR)/*.o
	rm -f $(PUBLIC_PROTOCOL_DEFINITION_DIR)/*.xml
	rm -f $(PUBLIC_PROTOCOL_DEFINITION_DIR)/*.d.*
	rm -f $(PRIVATE_PROTOCOL_DEFINITION_DIR)/*.d
	rm -f $(PRIVATE_PROTOCOL_DEFINITION_DIR)/*.dh
	rm -f $(PRIVATE_PROTOCOL_DEFINITION_DIR)/*.cpp
	rm -f $(PRIVATE_PROTOCOL_DEFINITION_DIR)/*.cc
	rm -f $(PRIVATE_PROTOCOL_DEFINITION_DIR)/*.o
	rm -f $(PRIVATE_PROTOCOL_DEFINITION_DIR)/*.xml
	rm -f $(PRIVATE_PROTOCOL_DEFINITION_DIR)/*.d.*

.PHONY: distclean
distclean: clean

