#!/bin/bash

echo "$*"

source_file=$(cat "$1" | sed -r s/$/\\\\n\\\\/g)
mkdir -p $(dirname "$2")

cat <<EOF > "$2"
#include "generated.h"

const char *z80_tab = "\
$source_file
";
EOF
