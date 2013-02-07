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
#include <sys/time.h>
#include <sys/resource.h>

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

void app_print_cpu_state(void) {

  FILE *statfp = NULL;
  cpucnt_t curCpu, oldCpu;
  double mult;
  int n; (void)n;

  statfp =  fopen("/proc/stat", "rt");
  urosAssert(statfp != NULL);
  n = fscanf(statfp, "%*s %lu %lu %lu %lu ",
             &curCpu.user, &curCpu.nice, &curCpu.system, &curCpu.idle);
  urosAssert(n == 4);
  fclose(statfp);
  urosMutexLock(&benchmark.lock);
  oldCpu = benchmark.oldCpu = benchmark.curCpu;
  benchmark.curCpu = curCpu;
  urosMutexUnlock(&benchmark.lock);

  mult = 100.0 /
         ((curCpu.user + curCpu.nice + curCpu.system + curCpu.idle) -
          (oldCpu.user + oldCpu.nice + oldCpu.system + oldCpu.idle));

  printf("CPU%%: user: %.3f nice: %.3f sys: %.3f idle: %.3f\n",
         (curCpu.user - oldCpu.user) * mult,
         (curCpu.nice - oldCpu.nice) * mult,
         (curCpu.system - oldCpu.system) * mult,
         (curCpu.idle - oldCpu.idle) * mult);
}

void app_print_cpu_usage(void) {

  struct timespec time;
  int err; (void)err;

#if defined(CLOCK_PROCESS_CPUTIME_ID)
  err = clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time);
#elif defined(CLOCK_VIRTUAL)
  err = clock_gettime(CLOCK_VIRTUAL, &time);
#else
#error "process' clock_gettime() not available"
#endif
  urosAssert(err == 0);

  printf("USER: %lu.%9.9lu\n",
         (unsigned long)time.tv_sec, (unsigned long)time.tv_nsec);
}

void app_print_thread_state(pthread_t threadId) {

  char namebuf[32];
  clockid_t clockId;
  struct timespec time;
  int err; (void)err;

  err = pthread_getcpuclockid(threadId, &clockId);
  urosAssert(err == 0);
  err = clock_gettime(clockId, &time);
  urosAssert(err == 0);
  err = pthread_getname_np(threadId, namebuf, 31);
  urosAssert(err == 0);
  namebuf[31] = 0;

  printf("%lu %lu.%9.9lu %s\n",
         (unsigned long)threadId,
         (unsigned long)time.tv_sec, (unsigned long)time.tv_nsec,
         namebuf);
}

uros_err_t app_printer_thread(void* argp) {

  static UrosNodeStatus *const stp = &urosNode.status;

  uint32_t oldTime, curTime, winTime;
  streamcnt_t inCount, outCount;
  uros_cnt_t i;
  int err; (void)err;
  (void)argp;

  /* Wait until all of the threads exist.*/
  do {
    urosMutexLock(&stp->stateLock);
    if (stp->exitFlag) {
      urosMutexUnlock(&stp->stateLock);
      return UROS_OK;
    }
    if (stp->state == UROS_NODE_RUNNING) {
      urosMutexUnlock(&stp->stateLock);
      break;
    }
    urosMutexUnlock(&stp->stateLock);
    urosThreadSleepMsec(20);
  } while (UROS_TRUE);

  oldTime = urosGetTimestampMsec() - 1;
  urosMutexLock(&stp->stateLock);
  while (!stp->exitFlag) {
    urosMutexUnlock(&stp->stateLock);

    urosMutexLock(&benchmark.lock);
    inCount = benchmark.inCount;
    outCount = benchmark.outCount;
    benchmark.inCount.deltaMsgs = 0;
    benchmark.inCount.deltaBytes = 0;
    benchmark.outCount.deltaMsgs = 0;
    benchmark.outCount.deltaBytes = 0;
    urosMutexUnlock(&benchmark.lock);
    curTime = urosGetTimestampMsec();
    winTime = curTime - oldTime;

    printf("@ %lu\n", (unsigned long)curTime);
    printf("IN: %lu msg %lu B %lu msg/s %lu B/s\n",
           (unsigned long)((1000 * inCount.numMsgs + 499) / winTime),
           (unsigned long)((1000 * inCount.numBytes + 499) / winTime),
           (unsigned long)((1000 * inCount.deltaMsgs + 499) / winTime),
           (unsigned long)((1000 * inCount.deltaBytes + 499) / winTime));
    printf("OUT: %lu msg %lu B %lu msg/s %lu B/s\n",
           (unsigned long)((1000 * outCount.numMsgs + 499) / winTime),
           (unsigned long)((1000 * outCount.numBytes + 499) / winTime),
           (unsigned long)((1000 * outCount.deltaMsgs + 499) / winTime),
           (unsigned long)((1000 * outCount.deltaBytes + 499) / winTime));

    /* Global CPU usage.*/
    app_print_cpu_state();
    app_print_cpu_usage();

    /* Printer and Node thread.*/
    app_print_thread_state(pthread_self());

    urosMutexLock(&stp->stateLock);

    /* Node and listeners.*/
    app_print_thread_state(stp->nodeThreadId);
    app_print_thread_state(stp->xmlrpcListenerId);
    app_print_thread_state(stp->tcprosListenerId);

    /* XMLRPC slave pool.*/
    urosMutexLock(&stp->slaveThdPool.readyMtx);
    urosMutexLock(&stp->slaveThdPool.busyMtx);
    for (i = 0; i < stp->slaveThdPool.size; ++i) {
      app_print_thread_state(stp->slaveThdPool.threadsp[i]);
    }
    urosMutexUnlock(&stp->slaveThdPool.busyMtx);
    urosMutexUnlock(&stp->slaveThdPool.readyMtx);

    /* TCPROS server pool.*/
    urosMutexLock(&stp->tcpsvrThdPool.readyMtx);
    urosMutexLock(&stp->tcpsvrThdPool.busyMtx);
    for (i = 0; i < stp->tcpsvrThdPool.size; ++i) {
      app_print_thread_state(stp->tcpsvrThdPool.threadsp[i]);
    }
    urosMutexUnlock(&stp->tcpsvrThdPool.busyMtx);
    urosMutexUnlock(&stp->tcpsvrThdPool.readyMtx);

    /* TCPROS client pool.*/
    urosMutexLock(&stp->tcpcliThdPool.readyMtx);
    urosMutexLock(&stp->tcpcliThdPool.busyMtx);
    for (i = 0; i < stp->tcpcliThdPool.size; ++i) {
      app_print_thread_state(stp->tcpcliThdPool.threadsp[i]);
    }
    urosMutexUnlock(&stp->tcpcliThdPool.busyMtx);
    urosMutexUnlock(&stp->tcpcliThdPool.readyMtx);

    urosMutexUnlock(&stp->stateLock);
    printf("\n");

    /* Sleep until the next second.*/
    urosThreadSleepMsec(urosGetTimestampMsec() - curTime + 1000);
    oldTime = curTime;
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
  benchmark.inCount.deltaMsgs = 0;
  benchmark.inCount.deltaBytes = 0;
  benchmark.outCount.deltaMsgs = 0;
  benchmark.outCount.deltaBytes = 0;

  urosInit();
  urosNodeCreateThread();

  err = urosThreadCreateStatic(&benchmark.printerId, "printer", 50,
                               app_printer_thread, NULL, printerstack,
                               PRINTER_STKLEN);
  urosAssert(err == UROS_OK);
}

void app_wait_exit(void) {

  urosThreadJoin(benchmark.printerId);
  urosThreadJoin(urosNode.status.nodeThreadId);
}
