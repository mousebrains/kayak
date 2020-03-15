-include ../Makefile.local # Overrides
-include ../Makefile.config # base configuration

PATHS_CSS_ROOT ?= $(PATHS_WEB_ROOT)/css

all clean: # Nothing to do

install: kayak.css levels.css
	install -d $(PATHS_CSS_ROOT)
	install $(INSTALLFLAGS) $+ $(PATHS_CSS_ROOT)
