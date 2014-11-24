path = ../water/net

objects = \
    \
    connector.o\
    listener.o\
    connection.o\
    packet.o\
    socket.o\
    endpoint.o\
    epoller.o\


CC = g++
cc = g++

cc_flags = -std=c++11 -g


target = libnet

.PHONY:
all: $(target).a


$(target).a : $(objects)
	ar rs $(target).a $(objects)

$(objects): %.o: $(path)/%.cpp
	$(cc) $(cc_flags) -c $< -o $@


%.d: %.cpp
	@#set -x ; 
	@set -e ; \
	rm -f $@; \
	$(cc) -MM $(cc_flags) $< > $@.$$$$;  \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@;  \
	rm -f $@.$$$$

-include $(objects:.o=.d)
#sed 's,/($*/)/.o[ :]*,/1.o $@ : ,g' < $@.$$$$ > $@; \

.PHONY: clean
clean:
	rm -f *.o *.d *.d.*
	rm -f $(target).a
