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
 * @brief   User-defined callback functions (TEMPLATE).
 */

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include <urosUser.h>

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

  urosAssert(formatp != NULL);

  /* TODO: Print a formatted error message.*/
  (void)formatp;
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

  urosAssert(msgp != NULL);

  /* TODO: Handle the shutdown() call and message.*/
  (void)msgp;
  return UROS_OK;
}

/**
 * @brief   Registers static message types.
 * @details This callback function is called at boot time to initialize the
 *          set of message types recognized by the system.
 *
 * @pre     The global static message type set has not been initialized yet.
 */
void urosUserRegisterStaticMsgTypes(void) {

  /* TODO: Register static message types.*/
}

/**
 * @brief   Registers all the published topics to the Master node.
 * @note    Should be called at node initialization.
 */
void urosUserPublishTopics(void) {

  /* TODO: Publish topics.*/
}

/**
 * @brief   Unregisters all the published topics to the Master node.
 * @note    Should be called at node shutdown.
 */
void urosUserUnpublishTopics(void) {

  /* TODO: Unpublish topics.*/
}

/**
 * @brief   Registers all the subscribed topics to the Master node.
 * @note    Should be called at node initialization.
 */
void urosUserSubscribeTopics(void) {

  /* TODO: Subscribe to topics.*/
}

/**
 * @brief   Unregisters all the subscribed topics to the Master node.
 * @note    Should be called at node shutdown.
 */
void urosUserUnsubscribeTopics(void) {

  /* TODO: Unsubscribe from topics.*/
}

/**
 * @brief   Registers all the published services to the Master node.
 * @note    Should be called at node initialization.
 */
void urosUserPublishServices(void) {

  /* TODO: Publish services.*/
}

/**
 * @brief   Unregisters all the published services to the Master node.
 * @note    Should be called at node shutdown.
 */
void urosUserUnpublishServices(void) {

  /* TODO: Unpublish services.*/
}

/**
 * @brief   Registers all the subscribed parameters to the Master node.
 * @note    Should be called at node initialization.
 */
void urosUserSubscribeParams(void) {

  /* TODO: Subscribe to parameters.*/
}

/**
 * @brief   Unregisters all the subscribed parameters to the Master node.
 * @note    Should be called at node shutdown.
 */
void urosUserUnsubscribeParams(void) {

  /* TODO: Unsubscribe from parameters.*/
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
