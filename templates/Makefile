-include ../Makefile.local # Overrides
-include ../Makefile.config # base configuration

PATHS_TEMPLATES_ROOT ?= $(PATHS_FILES_ROOT)/templates

all: index.head

clean:
	$(RM) index.head

install: index.head index.tail
	install -d $(PATHS_TEMPLATES_ROOT)
	install $(INSTALLFLAGS) $+ $(PATHS_TEMPLATES_ROOT)

index.head: index.head.template
	sed -e 's?PATHS_WEB_BASE?$(PATHS_WEB_BASE)?' $< >$@
