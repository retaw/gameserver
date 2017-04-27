#替换默认shell
SHELL = /bin/bash

MYSQL_CONFIG_CMD = mysql_config

#当前目录
CUR_DIR = $(shell pwd)

#源文件名
SOURCE_PACKGE_BASE_NAME = mysql++-3.2.3

#mysql libs_r 所在的目录
MYSQL_LIB_DIR = $(shell $(MYSQL_CONFIG_CMD) --libs_r | awk '{for(i=1;i<=NF;i++)if($$i~/^-L/) {gsub(/-L/, "", $$i); print $$i;}}')

#mysql include
MYSQL_INCLUDE = $(shell $(MYSQL_CONFIG_CMD) --include | awk '{gsub(/-I/, "", $$0); print $$0}')

#安装目录
INSTALL_DIR = $(CUR_DIR)/installed


all: $(INSTALL_DIR)

$(INSTALL_DIR):
	if [[ ! -d $(MYSQL_LIB_DIR) ]]; then exit 1; fi
	if [[ ! -d $(SOURCE_PACKGE_BASE_NAME) ]]; then \
		tar -xf $(SOURCE_PACKGE_BASE_NAME).tar.gz; \
		cp -rf ./patch/* $(SOURCE_PACKGE_BASE_NAME)/; \
		chmod +x $(SOURCE_PACKGE_BASE_NAME)/configure; \
	fi;
	cd $(SOURCE_PACKGE_BASE_NAME); \
		export LIBS="$$LIBS -lpthread" && export CXXFLAGS="$$CXXFLAGS -std=c++14" &&\
	   	./configure --enable-thread-check --with-mysql-lib=$(MYSQL_LIB_DIR) --with-mysql-include=$(MYSQL_INCLUDE) --prefix=$(INSTALL_DIR) || exit 1; \
		make && make install; \
		cd $(CUR_DIR);

clean:
	rm -rf $(INSTALL_DIR)

distclean: clean
	rm -rf $(SOURCE_PACKGE_BASE_NAME)
