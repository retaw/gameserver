export ROOT_DIR = $(shell pwd)

export BASE_DIR = $(ROOT_DIR)/water

export LIBS_DIR = $(BASE_DIR)/libs

export CONF_DIR = $(ROOT_DIR)/config

export PROCESSES_DIR = $(ROOT_DIR)/processes

#export optimization_flag = -O2
export optimization_flag = 


SUBDIR =  \
		  water\

#protocol\
#processes\


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

cleanlibs:
	cd $(LIBS_DIR) && $(MAKE) clean;

distcleanlibs:
	cd $(LIBS_DIR) && $(MAKE) distclean;

ctags:
	ctags --exclude=libs -R
