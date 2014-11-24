objects = \
    \
	xmlparse/xml_parse.o\
	xmlparse/xml_parse_doc.o\
	xmlparse/tinyxml.o\
	xmlparse/tinystr.o\
	xmlparse/tinyxmlerror.o\
	xmlparse/tinyxmlparser.o\
    string_kit.o\
    exception.o\
    scope_guard.o\
    datetime.o\
    raw_buffer.o\
    log/file_writer.o\
	log/stdout_writer.o\
    log/writer_file.o\
    log/logger.o\
    log.o\
    spinlock.o\
    timer.o\

CC = g++
cc = g++

cc_flags = -std=c++11 -fPIC -ggdb -Wall -Werror

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
	rm -f $(objects) $(objects:.o=.d) *.d *.d.* 
	rm -f $(target).a
