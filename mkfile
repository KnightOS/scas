</$objtype/mkfile

COMMON=`{walk -f common | sed 's/\.c/.'^$O^'/g'}
ASSEMBLER=`{walk -f assembler | sed 's/\.c/.'^$O^'/g'}
LINKER=`{walk -f linker | sed 's/\.c/.'^$O^'/g'}

$O.scas.out: scas.$O z80.$O amd64.$O $COMMON $ASSEMBLER $LINKER
	pcc $prereq -o $target

$O.scdump.out: scdump.$O $COMMON
	pcc $prereq -o $target

$O.scwrap.out: scwrap.$O $COMMON
	pcc $prereq -o $target

install:V: /$objtype/bin/knightos/scas /$objtype/bin/knightos/scdump /$objtype/bin/knightos/scwrap /sys/lib/knightos/scas/tables/z80.tab

/$objtype/bin/knightos/scdump: $O.scdump.out
	mkdir -p /$objtype/bin/knightos
	cp $O.scdump.out /$objtype/bin/knightos/scdump

/$objtype/bin/knightos/scwrap: $O.scwrap.out
	mkdir -p /$objtype/bin/knightos
	cp $O.scwrap.out /$objtype/bin/knightos/scwrap

/$objtype/bin/knightos/scas: $O.scas.out
	mkdir -p /$objtype/bin/knightos
	cp $O.scas.out /$objtype/bin/knightos/scas

/sys/lib/knightos/scas/tables/z80.tab: tables/z80.tab
	mkdir -p `{basename -d $target}
	cp $prereq $target

/sys/lib/knightos/scas/tables/amd64.tab: tables/amd64.tab
	mkdir -p `{basename -d $target}
	cp $prereq $target

scas.c:V: z80.h amd64.h
z80.c:V: z80.h
amd64.c:V: amd64.h

%.$O: %.c
	pcc $prereq -I include -B -c -o $target

z80.h: generate_tables
	./generate_tables z80 tables/z80.tab z80.c z80.h

amd64.h: generate_tables
	./generate_tables amd64 tables/amd64.tab amd64.c amd64.h

generate.$O: tables/generate.c
	pcc $prereq -B -c -o $target

generate_tables: generate.$O
	pcc $prereq -o $target

clean:V:
	rm -f generate_tables *.$O scas z80.c z80.h amd64.c amd64.h assembler/*.$O common/*.$O linker/*.$O *.out
