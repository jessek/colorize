
/* $Id$ */

#include "main.h"
#include <stdarg.h>

#define MD5DEEP_PRINT_MSG(HANDLE,MSG) \
va_list(ap);  \
va_start(ap,MSG); \
if (vfprintf(HANDLE,MSG,ap) < 0)  \
{ \
   fprintf(stderr, "%s: %s", __progname, strerror(errno)); \
   exit(EXIT_FAILURE);  \
} \
va_end(ap); fprintf (HANDLE,"%s", NEWLINE);


void print_status(char *fmt, ...)
{
  MD5DEEP_PRINT_MSG(stdout,fmt);
}

void print_error(char *fmt, ...)
{
  MD5DEEP_PRINT_MSG(stderr,fmt);
}


void fatal_error(char *fmt, ...)
{
  MD5DEEP_PRINT_MSG(stderr,fmt);
  exit(EXIT_FAILURE);
}

void try_msg(void)
{
  print_status ("Try `%s -h` for more information.", __progname);
}
