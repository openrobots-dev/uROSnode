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

/**
 * @file    urosHandlers.c
 * @author  Andrea Zoppi <texzk@email.it>
 *
 * @brief   TCPROS topic and service handlers.
 */

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include "urosHandlers.h"

#include <urosNode.h>
#include <urosTcpRos.h>
#include <urosUser.h>

#include "rtcan.h"
#include "Middleware.hpp"
#include "topics.h"
#include "msg/r2p_ir.h"


/*===========================================================================*/
/* GLOBAL VARIABLES                                                          */
/*===========================================================================*/

extern RemoteSubscriber rsubVelocity;
extern RemotePublisher rpubProximity;
extern Publisher<SpeedSetpoint3> lpubVelocity;
extern Subscriber<IRRaw, 8> lsubProximity;

/*===========================================================================*/
/* LOCAL FUNCTIONS                                                           */
/*===========================================================================*/

#if 0
static inline
int32_t float2fix(const float x) {
  return (int32_t)(x * (float)0x10000l);
}

static inline
float fix2float(const int32_t x) {
  return (float)x * (float)(1.0 / 0x10000l);
}

static inline
int32_t fixmul(const uint32_t a, const uint32_t b) {
  return (int32_t)(((int64_t)a * (int64_t)b + 0x8000l) >> 16u);
}

static inline
int32_t fixdiv(const uint32_t a, const uint32_t b) {
  return (int32_t)((((int64_t)a << 16u) + (int64_t)0x8000l) / (int64_t)b);
}
#endif

template<typename T> static inline
T clamp(T min, T value, T max) {
  return (value < min) ? min : ((value > max) ? max : value);
}

/*
 *  //_______________________\\
 * //            y            \\
 *   \  2        ^        1  /
 *    \          |          /
 *     \         |         /
 *       \       @---->x /
 *        \    z        /
 *         \           /
 *           \       /
 *            \  3  /
 *             \___/
 *             =====
 *
 * Body frame velocity to wheel angular velocity:
 * R * dth1 = cos(60°) * dx - cos(30°) * dy - L * dgamma
 * R * dth2 = cos(60°) * dx + cos(30°) * dy - L * dgamma
 * R * dth3 =           -dx                 - L * dgamma
 *
 * Name mapping and units:
 * dx     = strafe     [m/s]
 * dy     = forward    [m/s]
 * dgamma = angular    [rad/s]
 * dth{1,2,3}          [rad/s]
 */
static
void velocity_to_setpoints(const struct msg__triskar__Velocity &velocity,
                           SpeedSetpoint3 &setpoints) {

  // Model parameters
#define _L        0.300f    // Wheel distance from body origin [m]
#define _R        0.050f    // Wheel radius [m]
#define _MAX_DTH  26.0f     // Maximum wheel angular speed [rad/s]
#define _MAX_SP   4096.0f   // Maximum setpoint
#define _SP_SCALE (_MAX_SP / _MAX_DTH)

#define _m1_R     (-1.0f / _R)
#define _mL_R     (-_L / _R)
#define _C60_R    (0.500000000f / _R)   // cos(60°) / R
#define _C30_R    (0.866025404f / _R)   // cos(30°) / R

  // Wheel angular speeds
  const float dthz123 = _mL_R * velocity.angular;
  const float dx12 = _C60_R * velocity.strafe;
  const float dy12 = _C30_R * velocity.forward;

  float dth1 = dx12 - dy12 + dthz123;
  float dth2 = dx12 + dy12 + dthz123;
  float dth3 = _m1_R * velocity.strafe + dthz123;

  // Motor setpoints
  setpoints.speed1 = (int16_t)clamp(-10000.0f, dth1 * _SP_SCALE, 10000.0f);
  setpoints.speed2 = (int16_t)clamp(-10000.0f, dth2 * _SP_SCALE, 10000.0f);
  setpoints.speed3 = (int16_t)clamp(-10000.0f, dth3 * _SP_SCALE, 10000.0f);
}

static
void irraw_to_proximities(const IRRaw &irraw,
                          struct msg__triskar__Proximity &prox) {

  prox.proximities[0] = clamp(0.0f, 1.0f - (float)irraw.value1 * (1.0f / 4095.0f), 1.0f);
  prox.proximities[1] = clamp(0.0f, 1.0f - (float)irraw.value2 * (1.0f / 4095.0f), 1.0f);
  prox.proximities[2] = clamp(0.0f, 1.0f - (float)irraw.value3 * (1.0f / 4095.0f), 1.0f);
  prox.proximities[3] = clamp(0.0f, 1.0f - (float)irraw.value4 * (1.0f / 4095.0f), 1.0f);
}

/*===========================================================================*/
/* PUBLISHED TOPIC FUNCTIONS                                                 */
/*===========================================================================*/

/** @addtogroup tcpros_pubtopic_funcs */
/** @{ */

/*~~~ PUBLISHED TOPIC: /triskar/proximity ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Topic <tt>/triskar/proximity</tt> publisher */
/** @{ */

/**
 * @brief   TCPROS <tt>/triskar/proximity</tt> published topic handler.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @return
 *          Error code.
 */
uros_err_t pub_tpc__triskar__proximity(UrosTcpRosStatus *tcpstp) {

  /* Message allocation and initialization.*/
  UROS_TPC_INIT_S(msg__triskar__Proximity);
  IRRaw *r2p_msg;
  Subscriber<IRRaw, 8> lsubProximity("IRRaw");
  Node mwNode("mwNode");
  Middleware::instance().newNode(&mwNode);
  mwNode.subscribe(&lsubProximity);

  /* Published messages loop.*/
  while (!urosTcpRosStatusCheckExit(tcpstp)) {
    /* Republish the received R2P raw proximity message to ROS.*/
    mwNode.spin(MS2ST(1000));
    r2p_msg = lsubProximity.get();
    if (r2p_msg != NULL) {
      irraw_to_proximities(*r2p_msg, msg);
      lsubProximity.release(r2p_msg);

      /* Send the message.*/
      UROS_MSG_SEND_LENGTH(&msg, msg__triskar__Proximity);
      UROS_MSG_SEND_BODY(&msg, msg__triskar__Proximity);

      /* Dispose the contents of the message.*/
      clean_msg__triskar__Proximity(&msg);
    }
  }
  tcpstp->err = UROS_OK;

_finally:
  /* Message deinitialization and deallocation.*/
  UROS_TPC_UNINIT_S(msg__triskar__Proximity);
  return tcpstp->err;
}

/** @} */

/** @} */

/*===========================================================================*/
/* SUBSCRIBED TOPIC FUNCTIONS                                                */
/*===========================================================================*/

/** @addtogroup tcpros_subtopic_funcs */
/** @{ */

/*~~~ SUBSCRIBED TOPIC: /triskar/velocity ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Topic <tt>/triskar/velocity</tt> subscriber */
/** @{ */

/**
 * @brief   TCPROS <tt>/triskar/velocity</tt> subscribed topic handler.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @return
 *          Error code.
 */
uros_err_t sub_tpc__triskar__velocity(UrosTcpRosStatus *tcpstp) {

  /* Message allocation and initialization.*/
  UROS_TPC_INIT_S(msg__triskar__Velocity);
  SpeedSetpoint3 *r2p_msg;

  /* Subscribed messages loop.*/
  while (!urosTcpRosStatusCheckExit(tcpstp)) {
    /* Receive the next message.*/
    UROS_MSG_RECV_LENGTH();
    UROS_MSG_RECV_BODY(&msg, msg__triskar__Velocity);

    /* Republish the received ROS message to R2P DC motor modules.*/
    r2p_msg = lpubVelocity.alloc();
    if (r2p_msg != NULL) {
      velocity_to_setpoints(msg, *r2p_msg);
      lpubVelocity.broadcast(r2p_msg);
      palTogglePad(GPIOC, GPIOC_LED2);
    }

    /* Dispose the contents of the message.*/
    clean_msg__triskar__Velocity(&msg);
  }
  tcpstp->err = UROS_OK;

_finally:
  /* Message deinitialization and deallocation.*/
  UROS_TPC_UNINIT_S(msg__triskar__Velocity);
  return tcpstp->err;
}

/** @} */

/** @} */

/*===========================================================================*/
/* PUBLISHED SERVICE FUNCTIONS                                               */
/*===========================================================================*/

/** @addtogroup tcpros_pubservice_funcs */
/** @{ */

/* There are no published services.*/

/** @} */

/*===========================================================================*/
/* CALLED SERVICE FUNCTIONS                                                  */
/*===========================================================================*/

/** @addtogroup tcpros_callservice_funcs */
/** @{ */

/* There are no called services.*/

/** @} */

/*===========================================================================*/
/* GLOBAL FUNCTIONS                                                          */
/*===========================================================================*/

/** @addtogroup tcpros_funcs */
/** @{ */

/**
 * @brief   Registers all the published topics to the Master node.
 * @note    Should be called at node initialization.
 */
void urosHandlersPublishTopics(void) {

  /* /triskar/proximity */
  urosNodePublishTopicSZ(
    "/triskar/proximity",
    "triskar/Proximity",
    (uros_proc_f)pub_tpc__triskar__proximity,
    uros_nulltopicflags
  );
}

/**
 * @brief   Unregisters all the published topics to the Master node.
 * @note    Should be called at node shutdown.
 */
void urosHandlersUnpublishTopics(void) {

  /* /triskar/proximity */
  urosNodeUnpublishTopicSZ(
    "/triskar/proximity"
  );
}

/**
 * @brief   Registers all the subscribed topics to the Master node.
 * @note    Should be called at node initialization.
 */
void urosHandlersSubscribeTopics(void) {

  /* /triskar/velocity */
  urosNodeSubscribeTopicSZ(
    "/triskar/velocity",
    "triskar/Velocity",
    (uros_proc_f)sub_tpc__triskar__velocity,
    uros_nulltopicflags
  );
}

/**
 * @brief   Unregisters all the subscribed topics to the Master node.
 * @note    Should be called at node shutdown.
 */
void urosHandlersUnsubscribeTopics(void) {

  /* /triskar/velocity */
  urosNodeUnsubscribeTopicSZ(
    "/triskar/velocity"
  );
}

/**
 * @brief   Registers all the published services to the Master node.
 * @note    Should be called at node initialization.
 */
void urosHandlersPublishServices(void) {

  /* No services to publish.*/
}

/**
 * @brief   Unregisters all the published services to the Master node.
 * @note    Should be called at node shutdown.
 */
void urosHandlersUnpublishServices(void) {

  /* No services to unpublish.*/
}

/** @} */

