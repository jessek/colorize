
/* $Id$ */

#include "main.h"

typedef struct _state {
  int      padding;
  uint64_t mode;
  uint64_t block_size;
  off_t    size1, size2;
} state;

#define mode_none             0
#define mode_transitional  1<<0

int initialize_state(state *s)
{
  s->padding    = FALSE;
  s->mode       = mode_none;
  s->block_size = DEFAULT_BLOCK_SIZE;

  return FALSE;
}


void usage(void)
{
  print_status ("%s version %s by %s", __progname, VERSION, AUTHOR);
  print_status ("Usage: %s [-b size[bkmgpe]] [-Vh] FILE1 FILE2", __progname);
  print_status ("");
  print_status ("-b  Set block size with optional suffix b,k,m,g,p, or e");
  print_status ("    Note that the program will output at least one complete block");
  print_status ("    Make sure you have enough disk space!");
  print_status ("-t  Use transitional colors instead of default red or green");
  print_status ("-V  Display version number and exit");
  print_status ("-h  Display this help message");
}


int find_block_size(state *s, char *input_str)
{
  unsigned char c;
  uint64_t multiplier = 1;

  if (isalpha(input_str[strlen(input_str) - 1]))
    {
      c = tolower(input_str[strlen(input_str) - 1]);
      /* There are deliberately no break statements in this switch */
      switch (c) {
      case 'e':
	multiplier *= 1024;    
      case 'p':
	multiplier *= 1024;    
      case 't':
	multiplier *= 1024;    
      case 'g':
	multiplier *= 1024;    
      case 'm':
	multiplier *= 1024;
      case 'k':
	multiplier *= 1024;
      case 'b':
	break;
      default:
	print_error("%s: Improper multiplier, ignoring", __progname);
      }
      input_str[strlen(input_str) - 1] = 0;
    }

  s->block_size = atoll(input_str) * multiplier;

  return FALSE;
}

static int sanity_check(state *s)
{
  if (s->block_size > MAX_BLOCK_SIZE)
    {
      print_error("%s: Block size must be less than %"PRIu64,
		  __progname, MAX_BLOCK_SIZE);
      return TRUE;
    }
  return FALSE;
}

extern char *optarg;

static int process_cmd_line(state *s, int argc, char **argv)
{
  int i;

  while ((i=getopt(argc,argv,"tb:hV")) != -1) {
    switch(i) {

    case 't':
      s->mode |= mode_transitional;
      break;

    case 'b':
      find_block_size(s,optarg);
      break;

    case 'h':      
      usage();
      exit(EXIT_SUCCESS);

    case 'V':
      print_status ("%s", VERSION);
      exit(EXIT_SUCCESS);
      
    default:
      try_msg();
      exit(EXIT_FAILURE);
    }
  }

  return (sanity_check(s));
}


void transitional_compare(state *s, unsigned char *b1, unsigned char *b2)
{
  uint64_t count, sum = 0;
  unsigned char r;
  
  for (count = 0 ; count < s->block_size ; ++count)
    sum += abs(b1[count] - b2[count]);

  r = sum / s->block_size;

  /* RBF: Right now r can change from 0 to 255. We want to rescale that so
     RBF: 0 will turn out as a dark green and get "redder" from there */

  printf ("%c", r);
}


int compare_blocks(state *s, FILE *h1, FILE *h2)
{
  int done = FALSE;
  unsigned char *b1, *b2;

  b1 = (unsigned char *)malloc(sizeof(unsigned char) * s->block_size);
  if (b1 == NULL)
    fatal_error("Not enough memory");

  b2 = (unsigned char *)malloc(sizeof(unsigned char) * s->block_size);
  if (b2 == NULL)
    fatal_error("Not enough memory");

  while (!(feof(h1) && feof(h2)))
    {
      memset(b1,0,s->block_size);
      memset(b2,0,s->block_size);

      fread(b1,sizeof(unsigned char),s->block_size,h1);
      fread(b2,sizeof(unsigned char),s->block_size,h2);

      if (done)
	printf ("%c", padding());
      else 
	{
	  if (s->mode & mode_transitional)
	    transitional_compare(s,b1,b2);
	  else
	    {
	      if (memcmp(b1,b2,s->block_size))
		printf ("%c", CHAR_NOT_EQUAL);
	      else
		printf ("%c", CHAR_EQUAL);
	    }
	}
      if (feof(h1) || feof(h2))
	done = TRUE;
    }
  
  free(b1);
  free(b2);
  return FALSE;
}


int compare_files(state *s, char *fn1, char *fn2)
{
  FILE *h1, *h2;
  int status;

  if ((h1 = fopen(fn1,"rb")) == NULL)
    {
      perror(fn1);
      return TRUE;
    }

  if ((h2 = fopen(fn2,"rb")) == NULL)
    {
      perror(fn2);
      fclose(h1);
      return TRUE;
    }
  
  if (find_file_size(fn1,h1,&s->size1))
    return TRUE;
  if (find_file_size(fn2,h2,&s->size2))
    return TRUE;

  if (s->size1 != s->size2)
    print_error ("%s: files are not the same size", __progname);
  
  status = compare_blocks(s,h1,h2);

  fclose(h1);
  fclose(h2);

  return status;
}


int main(int argc, char **argv)
{
  state *s = (state *)malloc(sizeof(state));

#ifndef __GLIBC__
  __progname  = strdup(basename(argv[0]));
#endif

  if (s == NULL)
      fatal_error("Unable to allocate state");

  if (initialize_state(s))
    fatal_error("Unable to initialize state");

  if (process_cmd_line(s,argc,argv))
    return EXIT_FAILURE;

  argv += optind;
  argc -= optind;

  /* We must have two files to compare */
  if (argc != 2)
    {
      usage();
      return EXIT_FAILURE;
    }

  if (compare_files(s,*argv,*(argv+1)))
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}
