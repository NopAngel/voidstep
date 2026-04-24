/* hello.c - A simple target for our voidstep.
   Copyright (C) 2026 NopAngel.
   Licensed under the GNU General Public License v3 or later. */

#include <stdio.h>

/* A simple function to test call/ret instructions. */
void
say_hello (int count)
{
  printf ("Hello World! This is count: %d\n", count);
}

int
main (void)
{
  int i;

  /* We'll loop a few times so we can 'step' through the execution. */
  for (i = 0; i < 3; i++)
    {
      say_hello (i);
    }

  return 0;
}
