export SHELL = /bin/bash

export ROOT_DIR = $(shell pwd)

export BASE_DIR = $(ROOT_DIR)/water

export LIBS_DIR = $(BASE_DIR)/libs

export CONF_DIR = $(ROOT_DIR)/config

export PROCESSES_DIR = $(ROOT_DIR)/processes

#mysql_config
export MYSQL_CONFIG_CMD = mysql_config

#export optimization_flag = -O2
export optimization_flag = 


SUBDIR =  \
		  water\
		  protocol\
		  processes\


all: targets

targets:
	for subdir in $(SUBDIR); do  \
		$(MAKE) -C $$subdir SHELL=$(SHELL) || exit 1;\
		done

clean:
	@for subdir in $(SUBDIR); do  \
		$(MAKE) clean -C $$subdir; \
		done

distclean:
	@for subdir in $(SUBDIR); do  \
		$(MAKE) distclean -C $$subdir; \
		done

cleanlibs:
	cd $(LIBS_DIR) && $(MAKE) clean;

distcleanlibs:
	cd $(LIBS_DIR) && $(MAKE) distclean;

ctags:
	ctags --exclude=libs -R
