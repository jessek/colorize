#ifndef __MAIN_H
#define __MAIN_H

/* $Id$ */

#define AUTHOR "Jesse Kornblum"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_INTTYPES_H
# include <inttypes.h>
#endif

#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>

#ifndef HAVE_FSEEKO
# define fseeko  _fseeki64
# define ftello  _ftelli64
#endif

#ifdef HAVE_LIBGEN_H
# include <libgen.h>
#endif
/* RBF - Do we need to define basename for Win32? */


#define FALSE  0
#define TRUE   1


#include "bmp.h"


#ifdef __GLIBC__
extern char *__progname;
#else
char *__progname;
#endif


#ifndef WIN32
#define CMD_PROMPT "$"
#define DIR_SEPARATOR   '/'
#define NEWLINE "\n"
#else
#define CMD_PROMPT "C:\\>"
#define DIR_SEPARATOR   '\\'
#define NEWLINE "\n"
#endif


/* **********************
   File Comparison Values
   ********************** */
#define DEFAULT_BLOCK_SIZE  512
/* Max block size = 512MB */
#define MAX_BLOCK_SIZE      (uint64_t)(1048576 * 512)

#define CHAR_EQUAL     128
#define CHAR_NOT_EQUAL 254

#define RGB_BLACK    0
#define RGB_PADDING  0xff



/* ****************
   Helper Functions
   **************** */
unsigned char padding(void);

int find_file_size(char *fn, FILE *h, off_t *result);

#ifdef WORDS_BIGENDIAN
#define byteswap32(x)   bswap32(x)
#define byteswap16(x)   bswap16(x)
uint32_t bswap32(uint32_t b);
uint16_t bswap16(uint16_t b);
#else
#define byteswap32(x)   (x)
#define byteswap16(x)   (x)
#endif



/* **************
   User Interface
   ************** */

/* Display an ordinary status message to stdout */
void print_status(char *fmt, ...);

/* Display an error message to stderr */
void print_error(char *fmt, ...);

/* Display an error message and exit */
void fatal_error(char *fmt, ...);

void try_msg(void);



#endif   /* ifndef __MAIN_H */
