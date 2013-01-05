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
 * @file    urosconf.h
 * @author  Andrea Zoppi <texzk@email.it>
 *
 * @brief   User definitions for middleware configuration.
 */

#ifndef _UROSCONF_H_
#define _UROSCONF_H_

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include <stddef.h>
#include <stdint.h>

#include <ch.h>
#include <chprintf.h>

/*===========================================================================*/
/* NODE CONFIGURATION                                                        */
/*===========================================================================*/

/** @addtogroup node_config */
/** @{ */

/** @name Configuration */
/** @{ */

/** @brief Default ROS node name, C string.*/
#define UROS_NODE_NAME                      "/turtlesim"

/** @brief Node thread priorty.*/
#define UROS_NODE_THREAD_PRIO               LOWPRIO

/** @brief Node thread stack size.*/
#define UROS_NODE_THREAD_STKSIZE            2048

/** @} */
/** @} */

/*===========================================================================*/
/* XMLRPC CONFIGURATION                                                      */
/*===========================================================================*/

/** @addtogroup rpc_config */
/** @{ */

/*~~~ MASTER CONFIGURATION ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name XMLRPC Master configuration */
/** @{ */

/** @brief Default Master server IP address, little-endian dword.*/
#define UROS_XMLRPC_MASTER_IP               urosIpDword(192, 168, 1, 103)

/** @brief Default Master server IP address, C string.*/
#define UROS_XMLRPC_MASTER_IP_SZ            "192.168.1.103"

/** @brief Default Master server port.*/
#define UROS_XMLRPC_MASTER_PORT             11311

/** @} */

/*~~~ LISTENER CONFIGURATION ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name XMLRPC listener configuration */
/** @{ */

/** @brief Default XMLRPC listener IP address, little-endian dword.*/
#define UROS_XMLRPC_LISTENER_IP             urosIpDword(192, 168, 1, 88)

/** @brief Default XMLRPC listener IP address, C string.*/
#define UROS_XMLRPC_LISTENER_IP_SZ          "192.168.1.88"

/** @brief Default XMLRPC listener port.*/
#define UROS_XMLRPC_LISTENER_PORT           33333

/** @brief Maximum concurrent connections for XMLRPC Slave API.*/
#define UROS_XMLRPC_LISTENER_BACKLOG        4

/** @brief XMLRPC listener thread priority.*/
#define UROS_XMLRPC_LISTENER_PRIO           (LOWPRIO + 1)

/** @brief XMLRPC listener thread stack size.*/
#define UROS_XMLRPC_LISTENER_STKSIZE        1024

/** @} */

/*~~~ SLAVE CONFIGURATION ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name XMLRPC Slave configuration */
/** @{ */

/** @brief XMLRPC Slave server thread pool size.*/
#define UROS_XMLRPC_SLAVE_POOLSIZE          4

/** @brief XMLRPC Slave server thread priority.*/
#define UROS_XMLRPC_SLAVE_PRIO              (LOWPRIO + 3)

/** @brief XMLRPC Slave server thread stack size.*/
#define UROS_XMLRPC_SLAVE_STKSIZE           1024

/** @} */

/*~~~ OTHER OPTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name XMLRPC timeouts configuration */
/** @{ */

/** @brief Default timeout for incoming XMLRPC transactions, in milliseconds.*/
#define UROS_XMLRPC_RECVTIMEOUT             3000

/** @brief Default timeout for outgoing XMLRPC transactions, in milliseconds.*/
#define UROS_XMLRPC_SENDTIMEOUT             3000

/** @} */
/** @} */

/*===========================================================================*/
/* TCPROS CONFIGURATION                                                      */
/*===========================================================================*/

/** @addtogroup tcpros_config */
/** @{ */

/*~~~ LISTENER CONFIGURATION ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name TCPROS listener configuration */
/** @{ */

/** @brief Default TCPROS listener IP address, little-endian dword.*/
#define UROS_TCPROS_LISTENER_IP             UROS_XMLRPC_LISTENER_IP

/** @brief Default TCPROS listener IP address, C string.*/
#define UROS_TCPROS_LISTENER_IP_SZ          UROS_XMLRPC_LISTENER_IP_SZ

/** @brief Default TCPROS listener port.*/
#define UROS_TCPROS_LISTENER_PORT           44444

/** @brief Maximum number of partially set up TCPROS connections.*/
#define UROS_TCPROS_LISTENER_BACKLOG        4

/** @brief TCPROS listener thread priority.*/
#define UROS_TCPROS_LISTENER_PRIO           (LOWPRIO + 4)

/** @brief TCPROS listener thread stack size.*/
#define UROS_TCPROS_LISTENER_STKSIZE        1024

/** @} */

/*~~~ CLIENT CONFIGURATION ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name TCPROS Client configuration */
/** @{ */

/** @brief TCPROS Client thread pool size.*/
#define UROS_TCPROS_CLIENT_POOLSIZE         4

/** @brief TCPROS Client thread priority.*/
#define UROS_TCPROS_CLIENT_PRIO             (NORMALPRIO + 1)

/** @brief TCPROS Client thread stack size.*/
#define UROS_TCPROS_CLIENT_STKSIZE          1024

/** @} */

/*~~~ SERVER CONFIGURATION ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name TCPROS Server configuration */
/** @{ */

/** @brief TCPROS Server thread pool size.*/
#define UROS_TCPROS_SERVER_POOLSIZE         4

/** @brief TCPROS Server thread priority.*/
#define UROS_TCPROS_SERVER_PRIO             (NORMALPRIO + 2)

/** @brief TCPROS server thread stack size.*/
#define UROS_TCPROS_SERVER_STKSIZE          1024

/** @} */

/*~~~ MISC OPTIONS `~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name TCPROS misc options */
/** @{ */

/** @brief Reads the message definition, instead of skipping it.*/
#define UROS_TCPROS_USE_MSGDEF              0

/** @} */

/** @name TCPROS timeouts configuration */
/** @{ */

/** @brief Default timeout for incoming TCPROS transactions, in milliseconds.*/
#define UROS_TCPROS_RECVTIMEOUT             500

/** @brief Default timeout for outgoing TCPROS transactions, in milliseconds.*/
#define UROS_TCPROS_SENDTIMEOUT             500

/** @} */
/** @} */

/*===========================================================================*/
/* INTERNAL MODULES CONFIGURATION                                            */
/*===========================================================================*/

/** @addtogroup rpc_config */
/** @{ */

/*~~~ XMLRPC PARSER OPTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name XMLRPC parser configuration */
/** @{ */

/** @brief Default length of the reading buffer.*/
#define UROS_RPCPARSER_RDBUFLEN             128

/** @brief Reads the status message, instead of skipping it.*/
#define UROS_RPCPARSER_USE_STATMSG          0

/** @} */

/*~~~ XMLRPC STREAMER OPTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name XMLRPC streamer configuration */
/** @{ */

/** @brief Fixed Content-Length, when the message spans more packets.*/
#define UROS_RPCSTREAMER_FIXLEN             4000

/** @} */
/** @} */

/*~~~ MISC OPTIONS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~`*/

/** @addtogroup conn_config */
/** @{ */

/** @name Connectivity configuration */
/** @{ */

/** @brief Size of a MTU.*/
#define UROS_MTU_SIZE                       1500

/** @} */
/** @} */

/*===========================================================================*/
/* FEATURE FLAGS                                                             */
/*===========================================================================*/

/** @addtogroup base_config */
/** @{ */

/*~~~ GLOBAL SWITCHES ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Global switches */
/** @{ */

/** @brief Uses the built-in memory pool.*/
#define UROS_USE_BUILTIN_MEMPOOL            0

/** @brief Enables assertion evaluations.*/
#define UROS_USE_ASSERT                     1

/** @brief Enables error messages.*/
#define UROS_USE_ERROR_MSG                  1

/** @} */

/*~~~ PER-FILE ASSERTION SWITCHES ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Assertion switches */
/** @{ */

/** @brief Enables assertions for <tt>urosBase.c</tt>.*/
#define UROS_BASE_C_USE_ASSERT              1

/** @brief Enables assertions for <tt>urosConn.c</tt>.*/
#define UROS_CONN_C_USE_ASSERT              1

/** @brief Enables assertions for <tt>urosNode.c</tt>.*/
#define UROS_NODE_C_USE_ASSERT              1

/** @brief Enables assertions for <tt>urosRpcCall.c</tt>.*/
#define UROS_RPC_CALL_C_USE_ASSERT          1

/** @brief Enables assertions for <tt>urosRpcParser.c</tt>.*/
#define UROS_RPCPARSER_C_USE_ASSERT         1

/** @brief Enables assertions for <tt>urosRpcSlave.c</tt>.*/
#define UROS_RPCSLAVE_C_USE_ASSERT          1

/** @brief Enables assertions for <tt>urosRpcStreamer.c</tt>.*/
#define UROS_RPCSTREAMER_C_USE_ASSERT       1

/** @brief Enables assertions for <tt>urosTcpRos.c</tt>.*/
#define UROS_TCPROS_C_USE_ASSERT            1

/** @brief Enables assertions for <tt>urosThreading.c</tt>.*/
#define UROS_THREADING_C_USE_ASSERT         1

/** @} */

/*~~~ PER-FILE ERROR MESSAGE SWITCHES ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Error message switches */
/** @{ */

/** @brief Enables error messages for <tt>urosBase.c</tt>.*/
#define UROS_BASE_C_USE_ERROR_MSG           1

/** @brief Enables error messages for <tt>urosConn.c</tt>.*/
#define UROS_CONN_C_USE_ERROR_MSG           1

/** @brief Enables error messages for <tt>urosNode.c</tt>.*/
#define UROS_NODE_C_USE_ERROR_MSG           1

/** @brief Enables error messages for <tt>urosRpcCall.c</tt>.*/
#define UROS_RPC_CALL_C_USE_ERROR_MSG       1

/** @brief Enables error messages for <tt>urosRpcParser.c</tt>.*/
#define UROS_RPCPARSER_C_USE_ERROR_MSG      1

/** @brief Enables error messages for <tt>urosRpcSlave.c</tt>.*/
#define UROS_RPCSLAVE_C_USE_ERROR_MSG       1

/** @brief Enables error messages for <tt>urosRpcStreamer.c</tt>.*/
#define UROS_RPCSTREAMER_C_USE_ERROR_MSG    1

/** @brief Enables error messages for <tt>urosTcpRos.c</tt>.*/
#define UROS_TCPROS_C_USE_ERROR_MSG         1

/** @brief Enables error messages for <tt>urosThreading.c</tt>.*/
#define UROS_THREADING_C_USE_ERROR_MSG      1

/** @} */
/** @} */

/*===========================================================================*/
/* USER MACROS                                                               */
/*===========================================================================*/

#if !defined(__DOXYGEN__)
/* Define your own macros below.*/

#define UROS_STACK_BLKSIZE(stksize) \
  THD_WA_SIZE(stksize)

#define UROS_STACK(varname, stksize) \
  WORKING_AREA((varname), (size_t)(stksize))

#define UROS_STACKPOOL_BLKSIZE(stksize) \
  THD_WA_SIZE(sizeof(void*) + (size_t)(stksize))

#define UROS_STACKPOOL(arrname, stksize, numstacks) \
  stkalign_t arrname[(numstacks)] \
                    [UROS_STACKPOOL_BLKSIZE(stksize) / sizeof(stkalign_t)]

#define urosAssert(expr) \
  chDbgCheck((expr), "Assert "__FILE__":"UROS_STRINGIFY2(__LINE__)" failed ("#expr")")

/* msgargs as ("format", ...) */
#define urosError(when, action, msgargs) \
  { if (when) { \
      urosUserErrPrintf("Error at %s:%d\n" \
                        "  function: %s\n" \
                        "  reason:   %s\n" \
                        "  message:  ", \
                        __FILE__, __LINE__, __PRETTY_FUNCTION__, #when); \
      urosUserErrPrintf msgargs ; \
      { action; } } }

#endif /* !defined(__DOXYGEN__) */

/*===========================================================================*/
/*  PLATFORM-DEPENDENT TYPES                                                 */
/*===========================================================================*/

/** @addtogroup base_types */
/** @{ */

/** @brief Error type, compatible with thread return type.*/
typedef msg_t           uros_err_t;

/** @brief Memory pool type, platform-dependent.*/
typedef MemoryPool      UrosMemPool;

/** @} */

/** @addtogroup threading_types */
/** @{ */

/** @brief Thread ID type, platform-dependent.*/
typedef Thread          *UrosThreadId;
/** @brief Invalid thread ID.*/
#define UROS_NULL_THREADID  (NULL)

/** @brief Thread priority type, platform-dependent.*/
typedef tprio_t         uros_prio_t;

/** @brief Semaphore type, platform-dependent.*/
typedef Semaphore       UrosSem;

/** @brief Mutex type, platform-dependent.*/
typedef Mutex           UrosMutex;

/** @brief Condvar type, platform-dependent.*/
typedef CondVar         UrosCondVar;

/** @} */

/** @addtogroup base_macros */
/** @{ */

/** @brief Platform-dependent variables for @p UrosConn.*/
#define UrosConn__LLD \
    int8_t            lwiperr;        /**< @brief Last Netconn error code. FIXME: Already stored by LWIP.*/ \
    struct netconn    *netconnp;      /**< @brief Netconn object pointer.*/ \
    struct netbuf     *recvnetbufp;   /**< @brief Current received buffer.*/ \
    size_t            recvchkoff;     /**< @brief Offset inside the current Netbuf chunk.*/

/** @} */

#endif /* _UROSCONF_H_ */
