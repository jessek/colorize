
/* $Id$ */

#include "main.h"

unsigned char padding(void)
{
  return RGB_PADDING;
}
 
/* Previous versions of this program used a stippled pattern to show padding */
/*
static unsigned char padding(state *s)
{
  uint32_t ret;

  if (s->padding)
    ret = RGB_BLACK;
  else
    ret = RGB_PADDING;
  
  s->padding = !(s->padding);
  return ret;
}
*/


int find_file_size(char *fn, FILE *h, off_t *sz)
{
  off_t orig = ftello(h);
  
  if (fseeko(h, 0, SEEK_END)) {
    perror(fn);
    return TRUE;
  }
  
  *sz = ftello(h);
  
  if (fseeko(h, orig, SEEK_SET)) { 
    perror(fn);
    return TRUE;
  }

  return FALSE;
}


#ifdef WORDS_BIGENDIAN
uint32_t bswap32(uint32_t b)
{
  uint32_t res;
  
  res = ((b & 0xff) << 24) + ((b & 0xff00) << 8) + ((b & 0xff0000) >> 8) + 
    ((b & 0xff000000) >> 24);

  return res;
}

uint16_t bswap16(uint16_t b)
{
  uint16_t res = ((b & 0xff) << 8) + ((b & 0xff00) >> 8);
  return res;
}
#else
uint16_t bswap16(uint16_t b)
{
  return b;
}
#endif
