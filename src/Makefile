USRROOT = /home/tpw

ROOTDIR = $(USRROOT)/kayaking2
BINDIR = $(ROOTDIR)/bin

WEBROOT = $(USRROOT)/public_html/kayaking2
CGIDIR = $(WEBROOT)/cgi

MYLIB = obj/libKayak.a

STANDARD=-std=c++0x

CXXFLAGS += -Wall -pedantic
CXXFLAGS += $(STANDARD)
CXXFLAGS += -O3
# LIBS += -Lobj -lKayak
LIBS += -L/home/tpw/local/lib
LIBS += -lsqlite3
LIBS += -lpng

# Freetype additions
CXXFLAGS += $(shell freetype-config --cflags)
LIBS += $(shell freetype-config --libs)

ifndef SKIPMYSQL # MySQL Additions
CXXFLAGS += -Wno-long-long
CXXFLAGS += -I/home/tpw/local/include
CXXFLAGS += $(shell mysql_config --cflags)
LIBS += -l mysqlpp -Wl,-rpath /home/tpw/local/lib 
LIBS += -Wl,-rpath $(shell mysql_config --libs)
EXE += bin/cloneMySQL
endif

OBJ = $(patsubst %.C, obj/%.o, $(SRC))

EXE += bin/key2hash
EXE += bin/dbBackup
EXE += bin/updateLevels

BINEXE = bin/prune bin/mkMainPage 
EXE += $(BINEXE)

CGIEXE += bin/printenv 
CGIEXE += bin/png bin/disp
EXE += $(CGIEXE)

ARFLAGS = -crv

.PHONY: all distclean clean install install.cgi install.programs
.PRECIOUS: obj/%.o

all: $(EXE)

$(MYLIB): $(MYLIB)(obj/Paths.o) $(MYLIB)(obj/Convert.o) $(MYLIB)(obj/Tokens.o) $(MYLIB)(obj/Env.o)
$(MYLIB): $(MYLIB)(obj/MyDB.o) $(MYLIB)(obj/Data.o) $(MYLIB)(obj/CommonData.o) 
$(MYLIB): $(MYLIB)(obj/CommonSource.o) $(MYLIB)(obj/CommonURLs.o) $(MYLIB)(obj/CommonUnknown.o) 
$(MYLIB): $(MYLIB)(obj/DataGet.o) $(MYLIB)(obj/Table.o) $(MYLIB)(obj/Gauges.o) 
$(MYLIB): $(MYLIB)(obj/Master.o) $(MYLIB)(obj/Canvas.o) $(MYLIB)(obj/BitMapCanvas.o) 
$(MYLIB): $(MYLIB)(obj/PNGCanvas.o) $(MYLIB)(obj/Axis.o) $(MYLIB)(obj/MakePlot.o) 
$(MYLIB): $(MYLIB)(obj/CGI.o) $(MYLIB)(obj/HTML.o) $(MYLIB)(obj/HTTP.o) 
$(MYLIB): $(MYLIB)(obj/URL.o) $(MYLIB)(obj/Calc.o) $(MYLIB)(obj/PruneData.o) 
$(MYLIB): $(MYLIB)(obj/ReadFile.o) $(MYLIB)(obj/afstream.o) $(MYLIB)(obj/File.o) 
$(MYLIB): $(MYLIB)(obj/Display.o) $(MYLIB)(obj/DisplayTable.o) $(MYLIB)(obj/DisplayPlot.o)
$(MYLIB): $(MYLIB)(obj/DisplayMainPage.o) $(MYLIB)(obj/DisplayInfo.o) 
$(MYLIB): $(MYLIB)(obj/DisplayDatabase.o) $(MYLIB)(obj/DisplayLevels.o)
$(MYLIB): $(MYLIB)(obj/GuideBook.o) $(MYLIB)(obj/JSON.o) $(MYLIB)(obj/Levels.o)

$(EXE): obj/libKayak.a

bin/cloneMySQL: obj/MySQL.o 

bin/%: obj/%.o
	@ mkdir -p $(dir $@)
	$(LINK.cc) $(CXXFLAGS) -MD -MT $@ -o $@ $+ $(LIBS)

obj/%.o: %.C
	@ mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MD -MT $@ -c -o $@ $<

distclean clean:
	$(RM) -rf obj bin

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
