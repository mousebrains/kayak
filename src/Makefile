BINDIR = /home/tpw/kayaking2/bin
CGIDIR = /home/tpw/public_html/kayaking2/cgi
LOCALLIB = /home/tpw/local/lib
LOCALINC = /home/tpw/local/include

CXXFLAGS += -Wall -pedantic
CXXFLAGS += -std=c++0x
CXXFLAGS += -O3 -g

LDFLAGS += -L$(LOCALLIB) -lsqlite3

FTFLAGS = $(shell freetype-config --cflags) 
FTLIBS = $(shell freetype-config --libs)
PNGLIBS = -lpng -Wl,-rpath $(LOCALLIB)
SQLITEFLAGS = -I$(LOCALINC)

ifndef SKIPMYSQL
MYSQLFLAGS  = $(shell mysql_config --cflags)
MYSQLFLAGS += -I$(LOCALINC)
MYSQLLIBS  = $(shell mysql_config --libs)
MYSQLLIBS += -l mysqlpp -Wl,-rpath $(LOCALLIB)
endif

ARFLAGS = rouc

MYLIB = obj/libKayak.a

CGIEXE += bin/printenv
CGIEXE += bin/png
CGIEXE += bin/disp

BINEXE += bin/prune
BINEXE += bin/dbBackup

EXE += bin/key2hash
EXE += bin/updateLevels
EXE += bin/cloneMySQL

EXE += $(BINEXE) $(CGIEXE)

LIBSRC  = CGI.C HTML.C HTTP.C Tokens.C URL.C Axis.C BitMapCanvas.C Calc.C Canvas.C CommonData.C
LIBSRC += CommonSource.C CommonURLs.C CommonUnknown.C Convert.C Data.C DataGet.C Display.C
LIBSRC += DisplayDatabase.C DisplayInfo.C DisplayLevels.C DisplayMainPage.C DisplayPlot.C
LIBSRC += DisplayTable.C Env.C File.C Gauges.C GuideBook.C JSON.C Levels.C MakePlot.C Master.C
LIBSRC += MyDB.C PNGCanvas.C Paths.C PruneData.C ReadFile.C Table.C afstream.C

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

$(MYLIB): $(LIBSRC:%.C=obj/%.o)
	$(AR) $(ARFLAGS) $@ $+

obj/BitMapCanvas.o: BitMapCanvas.C
	@ mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(FTFLAGS) -MD -MT $@ -c -o $@ $<

obj/MyDB.o: MyDB.C
	@ mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(SQLITEFLAGS) -MD -MT $@ -c -o $@ $<

obj/%.o: %.C
	@ mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MD -MT $@ -c -o $@ $<

distclean clean:
	$(RM) -rf bin obj

install: $(CGIDIR)/.htaccess install.cgi install.programs

install.programs: $(BINEXE)
	install -d $(BINDIR)
	install -b --compare --target-directory=$(BINDIR) $+

install.cgi: $(CGIEXE)
	strip --preserve-dates $+
	install -d $(CGIDIR)
	install -b --compare --target-directory=$(CGIDIR) $+

$(CGIDIR)/.htaccess: Makefile
	echo "# Auto generated `date` by make" >$@
	echo Options ExecCGI >>$@
	echo Options +FollowSymLinks >>$@
	echo SetHandler cgi-script >>$@
	chmod 744 $@

-include obj/*.d
