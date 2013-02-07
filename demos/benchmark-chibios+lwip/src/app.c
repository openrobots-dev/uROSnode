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

#include <chprintf.h>
#include <urosNode.h>

/*===========================================================================*/
/* GLOBAL VARIABLES                                                          */
/*===========================================================================*/

const UrosString rateparamname = { 15, "/benchmark_rate" };
const UrosString sizeparamname = { 15, "/benchmark_size" };

/** @brief Benchmark status.*/
benchmark_t benchmark;

/*===========================================================================*/
/* GLOBAL FUNCTIONS                                                          */
/*===========================================================================*/

void app_initialize(void) {

  urosMutexObjectInit(&benchmark.lock);
  benchmark.rate = 1;
  urosStringObjectInit(&benchmark.payload);

  benchmark.hasOutPub = UROS_TRUE;
  benchmark.hasInSub = UROS_TRUE;
  benchmark.hasOutSub = UROS_FALSE;

  benchmark.inCount.numMsgs = 0;
  benchmark.inCount.numBytes = 0;
  benchmark.inCount.deltaMsgs = 0;
  benchmark.inCount.deltaBytes = 0;

  benchmark.outCount.numMsgs = 0;
  benchmark.outCount.numBytes = 0;
  benchmark.outCount.deltaMsgs = 0;
  benchmark.outCount.deltaBytes = 0;

  urosInit();
  urosNodeCreateThread();
}
