objects = \
    \
    tcp_client.o\
    tcp_server.o\
    packet_connection_manager.o\
    process_config.o\
    process_id.o\
    process_thread.o\
    #process.o\



CC = g++
cc = g++

cc_flags = -std=c++11 -ggdb
ln_flags = -lpthread  -Lnet -lnet -Lcomponet -lcomponet

target = libprocess.a

.PHONY:
all: $(target)

$(target) : $(objects)
	@set -e -x; \
	echo $(ln_flags)
#	g++ $(objects) $(ln_flags) -o $(target) 
	ar rs $(target) $(objects)

%.o: %.cpp
	$(cc) $(cc_flags) -c $< -o $@

%.d: %.cpp
#此行用于调试	@set -e -x; \ #此行用于调试
	@rm -f $@; \
	$(cc) $(cc_flags) -MM $< > $@.$$$$;  \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@;  \
	rm -f $@.$$$$

-include $(objects:.o=.d)

.PHONY: clean
clean:
	rm -f $(objects) $(objects:.o=.d) *.d *.d.* 
	rm -f $(target)
