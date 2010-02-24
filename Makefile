name = jefliks
version = 0.0.2b
comment = Simple Elementary-based XMPP/Jabber client for handheld devices.
description = This project is a try to write real tiny and fast XMPP/Jabber client for handheld devices, supported by Enlightment Foundation Library. It use Elementary widget toolkit and Iksemel library to make it possible.
section = openmoko/applications
priority = optional
author = Phoenix Kayo <kayo.k11.4@gmail.com>
copyright = 2010, $(author)
maintainer = $(author)
homepage = http://sourceforge.net/projects/jefliks/
source = http://sourceforge.net/projects/jefliks/
architecture = armv4t

sources = main.c jabber.c ui_config.c ui_about.c ui_roster.c ui_chat.c ui_main.c base64.c
dynlibs = evas ecore edje ecore-evas eina-0 elementary
stclibs = iksemel
libs = $(dynlibs) $(stclibs)
pos = ru.po

depend-libs = $(shell $(patsubst %-strip,%-strings,$(STRIP)) $(name) | grep -v '^/' | grep '\.so')
depends = evas ecore edje ecore_evas eina libelementary-ver-pre-svn-05-0 libc6 libeet1 libgnutls26

prefix=$(DESTDIR)
objects = $(sources:.c=.o)
mos = $(pos:.po=.mo)

#debug=1
#devel=1

CFLAGS += -Wall $(shell pkg-config --cflags $(libs)) -DHAVE_GETTEXT -DBUTTONS_RESCALE=0.9
LDFLAGS += -Wl,-Bstatic $(shell pkg-config --libs $(stclibs)) -Wl,-Bdynamic $(shell pkg-config --libs $(dynlibs))

ifdef debug
CFLAGS += -g -DDEBUG_MODE=1
endif

ifdef devel
CFLAGS += -DDEVEL_MODE=1
endif

ifndef devel
CFLAGS += -DTHEME_PATH=\"/usr/share/$(name)/default.edj\"
endif

#CFLAGS+=-DTEST_WIDGET_MODE=1 -DTEST_JABBER_CONFIG
CFLAGS+=-DNAME=\"$(name)\" -DVERSION=\"$(version)\" #-DAUTHOR=\"$(author)\"

all: $(name) $(name).edj $(mos) $(name).desktop

$(name).pot: $(sources)
	xgettext --language=C --keyword=_ --default-domain="$(name)" --package-name="$(name)" --package-version="$(version)" --copyright-holder="$(author)" -o $@ $^

gettext: $(name).pot

%.mo: %.po
	msgfmt -o $@ $<

%.po: $(name).pot
	[ -f $@ ] || msginit --locale=$* -o $@ -i $<; [ -f $@ ] && msgmerge --update $@ $<

$(name).edj: theme.edc
	edje_cc $< $@

$(name): $(objects)
	$(CC) -o $@ $^ $(LDFLAGS)

install: all
	@echo 'Installing..'
	@install -m 755 -d $(prefix)/usr/bin
	@install -m 755 -t $(prefix)/usr/bin $(name)
	@install -m 755 -d $(prefix)/usr/share/applications
	@install -m 644 -t $(prefix)/usr/share/applications $(name).desktop
	@install -m 755 -d $(prefix)/usr/share/pixmaps
	@install -m 644 -t $(prefix)/usr/share/pixmaps $(name).png
	@install -m 755 -d $(prefix)/usr/share/$(name)
	@install -m 644 $(name).edj $(prefix)/usr/share/$(name)/default.edj
	@$(foreach mo,$(mos),mkdir -p $(prefix)/usr/share/locale/$(mo:.mo=)/LC_MESSAGES; install -m 644 $(mo) $(prefix)/usr/share/locale/$(mo:.mo=)/LC_MESSAGES/$(name).mo; )

$(name).control:
	@echo 'Generating $@..'
	@echo "Package: $(name)" > $@
	@echo "Version: $(version)" >> $@
	@echo "Description: $(description)" >> $@
	@echo "Section: $(section)" >> $@
	@echo "Priority: $(priority)" >> $@
	@echo "Maintainer: $(maintainer)" >> $@
	@echo "Architecture: $(architecture)" >> $@
	@echo "Homepage: $(homepage)" >> $@
	@echo "Depends: $(depends)" >> $@
	@echo "Source: $(source)" >> $@

$(name).desktop:
	@echo 'Genarating $@..'
	@echo '[Desktop Entry]' > $@
	@echo 'Encoding=UTF-8' >> $@
	@echo 'Name=$(name)' >> $@
	@echo 'Comment=$(comment)' >> $@
	@echo 'Exec=/usr/bin/$(name)' >> $@
	@echo 'Icon=/usr/share/pixmaps/$(name).png' >> $@
	@echo 'Terminal=false' >> $@
	@echo 'Type=Application' >> $@
	@echo 'Categories=PIM;Jabber;' >> $@
	@echo 'SingleInstance=true' >> $@
	@echo 'StartupNotify=true' >> $@

.PHONY: install control

ipk: $(name).control
	rm -rf /tmp/.-ipkg-tmp
	om-make-ipkg . $<

clean:
	rm -f $(name) *.o *.bin *.elf *.edj *~ *.ipk *.desktop *.control *.mo *.pot

s2n: all
	scp $(name) $(name).edj root@kayo-neo:/home/root
