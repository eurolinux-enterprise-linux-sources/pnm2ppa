#
# Makefile for pnm2ppa
#

INSTALLDIR=/usr/local/bin
CONFDIR=/etc
MANDIR=/usr/local/man/man1

CC=gcc
INSTALL=install

# production build
CFLAGS=-Wall -pedantic -O2 -g -DNDEBUG 
LDFLAGS=-lm
OBJS = pnm2ppa.o syslog.o ppa.o image.o dither.o cutswath.o hash_ink.o gamma.o
SRC =  pnm2ppa.c syslog.c ppa.c image.c dither.c cutswath.c hash_ink.c gamma.c

# production build (beos)
#CFLAGS=-Wall -pedantic -O2 -g -DNDEBUG -D__BEOS__
#OBJS = pnm2ppa.o syslog.o ppa.o image.o dither.o cutswath.o hash_ink.o gamma.o
#SRC =  pnm2ppa.c syslog.c ppa.c image.c dither.c cutswath.c hash_ink.c gamma.c 

# debug build
#CFLAGS=-Wall -pedantic -g -O2 -DDEBUG=1 
#LDFLAGS=-lm
#-lefence
#DOBJS = pnm2ppa.o syslog.o ppa.o image.o cutswath.o debug.o dither.o gamma.o hash_ink.o 
#OBJS = $(DOBJS)
#SRC =  pnm2ppa.c syslog.c ppa.c image.c cutswath.c debug.c dither.c gamma.c hash_ink.c

# debug build (beos)
#CFLAGS=-Wall -pedantic -g -DDEBUG=1 -D__BEOS__
#OBJS = pnm2ppa.o ppa.o image.o cutswath.o debug.o dither.o hash_ink.o gamma.o syslog.o
#SRC =  pnm2ppa.c ppa.c image.c cutswath.c debug.c dither.c hash_ink.c gamma.c syslog.c

# If you have FreeBSD, you need to uncomment the next LDFLAG.
# You may also need to install libgnugetopt from the ports collection.
# LDFLAGS+=-L/usr/local/lib -lgnugetopt

# Install under Linux and other versions with the -b (backup) option
INSTALL=install -b
INSTALLD=install -b -d

# INSTALL for FreeBSD (doesn't have the -b (backup) option)
# XXX FreeBSD users, please tell me what the flag for backup is!
#
# If you have FreeBSD, uncomment the next line
# INSTALL=install
# INSTALLD=install -d

# Language selection. Choose one only.

LANG=EN
#LANG=CZ
#LANG=ES
#LANG=FR
#LANG=IT
#LANG=NL

# Normal Users!
#
# Do not edit below this line! No user servicable parts below!

CFLAGS+=-DLANG_$(LANG)

default: all

profile:
	make "CFLAGS=-Wall -pedantic -O2 -g -pg -DNDEBUG"

debug:
	make "CFLAGS=-Wall -pedantic -g -O2 -DDEBUG=1 -DLANG_EN" "LDFLAGS=-lm -lefence" "OBJS=$(DOBJS)"

# the following have been left in for those with good memories

720: all
	@@echo After running make install, edit /etc/pnm2ppa.conf and set your printer

820: all
	@@echo After running make install, edit /etc/pnm2ppa.conf and set your printer

1000: all
	@@echo After running make install, edit /etc/pnm2ppa.conf and set your printer

all: pnm2ppa calibrate_ppa 

clean:
	rm -f *.o *~ calibrate_ppa pnm2ppa make_hash_ink gmon.out hash_ink.c


install: pnm2ppa calibrate_ppa 
	$(INSTALL) -m 755 pnm2ppa $(INSTALLDIR)
	$(INSTALLD) $(MANDIR)
	$(INSTALL) -m 644 docs/en/pnm2ppa.1 $(MANDIR)
# development only - allows root to edit
	$(INSTALL) -m 644 pnm2ppa.conf $(CONFDIR)
# calibration tool
	$(INSTALL) -m 755 calibrate_ppa $(INSTALLDIR) 
# final production - much tighter
#	$(INSTALL) -m 444 pnm2ppa.conf $(CONFDIR)
	@@echo 
	@@echo Now, edit /etc/pnm2ppa.conf to choose your printer

uninstall: 
	rm -f $(INSTALLDIR)/pnm2ppa
	rm -f $(INSTALLDIR)/calibrate_ppa
	rm -f $(MANDIR)/pnm2ppa.1
	rm -f $(CONFDIR)/pnm2ppa.conf

pnm2ppa: $(OBJS) 
	$(CC) $(LDFLAGS) $(CFLAGS) -o pnm2ppa $(OBJS)

calibrate_ppa: calibrate_ppa.o pnmtools.o gamma.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o calibrate_ppa calibrate_ppa.o pnmtools.o gamma.o 

calibrate_ppa.o: calibrate_ppa.c global.h image.h defaults.h pnmtools.h
	$(CC) $(CFLAGS) -c calibrate_ppa.c

pnmtools.o: pnmtools.c pnmtools.h
	$(CC) $(CFLAGS) -c pnmtools.c 

cutswath.o: cutswath.c debug.h ppa.h image.h global.h syslog.h
	$(CC) $(CFLAGS) -c cutswath.c

image.o: image.c image.h palette.h debug.h fscoeff.h global.h gamma.h syslog.h
	$(CC) $(CFLAGS) -c image.c

gamma.o: gamma.c image.h global.h gamma.h
	$(CC) $(CFLAGS) -c gamma.c

dither.o: dither.c dither.h image.h palette.h debug.h fscoeff.h global.h hash_ink.h
	$(CC) $(CFLAGS) -c dither.c

pnm2ppa.o: pnm2ppa.c pnm2ppa.h image.h ppa.h debug.h global.h gamma.h syslog.h
	$(CC) $(CFLAGS) -c pnm2ppa.c

ppa.o: ppa.c ppa.h debug.h global.h syslog.h
	$(CC) $(CFLAGS) -c ppa.c

debug.o: debug.c debug.h global.h
	$(CC) $(CFLAGS) -c debug.c

syslog.o: syslog.c syslog.h global.h
	$(CC) $(CFLAGS) -c syslog.c

make_hash_ink: 	make_hash_ink.c palette.h hash_ink.h
	$(CC) $(CFLAGS) -o make_hash_ink make_hash_ink.c

hash_ink.c: make_hash_ink
	./make_hash_ink 4 3 > hash_ink.c
	./make_hash_ink 1 5 >> hash_ink.c

hash_ink.o: hash_ink.c hash_ink.c
	$(CC) $(CFLAGS) -c hash_ink.c

# a default rule, in case new .c files pop in

.c.o: $*.c ppa.h image.h enh-curve.h palette.h debug.h global.h
	$(CC) $(CFLAGS) -c $*.c

distrib: clean
	cd .. && tar cf - pnm2ppa/ | gzip -9 > pnm2ppa.tar.gz
	sync














