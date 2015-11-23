objects = \
		  super.o\
		  main.o\
		  register_tcp_msg_handler.o\
		  register_timer_handler.o\
		  client_connection_checker.o\
		  register_role_msg_relay.o\
		  account_table_structure.o\
		  zone_manager.o\
		  account_manager.o\
		  load_cfg.o\
		  load_db.o\
		  test.o\



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

target = super_exec

.PHONY: all
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
