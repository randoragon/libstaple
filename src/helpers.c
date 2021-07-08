#include "helpers.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

void stderr_printf(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}
