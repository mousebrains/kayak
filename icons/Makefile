-include ../Makefile.local # Overrides
-include ../Makefile.config # base configuration

ICONDIR = $(PATHS_WEB_ROOT)

.PHONEY: all install clean

all clean: 

install: favicon.ico
	install -d $(ICONDIR)
	install $(INSTALLFLAGS) $+ $(ICONDIR)
