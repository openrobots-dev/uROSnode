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

#include "rtcan.h"
#include "Middleware.hpp"
#include "topics.h"
#include "msg/r2p_ir.h"

/*===========================================================================*/
/* Main features.                                                            */
/*===========================================================================*/

extern RTCANDriver RTCAND;

RemoteSubscriberT<SpeedSetpoint3, 8> rsubVelocity("pwm123");
RemotePublisher rpubProximity("IRRaw", sizeof(IRRaw));

Node mwNode("mwNode");
Publisher<SpeedSetpoint3> lpubVelocity("pwm123");

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

  /*
   * Activates the RTCAN driver.
   */
  rtcanInit();
  rtcanStart (NULL);

  /* Creates the LWIP threads (it changes priority internally).*/
  chThdCreateStatic(wa_lwip_thread, THD_WA_SIZE(LWIP_THREAD_STACK_SIZE),
                    NORMALPRIO + 1, lwip_thread, NULL);

  rsubVelocity.id(SPEED123_ID);
  rpubProximity.id(IRRAW_ID);
  Middleware &mw = Middleware::instance();
  mw.newNode(&mwNode);
  mw.advertise(&rpubProximity);
  mwNode.advertise(&lpubVelocity);
  LocalPublisher *pubp = mw.findLocalPublisher("pwm123");
  if (pubp != NULL) {
    rsubVelocity.subscribe(pubp);
  }

  app_initialize();

  /*
   * Normal main() thread activity. It prints the useful thread information.
   */
  while (TRUE) {
    systime_t deadline = chTimeNow() + MS2ST(1000);
    chThdSleepUntil(deadline);
  }
}
