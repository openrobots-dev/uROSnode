/*
 * main.c
 *
 *  Created on: 19/ott/2012
 *      Author: texzk
 */

#include <stdio.h>
#include "app.h"

int main (int argc, char *argv[]) {

  (void)argc;
  (void)argv;

  printf("=> uROS Node Test <=\n");

  /* Initialize the application.*/
  app_initialize();

  /* Enter the background service loop.*/
  app_background_service();

  return 0;
}

