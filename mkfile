COMMON=`{cd scas/common; walk -f | sed 's/\.c/.'^$O^'/g'} z80.$O arm64.$O amd64.$O
ASSEMBLER=`{cd scas/assembler ; walk -f | sed 's/\.c/.'^$O^'/g'}
LINKER=`{cd scas/linker ; walk -f | sed 's/\.c/.'^$O^'/g'}

TARG=$TARG scas scdump scwrap tablegen

%.$O: scas/common/%.c
	$CC $CFLAGS $prereq

%.$O: scas/linker/%.c
	$CC $CFLAGS $prereq

%.$O: scas/assembler/%.c
	$CC $CFLAGS $prereq

%.$O: scas/%.c
	$CC $CFLAGS $prereq

$O.scas: $ASSEMBLER $LINKER $COMMON scas.$O
	$LD -o $target $LDFLAGS $prereq 

$O.scwrap: $COMMON scwrap.$O
	$LD -o $target $LDFLAGS $prereq 

$O.scdump: $COMMON scdump.$O
	$LD -o $target $LDFLAGS $prereq 

/$objtype/lib/knightos/libscas.a$O: libscas.a$O
	mkdir -p /$objtype/lib/knightos
	cp $prereq $target

libscas.a$O: $COMMON
	ar ruv $target $prereq

/sys/lib/knightos/scas/%.tab: tables/%.tab
	mkdir -p `{basename -d $target}
	cp $prereq $target

instructions.$O: z80.h amd64.h arm64.h
	$CC $CFLAGS scas/common/instructions.c

z80.c z80.h: $O.tablegen scas/tables/z80.tab
	./$O.tablegen z80 scas/tables/z80.tab z80.c z80.h

amd64.c amd64.h: $O.tablegen scas/tables/amd64.tab
	./$O.tablegen amd64 scas/tables/amd64.tab amd64.c amd64.h

arm64.c arm64.h: $O.tablegen scas/tables/arm64.tab
	./$O.tablegen arm64 scas/tables/arm64.tab arm64.c arm64.h

tablegen.$O: scas/tables/generate.c
	pcc $prereq -B -c -o $target

$O.tablegen: tablegen.$O
	pcc $prereq -o $target

CLEANFILES=$CLEANFILES scas/assembler/*.[$OS] scas/common/*.[$OS] scas/linker/*.[$OS] scas/*.[$OS] z80.c z80.h amd64.c amd64.h arm64.c arm64.h

install:V: /$objtype/lib/knightos/libscas.a$O
