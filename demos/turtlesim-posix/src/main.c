/*
 * main.c
 *
 *  Created on: 19/ott/2012
 *      Author: texzk
 */

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include <stdio.h>
#include "app.h"

int main (int argc, char *argv[]) {

  (void)argc;
  (void)argv;

  printf("=> uROS Node Test <=\n");

  /* Initialize the application.*/
  app_initialize();

  /* Enter the background service loop.*/
  for (;;) {
    /* Nothing to do.*/
    urosThreadSleepSec(1);
  }

  return 0;
}

