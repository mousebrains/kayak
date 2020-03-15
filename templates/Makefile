-include ../Makefile.local # Overrides
-include ../Makefile.config # base configuration

SED = sed

PATHS_TEMPLATES_ROOT ?= $(PATHS_FILES_ROOT)/templates

.dummy: all clean install installUncooked installCooked

all clean:

install: installUncooked installCooked

installUncooked: index.tail
	install -d $(PATHS_TEMPLATES_ROOT)
	install $(INSTALLFLAGS) $+ $(PATHS_TEMPLATES_ROOT)

installCooked: index.head
	install -d $(PATHS_TEMPLATES_ROOT)
	$(SED) -e "s?BASE_HREF?$(PATHS_WEB_BASE)?" <index.head >$(PATHS_TEMPLATES_ROOT)/index.head
