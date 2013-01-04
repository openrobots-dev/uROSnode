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
 * @file    urosTcpRos.h
 * @author  Andrea Zoppi <texzk@email.it>
 *
 * @brief   TCPROS features of the middleware.
 */

#ifndef _UROSTCP_H_
#define _UROSTCP_H_

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include "urosBase.h"
#include "urosConn.h"
#include "urosThreading.h"

/*===========================================================================*/
/* TYPES & MACROS                                                            */
/*===========================================================================*/

/** @addtogroup tcpros_types */
/** @{ */

/**
 * @brief   TCPROS Client thread creation arguments.
 */
typedef struct uros_tcpcliargs_t {
  UrosString    topicName;          /**< @brief Topic or service name.*/
  UrosAddr      remoteAddr;         /**< @brief Remote connection address.*/
} uros_tcpcliargs_t;

/**
 * @brief   TCPROS connection status object.
 */
typedef struct UrosTcpRosStatus {
  UrosConn          *csp;           /**< @brief Connection status pointer.*/
  UrosString        callerId;       /**< @brief Caller ID.*/
  UrosTopic         *topicp;        /**< @brief Referenced topic/service.*/
  uros_err_t        err;            /**< @brief Last error code.*/
  uros_topicflags_t flags;          /**< @brief Topic/Service flags.*/
  uros_bool_t       threadExit;     /**< @brief Thread exit request.*/
  UrosMutex         threadExitMtx;  /**< @brief Thread exit request mutex.*/
  UrosString        errstr;         /**< @brief Error string.*/
} UrosTcpRosStatus;

/**
 * @brief   TCPROS variable array descriptor.
 */
typedef struct UrosTcpRosArray {
  uint32_t          length;         /**< @brief Number of entries.*/
  void              *entriesp;      /**< @brief Pointer to the entries chunk.*/
} UrosTcpRosArray;

/** @} */

/** @addtogroup tcpros_macros */
/** @{ */

/**
 * @brief   Reads a raw value.
 *
 * @param[in,out] tcpstp
 *          Pointer to a TCPROS status with a working connection.
 * @param[in] value
 *          Value to be read. It must be addressable by a pointer.
 * @return
 *          Error code.
 */
#define urosTcpRosRecvRaw(tcpstp, value) \
  urosTcpRosRecv((tcpstp), &(value), sizeof(value))

/**
 * @brief   Writes a raw value.
 *
 * @param[in,out] tcpstp
 *          Pointer to a TCPROS status with a working connection.
 * @param[in] value
 *          Value to be written. It must be addressable by a pointer.
 * @return
 *          Error code.
 */
#define urosTcpRosSendRaw(tcpstp, value) \
  urosTcpRosSend((tcpstp), &(value), sizeof(value))

/**
 * @brief   Generate a variable-length array structure.
 * @details Generates a structure compatible with @p UrosTcpRosArray, where the
 *          entries pointer is correctly typed.
 *
 * @param[in] type
 *          Type of an array entry object. To be valid, a valid pointer
 *          type must be obtained by appending a @p * to @p type.
 */
#define UROS_VARARR(type) \
  struct { uint32_t length; type *entriesp; }

/**
 * @brief   Indexes an array item at the provided index.
 *
 * @param[in,out] array
 *          Pointer to an initialized @p UrosTcpRosArray object.
 * @param[in] type
 *          Type of an array entry object. To be valid, a valid pointer
 *          type must be obtained by appending a @p * to @p type.
 * @param[in] index
 *          Index of the involved array entry.
 * @return
 *          The object value at position @p index.
 */
#define urosTcpRosArrayAt(array, type, index) \
  ((type *)(array))[(index)]

/** @} */

/*===========================================================================*/
/* GLOBAL PROTOTYPES                                                         */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

void urosTopicSubParamsDelete(uros_tcpcliargs_t *parp);

void urosTpcRosStatusObjectInit(UrosTcpRosStatus *tcpstp, UrosConn *csp);
void urosTpcRosStatusClean(UrosTcpRosStatus *tcpstp, uros_bool_t deep);
void urosTpcRosStatusDelete(UrosTcpRosStatus *tcpstp, uros_bool_t deep);
void urosTcpRosStatusIssueExit(UrosTcpRosStatus *tcpstp);
uros_bool_t urosTcpRosStatusCheckExit(UrosTcpRosStatus *tcpstp);

void urosTcpRosArrayObjectInit(UrosTcpRosArray *arrayp);
void urosTcpRosArrayClean(UrosTcpRosArray *arrayp);
void urosTcpRosArrayDelete(UrosTcpRosArray *arrayp, uros_bool_t deep);

uros_err_t urosTcpRosSkip(UrosTcpRosStatus *tcpstp, size_t length);
uros_err_t urosTcpRosExpect(UrosTcpRosStatus *tcpstp, void *tokp, size_t toklen);
uros_err_t urosTcpRosRecv(UrosTcpRosStatus *tcpstp,
                          void *bufp, size_t buflen);
uros_err_t urosTcpRosRecvString(UrosTcpRosStatus *tcpstp,
                                UrosString *strp);
uros_err_t urosTcpRosSend(UrosTcpRosStatus *tcpstp,
                          const void *bufp, size_t buflen);
uros_err_t urosTcpRosSendString(UrosTcpRosStatus *tcpstp,
                                const UrosString *strp);
uros_err_t urosTcpRosSendStringSZ(UrosTcpRosStatus *tcpstp,
                                  const char *strp);
uros_err_t urosTcpRosSendError(UrosTcpRosStatus *tcpstp);
uros_err_t urosTcpRosSentMsgLength(UrosTcpRosStatus *tcpstp,
                                   size_t length);

uros_err_t urosTcpRosListenerThread(void *data);
uros_err_t urosTcpRosServerThread(UrosConn *csp);
uros_err_t urosTcpRosClientThread(uros_tcpcliargs_t *argsp);
void urosTcpRosTopicSubscriberDone(UrosTcpRosStatus *tcpstp);
void urosTcpRosTopicPublisherDone(UrosTcpRosStatus *tcpstp);
void urosTcpRosServiceDone(UrosTcpRosStatus *tcpstp);

#ifdef __cplusplus
}
#endif
#endif /* _UROSTCP_H_ */
