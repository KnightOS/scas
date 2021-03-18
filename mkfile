</$objtype/mkfile

COMMON=`{walk -f common | sed 's/\.c/.'^$O^'/g'}
ASSEMBLER=`{walk -f assembler | sed 's/\.c/.'^$O^'/g'}
LINKER=`{walk -f linker | sed 's/\.c/.'^$O^'/g'}

$O.scas.out: scas.$O z80.$O $COMMON $ASSEMBLER $LINKER
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

scas.c:V: generated.h
z80.c:V: generated.h

%.$O: %.c
	pcc $prereq -I include -B -c -o $target

generated.h: generate_tables
	./generate_tables tables/z80.tab z80.c generated.h

generate.$O: tables/generate.c
	pcc $prereq -B -c -o $target

generate_tables: generate.$O
	pcc $prereq -o $target

clean:V:
	rm -f generate_tables *.$O scas z80.c generated.h assembler/*.$O common/*.$O linker/*.$O *.out
