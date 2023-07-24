CFLAGS=-Iinclude/ -O2 -Ibin/ -Wall -Wextra -pedantic -std=c99 -D_XOPEN_SOURCE=700 -D_DEFAULT_SOURCE -g
CC_NATIVE=$(CC)
SO_EXT=so

ASSEMBLER=assembler/privatize.c assembler/directives.c assembler/assembler.c
COMMON=bin/amd64.c bin/z80.c bin/arm64.c common/functions.c common/hashtable.c common/expression.c common/list.c common/operators.c common/runtime.c common/stringop.c common/errors.c common/stack.c common/format.c common/instructions.c common/log.c common/match.c common/md5.c common/objects.c common/readline.c
LINKER=linker/8xp.c linker/plan9.c linker/bin.c linker/linker.c linker/merge.c
SOURCES=$(ASSEMBLER) $(COMMON) $(LINKER)

all:bin/scas bin/scdump bin/scwrap

DESTDIR=/usr/local
SHAREDIR=$(DESTDIR)/share/
DATADIR=$(SHAREDIR)/knightos/scas/
MANDIR=$(SHAREDIR)/man/
BINDIR=$(DESTDIR)/bin/
LIBDIR=$(DESTDIR)/lib/
INCDIR=$(DESTDIR)/include/

install: all
	mkdir -p $(BINDIR) $(INCDIR)/scas/ $(LIBDIR)/ $(DATADIR)
	cp bin/scas bin/scwrap bin/scdump $(BINDIR)
	cp include/* $(INCDIR)/scas/
	cp tables/amd64.tab tables/z80.tab tables/arm64.tab $(DATADIR)

install_man: bin/scas.1 bin/scdump.1 bin/scwrap.1
	mkdir -p $(MANDIR)/man1/
	cp $^ $(MANDIR)/man1/

install_lib_static: bin/scas.a
	mkdir -p $(LIBDIR)/
	cp $^ $(LIBDIR)/libscas.a

install_lib_shared: bin/libscas.$(SO_EXT)
	mkdir -p $(LIBDIR)/
	cp $^ $(LIBDIR)/

uninstall:
	$(RM) $(BINDIR)/scas $(BINDIR)/scdump $(BINDIR)/scwrap -v
	$(RM) $(INCDIR)/scas/ -rv
	$(RM) $(LIBDIR)/libscas.a
	$(RM) $(LIBDIR)/libscas.so
	$(RM) $(DATADIR)/amd64.tab
	$(RM) $(DATADIR)/arm64.tab
	$(RM) $(DATADIR)/z80.tab
	$(RM) $(MANDIR)/man1/scas.1
	$(RM) $(MANDIR)/man1/scdump.1
	$(RM) $(MANDIR)/man1/scwrap.1

bin/scas.a: $(SOURCES:.c=.o)
	$(AR) $(ARFLAGS) $@ $^

bin/libscas.$(SO_EXT): bin/scas.a
	$(CC) $(LDFLAGS) -shared $^ -o $@

bin/scas: scas.o bin/scas.a
	$(CC) $(LDFLAGS) $^ -o $@

bin/scdump: scdump.o bin/scas.a
	$(CC) $(LDFLAGS) $^ -o $@

bin/scwrap: scwrap.o bin/scas.a
	$(CC) $(LDFLAGS) $^ -o $@

bin/z80.c: bin/generate_tables tables/z80.tab
	./bin/generate_tables z80 ./tables/z80.tab ./bin/z80.c ./bin/z80.h

bin/amd64.c: bin/generate_tables tables/amd64.tab
	./bin/generate_tables amd64 ./tables/amd64.tab ./bin/amd64.c ./bin/amd64.h

bin/arm64.c: bin/generate_tables tables/arm64.tab
	./bin/generate_tables arm64 ./tables/arm64.tab ./bin/arm64.c ./bin/arm64.h

bin/arm64.h: bin/arm64.c
bin/amd64.h: bin/amd64.c
bin/z80.h: bin/z80.c

bin/generate_tables: tables/generate.c
	mkdir -p bin/
	$(CC_NATIVE) $(CFLAGS) $< -o $@

$(SOURCES:.c=.o) scas.o: bin/z80.h bin/amd64.h bin/arm64.h include/linker.h include/match.h include/format.h include/merge.h include/list.h include/assembler.h include/md5.h include/objects.h include/8xp.h include/plan9.h include/stack.h include/bin.h include/directives.h include/errors.h include/instructions.h include/readline.h include/runtime.h include/stringop.h include/hashtable.h include/functions.h include/log.h include/expression.h include/privatize.h include/operators.h

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(SOURCES:.c=.o) scas.o scdump.o scwrap.o bin/ -vr

bin/scas.1: doc/scas.1.scdoc
	mkdir -p bin/
	scdoc < doc/scas.1.scdoc > bin/scas.1

bin/scdump.1: doc/scdump.1.scdoc
	mkdir -p bin/
	scdoc < doc/scdump.1.scdoc > bin/scdump.1

bin/scwrap.1: doc/scwrap.1.scdoc
	mkdir -p bin/
	scdoc < doc/scwrap.1.scdoc > bin/scwrap.1

