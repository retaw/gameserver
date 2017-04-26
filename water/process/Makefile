objects = \
    \
	shell_arg_parser.o\
    tcp_client.o\
    tcp_server.o\
    process_config.o\
    process_id.o\
    process_thread.o\
	connection_checker.o\
	private_connection_checker.o\
    tcp_packet.o\
    tcp_connection_manager.o\
    process_timer.o\
    process.o\
	http_packet.o\
	http_connection_manager.o\
    signal_handler.o\
	flash_sanbox_handler.o\


#	tcp_msg_dispatcher.o\
#	async_msg_handler_manager.o\


CC = g++
cc = g++

cc_flags = -std=c++11 -ggdb -Wall -Werror $(optimization_flag) \
		   -I$(BASE_DIR)
ln_flags = -lpthread 
#-Lnet -lnet -Lcomponet -lcomponet

target = libprocess.a

.PHONY:
all: $(target)

$(target) : $(objects)
#	@set -e -x; \
#	echo $(ln_flags)
#	g++ $(objects) $(ln_flags) -o $(target) 
	ar rs $(target) $(objects)

%.o: %.cpp
	$(cc) $(cc_flags) -c $< -o $@

%.d: %.cpp
#此行用于调试	@set -e -x; \ #此行用于调试
	@rm -f $@; \
	$(cc) $(cc_flags) -MM $< > $@.$$$$;  \
	sed -r 's,^(.*)\.o:,$@ $*\.o:,' $@.$$$$ > $@;  \
	rm -f $@.$$$$

ifneq ($(MAKECMDGOALS), clean)
ifneq ($(MAKECMDGOALS), distclean)
-include $(objects:.o=.d)
endif
endif

.PHONY: clean distclean
clean:
	rm -f $(objects) $(objects:.o=.d) *.d *.d.* 
	rm -f $(target)

distclean: clean
