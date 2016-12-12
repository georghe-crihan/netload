PREFIX = /usr/local

all:
	make -f netloadmod.mk all
	make -f netload.mk all

clean:
	make -f netloadmod.mk clean
	make -f netload.mk clean

install: all
	cp netload $(PREFIX)/bin/
	cp netload.ko $(PREFIX)/libexec/
	cp netload.sh $(PREFIX)/etc/rc.d/
