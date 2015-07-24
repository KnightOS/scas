#!/bin/bash

source_file=$(cat tables/z80.tab | sed -r s/$/\\\\n\\\\/g)

cat <<EOF > tables/z80.c
#include "generated.h"

const char *z80_tab = "\
$source_file
";
EOF
