</$objtype/mkfile

COMMON=`{walk -f common | sed 's/\.c/.'^$O^'/g'} z80.$O arm64.$O amd64.$O
ASSEMBLER=`{walk -f assembler | sed 's/\.c/.'^$O^'/g'}
LINKER=`{walk -f linker | sed 's/\.c/.'^$O^'/g'}
CFLAGS=-I . -I include -I /sys/include/npe
BIN=/$objtype/bin/knightos

TARG=scas scdump scwrap
OFILES=$COMMON

$O.scas: $ASSEMBLER $LINKER

/$objtype/lib/knightos/libscas.a$O: libscas.a$O
	mkdir -p /$objtype/lib/knightos
	cp $prereq $target

libscas.a$O: $COMMON
	ar ruv $target $prereq

/sys/lib/knightos/scas/%.tab: tables/%.tab
	mkdir -p `{basename -d $target}
	cp $prereq $target

common/instructions.$O: z80.h amd64.h arm64.h

%.$O: %.c
	$CC $CFLAGS $prereq  -B -c -o $target

z80.c z80.h: generate_tables tables/z80.tab
	./generate_tables z80 tables/z80.tab z80.c z80.h

amd64.c amd64.h: generate_tables tables/amd64.tab
	./generate_tables amd64 tables/amd64.tab amd64.c amd64.h

arm64.c arm64.h: generate_tables tables/arm64.tab
	./generate_tables arm64 tables/arm64.tab arm64.c arm64.h

generate.$O: tables/generate.c
	pcc $prereq -B -c -o $target

generate_tables: generate.$O
	pcc $prereq -o $target

< /sys/src/cmd/mkmany 

%.$O: %.c
	$CC $CFLAGS -o $target $prereq

clean:V:
	rm -f generate_tables z80.c z80.h amd64.c amd64.h arm64.c arm64.h assembler/*.[$OS] common/*.[$OS] linker/*.[$OS] *.[$OS] *.a[$OS] y.tab.? lex.yy.c y.debug y.output [$OS].??* $TARG $CLEANFILES

install:V: /$objtype/lib/knightos/libscas.a$O

all:V: libscas.a$O
