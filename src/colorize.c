
// $Id$

#include "main.h"

#define ORIENTATION_HORIZONTAL  FALSE
#define ORIENTATION_VERTICAL    TRUE
#define DIRECTION_DOWN          FALSE
#define DIRECTION_UP            TRUE

#define DEFAULT_ORIENTATION     ORIENTATION_VERTICAL
#define DEFAULT_DIRECTION       DIRECTION_UP
#define DEFAULT_IMAGE_WIDTH     100


typedef struct _state 
{
  int      verbose, orientation, direction, padding;
  char     *input_fn;
  off_t    input_length;
  int32_t  image_width, image_height;
  FILE     *in_handle, *out_handle; 
} state;


static void usage(void)
{
  print_status ("%s by Jesse Kornblum version %s", __progname, VERSION);
  print_status ("%s %s [-h|-V] [-w <num>] [-ovd] FILES", CMD_PROMPT, __progname);
  print_status ("");

  print_status ("-d  - Change direction data flows, defaults to down or right");
  print_status ("-o  - Change image orientation, defaults to vertical");
  print_status ("-v  - Verbose mode");
  print_status ("-w  - Set output image width, defaults to %"PRIu32, 
		DEFAULT_IMAGE_WIDTH);
  print_status ("-V  - Display version number and exit");
  print_status ("-h  - Display this help message and exit");
}


static int initialize_state(state *s)
{
  if ((s->input_fn = (char *)malloc(sizeof(char) * PATH_MAX)) == NULL)
    {
      perror(__progname);
      return TRUE;
    }

  s->padding      = FALSE;
  s->orientation  = DEFAULT_ORIENTATION;
  s->direction    = DEFAULT_DIRECTION;
  s->verbose      = FALSE;
  s->input_length = 0;
  s->image_width  = DEFAULT_IMAGE_WIDTH;

  return FALSE;
}




static int write_file_header(state *s)
{
  BITMAPFILEHEADER b;

  b.bfType = BF_TYPE;
  b.bfSize = byteswap32((s->image_width * s->image_height * 3) + 54);
  b.bfReserved1 = 0;

  // The offset in the file where RGB image data begins 
  b.bfOffBits   = byteswap32(0x36);

  fwrite(&b.bfType,1,2,s->out_handle);
  fwrite(&b.bfSize,1,4,s->out_handle);

  // Write both reserved values at once 
  fwrite(&b.bfReserved1,1,2,s->out_handle);
  fwrite(&b.bfReserved1,1,2,s->out_handle);

  fwrite(&b.bfOffBits,sizeof(uint32_t),1,s->out_handle);

  return FALSE;
}

#define HEADERINFOSIZE  40

static int write_file_info(state *s)
{
  BITMAPINFOHEADER b;

  // Size of the this header information
  b.biSize = byteswap32(HEADERINFOSIZE);  

  if (s->orientation == ORIENTATION_VERTICAL)
    {
      b.biWidth = byteswap32(s->image_width);
      b.biHeight = byteswap32(s->image_height);
    }
  else
    {
      b.biWidth = byteswap32(s->image_height);
      b.biHeight = byteswap32(s->image_width);
    }

  b.biPlanes      = byteswap16(1);
  b.biBitCount    = byteswap16(24);
  b.biCompression = BI_RGB;

  // How much RGB data follows this header 
  b.biSizeImage = byteswap32((s->image_width * s->image_height) * 3);

  b.biXPelsPerMeter = 0;
  b.biYPelsPerMeter = 0;
  // These values denote how many values are in the RGB color map
  // that follows. 
  b.biClrUsed       = 0;
  b.biClrImportant  = 0;

  if (s->verbose)
    print_status ("%s: Image will be %"PRIu32" x %"PRIu32" x %"PRIu32, 
		  __progname,
		  byteswap32(b.biWidth), 
		  byteswap32(b.biHeight), 
		  byteswap16(b.biBitCount));
  
  fwrite(&b,HEADERINFOSIZE,1,s->out_handle);

  return FALSE;
}
  

// We must accept a signed value to handle the negative values
// we may be passed by the file read routine. The Red computation
// requests lookup_color(input char - 128) which can easily be -128. */

// RBF - Document the process of lookup_color 
static unsigned char lookup_color(int16_t c)
{
  if (c < 0)
    return 0;
  if (c < 64) 
    return c * 4;
  if (c <= 192)
    return 0xff;
  if (c < 256)
    return 0x100 - ((c - 192) * 4);

  return 0;
}


static int write_byte(state *s, unsigned char c)
{
  RGBQUAD r;

  if (0 == c)
    r.rgbBlue = r.rgbRed = r.rgbGreen = 0;
  else if (0xff == c)
    // RBF - What happens if 0xff characters are just dark red? 
    r.rgbBlue = r.rgbRed = r.rgbGreen = 0xff;
  else
    {
      r.rgbBlue  = lookup_color( (int16_t)c+128);
      r.rgbGreen = lookup_color( (int16_t)c );
      r.rgbRed   = lookup_color( (int16_t)c-128);
    }
  
  fwrite(&r,sizeof(RGBQUAD),1,s->out_handle);
  
  return FALSE;
}


static void pad_image(state *s, uint32_t val) { 
  switch (val % 4)  { 
    // I'm not entirely sure why this works, but it does. 
    // Note that there are deliberately no breaks in this switch.
  case 3:
    fputc(0,s->out_handle);
  case 2:
    fputc(0,s->out_handle);
  case 1:
    fputc(0,s->out_handle);
  }
}



static int write_vertical_file(state *s) { 
  int32_t i,j, bytes_read = 0;
  off_t location;
  int32_t offset = (s->image_width * s->image_height) - s->image_width;
  unsigned char c;
  
#ifdef DEBUG
  print_status ("offset %"PRId32"  length %"PRId32"  width %"PRId32"\n",
		offset, s->input_length, s->image_width);
#endif

  for (i = 0 ; i < s->image_height ; ++i) {
    for (j = 0 ; j < s->image_width ; ++j) {
      if (s->direction) { 
	location =  offset - (s->image_width * i) + j;
	if (location < s->input_length) { 
#ifdef DEBUG
	  print_status ("bytes read %06"PRId32"  seeking to %06"PRId32,
			bytes_read, location);
#endif
	  if (fseeko(s->in_handle, location, SEEK_SET)) {
	    return TRUE;
	  }
	  c = fgetc(s->in_handle); 
	} else { 
#ifdef DEBUG
	  print_status ("padding"); 
#endif
	  // We pad the image with black when necessary 
	  c = padding();
	}
      } else { 
	if (bytes_read < s->input_length)
	  c = fgetc(s->in_handle);
	else
	  c = padding();
      }
      
      ++bytes_read;
      write_byte(s, c);
    }

    if (s->direction)
      pad_image(s,s->image_width);
  }    
  
  return FALSE;
}


static int write_horizontal_file(state *s)
{
  int32_t i,j,location;
  unsigned char c;

  for (i = 0 ; i < s->image_width ; i++) {
    for (j = 0 ; j < s->image_height ; ++j) {
      
      if (s->direction)
	location = (s->image_width * (j+1)) - (i+1); 
      else
	location = s->image_height * (s->image_width - i -1) + j;

      if (location < s->input_length) {
	fseeko(s->in_handle,location,SEEK_SET);
	c = fgetc(s->in_handle);
      } else {
	// We pad the image with black when necessary 
	c = padding();
      }
	  
      write_byte(s,c);
    }
    pad_image(s,s->image_height);
  }
  
  return FALSE;
}


static int make_bmp_from_file(state *s, char *input)
{
  char *o1, *o2, *output = (char *)malloc(sizeof(char) * PATH_MAX);

  if (s->verbose)
    print_status ("%s: Processing %s", __progname, input);

  if ((o1 = strdup(input)) == NULL)
    {
      perror(__progname);
      return TRUE;
    }

  o2 = basename(o1);
  snprintf(output, PATH_MAX, "%s.bmp", o2);
  // We don't free o2 as it's statically allocated 
  free(o1);

  if ((s->in_handle = fopen(input,"rb")) == NULL)
    {
      free(output);
      perror(input);
      return TRUE;
    }

  snprintf(s->input_fn,PATH_MAX,"%s", input);

  if ((s->out_handle = fopen(output,"wb")) == NULL)
    {
      fclose(s->in_handle);
      perror(output);
      free(output);
      return TRUE;
    }

  if (find_file_size(input, s->in_handle, &s->input_length))
    return TRUE;

  s->image_height = (s->input_length / s->image_width);
  if (s->input_length % s->image_width != 0)
    s->image_height++;

  if (write_file_header(s))
    return TRUE;

  if (write_file_info(s))
    return TRUE;

  if (s->orientation == ORIENTATION_VERTICAL) {
    if (write_vertical_file(s)) {
      perror(output);
    }
  } else {
    write_horizontal_file(s);
  }

  if (s->verbose)
    print_status ("%s: Done.", __progname);

  fclose(s->in_handle);
  fclose(s->out_handle);
  free(output);

  return FALSE;
}


int process_cmd_line(state *s, int argc, char **argv)
{
  int i;

  while ((i = getopt(argc,argv,"w:ovhdV")) != -1) {
    switch (i) {

    case 'd':
      s->direction   = !s->direction;
      break;

    case 'o':
      s->orientation = !s->orientation;
      break;

    case 'w':
      s->image_width = atoll(optarg);
      if (0 == s->image_width)
	fatal_error("%s: Illegal width", __progname);
      break;

    case 'v':
      if (s->verbose)
	{
	  print_error ("%s: Already at maximum verbosity", __progname);
	  print_error ("%s: Error message displayed successfully", __progname);
	}
      else
	s->verbose = TRUE;
      break;
      
    case 'h':
      usage();
      exit (EXIT_SUCCESS);

    case 'V':
      print_status ("%s", VERSION);
      exit (EXIT_SUCCESS);
      
    default:
      try_msg();
      exit (EXIT_FAILURE);
    }
  }

  return FALSE;
}

int main(int argc, char **argv)
{
  state *s = (state *)malloc(sizeof(state));

#ifndef __GLIBC__
  __progname  = strdup(basename(argv[0]));
#endif

  if (initialize_state(s))
    return EXIT_FAILURE;

  if (process_cmd_line(s,argc,argv))
    return EXIT_FAILURE;

  argv += optind;
      
  while (*argv != NULL) {
    make_bmp_from_file(s,*argv);
    ++argv;
  }

  // We don't bother cleaning up the state as we're about to exit. 
  // All of the memory we have
  // allocated is going to be returned to the operating system, so
  // there's no point in our explicitly free'ing it. 
  
  return EXIT_SUCCESS;
}
