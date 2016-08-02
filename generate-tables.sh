#!/bin/bash

echo "$*"

if [ "$(uname -s)" == "Darwin" ]; then
    sed_opt="-E"
else
    sed_opt="-r"
fi

source_file=$(cat "$1" | sed $sed_opt s/$/\\\\n\\\\/g)
mkdir -p $(dirname "$2")

cat <<EOF > "$2"
#include "generated.h"

const char *z80_tab = "\
$source_file
";
EOF
