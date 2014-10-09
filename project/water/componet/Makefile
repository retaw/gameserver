objects = \
    \
    string_kit.o\
    exception.o\
    scope_guard.o\
    datetime.o\
    buffer.o\
    backend_logger.o\
    log_file.o\
    logger.o\

CC = g++
cc = g++

cc_flags = -std=c++11 -fPIC -g

target = libcomponet

.PHONY:
all: $(target).a 

$(target).a : $(objects)
	ar rs $(target).a $(objects)

$(target).so : $(objects)
	$(cc) -shared $(objects) -o $(target).so

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
	rm -f *.o *.d *.d.* 
	rm -f $(target).a $(target).so 
