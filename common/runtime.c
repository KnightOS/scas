#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "list.h"
#include "linker.h"
#include "enums.h"
#include "runtime.h"

// Defines this when building for a shared library
struct runtime scas_runtime;
