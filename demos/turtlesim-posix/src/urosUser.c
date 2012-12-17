/*
Copyright (c) 2012, Politecnico di Milano. All rights reserved.

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
#include "urosTcpRosTypes.h"

#include <stdarg.h>

/*===========================================================================*/
/* LOCAL VARIABLES                                                           */
/*===========================================================================*/

/** @brief XMLRPC Listener thread stack.*/
static uint8_t xmlrpcListenerStack[UROS_XMLRPC_LISTENER_STKSIZE];

/** @brief TCPROS Listener thread stack.*/
static uint8_t tcprosListenerStack[UROS_TCPROS_LISTENER_STKSIZE];

/** @brief XMLRPC Slave server worker thread stacks.*/
static uint8_t slaveMemPoolChunk[UROS_XMLRPC_SLAVE_POOLSIZE]
                                [sizeof(void*) + UROS_XMLRPC_SLAVE_STKSIZE];

/** @brief TCPROS Client worker thread stacks.*/
static uint8_t tcpcliMemPoolChunk[UROS_TCPROS_CLIENT_POOLSIZE]
                                 [sizeof(void*) + UROS_TCPROS_CLIENT_STKSIZE];

/** @brief TCPROS Server worker thread stacks.*/
static uint8_t tcpsvrMemPoolChunk[UROS_TCPROS_SERVER_POOLSIZE]
                                 [sizeof(void*) + UROS_TCPROS_SERVER_STKSIZE];

/** @brief XMLRPC Listener thread id.*/
static UrosThreadId xmlrpcListenerId;

/** @brief TCPROS Listener thread id.*/
static UrosThreadId tcprosListenerId;

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
 * @brief   Initializes and allocates memory pools.
 * @details Function called at boot time, with platform-dependent stack size
 *          and alignment declarations.
 *
 * @pre     The memory pools of @p np have not been initialized yet.
 *
 * @param[in,out] np
 *          Pointer to an @p UrosNode object being initialized.
 */
void urosUserAllocMemPools(UrosNode *np) {

  UrosNodeStatus *stp = &np->status;

  /* Initialize mempools with their description.*/
  urosMemPoolObjectInit(&stp->tcpcliMemPool,
                        sizeof(void*) + UROS_TCPROS_CLIENT_STKSIZE, NULL);
  urosMemPoolObjectInit(&stp->tcpsvrMemPool,
                        sizeof(void*) + UROS_TCPROS_SERVER_STKSIZE, NULL);
  urosMemPoolObjectInit(&stp->slaveMemPool,
                        sizeof(void*) + UROS_XMLRPC_SLAVE_STKSIZE, NULL);

  /* Load the actual memory chunks for worker thread stacks.*/
  urosMemPoolLoadArray(&stp->tcpcliMemPool, tcpcliMemPoolChunk,
                       UROS_TCPROS_CLIENT_POOLSIZE);

  urosMemPoolLoadArray(&stp->tcpsvrMemPool, tcpsvrMemPoolChunk,
                       UROS_TCPROS_SERVER_POOLSIZE);

  urosMemPoolLoadArray(&stp->slaveMemPool, slaveMemPoolChunk,
                       UROS_XMLRPC_SLAVE_POOLSIZE);
}

/**
 * @brief   Registers static message and service types.
 * @details This callback function is called at boot time to initialize the
 *          set of message types recognized by the system.
 *
 * @pre     The global static message type set has not been initialized yet.
 */
void urosUserRegisterStaticTypes(void) {

  /* Register types used by TCPROS topics and services.*/
  urosTcpRosRegStaticTypes();
}

/**
 * @brief   Creates the listener threads.
 * @details This callback function is called at boot time to create the
 *          listener threads of the middleware.
 *
 * @pre     The listener threads have not been created before.
 */
void urosUserCreateListeners(void) {

  uros_err_t err;
  (void)err;

  err = urosThreadCreateStatic(&xmlrpcListenerId, "RpcSlaveListener",
                               UROS_XMLRPC_LISTENER_PRIO,
                               (uros_proc_f)urosRpcSlaveListenerThread, NULL,
                               xmlrpcListenerStack, UROS_XMLRPC_LISTENER_STKSIZE);
  urosAssert(err == UROS_OK);

  err = urosThreadCreateStatic(&tcprosListenerId, "TcpRosListener",
                               UROS_TCPROS_LISTENER_PRIO,
                               (uros_proc_f)urosTcpRosListenerThread, NULL,
                               tcprosListenerStack, UROS_TCPROS_LISTENER_STKSIZE);
  urosAssert(err == UROS_OK);
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

  /* Handle the new parameter value.*/
  printf("<paramUpdate: [%.*s] = [", UROS_STRARG(keyp));
  switch (paramp->class) {
  case UROS_RPCP_INT: {
    printf("%ld", (long)paramp->value.int32); break;
  }
  case UROS_RPCP_BOOLEAN: {
    printf(paramp->value.boolean ? "true" : "false"); break;
  }
  case UROS_RPCP_STRING: {
    printf("%.*s", UROS_STRARG(&paramp->value.string)); break;
  }
  case UROS_RPCP_DOUBLE: {
    printf("%f", paramp->value.real); break;
  }
  case UROS_RPCP_BASE64: {
    printf("(base64)"); break; /* TODO */
  }
  case UROS_RPCP_STRUCT: {
    printf("(struct)"); break; /* TODO */
  }
  case UROS_RPCP_ARRAY: {
    printf("(array)"); break; /* TODO */
  }
  default: {
    printf("(UNKNOWN)"); break;
  }
  }
  printf("]>\n");
  return UROS_OK;
}

/** @} */
