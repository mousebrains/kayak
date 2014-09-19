-include ../Makefile.local # Local overrides
-include ../Makefile.config # Base configuration

SCRIPTDIR = $(PATHS_WEB_ROOT)/scripts

.PHONEY: all distclean clean install

all: kayak.min.js

%.min.js: %.js
	uglifyjs --output $@ $<

install: kayak.min.js kayak.js
	install -d $(SCRIPTDIR)
	install $(INSTALLFLAGS) $+ $(SCRIPTDIR)

clean:
	$(RM) *.min.js
