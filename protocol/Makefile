export PROTOBUF_INSTALLED_DIR = $(LIBS_DIR)/google_protobuf/installed


SUBDIR =  \
		  rawmsg\
		  protobuf\

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
