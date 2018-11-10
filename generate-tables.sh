#!/bin/bash

echo "$*"

if [ "$(uname -s)" == "Darwin" ]; then
    sed_opt="-E"
else
    sed_opt="-r"
fi

source_file1=$(cat "$1" | sed $sed_opt s/$/\\\\n\\\\/g)
source_file1=${source_file1%?}
source_file2=$(cat "$2" | sed $sed_opt s/$/\\\\n\\\\/g)
mkdir -p $(dirname "$3")

cat <<EOF > "$3"
#include "generated.h"

const char *z80_tab = "\
$source_file1"
"\
$source_file2
";
EOF
