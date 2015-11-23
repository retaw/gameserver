INCLUDE_DIR=$(shell pwd)/../installed/include/mysql++ $(shell /usr/local/mysql/bin/mysql_config --include)
LIB_DIR=$(shell pwd)/../installed/lib
LIBS=mysqlpp

source=test1.exec test2.exec test3.exec


all: $(source)

%.exec: %.cpp
	g++ -ggdb -std=c++11 -I $(INCLUDE_DIR) -L$(LIB_DIR) -lmysqlpp $(shell /usr/local/mysql/bin/mysql_config --libs_r) -Wl,-rpath=$(LIB_DIR)  $< -o $@

clean:
	rm -f $(source)




