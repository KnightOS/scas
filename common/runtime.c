#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "list.h"
#include "stack.h"
#include "instructions.h"
#include "expression.h"
#include "objects.h"
#include "linker.h"
#include "runtime.h"

// Defines this when building for a shared library
struct runtime scas_runtime;
