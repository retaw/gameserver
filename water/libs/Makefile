SUBDIR =\
		 mysql++\
		 google_protobuf \
		 
		 
		 
all: libs

libs:
	@for subdir in $(SUBDIR); do  \
		$(MAKE) -C $$subdir || exit 1 ; \
	done;

clean:
	@for subdir in $(SUBDIR); do  \
		(cd $$subdir && $(MAKE) clean); \
	done

distclean: clean
	@for subdir in $(SUBDIR); do  \
		(cd $$subdir && $(MAKE) distclean); \
	done

