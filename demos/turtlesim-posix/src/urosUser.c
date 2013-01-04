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
 * @file    urosUser.c
 * @author  Andrea Zoppi <texzk@email.it>
 *
 * @brief   User-defined callback functions.
 */

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include <urosBase.h>
#include <urosNode.h>
#include <urosRpcCall.h>
#include <urosRpcSlave.h>
#include <urosTcpRos.h>
#include <stdarg.h>

#include "urosTcpRosTypes.h"
#include "app.h"

/*===========================================================================*/
/* GLOBAL FUNCTIONS                                                          */
/*===========================================================================*/

/** @addtogroup user_funcs */
/** @{ */

/**
 * @brief   Prints a formatted error message.
 * @details User-defined callback function to print an error message on the
 *          desired output stream.
 *
 * @param[in] formatp
 *          Format string.
 * @param[in] ...
 *          @p printf() style arguments.
 */
void urosUserErrPrintf(const char *formatp, ...) {

  va_list args;

  urosAssert(formatp != NULL);

  va_start(args, formatp);
  vfprintf(stderr, formatp, args);
  va_end(args);
}

/**
 * @brief   Registers static message types.
 * @details This callback function is called at boot time to initialize the
 *          set of message types recognized by the system.
 *
 * @pre     The global static message type set has not been initialized yet.
 */
void urosUserRegisterStaticTypes(void) {

  urosTcpRosRegStaticTypes();
}

/**
 * @brief   Shutdown callback function.
 * @details This callback function notifies the user that a @p shutdown()
 *          XMLRPC call was issued by the Master node, and has to be handled.
 *
 * @param[in] msgp
 *          Pointer to a string which explains the reason why it is asked to be
 *          shutdown.
 * @return
 *          Error code.
 */
uros_err_t urosUserShutdown(const UrosString *msgp) {

  static UrosNodeStatus *const stp = &urosNode.status;

  (void)msgp;

#if UROS_USE_ASSERT
  urosAssert(msgp != NULL);
  urosMutexLock(&stp->stateLock);
  urosAssert(stp->state == UROS_NODE_SHUTDOWN);
  urosMutexUnlock(&stp->stateLock);
#endif

  urosMutexLock(&turtleCanSpawnLock);
  turtleCanSpawn = UROS_FALSE;
  urosMutexUnlock(&turtleCanSpawnLock);

  /* Kill all the turtles.*/
  {
    unsigned i;
    for (i = 0; i < MAX_TURTLES; ++i) {
      turtle_t *tp = &turtles[i];
      urosMutexLock(&tp->lock);
      if (tp->status == TURTLE_ALIVE) {
        urosMutexUnlock(&tp->lock);
        turtle_kill(tp);
      } else {
        urosMutexUnlock(&tp->lock);
      }
    }
  }

  /* Send a dummy getPid() request, to unlock XMLRPC listener and pool.*/
  {
    UrosRpcResponse res;
    urosRpcResponseObjectInit(&res);
    urosRpcCallGetPid(
      &urosNode.config.xmlrpcAddr,
      &urosNode.config.nodeName,
      &res
    );
    urosRpcResponseClean(&res);
  }

  /* Send a dummy message to /rosout, to unlock TCPROS listener and pool.*/
  {
    UrosString shdnstr;
    shdnstr = urosStringAssignZ("\nNode is shutting down\n");
    rosout_debug(&shdnstr, UROS_TRUE);
  }

  /* Join the turtle brain thread pool.*/
  {
    uros_err_t err; (void)err;
    err = urosThreadPoolJoinAll(&turtlesThreadPool);
    urosAssert(err == UROS_OK);
  }

  return UROS_OK;
}

/**
 * @brief   Registers all the published topics to the Master node.
 * @note    Should be called at node initialization.
 */
void urosUserPublishTopics(void) {

  urosTcpRosPublishTopics();
}

/**
 * @brief   Unregisters all the published topics to the Master node.
 * @note    Should be called at node shutdown.
 */
void urosUserUnpublishTopics(void) {

  urosTcpRosUnpublishTopics();
}

/**
 * @brief   Registers all the subscribed topics to the Master node.
 * @note    Should be called at node initialization.
 */
void urosUserSubscribeTopics(void) {

  urosTcpRosSubscribeTopics();
}

/**
 * @brief   Unregisters all the subscribed topics to the Master node.
 * @note    Should be called at node shutdown.
 */
void urosUserUnsubscribeTopics(void) {

  urosTcpRosUnsubscribeTopics();
}

/**
 * @brief   Registers all the published services to the Master node.
 * @note    Should be called at node initialization.
 */
void urosUserPublishServices(void) {

  urosTcpRosPublishServices();
}

/**
 * @brief   Unregisters all the published services to the Master node.
 * @note    Should be called at node shutdown.
 */
void urosUserUnpublishServices(void) {

  urosTcpRosUnpublishServices();
}

/**
 * @brief   Registers all the subscribed parameters to the Master node.
 * @note    Should be called at node initialization.
 */
void urosUserSubscribeParams(void) {

  static const UrosString *const namep = &urosNode.config.nodeName;

  char *paramp = urosAlloc(namep->length + 14);
  urosAssert(paramp != NULL);
  memcpy(paramp, namep->datap, namep->length);
  memcpy(paramp + namep->length, "/background_X\0", 14);

  /* Subscribe to background color components.*/
  paramp[namep->length + 12] = 'r';
  urosNodeSubscribeParamSZ(paramp);
  paramp[namep->length + 12] = 'g';
  urosNodeSubscribeParamSZ(paramp);
  paramp[namep->length + 12] = 'b';
  urosNodeSubscribeParamSZ(paramp);

  urosFree(paramp);
}

/**
 * @brief   Unregisters all the subscribed parameters to the Master node.
 * @note    Should be called at node shutdown.
 */
void urosUserUnsubscribeParams(void) {

  static const UrosString *const namep = &urosNode.config.nodeName;

  char *paramp = urosAlloc(namep->length + 14);
  urosAssert(paramp != NULL);
  memcpy(paramp, namep->datap, namep->length);
  memcpy(paramp + namep->length, "/background_X\0", 14);

  /* Unsubscribe from background color components.*/
  paramp[namep->length + 12] = 'r';
  urosNodeUnsubscribeParamSZ(paramp);
  paramp[namep->length + 12] = 'g';
  urosNodeUnsubscribeParamSZ(paramp);
  paramp[namep->length + 12] = 'b';
  urosNodeUnsubscribeParamSZ(paramp);

  urosFree(paramp);
}

/**
 * @brief   Updates a subscribed ROS parameter locally.
 * @details This callback function notifies the user that the value of a
 *          subscribed ROS parameter has changed.
 *
 * @param[in] keyp
 *          Pointer to the parameter name string.
 * @param[in] paramp
 *          Pointer to the parameter value.
 * @return
 *          Error code.
 */
uros_err_t urosUserParamUpdate(const UrosString *keyp,
                               const UrosRpcParam *paramp) {

  urosAssert(urosStringNotEmpty(keyp));
  urosAssert(paramp != NULL);

  /* TODO: Handle the new parameter value.*/
  (void)keyp;
  (void)paramp;
  return UROS_OK;
}

/** @} */
