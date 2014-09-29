objects = \
    \
    tools.o\
    exception.o\
    scope_guard.o\
    datetime.o\
    buffer.o\

CC = g++
cc = g++

cc_flags = -std=c++11 -fPIC -g

target = libbase

.PHONY:
all: $(target).a $(target).so


$(target).a : $(objects)
	ar rs $(target).a $(objects)

$(target).so : $(objects)
	$(cc) -shared $(objects) -o $(target).so

%.o: %.cpp
	$(cc) $(cc_flags) -c $< -o $@

%.d: %.cpp
	@set -e -x; \
	rm -f $@; \
	$(cc) $(cc_flags) -MM $< > $@.$$$$;  \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@;  \
	rm -f $@.$$$$

-include $(objects:.o=.d)

.PHONY: clean
clean:
	-rm -f *.o *.d *.d.* 
	-rm $(target).so $(target).a
