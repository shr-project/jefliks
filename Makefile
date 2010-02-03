name = jabelm
sources = main.c jabber.c
dynlibs = evas ecore edje ecore-evas eina-0 elementary  # 
stclibs = iksemel
libs = $(dynlibs) $(stclibs)

prefix=$(DESTDIR)
objects = $(sources:.c=.o)
CFLAGS += $(shell pkg-config --cflags $(libs)) -DNAME=\"$(name)\"
LDFLAGS += -Wl,-Bstatic $(shell pkg-config --libs $(stclibs)) -Wl,-Bdynamic $(shell pkg-config --libs $(dynlibs))
ifdef debug
CFLAGS += -g
endif

all: $(name).elf #$(name).edj

$(name).edj: theme.edc
	edje_cc $< $@

$(name).elf: $(objects)
	$(CC) -o $@ $^ $(LDFLAGS)

install:
	install -m 755 -d $(prefix)/usr/bin
	install -m 755 -t $(prefix)/usr/bin $(name) $(name).elf
	install -m 755 -d $(prefix)/usr/share/applications
	install -m 644 -t $(prefix)/usr/share/applications $(name).desktop
	install -m 755 -d $(prefix)/usr/share/pixmaps
	install -m 644 -t $(prefix)/usr/share/pixmaps $(name).png
	install -m 755 -d $(prefix)/usr/share/$(name)
	install -m 644 $(name).edj $(prefix)/usr/share/$(name)/theme.edj

build-package:
	om-make-ipkg . control

clean:
	rm -f *.o *.bin *.elf *.edj *~ *.ipk

s2n:
	scp $(name).elf root@kayo-neo:/home/root
