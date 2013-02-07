/*
    ChibiOS/RT - Copyright (C) 2006,2007,2008,2009,2010,
                 2011,2012 Giovanni Di Sirio.

    This file is part of ChibiOS/RT.

    ChibiOS/RT is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS/RT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

                                      ---

    A special exception to the GPL can be applied should you wish to distribute
    a combined work that includes ChibiOS/RT, without being obliged to provide
    the source code for any proprietary components. See the file exception.txt
    for full details of how and when the exception can be applied.
*/

#include "app.h"

/*===========================================================================*/
/* Benchmark related.                                                        */
/*===========================================================================*/

#define MAX_THREADS 50

typedef struct thread_info_t {
  Thread        *threadp;
  systime_t     time;
  const char    *namep;
} thread_info_t;

static thread_info_t infos[MAX_THREADS];

static void log_thread_info(void) {

  Thread *tp;
  unsigned i, infosize;
  streamcnt_t inCount, outCount;

  /* Capture the state with the lowest jitter possible.*/
  urosMutexLock(&benchmark.lock);
  inCount = benchmark.inCount;
  outCount = benchmark.outCount;
  benchmark.inCount.deltaMsgs = 0;
  benchmark.inCount.deltaBytes = 0;
  benchmark.outCount.deltaMsgs = 0;
  benchmark.outCount.deltaBytes = 0;
  urosMutexUnlock(&benchmark.lock);
  for (i = 0, tp = chRegFirstThread();
       tp != NULL && i < MAX_THREADS;
       ++i, tp = chRegNextThread(tp)) {
    thread_info_t *const ip = &infos[i];
    ip->threadp = tp;
    ip->time = tp->p_time;
    ip->namep = tp->p_name;
  }
  infosize = i;

  /* Log the captured info.*/
  chprintf(SS1, "@ %U\n", (uint32_t)chTimeNow());
  chprintf(SS1, "IN: %U msg %U B %U msg/s %U B/s\n",
           inCount.numMsgs, inCount.numBytes,
           inCount.deltaMsgs, inCount.deltaBytes);
  chprintf(SS1, "OUT: %U msg %U B %U msg/s %U B/s\n",
           outCount.numMsgs, outCount.numBytes,
           outCount.deltaMsgs, outCount.deltaBytes);
  for (i = 0; i < infosize; ++i) {
    thread_info_t *const ip = &infos[i];
    chprintf(SS1, "%X %U %s\n",
             (uint32_t)ip->threadp, (uint32_t)ip->time, ip->namep);
  }
  chprintf(SS1, "\n");
}

/*===========================================================================*/
/* Main features.                                                            */
/*===========================================================================*/

#define BLINKER_STKSIZE 128
#define BLINKER_WA_SIZE THD_WA_SIZE(BLINKER_STKSIZE)

static WORKING_AREA(wa_blinker, BLINKER_STKSIZE);

msg_t blinker_thread(void *argp) {

  (void)argp;

#if CH_USE_REGISTRY
  chSysLock();
  currp->p_name = "blue_led_blinker";
  chSysUnlock();
#endif
  while (TRUE) {
    palClearPad(GPIOC, GPIOC_LED1);
    chThdSleepMilliseconds(500);
    palSetPad(GPIOC, GPIOC_LED1);
    chThdSleepMilliseconds(500);
  }
  return RDY_OK;
}

/*
 * Application entry point.
 */
int main(void) {

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  /* Creates the blinker thread.*/
  chThdCreateStatic(wa_blinker, BLINKER_WA_SIZE, HIGHPRIO,
                    blinker_thread, NULL);

  /* Make the PHY wake up.*/
  palSetPad(GPIOC, GPIOC_ETH_NOT_PWRDN);

  /* Activates the serial driver 1 using the driver default configuration.*/
  sdStart(&SD1, NULL);

  /* Creates the LWIP threads (it changes priority internally).*/
  chThdCreateStatic(wa_lwip_thread, THD_WA_SIZE(LWIP_THREAD_STACK_SIZE),
                    NORMALPRIO + 1, lwip_thread, NULL);

  /* Initializes the application.*/
  app_initialize();

  /*
   * Normal main() thread activity. It prints the useful thread information.
   */
  while (TRUE) {
    systime_t deadline = chTimeNow() + MS2ST(1000);
    log_thread_info();
    chThdSleepUntil(deadline);
  }
}
