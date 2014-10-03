-include ../Makefile.local # Overrides
-include ../Makefile.config # base configuration

PATHS_TEMPLATES_ROOT ?= $(PATHS_FILES_ROOT)/templates

all clean:

install: index.head index.tail
	install -d $(PATHS_TEMPLATES_ROOT)
	install $(INSTALLFLAGS) $+ $(PATHS_TEMPLATES_ROOT)
