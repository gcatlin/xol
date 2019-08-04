#pragma once

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG_TRACE_EXECUTION

#define ANSI_RESET     "\x1b[0m"
#define ANSI_BOLD      "\x1b[1m"
#define ANSI_FG_RED    "\x1b[31m"
#define ANSI_FG_GREEN  "\x1b[32m"
#define ANSI_FG_YELLOW "\x1b[33m"
#define ANSI_FG_BLUE   "\x1b[34m"
#define ANSI_FG_CYAN   "\x1b[36m"

#define ERR_USAGE   64
#define ERR_COMPILE 65
#define ERR_RUNTIME 70
#define ERR_FILE    74

#define countof(x) ((sizeof(x) / sizeof(0 [x])) / ((size_t)(!(sizeof(x) % sizeof(0 [x])))))

typedef uint8_t byte;
typedef double Value;
