-include Makefile.local # Local overrides to paths and flags
-include Makefile.config # Paths, which might be overridden by .local

all clean install::
	make -C src $@
	make -C templates $@
	make -C css $@
	make -C javascripts $@
	make -C icons $@

install::
	rsync --archive fonts $(PATHS_FILES_ROOT)/fonts
