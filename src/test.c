#include <stdio.h>

int main(int argc, char **argv)
{
  int count, i;
  for (count = 0 ; count <= 0xff ; ++count)
    {
      for (i = 0 ; i < 27 ; ++i)
	printf ("%c", count);
    }
  return 0;
}
