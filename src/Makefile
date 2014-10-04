-include ../Makefile.local # Local variable overrides
-include ../Makefile.config # base paths

LOCALLIB ?= $(PATHS_LOCAL_ROOT)/lib
LOCALINC ?= $(PATHS_LOCAL_ROOT)/include

CXXSTD ?= -std=c++0x

CXXFLAGS += -Wall -pedantic
CXXFLAGS += $(CXXSTD)
CXXFLAGS += -O3 -g

FTFLAGS = $(shell freetype-config --cflags) 
FTLIBS = $(shell freetype-config --libs)

PNGLIBS = -lpng
CURLLIBS = -lcurl

ifneq ($(LOCALLIB),)
PNGLIBS += -Wl,-rpath $(LOCALLIB)
LDFLAGS += -L$(LOCALLIB)
endif

LDFLAGS += -lsqlite3

SQLITEFLAGS = -I$(LOCALINC)

ifndef SKIPMYSQL
MYSQLFLAGS  = $(shell mysql_config --cflags)
MYSQLFLAGS += -I$(LOCALINC)
MYSQLLIBS  = $(shell mysql_config --libs)
MYSQLLIBS += -l mysqlpp -Wl,-rpath $(LOCALLIB)

EXE += bin/cloneMySQL
endif

ARFLAGS = ruc

MYLIB = obj/libKayak.a

CGIEXE += bin/printenv
CGIEXE += bin/png
CGIEXE += bin/disp

BINEXE += bin/prune
BINEXE += bin/dbBackup

EXE += bin/key2hash
EXE += bin/updateLevels
EXE += bin/updateCalcs
EXE += bin/fetch

EXE += $(BINEXE) $(CGIEXE)

LIBSRC  = CGI.C HTML.C HTTP.C Tokens.C URL.C Axis.C BitMapCanvas.C Canvas.C CommonData.C
LIBSRC += CommonSource.C CommonURLs.C CommonUnknown.C Convert.C Data.C DataGet.C Display.C
LIBSRC += DisplayDatabase.C DisplayInfo.C DisplayLevels.C DisplayMainPage.C DisplayPlot.C
LIBSRC += DisplayTable.C Env.C File.C Gauges.C GuideBook.C JSON.C Levels.C MakePlot.C Master.C
LIBSRC += MyDB.C PNGCanvas.C Paths.C PruneData.C Rating.C ReadFile.C Table.C afstream.C
LIBSRC += DisplayData.C DisplayGauges.C
LIBSRC += Calc.C TimeIt.C Curl.C XMLParser.C GaugeTranslate.C StateCounty.C
LIBSRC += ParserUSGS0.C ParserUSGS1.C ParserNOAA0.C 
LIBSRC += String.C Types.C

.PHONY: all distclean clean install install.cgi install.programs
.PRECIOUS: obj/%.o

all: $(EXE)

bin/%: %.C $(MYLIB)
	@ mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -o $@ $+ $(LDFLAGS) $(LIBS)

bin/cloneMySQL: cloneMySQL.C MySQL.C $(MYLIB)
	@ mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(MYSQLFLAGS) -o $@ $+ $(LDFLAGS) $(LIBS) $(MYSQLLIBS)

bin/png: png.C $(MYLIB)
	@ mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -o $@ $+ $(LDFLAGS) $(LIBS) $(FTLIBS) $(PNGLIBS)

bin/fetch: fetch.C $(MYLIB)
	@ mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -o $@ $+ $(LDFLAGS) $(LIBS) $(CURLLIBS)

$(MYLIB): $(LIBSRC:%.C=obj/%.o)
	$(AR) $(ARFLAGS) $@ $+

obj/BitMapCanvas.o: BitMapCanvas.C
	@ mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(FTFLAGS) -MD -MT $@ -c -o $@ $<

obj/PNGCanvas.o: PNGCanvas.C
	@ mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I/usr/local/include -MD -MT $@ -c -o $@ $<

obj/MyDB.o: MyDB.C
	@ mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(SQLITEFLAGS) -MD -MT $@ -c -o $@ $<

obj/Paths.o: Paths.C
	@ mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(SQLITEFLAGS) -D PATHS_USER_ROOT='"$(PATHS_USER_ROOT)"' -D PATHS_FILES_ROOT='"$(PATHS_FILES_ROOT)"' -D PATHS_WEB_BASE='"$(PATHS_WEB_BASE)"' -MD -MT $@ -c -o $@ $<

obj/%.o: %.C
	@ mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MD -MT $@ -c -o $@ $<

distclean clean:
	$(RM) -rf bin obj

install: $(CGIDIR)/.htaccess $(PATHS_WEB_ROOT)/.htaccess install.cgi install.programs

install.programs: $(BINEXE)
	install -d $(BINDIR)
	install $(INSTALLFLAGS) $+ $(BINDIR)

install.cgi: $(CGIEXE)
	install -d $(CGIDIR)
	install $(INSTALLFLAGS) -s $+ $(CGIDIR)

$(CGIDIR)/.htaccess: Makefile
	install -d $(CGIDIR)
	echo "# Auto generated `date` by make" >$@
	echo Options ExecCGI >>$@
	echo Options +FollowSymLinks >>$@
	echo SetHandler cgi-script >>$@
	chmod 644 $@

$(PATHS_WEB_ROOT)/.htaccess: Makefile
	echo DirectoryIndex cgi/disp >$@
	chmod 644 $@

-include obj/*.d
