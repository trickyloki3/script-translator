#ifndef utility_h
#define utility_h

#include "assert.h"
#include "ctype.h"
#include "stdio.h"
#include "stdarg.h"
#include "stdlib.h"
#include "stdint.h"
#include "string.h"
#include "unistd.h"

#define panic(format, ...) panic_("%s (%s:%zu): " format ".\n", __FILE__, __func__, __LINE__, ## __VA_ARGS__)
int panic_(const char *, ...);

#endif
