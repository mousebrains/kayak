-include ../Makefile.local # local overrides
-include ../Makefile.config # base configuration

DB = $(PATHS_FILES_ROOT)/files/a.db
SQLITE ?= sqlite3 -batch -init

.phoney: all clean install master data levels translate stateCounty extra

all: # Do nothing

install: clean 
	$(MAKE) master gauges guides rating data levels translate stateCounty

clean:
	$(RM) $(DB)

$(dir $(DB)):
	mkdir $@

%: %.txt $(dir $(DB)) ; $(SQLITE) $< $(DB) </dev/null
