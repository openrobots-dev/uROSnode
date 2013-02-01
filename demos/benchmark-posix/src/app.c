/*
Copyright (c) 2012-2013, Politecnico di Milano. All rights reserved.

Andrea Zoppi <texzk@email.it>
Martino Migliavacca <martino.migliavacca@gmail.com>

http://airlab.elet.polimi.it/
http://www.openrobots.com/

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include "app.h"

#include <stdio.h>
#include <urosNode.h>

/*===========================================================================*/
/* GLOBAL VARIABLES                                                          */
/*===========================================================================*/

const UrosString rateparamname = { 15, "/benchmark_rate" };
const UrosString sizeparamname = { 15, "/benchmark_size" };

/** @brief Benchmark status.*/
benchmark_t benchmark;

/** @brief Printer thread stack.*/
static UROS_STACK(printerstack, PRINTER_STKLEN);

/*===========================================================================*/
/* GLOBAL FUNCTIONS                                                          */
/*===========================================================================*/

uros_err_t app_printer_thread(void* argp) {

  static UrosNodeStatus *const stp = &urosNode.status;

  uint32_t oldtime, newtime;
  uint32_t numpackets, wintime;
  size_t numbytes;
  (void)argp;

  oldtime = urosGetTimestampMsec() - 1;
  urosMutexLock(&stp->stateLock);
  while (!stp->exitFlag) {
    urosMutexUnlock(&stp->stateLock);

    urosMutexLock(&benchmark.lock);
    numpackets = benchmark.numPackets;
    numbytes = benchmark.numBytes;
    benchmark.numPackets = 0;
    benchmark.numBytes = 0;
    urosMutexUnlock(&benchmark.lock);
    newtime = urosGetTimestampMsec();
    wintime = newtime - oldtime;

    printf("%lu pkt/s @ %lu B/s\n",
           (long unsigned)((numpackets * 1000) / wintime),
           (long unsigned)((numbytes * 1000) / wintime));

    /* Sleep until the next second.*/
    urosThreadSleepMsec(urosGetTimestampMsec() - newtime + 1000);
    oldtime = newtime;
  }
  urosMutexUnlock(&stp->stateLock);
  return UROS_OK;
}

void app_printusage(void) {

  puts("Usage:");
  puts("  benchmark <[o][p][s]>");
  puts("");
  puts("Options:");
  puts("  o   Creates the /benchmark/output subscriber (default off)");
  puts("  p   Creates the /benchmark/output publisher (default on)");
  puts("  s   Creates the /benchmark/input subscriber (default on)");
  puts("");
}

uros_bool_t app_parseargs(int argc, char *argv[]) {

  if (argc > 2) {
    return UROS_FALSE;
  }

  if (argc == 2) {
    size_t i;
    benchmark.hasOutPub = UROS_FALSE;
    benchmark.hasInSub = UROS_FALSE;
    benchmark.hasOutSub = UROS_FALSE;
    for (i = 0; i < strlen(argv[1]); ++i) {
      switch (argv[1][i]) {
      case 'p': benchmark.hasOutPub = UROS_TRUE; break;
      case 's': benchmark.hasInSub = UROS_TRUE; break;
      case 'o': benchmark.hasOutSub = UROS_TRUE; break;
      default:
        printf("Invalid option: %c\n\n", argv[1][i]);
        return UROS_FALSE;
      }
    }
  } else {
    benchmark.hasOutPub = UROS_TRUE;
    benchmark.hasInSub = UROS_TRUE;
    benchmark.hasOutSub = UROS_FALSE;
  }
  return UROS_TRUE;
}

void app_initialize(void) {

  uros_err_t err;
  (void)err;

  urosMutexObjectInit(&benchmark.lock);
  benchmark.rate = 1;
  urosStringObjectInit(&benchmark.payload);
  benchmark.numPackets = 0;

  urosInit();
  urosNodeCreateThread();

  err = urosThreadCreateStatic(&benchmark.printerThread, "printer", 10,
                               app_printer_thread, NULL, printerstack,
                               PRINTER_STKLEN);
  urosAssert(err == UROS_OK);
}

void app_wait_exit(void) {

  urosThreadJoin(benchmark.printerThread);
  urosThreadJoin(urosNode.status.nodeThreadId);
}
