</$objtype/mkfile

COMMON=`{walk -f common | sed 's/\.c/.'^$O^'/g'} z80.$O arm64.$O amd64.$O
ASSEMBLER=`{walk -f assembler | sed 's/\.c/.'^$O^'/g'}
LINKER=`{walk -f linker | sed 's/\.c/.'^$O^'/g'}

$O.scas.out: scas.$O $COMMON $ASSEMBLER $LINKER
	pcc $prereq -o $target

$O.scdump.out: scdump.$O $COMMON
	pcc $prereq -o $target

$O.scwrap.out: scwrap.$O $COMMON
	pcc $prereq -o $target

install:V: /$objtype/bin/knightos/scas /$objtype/bin/knightos/scdump /$objtype/bin/knightos/scwrap /sys/lib/knightos/scas/z80.tab /sys/lib/knightos/scas/arm64.tab /sys/lib/knightos/scas/amd64.tab

/$objtype/bin/knightos/scdump: $O.scdump.out
	mkdir -p /$objtype/bin/knightos
	cp $O.scdump.out /$objtype/bin/knightos/scdump

/$objtype/bin/knightos/scwrap: $O.scwrap.out
	mkdir -p /$objtype/bin/knightos
	cp $O.scwrap.out /$objtype/bin/knightos/scwrap

/$objtype/bin/knightos/scas: $O.scas.out
	mkdir -p /$objtype/bin/knightos
	cp $O.scas.out /$objtype/bin/knightos/scas

/sys/lib/knightos/scas/z80.tab: tables/z80.tab
	mkdir -p `{basename -d $target}
	cp $prereq $target

/sys/lib/knightos/scas/arm64.tab: tables/arm64.tab
	mkdir -p `{basename -d $target}
	cp $prereq $target

/sys/lib/knightos/scas/amd64.tab: tables/amd64.tab
	mkdir -p `{basename -d $target}
	cp $prereq $target

common/instructions.$O:V: z80.h amd64.h arm64.h
z80.c:V: z80.h
amd64.c:V: amd64.h
arm64.c:V: arm64.h

%.$O: %.c
	pcc $prereq -I . -I include -B -c -o $target

z80.h: generate_tables tables/z80.tab
	./generate_tables z80 tables/z80.tab z80.c z80.h

amd64.h: generate_tables tables/amd64.tab
	./generate_tables amd64 tables/amd64.tab amd64.c amd64.h

arm64.h: generate_tables tables/arm64.tab
	./generate_tables arm64 tables/arm64.tab arm64.c arm64.h

generate.$O: tables/generate.c
	pcc $prereq -B -c -o $target

generate_tables: generate.$O
	pcc $prereq -o $target

clean:V:
	rm -f generate_tables *.$O scas z80.c z80.h amd64.c amd64.h arm64.c arm64.h assembler/*.$O common/*.$O linker/*.$O *.out
