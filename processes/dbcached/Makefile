objects = \
		  dbcached.o\
		  main.o\
		  register_tcp_msg_handler.o\
		  register_timer_handler.o\
		  role_table_structure.o\
		  object_table_structure.o\
		  role.o\
		  role_manager.o\
		  object_manager.o\
		  randname_manager.o\
		  skill_manager.o\
		  skill_table_structure.o\
		  hero_manager.o\
		  hero_table_structure.o\
		  buff_manager.o\
		  buff_table_structure.o\
		  counter_table_structure.o\
		  counter_manager.o\
		  mail.o\
		  mail_table_structure.o\
		  horse.o\
		  horse_table_structure.o\
		  title_manager.o\
		  title_table_structure.o\
		  wash_manager.o\
		  wash_table_structure.o\
		  dragon_ball_manager.o\
		  dragonBall_table_structure.o\
		  task_table_structure.o\
		  task_manager.o\
		  exp_area_manager.o\
		  expArea_table_structure.o\
		  sundry_table_structure.o\
		  sundry_manager.o\
		  stallLog_table_structure.o\
		  stall_sell_log.o\
		  faction_table_structure.o\
		  faction_role_manager.o\

CC = g++
cc = g++

cc_flags = -std=c++11 -ggdb -Wall -Werror \
		   $(optimization_flag)\
		   $(google_protobuf_cc_flag)\
		   $(mysqlcppconn_cc_flag)\
		   $(base_cc_flag)

ln_flags = -Wl,-dn \
		   -L$(PROTOCOL_DIR)/protobuf -lprotobuf_message \
		   -L$(PROTOCOL_DIR)/rawmsg -lrawmsg \
		   $(base_ln_flag) \
		   $(google_protobuf_ln_flag) \
		   -Wl,-dy \
		   $(mysqlcppconn_ln_flag)\
		   -lpthread\

target_ln_dependents = $(dependent_water_files) \
					   $(PROTOCOL_DIR)/protobuf/libprotobuf_message.a \
					   $(PROTOCOL_DIR)/rawmsg/librawmsg.a

target = dbcached_exec

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
