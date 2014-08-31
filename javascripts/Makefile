USRROOT = /home/tpw

WEBROOT = $(USRROOT)/public_html/kayaking2
SCRIPTDIR = $(WEBROOT)/scripts

.PHONEY: all distclean clean install

all: plot.min.js kayak.min.js

%.min.js: %.js
	uglifyjs --output $@ $<

install: plot.min.js plot.js
	install -d $(SCRIPTDIR)
	install -b --compare --target-directory=$(SCRIPTDIR) $+

distclean clean:
	$(RM) *.min.js
