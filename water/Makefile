SUBDIR =  \
		  componet\
		  net\
		  process\
		  dbadaptcher\

all: targets

targets:
	$(MAKE) -C $(LIBS_DIR)
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
