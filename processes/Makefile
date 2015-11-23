export GOOGLE_PROTOBUF_INSTALLED_DIR = $(LIBS_DIR)/google_protobuf/installed
export google_protobuf_cc_flag       = -I$(GOOGLE_PROTOBUF_INSTALLED_DIR)/include
export google_protobuf_ln_flag       = -L$(GOOGLE_PROTOBUF_INSTALLED_DIR)/lib -lprotobuf

export MYSQL_BIN_DIR	 = /usr/local/mysql/bin
export MYSQLCPPCONN_INSTALLED_DIR = $(LIBS_DIR)/mysql++/installed
export mysqlcppconn_cc_flag       = $(shell $(MYSQL_BIN_DIR)/mysql_config --include)\
									-I$(MYSQLCPPCONN_INSTALLED_DIR)/include
export mysqlcppconn_ln_flag       = $(shell $(MYSQL_BIN_DIR)/mysql_config --libs_r)\
									-L$(MYSQLCPPCONN_INSTALLED_DIR)/lib -lmysqlpp -Wl,-rpath=$(MYSQLCPPCONN_INSTALLED_DIR)/lib

export PROTOCOL_DIR     = $(ROOT_DIR)/protocol

export base_cc_flag = -I$(ROOT_DIR) -I$(BASE_DIR) -I$(ROOT_DIR)/processes
export base_ln_flag = \
	-L$(ROOT_DIR)/water/process -lprocess\
	-L$(ROOT_DIR)/water/net -lnet\
	-L$(ROOT_DIR)/water/componet -lcomponet\
	-L$(ROOT_DIR)/water/dbadaptcher -ldbadaptcher

export dependent_water_files =\
	$(ROOT_DIR)/water/process/libprocess.a\
	$(ROOT_DIR)/water/net/libnet.a\
	$(ROOT_DIR)/water/componet/libcomponet.a\
	$(ROOT_DIR)/water/dbadaptcher/libdbadaptcher.a


SUBDIR =  \
		  router\
		  gateway\
		  dbcached\
		  session\
		  world\
		  func\
#		  http\


all: targets

targets:
	@for subdir in $(SUBDIR); do  \
		$(MAKE) -C $$subdir || exit 1 ; \
		done 

clean:
	@for subdir in $(SUBDIR); do  \
		(cd $$subdir && $(MAKE) clean); \
		done

distclean:
	@for subdir in $(SUBDIR); do  \
		(cd $$subdir && $(MAKE) distclean); \
		done

ctags:
	ctags -R
