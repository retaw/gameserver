#当前目录
CUR_DIR = $(shell pwd)

#源文件名
SOURCE_PACKGE_BASE_NAME = mysql++-3.2.1

#安装目录
INSTALL_DIR = $(CUR_DIR)/installed


all: $(INSTALL_DIR)

$(INSTALL_DIR):
	if [[ ! -d $(SOURCE_PACKGE_BASE_NAME) ]]; then tar -xf $(SOURCE_PACKGE_BASE_NAME).tar.gz; fi;
	cd $(SOURCE_PACKGE_BASE_NAME); \
		./configure --enable-thread-check --prefix=$(INSTALL_DIR) || exit 1; \
		make && make install; \
		cd $(CUR_DIR);

clean:
	rm -rf $(INSTALL_DIR)

distclean:
	rm -rf $(SOURCE_PACKGE_BASE_NAME)
