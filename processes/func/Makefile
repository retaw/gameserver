objects = \
		  all_role_info_manager.o\
		  first_table_structure.o\
		  first_manager.o\
		  first_config.o\
		  func.o\
		  main.o\
		  register_tcp_msg_handler.o\
		  register_timer_handler.o\
		  role.o\
		  role_manager.o\
		  channel.o\
		  friend_manager.o\
		  friend_table_structure.o\
		  role_table_structure.o\
		  load_cfg.o\
		  store_consume_record.o\
		  team.o\
		  team_manager.o\
		  watch_manager.o\
		  faction.o\
		  faction_table_structure.o\
		  faction_manager.o\
		  global_sundry.o\
		  global_sundry_table_structure.o\
		  relay_msg.o\
		  world_boss.o\
		  field_boss_manager.o\
		  shabake.o\
		  boss_home_manager.o\


CC = g++
cc = g++

cc_flags = -std=c++11 -ggdb -Wall -Werror \
		   $(optimization_flag)\
		   $(google_protobuf_cc_flag)\
		   $(mysqlcppconn_cc_flag)\
		   $(base_cc_flag)\

ln_flags = -Wl,-dn\
		   -L$(PROTOCOL_DIR)/protobuf -lprotobuf_message\
		   -L$(PROTOCOL_DIR)/rawmsg -lrawmsg\
		   $(base_ln_flag)\
		   $(google_protobuf_ln_flag)\
		   -Wl,-dy\
		   $(mysqlcppconn_ln_flag)\
		   -lpthread\

target_ln_dependents = $(dependent_water_files)\
					   $(PROTOCOL_DIR)/protobuf/libprotobuf_message.a\
					   $(PROTOCOL_DIR)/rawmsg/librawmsg.a


target = func_exec

.PHONY:
all: $(target)

$(target) : $(objects) $(target_ln_dependents)
	@#此行用于调试   @set -e -x; \ #此行用于调试
	g++ $(objects) $(ln_flags) -o $(target) 

%.o: %.cpp
	$(cc) $(cc_flags) -c $< -o $@

%.d: %.cpp
	@#此行用于调试	@set -e -x; \ #此行用于调试
	@rm -f $@; \
		$(cc) $(cc_flags) -MM $< > $@.$$$$;  \
		sed -r 's,^(.*)\.o:,$@ $*\.o:,' $@.$$$$ > $@;  \
		rm -f $@.$$$$

-include $(objects:.o=.d)

.PHONY: clean distclean
clean:
	rm -f $(objects) $(objects:.o=.d) *.d *.d.* 
	rm -f $(target)

distclean: clean
