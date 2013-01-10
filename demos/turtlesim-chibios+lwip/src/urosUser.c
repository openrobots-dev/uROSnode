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

#include <urosUser.h>
#include <urosRpcSlave.h>
#include <urosTcpRos.h>
#include <stdarg.h>

#include "urosHandlers.h"
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
  /* TODO */
  va_end(args);
}

/**
 * @brief   Loads node configuration.
 * @details Any previously allocated data is freed, then the configuration is
 *          loaded from a static non-volatile memory chunk.
 * @see     uros_lld_nodeconfig_load()
 *
 * @pre     The related @p UrosNode is initialized.
 *
 * @param[in,out] cfgp
 *          Pointer to the target configuration descriptor.
 */
void urosUserNodeConfigLoad(UrosNodeConfig *cfgp) {

  urosAssert(cfgp != NULL);

  /* Clean any allocated variables.*/
  urosStringClean(&cfgp->nodeName);
  urosStringClean(&cfgp->xmlrpcUri);
  urosStringClean(&cfgp->tcprosUri);
  urosStringClean(&cfgp->masterUri);

  /* TODO: Load from EEPROM.*/
  urosNodeConfigLoadDefaults(cfgp);
}

/**
 * @brief   Saves the node configuration.
 * @details The node configuration is saved to a static non-volatile memory
 *          chunk.
 * @see     uros_lld_nodeconfig_save()
 *
 * @pre     The related @p UrosNode is initialized.
 *
 * @param[in] cfgp
 *          Pointer to the configuration descriptor to be saved.
 */
void urosUserNodeConfigSave(const UrosNodeConfig *cfgp) {

  /* TODO: Save to EEPROM.*/
  (void)cfgp;
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
 * @brief   Registers static message types.
 * @details This callback function is called at boot time to initialize the
 *          set of message types recognized by the system.
 *
 * @pre     The global static message type set has not been initialized yet.
 */
void urosUserRegisterStaticTypes(void) {

  urosMsgTypesRegStaticTypes();
}

/**
 * @brief   Registers all the published topics to the Master node.
 * @note    Should be called at node initialization.
 *
 * @return  Error code.
 */
uros_err_t urosUserPublishTopics(void) {

  urosHandlersPublishTopics();
  return UROS_OK;
}

/**
 * @brief   Unregisters all the published topics to the Master node.
 * @note    Should be called at node shutdown.
 *
 * @return  Error code.
 */
uros_err_t urosUserUnpublishTopics(void) {

  urosHandlersUnpublishTopics();
  return UROS_OK;
}

/**
 * @brief   Registers all the subscribed topics to the Master node.
 * @note    Should be called at node initialization.
 *
 * @return  Error code.
 */
uros_err_t urosUserSubscribeTopics(void) {

  urosHandlersSubscribeTopics();
  return UROS_OK;
}

/**
 * @brief   Unregisters all the subscribed topics to the Master node.
 * @note    Should be called at node shutdown.
 *
 * @return  Error code.
 */
uros_err_t urosUserUnsubscribeTopics(void) {

  urosHandlersUnsubscribeTopics();
  return UROS_OK;
}

/**
 * @brief   Registers all the published services to the Master node.
 * @note    Should be called at node initialization.
 *
 * @return  Error code.
 */
uros_err_t urosUserPublishServices(void) {

  urosHandlersPublishServices();
  return UROS_OK;
}

/**
 * @brief   Unregisters all the published services to the Master node.
 * @note    Should be called at node shutdown.
 *
 * @return  Error code.
 */
uros_err_t urosUserUnpublishServices(void) {

  urosHandlersUnpublishServices();
  return UROS_OK;
}

/**
 * @brief   Registers all the subscribed parameters to the Master node.
 * @note    Should be called at node initialization.
 *
 * @return  Error code.
 */
uros_err_t urosUserSubscribeParams(void) {

  static const UrosNodeConfig *const cfgp = &urosNode.config;

  UrosRpcParam paramval;
  UrosRpcResponse res;

  urosRpcResponseObjectInit(&res);
  urosRpcParamObjectInit(&paramval, UROS_RPCP_INT);

  /* Red component set and subscribed to.*/
  paramval.value.int32 = 123;
  urosRpcCallSetParam(&cfgp->masterAddr, &cfgp->nodeName,
                      &backcolparnameR, &paramval, &res);
  urosRpcResponseClean(&res);
  urosNodeSubscribeParam(&backcolparnameR);

  /* Green component set and subscribed to.*/
  urosRpcResponseObjectInit(&res);
  paramval.value.int32 = 132;
  urosRpcCallSetParam(&cfgp->masterAddr, &cfgp->nodeName,
                      &backcolparnameG, &paramval, &res);
  urosRpcResponseClean(&res);
  urosNodeSubscribeParam(&backcolparnameG);

  /* Blue component set and subscribed to.*/
  urosRpcResponseObjectInit(&res);
  paramval.value.int32 = 213;
  urosRpcCallSetParam(&cfgp->masterAddr, &cfgp->nodeName,
                      &backcolparnameB, &paramval, &res);
  urosRpcResponseClean(&res);
  urosNodeSubscribeParam(&backcolparnameB);

  urosRpcParamClean(&paramval, UROS_TRUE);
  return UROS_OK;
}

/**
 * @brief   Unregisters all the subscribed parameters to the Master node.
 * @note    Should be called at node shutdown.
 *
 * @return  Error code.
 */
uros_err_t urosUserUnsubscribeParams(void) {

  static const UrosNodeConfig *const cfgp = &urosNode.config;

  UrosRpcResponse res;

  urosRpcResponseObjectInit(&res);

  /* Red component unsubscribed from and deleted.*/
  urosNodeUnsubscribeParam(&backcolparnameR);
  urosRpcCallDeleteParam(&cfgp->masterAddr, &cfgp->nodeName,
                         &backcolparnameR, &res);
  urosRpcResponseClean(&res);

  /* Green component unsubscribed from and deleted.*/
  urosNodeUnsubscribeParam(&backcolparnameG);
  urosRpcCallDeleteParam(&cfgp->masterAddr, &cfgp->nodeName,
                         &backcolparnameG, &res);
  urosRpcResponseClean(&res);

  /* Blue component unsubscribed from and deleted.*/
  urosNodeUnsubscribeParam(&backcolparnameB);
  urosRpcCallDeleteParam(&cfgp->masterAddr, &cfgp->nodeName,
                         &backcolparnameB, &res);
  urosRpcResponseClean(&res);

  return UROS_OK;
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

  /* Check if [<node>/background_r].*/
  if (0 == urosStringCmp(keyp, &backcolparnameR)) {
    urosError(paramp->class != UROS_RPCP_INT, return UROS_ERR_BADPARAM,
              ("Parameter [%.*s] class id %d, expected %d (UROS_RPCP_INT)\n",
               UROS_STRARG(keyp), (int)paramp->class, (int)UROS_RPCP_INT));
    urosError(paramp->value.int32 < 0 || paramp->value.int32 > 255,
              return UROS_ERR_BADPARAM,
              ("Parameter [%.*s] value %d outside [0..255]\n",
               UROS_STRARG(keyp), paramp->value.int32));
    urosMutexLock(&backgroundColorLock);
    backgroundColor.r = (uint8_t)paramp->value.int32;
    urosMutexUnlock(&backgroundColorLock);
    return UROS_OK;
  }

  /* Check if [<node>/background_g].*/
  if (0 == urosStringCmp(keyp, &backcolparnameG)) {
    urosError(paramp->class != UROS_RPCP_INT, return UROS_ERR_BADPARAM,
              ("Parameter [%.*s] class id %d, expected %d (UROS_RPCP_INT)\n",
               UROS_STRARG(keyp), (int)paramp->class, (int)UROS_RPCP_INT));
    urosError(paramp->value.int32 < 0 || paramp->value.int32 > 255,
              return UROS_ERR_BADPARAM,
              ("Parameter [%.*s] value %d outside [0..255]\n",
               UROS_STRARG(keyp), paramp->value.int32));
    urosMutexLock(&backgroundColorLock);
    backgroundColor.g = (uint8_t)paramp->value.int32;
    urosMutexUnlock(&backgroundColorLock);
    return UROS_OK;
  }

  /* Check if [<node>/background_b].*/
  if (0 == urosStringCmp(keyp, &backcolparnameB)) {
    urosError(paramp->class != UROS_RPCP_INT, return UROS_ERR_BADPARAM,
              ("Parameter [%.*s] class id %d, expected %d (UROS_RPCP_INT)\n",
               UROS_STRARG(keyp), (int)paramp->class, (int)UROS_RPCP_INT));
    urosError(paramp->value.int32 < 0 || paramp->value.int32 > 255,
              return UROS_ERR_BADPARAM,
              ("Parameter [%.*s] value %d outside [0..255]\n",
               UROS_STRARG(keyp), paramp->value.int32));
    urosMutexLock(&backgroundColorLock);
    backgroundColor.b = (uint8_t)paramp->value.int32;
    urosMutexUnlock(&backgroundColorLock);
    return UROS_OK;
  }

  /* Unknown parameter name.*/
  return UROS_ERR_BADPARAM;
}

/** @} */
