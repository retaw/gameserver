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
	public/test.codedef.public.h\
	public/login.codedef.public.h\
	public/role_scene.codedef.public.h\
	public/role_action.codedef.public.h\
	public/role_pk.codedef.public.h\
	public/channel_info.codedef.public.h\
	public/npc_scene.codedef.public.h\
	public/package.codedef.public.h\
	public/object_scene.codedef.public.h\
	public/friend.codedef.public.h\
	public/buff_scene.codedef.public.h\
	public/pk_state.codedef.public.h\
	public/hero_scene.codedef.public.h\
	public/store.codedef.public.h\
	public/team.codedef.public.h\
	public/watch.codedef.public.h\
	public/mail.codedef.public.h\
	public/horse.codedef.public.h\
	public/guanzhi.codedef.public.h\
	public/title.codedef.public.h\
	public/faction.codedef.public.h\
	public/wash.codedef.public.h\
	public/merge.codedef.public.h\
	public/equip_equality.codedef.public.h\
	public/wing.codedef.public.h\
	public/pet_scene.codedef.public.h\
	public/fire_scene.codedef.public.h\
	public/dragon_ball.codedef.public.h\
	public/use_object.codedef.public.h\
	public/zhuansheng.codedef.public.h\
	public/fenjie.codedef.public.h\
	public/task.codedef.public.h\
	public/dragon_heart.codedef.public.h\
	public/expbead.codedef.public.h\
	public/exp_area.codedef.public.h\
	public/trigger_scene.codedef.public.h\
	public/world_boss.codedef.public.h\
	public/bubble_point.codedef.public.h\
	public/action.codedef.public.h\
	public/faction_active.codedef.public.h\
	public/function.codedef.public.h\
	public/first.codedef.public.h\
	public/bonfire.codedef.public.h\
	public/field_boss.codedef.public.h\
	public/stall.codedef.public.h\
	public/trade.codedef.public.h\
	public/private_boss.codedef.public.h\
	public/boss_home.codedef.public.h\
	public/daily_task.codedef.public.h\
	public/shabake.codedef.public.h\

private_msgcodedef_list = \
	private/login.codedef.private.h\
	private/relay.codedef.private.h\
	private/role_scene.codedef.private.h\
	private/package.codedef.private.h\
	private/skill.codedef.private.h\
	private/friend.codedef.private.h\
	private/channel_info.codedef.private.h\
	private/buff.codedef.private.h\
	private/counter.codedef.private.h\
	private/store.codedef.private.h\
	private/hero.codedef.private.h\
	private/team.codedef.private.h\
	private/watch.codedef.private.h\
	private/mail.codedef.private.h\
	private/faction.codedef.private.h\
	private/guanzhi.codedef.private.h\
	private/horse.codedef.private.h\
	private/title.codedef.private.h\
	private/wash.codedef.private.h\
	private/dragon_ball.codedef.private.h\
	private/task.codedef.private.h\
	private/exp_area.codedef.private.h\
	private/sundry.codedef.private.h\
	private/world_boss.codedef.private.h\
	private/first.codedef.private.h\
	private/field_boss.codedef.private.h\
	private/stall_log.codedef.private.h\
	private/shabake.codedef.private.h\
	private/boss_home.codedef.private.h\

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

-include $(objects:.o=.d)

clean:
	rm -rf *.temp
	rm -f *.o *.d *.d.*
	rm -f $(target_lib).a
	rm $(PUBLIC_PROTOCOL_DEFINITION_DIR)/codedef.* 
	rm $(PUBLIC_PROTOCOL_DEFINITION_DIR)/*.codedef.*.h 
	rm $(PUBLIC_PROTOCOL_DEFINITION_DIR)/rawmsg.codedef.*.xml
	rm $(PRIVATE_PROTOCOL_DEFINITION_DIR)/codedef.* 
	rm $(PRIVATE_PROTOCOL_DEFINITION_DIR)/*.codedef.*.h 
	rm $(PRIVATE_PROTOCOL_DEFINITION_DIR)/rawmsg.codedef.*.xml

distclean: clean

