objects = \
    \
    connector.o\
    listener.o\
    connection.o\
    socket.o\
    endpoint.o\
    epoller.o\
    packet.o\
    packet_connection.o\

CC = g++
cc = g++

cc_flags = -std=c++11 -ggdb -Wall -Werror $(optimization_flag)\
-I$(BASE_DIR)


target = libnet

.PHONY:
all: $(target).a


$(target).a : $(objects)
	ar rs $(target).a $(objects)

$(objects): %.o: %.cpp
	$(cc) $(cc_flags) -c $< -o $@


%.d: %.cpp
	@#set -x ; 
	@set -e ; \
	rm -f $@; \
	$(cc) -MM $(cc_flags) $< > $@.$$$$;  \
	sed -r 's,^(.*)\.o:,$@ $*\.o:,' $@.$$$$ > $@;  \
	rm -f $@.$$$$

-include $(objects:.o=.d)
#sed 's,/($*/)/.o[ :]*,/1.o $@ : ,g' < $@.$$$$ > $@; \

.PHONY: clean distclean
clean:
	rm -f *.o *.d *.d.*
	rm -f $(target).a

distclean: clean
