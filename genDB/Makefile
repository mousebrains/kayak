-include ../Makefile.local # local overrides
-include ../Makefile.config # base configuration

DB = $(PATHS_FILES_ROOT)/files/a.db
SQLITE ?= sqlite3 -batch -init

.phoney: all clean install master data levels translate stateCounty extra

all: # Do nothing

install: clean 
	$(MAKE) master data levels translate stateCounty

clean:
	$(RM) $(DB)

$(dir $(DB)):
	mkdir $@

master: master.txt $(dir $(DB)) ; $(SQLITE) $< $(DB) </dev/null
data: data.txt $(dir $(DB)) ; $(SQLITE) $< $(DB) </dev/null
levels: levels.txt $(dir $(DB)) ; $(SQLITE) $< $(DB) </dev/null
translate: translate.txt $(dir $(DB)) ; $(SQLITE) $< $(DB) </dev/null
stateCounty: stateCounty.txt $(dir $(DB)) ; $(SQLITE) $< $(DB) </dev/null
extra: extra.txt $(dir $(DB)) ; $(SQLITE) $< $(DB) </dev/null
