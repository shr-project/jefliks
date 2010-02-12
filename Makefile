name = jefliks
version = 0.0.2a
comment = Simple Elementary-based XMPP/Jabber client for handheld devices.
description = This project is a try to write real tiny and fast XMPP/Jabber client for handheld devices, supported by Enlightment Foundation Library. It use Elementary widget toolkit and Iksemel library to make it possible.
section = openmoko/applications
priority = optional
author = Phoenix Kayo <kayo.k11.4@gmail.com>
maintainer = $(author)
homepage = http://sourceforge.net/projects/jefliks/
source = http://sourceforge.net/projects/jefliks/
architecture = armv4t

sources = main.c jabber.c ui_config.c ui_about.c ui_roster.c ui_chat.c ui_main.c base64.c
dynlibs = evas ecore edje ecore-evas eina-0 elementary
stclibs = iksemel
libs = $(dynlibs) $(stclibs)

depends = $(shell $(patsubst %-strip,%-strings,$(STRIP)) $(name) | grep -v '^/' | grep '\.so')

prefix=$(DESTDIR)
objects = $(sources:.c=.o)

#debug=1
#devel=1

CFLAGS += -Wall $(shell pkg-config --cflags $(libs))
LDFLAGS += -Wl,-Bstatic $(shell pkg-config --libs $(stclibs)) -Wl,-Bdynamic $(shell pkg-config --libs $(dynlibs))

ifdef debug
CFLAGS += -g -DDEBUG_MODE=1
endif

ifndef devel
CFLAGS += -DTHEME_PATH=\"/usr/share/$(name)/default.edj\"
endif

#CFLAGS+=-DTEST_WIDGET_MODE=1 -DTEST_JABBER_CONFIG
CFLAGS+=-DNAME=\"$(name)\" -DVERSION=\"$(version)\" #-DAUTHOR=\"$(author)\"

all: $(name) $(name).edj

$(name).edj: theme.edc
	edje_cc $< $@

$(name): $(objects)
	$(CC) -o $@ $^ $(LDFLAGS)

install: $(name) $(name).edj
	@echo 'Installing..'
	@install -m 755 -d $(prefix)/usr/bin
	@install -m 755 -t $(prefix)/usr/bin $(name)
	@install -m 755 -d $(prefix)/usr/share/applications
	@install -m 644 -t $(prefix)/usr/share/applications $(name).desktop
	@install -m 755 -d $(prefix)/usr/share/pixmaps
	@install -m 644 -t $(prefix)/usr/share/pixmaps $(name).png
	@install -m 755 -d $(prefix)/usr/share/$(name)
	@install -m 644 $(name).edj $(prefix)/usr/share/$(name)/default.edj

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

ipk: $(name).control $(name).desktop
	rm -rf /tmp/.-ipkg-tmp
	om-make-ipkg . $<

clean:
	rm -f $(name) *.o *.bin *.elf *.edj *~ *.ipk *.desktop *.control

s2n: all
	scp $(name).elf $(name).edj root@kayo-neo:/home/root
