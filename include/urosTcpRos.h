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

#ifndef _UROSTCPROS_H_
#define _UROSTCPROS_H_

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
  UrosString        topicName;      /**< @brief Topic name.*/
  uros_topicflags_t topicFlags;     /**< @brief Topic flags.*/
  UrosAddr          remoteAddr;     /**< @brief Remote connection address.*/
} uros_tcpcliargs_t;

/**
 * @brief   TCPROS connection status object.
 */
typedef struct UrosTcpRosStatus {
  uros_err_t        err;            /**< @brief Last error code.*/
  UrosConn          *csp;           /**< @brief Connection status pointer.*/
  UrosString        callerId;       /**< @brief Caller ID.*/
  UrosTopic         *topicp;        /**< @brief Referenced topic/service.*/
  uros_topicflags_t remoteFlags;    /**< @brief Remote topic/service flags.*/
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

/**
 * @brief   TCPROS Service call handler.
 *
 * @param[in,out] tcpstp
 *          Pointer to a TCPROS statuc with a working connection.
 * @param[out] resobjp
 *          Pointer to the allocated object being received.
 * @return
 *          Error code.
 */
typedef uros_err_t (*uros_tcpsrvcall_t)(UrosTcpRosStatus *tcpstp,
                                        void *resobjp);

/** @} */

/** @addtogroup tcpros_macros */
/** @{ */

/*~~~ TCPROS CONNECTION ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name TCPROS connection */
/** @{ */

/**
 * @brief   Reads a raw value.
 * @details The raw value is received from a little-endian fashion.
 * @warning On big endian architectures, be careful not to specify a @p value
 *          of complex (@e struct or @e union) type, because the @b whole value
 *          will be received in reverse order, not its primitive fields
 *          individually as expected.
 *
 * @param[in,out] tcpstp
 *          Pointer to a TCPROS status with a working connection.
 * @param[in] value
 *          Value to be read. It must be addressable by a pointer.
 * @return
 *          Error code.
 */
#if UROS_ENDIANNESS == 321
#define urosTcpRosRecvRaw(tcpstp, value) \
  urosTcpRosRecvRev((tcpstp), &(value), sizeof(value))
#else
#define urosTcpRosRecvRaw(tcpstp, value) \
  urosTcpRosRecv((tcpstp), &(value), sizeof(value))
#endif

/**
 * @brief   Writes a raw value.
 * @details The raw value is sent in a little-endian fashion.
 * @warning On big endian architectures, be careful not to specify a @p value
 *          of complex (@e struct or @e union) type, because the @b whole value
 *          will be sent in reverse order, not its primitive fields
 *          individually as expected.
 *
 * @param[in,out] tcpstp
 *          Pointer to a TCPROS status with a working connection.
 * @param[in] value
 *          Value to be written. It must be addressable by a pointer.
 * @return
 *          Error code.
 */
#if UROS_ENDIANNESS == 321
#define urosTcpRosSendRaw(tcpstp, value) \
  urosTcpRosSendRev((tcpstp), &(value), sizeof(value))
#else
#define urosTcpRosSendRaw(tcpstp, value) \
  urosTcpRosSend((tcpstp), &(value), sizeof(value))
#endif

/** @} */

/*~~~ TCPROS ARRAY ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name TCPROS array */
/** @{ */

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

/*~~~ TCPROS HANDLERS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name TCPROS handlers */
/** @{ */

/**
 * @brief   Name of the message length variable in TCPROS handlers.
 */
#if !defined(UROS_HND_LENGTH) || defined(__DOXYGEN__)
#define UROS_HND_LENVAR         msglen
#endif

/**
 * @brief   Name of the TCPROS status pointer in handlers.
 */
#if !defined(UROS_HND_TCPSTP) || defined(__DOXYGEN__)
#define UROS_HND_TCPSTP         tcpstp
#endif

/**
 * @brief   Name of the label called when exiting from a handler.
 */
#if !defined(UROS_HND_FINALLY) || defined(__DOXYGEN__)
#define UROS_HND_FINALLY        _finally
#endif

/*~~~ TCPROS MESSAGES ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name TCPROS messages */
/** @{ */

/**
 * @brief   Message descriptor initialization.
 * @details Tries to allocate the descriptor on the heap. In unsuccessful,
 *          goes to @p UROS_HND_FINALLY with an @p UROS_ERR_NOMEM error.
 *
 * @param[in] msgvarp
 *          Pointer to the message.
 * @param[in] ctypename
 *          Mangled version of the type name (@p msg_*, @p in_srv_*,
 *          @p out_srv_*).
 */
#define UROS_MSG_INIT_H(msgvarp, ctypename) \
  { msgvarp = urosNew(NULL, struct ctypename); \
    if (msgvarp == NULL) { \
      (UROS_HND_TCPSTP)->err = UROS_ERR_NOMEM; goto UROS_HND_FINALLY; } \
    init_##ctypename(msgvarp); }

/**
 * @brief   Message descriptor initialization.
 * @details Initializes a message descriptor located on the stack.
 *
 * @param[in] msgvarp
 *          Pointer to the message.
 * @param[in] ctypename
 *          Mangled version of the type name (@p msg_*, @p in_srv_*,
 *          @p out_srv_*).
 */
#define UROS_MSG_INIT_S(msgvarp, ctypename) \
  { init_##ctypename(msgvarp); }

/**
 * @brief   Uninitializes a message descriptor located in the heap.
 * @details The allocated descriptor is deleted from the heap.
 *
 * @param[in] msgvarp
 *          Pointer to the message.
 * @param[in] ctypename
 *          Mangled version of the type name (@p msg_*, @p in_srv_*,
 *          @p out_srv_*).
 */
#define UROS_MSG_UNINIT_H(msgvarp, ctypename) \
  { clean_##ctypename(msgvarp); \
    urosFree(msgvarp); }

/**
 * @brief   Uninitializes a message descriptor located on the stack.
 *
 * @param[in] msgvarp
 *          Pointer to the message.
 * @param[in] ctypename
 *          Mangled version of the type name (@p msg_*, @p in_srv_*,
 *          @p out_srv_*).
 */
#define UROS_MSG_UNINIT_S(msgvarp, ctypename) \
  { clean_##ctypename(msgvarp); }

/**
 * @brief   Sends the length of the message (message header).
 * @details This macro handles timeouts and errors. If unsuccessful, it goes to
 *          @p UROS_HND_FINALLY.
 *
 * @param[in] msgvarp
 *          Pointer to an initialized message.
 * @param[in] ctypename
 *          Mangled version of the type name (@p msg_*, @p in_srv_*,
 *          @p out_srv_*).
 */
#define UROS_MSG_SEND_LENGTH(msgvarp, ctypename) \
  { size_t start = (UROS_HND_TCPSTP)->csp->sentlen; \
    UROS_HND_LENVAR = (uint32_t)length_##ctypename(msgvarp); \
    while (urosTcpRosSendRaw(UROS_HND_TCPSTP, UROS_HND_LENVAR) != UROS_OK) { \
      if ((UROS_HND_TCPSTP)->err != UROS_ERR_TIMEOUT || \
          (UROS_HND_TCPSTP)->csp->sentlen != start || \
          urosTcpRosStatusCheckExit(UROS_HND_TCPSTP)) { \
        goto UROS_HND_FINALLY; } } }

/**
 * @brief   Receives the length of the message (message header).
 * @details This macro handles timeouts and errors. If unsuccessful, it goes to
 *          @p UROS_HND_FINALLY.
 */
#define UROS_MSG_RECV_LENGTH() \
  { size_t start = (UROS_HND_TCPSTP)->csp->recvlen; \
    while (urosTcpRosRecvRaw(UROS_HND_TCPSTP, UROS_HND_LENVAR) != UROS_OK) { \
      if ((UROS_HND_TCPSTP)->err != UROS_ERR_TIMEOUT || \
          start != (UROS_HND_TCPSTP)->csp->recvlen || \
          urosTcpRosStatusCheckExit(UROS_HND_TCPSTP)) { \
        goto UROS_HND_FINALLY; } } }

/**
 * @brief   Sends the body of the message.
 * @details This macro handles timeouts and errors. If unsuccessful, it goes to
 *          @p UROS_HND_FINALLY.
 *
 * @param[in] msgvarp
 *          Pointer to an initialized message.
 * @param[in] ctypename
 *          Mangled version of the type name (@p msg_*, @p in_srv_*,
 *          @p out_srv_*).
 */
#define UROS_MSG_SEND_BODY(msgvarp, ctypename) \
  { send_##ctypename(UROS_HND_TCPSTP, msgvarp); \
    if ((UROS_HND_TCPSTP)->err != UROS_OK) { \
      goto UROS_HND_FINALLY; } }

/**
 * @brief   Receives the body of the message.
 * @details This macro handles timeouts and errors. If unsuccessful, it goes to
 *          @p UROS_HND_FINALLY.
 *
 * @param[in] msgvarp
 *          Pointer to an initialized message.
 * @param[in] ctypename
 *          Mangled version of the type name (@p msg_*, @p in_srv_*,
 *          @p out_srv_*).
 */
#define UROS_MSG_RECV_BODY(msgvarp, ctypename) \
  { recv_##ctypename(UROS_HND_TCPSTP, msgvarp); \
    if ((UROS_HND_TCPSTP)->err != UROS_OK) { goto UROS_HND_FINALLY; } \
    urosError((size_t)UROS_HND_LENVAR != length_##ctypename(msgvarp), \
              { (UROS_HND_TCPSTP)->err = UROS_ERR_BADPARAM; \
                goto UROS_HND_FINALLY; }, \
              ("Wrong message length %u, expected %u\n", \
                (unsigned)UROS_HND_LENVAR, \
                (unsigned)length_##ctypename(msgvarp))); }

/** @} */

/*~~~ TCPROS TOPICS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name TCPROS topics */
/** @{ */

/**
 * @brief   Declaration of a topic message in the heap.
 * @note    To be used inside <code>UROS_TPC_INIT_H()</code>.
 */
#if !defined(UROS_TPC_MSGDECL_H) || defined(__DOXYGEN__)
#define UROS_TPC_MSGDECL_H      *msgp = NULL
#endif

/**
 * @brief   Declaration of a topic message on the stack.
 * @note    To be used inside <code>UROS_TPC_INIT_S()</code>.
 */
#if !defined(UROS_TPC_MSGDECL_S) || defined(__DOXYGEN__)
#define UROS_TPC_MSGDECL_S      msg
#endif

/**
 * @brief   Pointer to a topic message in the heap.
 */
#if !defined(UROS_TPC_MSGPTR_H) || defined(__DOXYGEN__)
#define UROS_TPC_MSGPTR_H       msgp
#endif

/**
 * @brief   Pointer to a topic message on the stack.
 */
#if !defined(UROS_TPC_MSGPTR_S) || defined(__DOXYGEN__)
#define UROS_TPC_MSGPTR_S       (&msg)
#endif

/**
 * @brief   Topic handler prologue.
 * @details This macro defines the following:
 *          -# declaration of the @p UROS_TPC_MSGDECL_H message pointer and the
 *             @p UROS_HND_LENGTH variable, used by other macros;
 *          -# assertions about the @p UROS_HND_TCPSTP object
 *          -# allocation and initialization of the message descriptor, with
 *             error checks.
 * @note    This macro should be placed at the beginning of the handler
 *          function, just after variable declarations (if any).
 *
 * @param[in] ctypename
 *          Mangled version of the type name (@p msg_*).
 */
#define UROS_TPC_INIT_H(ctypename) \
  struct ctypename UROS_TPC_MSGDECL_H; \
  uint32_t UROS_HND_LENVAR; \
  urosAssert((UROS_HND_TCPSTP) != NULL); \
  urosAssert((UROS_HND_TCPSTP)->topicp != NULL); \
  urosAssert(!(UROS_HND_TCPSTP)->topicp->flags.service); \
  urosAssert(urosConnIsValid((UROS_HND_TCPSTP)->csp)); \
  UROS_MSG_INIT_H(UROS_TPC_MSGPTR_H, ctypename)

/**
 * @brief   Topic handler prologue.
 * @details This macro defines the following:
 *          -# declaration of the @p UROS_TPC_MSGDECL_S message descriptor and
 *            the @p UROS_HND_LENGTH variable, used by other macros;
 *          -# assertions about the @p UROS_HND_TCPSTP object
 *          -# initialization of the message descriptor.
 * @note    This macro should be placed at the beginning of the handler
 *          function, just after variable declarations (if any).
 *
 * @param[in] ctypename
 *          Mangled version of the type name (@p msg_*).
 */
#define UROS_TPC_INIT_S(ctypename) \
  struct ctypename UROS_TPC_MSGDECL_S; \
  uint32_t UROS_HND_LENVAR; \
  urosAssert((UROS_HND_TCPSTP) != NULL); \
  urosAssert((UROS_HND_TCPSTP)->topicp != NULL); \
  urosAssert(!(UROS_HND_TCPSTP)->topicp->flags.service); \
  urosAssert(urosConnIsValid((UROS_HND_TCPSTP)->csp)); \
  UROS_MSG_INIT_S(UROS_TPC_MSGPTR_S, ctypename)

/**
 * @brief   Topic handler epilogue.
 * @details This macro cleans the message descriptor and deallocates it from
 *          the heap.
 * @note    This macro should be placed after the @p UROS_HND_FINALLY label.
 *
 * @param[in] ctypename
 *          Mangled version of the type name (@p msg_*, @p in_srv_*,
 *          @p out_srv_*).
 */
#define UROS_TPC_UNINIT_H(ctypename) \
  UROS_MSG_UNINIT_H(UROS_TPC_MSGPTR_H, ctypename);

/**
 * @brief   Topic handler epilogue.
 * @details This macro cleans the message descriptor.
 * @note    This macro should be placed after the @p UROS_HND_FINALLY label.
 *
 * @param[in] ctypename
 *          Mangled version of the type name (@p msg_*, @p in_srv_*,
 *          @p out_srv_*).
 */
#define UROS_TPC_UNINIT_S(ctypename) \
  UROS_MSG_UNINIT_S(UROS_TPC_MSGPTR_S, ctypename);

/** @} */

/*~~~ TCPROS SERVICES ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name TCPROS services */
/** @{ */

/**
 * @brief   Declaration of a service request message in the heap.
 * @note    To be used inside @p UROS_SRV_INIT_H().
 */
#if !defined(UROS_SRV_INDECL_H) || defined(__DOXYGEN__)
#define UROS_SRV_INDECL_H       *inmsgp = NULL
#endif

/**
 * @brief   Declaration of a service response message in the heap.
 * @note    To be used inside @p UROS_SRV_INIT_H().
 */
#if !defined(UROS_SRV_OUTDECL_H) || defined(__DOXYGEN__)
#define UROS_SRV_OUTDECL_H      *outmsgp = NULL
#endif

/**
 * @brief   Declaration of a service request message on the stack.
 * @note    To be used inside @p UROS_SRV_INIT_S().
 */
#if !defined(UROS_SRV_INDECL_S) || defined(__DOXYGEN__)
#define UROS_SRV_INDECL_S       inmsg
#endif

/**
 * @brief   Declaration of a service response message on the stack.
 * @note    To be used inside @p UROS_SRV_INIT_S().
 */
#if !defined(UROS_SRV_OUTDECL_S) || defined(__DOXYGEN__)
#define UROS_SRV_OUTDECL_S      outmsg
#endif

/**
 * @brief   Pointer to a service request message in the heap.
 */
#if !defined(UROS_SRV_INPTR_H) || defined(__DOXYGEN__)
#define UROS_SRV_INPTR_H        inmsgp
#endif

/**
 * @brief   Pointer to a service response message in the heap.
 */
#if !defined(UROS_SRV_OUTPTR_H) || defined(__DOXYGEN__)
#define UROS_SRV_OUTPTR_H       outmsgp
#endif

/**
 * @brief   Pointer to a service request message on the stack.
 */
#if !defined(UROS_SRV_INPTR_S) || defined(__DOXYGEN__)
#define UROS_SRV_INPTR_S        (&inmsg)
#endif

/**
 * @brief   Pointer to a service response message on the stack.
 */
#if !defined(UROS_SRV_OUTPTR_S) || defined(__DOXYGEN__)
#define UROS_SRV_OUTPTR_S       (&outmsg)
#endif

/**
 * @brief   Name of the <em>OK byte</em> variable in TCPROS service handlers.
 */
#if !defined(UROS_SRV_OKVAR) || defined(__DOXYGEN__)
#define UROS_SRV_OKVAR          okByte
#endif

/**
 * @brief   Service handler prologue.
 * @details This macro defines the following:
 *          -# declaration of the @p UROS_SRV_*DECL_* messages and the
 *             @p UROS_HND_LENVAR variable, used by other macros;
 *          -# assertions about the @p UROS_HND_TCPSTP object
 * @note    This macro should be placed at the beginning of the handler
 *          function, just after variable declarations (if any).
 *
 * @param[in] inctypename
 *          Mangled version of the request type name (@p in_srv_*).
 * @param[in] indecl
 *          Declaration of the request message variable:
 *          - <code>*inmsgp = NULL</code> when in the heap,
 *          - <code>inmsg</code> when on the stack.
 * @param[in] outctypename
 *          Mangled version of the response type name (@p out_srv_*).
 * @param[in] outdecl
 *          Declaration of the response message variable:
 *          - <code>*outmsgp = NULL</code> when in the heap,
 *          - <code>outmsg</code> when on the stack.
 */
#define UROS_SRV_INIT(inctypename, indecl, \
                          outctypename, outdecl) \
  struct inctypename indecl; \
  struct outctypename outdecl; \
  uint32_t UROS_HND_LENVAR; \
  uint8_t UROS_SRV_OKVAR; \
  urosAssert((UROS_HND_TCPSTP) != NULL); \
  urosAssert((UROS_HND_TCPSTP)->topicp != NULL); \
  urosAssert((UROS_HND_TCPSTP)->topicp->flags.service); \
  urosAssert(urosConnIsValid((UROS_HND_TCPSTP)->csp));

/**
 * @brief   Service request/response initialization.
 * @details Request and response on the stack.
 *          This macro defines the following:
 *          -# declaration of the @p UROS_SRV_INDECL_S and
 *             @p UROS_SRV_OUTDECL_S messages, and the @p UROS_HND_LENVAR
 *             variable, used by other macros;
 *          -# assertions about the @p UROS_HND_TCPSTP object
 *          -# allocation and initialization of the message descriptors, with
 *             error checks.
 * @note    This macro should be placed at the beginning of the handler
 *          function, just after variable declarations (if any).
 *
 * @param[in] inctypename
 *          Mangled version of the request type name (@p in_srv_*).
 * @param[in] outctypename
 *          Mangled version of the response type name (@p out_srv_*).
 */
#define UROS_SRV_INIT_SISO(inctypename, outctypename) \
  UROS_SRV_INIT(inctypename, UROS_SRV_INDECL_S, \
                outctypename, UROS_SRV_OUTDECL_S) \
  UROS_MSG_INIT_S(UROS_SRV_INPTR_S, inctypename) \
  UROS_MSG_INIT_S(UROS_SRV_OUTPTR_S, outctypename)

/**
 * @brief   Service request/response initialization.
 * @details Request on the stack, response in the heap.
 *          This macro defines the following:
 *          -# declaration of the @p UROS_SRV_INDECL_S and
 *             @p UROS_SRV_OUTDECL_H messages, and the @p UROS_HND_LENVAR
 *             variable, used by other macros;
 *          -# assertions about the @p UROS_HND_TCPSTP object
 *          -# allocation and initialization of the message descriptors, with
 *             error checks.
 * @note    This macro should be placed at the beginning of the handler
 *          function, just after variable declarations (if any).
 * @see     UROS_SRV_INIT()
 *
 * @param[in] inctypename
 *          Mangled version of the request type name (@p in_srv_*).
 * @param[in] outctypename
 *          Mangled version of the response type name (@p out_srv_*).
 */
#define UROS_SRV_INIT_SIHO(inctypename, outctypename) \
  UROS_SRV_INIT(inctypename, UROS_SRV_INDECL_S, \
                outctypename, UROS_SRV_OUTDECL_H) \
  UROS_MSG_INIT_S(UROS_SRV_INPTR_S, inctypename) \
  UROS_MSG_INIT_H(UROS_SRV_OUTPTR_H, outctypename)

/**
 * @brief   Service request/response initialization.
 * @details Request in the heap, response on the stack.
 *          This macro defines the following:
 *          -# declaration of the @p UROS_SRV_INDECL_H and
 *             @p UROS_SRV_OUTDECL_S messages, and the @p UROS_HND_LENVAR
 *             variable, used by other macros;
 *          -# assertions about the @p UROS_HND_TCPSTP object
 *          -# allocation and initialization of the message descriptors, with
 *             error checks.
 * @note    This macro should be placed at the beginning of the handler
 *          function, just after variable declarations (if any).
 * @see     UROS_SRV_INIT()
 *
 * @param[in] inctypename
 *          Mangled version of the request type name (@p in_srv_*).
 * @param[in] outctypename
 *          Mangled version of the response type name (@p out_srv_*).
 */
#define UROS_SRV_INIT_HISO(inctypename, outctypename) \
  UROS_SRV_INIT(inctypename, UROS_SRV_INDECL_H, \
                outctypename, UROS_SRV_OUTDECL_S) \
  UROS_MSG_INIT_H(UROS_SRV_INPTR_H, inctypename) \
  UROS_MSG_INIT_S(UROS_SRV_OUTPTR_S, outctypename)

/**
 * @brief   Service request/response initialization.
 * @details Request and response in the heap.
 *          This macro defines the following:
 *          -# declaration of the @p UROS_SRV_INDECL_H and
 *             @p UROS_SRV_OUTDECL_H messages, and the @p UROS_HND_LENVAR
 *             variable, used by other macros;
 *          -# assertions about the @p UROS_HND_TCPSTP object
 *          -# allocation and initialization of the message descriptors, with
 *             error checks.
 * @note    This macro should be placed at the beginning of the handler
 *          function, just after variable declarations (if any).
 * @see     UROS_SRV_INIT()
 *
 * @param[in] inctypename
 *          Mangled version of the request type name (@p in_srv_*).
 * @param[in] outctypename
 *          Mangled version of the response type name (@p out_srv_*).
 */
#define UROS_SRV_INIT_HIHO(inctypename, outctypename) \
  UROS_SRV_INIT(inctypename, UROS_SRV_INDECL_H, \
                outctypename, UROS_SRV_OUTDECL_H) \
  UROS_MSG_INIT_H(UROS_SRV_INPTR_H, inctypename) \
  UROS_MSG_INIT_H(UROS_SRV_OUTPTR_H, outctypename)

/**
 * @brief   Service request/response uninitialization.
 * @details Request and response on the stack.
 * @note    This macro should be placed after the @p UROS_HND_FINALLY label.
 *
 * @param[in] inctypename
 *          Mangled version of the request type name (@p in_srv_*).
 * @param[in] outctypename
 *          Mangled version of the response type name (@p out_srv_*).
 */
#define UROS_SRV_UNINIT_SISO(inctypename, outctypename) \
  UROS_MSG_UNINIT_S(UROS_SRV_INPTR_S, inctypename) \
  UROS_MSG_UNINIT_S(UROS_SRV_OUTPTR_S, outctypename)

/**
 * @brief   Service request/response uninitialization.
 * @details Request on the stack, response in the heap.
 * @note    This macro should be placed after the @p UROS_HND_FINALLY label.
 *
 * @param[in] inctypename
 *          Mangled version of the request type name (@p in_srv_*).
 * @param[in] outctypename
 *          Mangled version of the response type name (@p out_srv_*).
 */
#define UROS_SRV_UNINIT_SIHO(inctypename, outctypename) \
  UROS_MSG_UNINIT_S(UROS_SRV_INPTR_S, inctypename) \
  UROS_MSG_UNINIT_H(UROS_SRV_OUTPTR_H, outctypename)

/**
 * @brief   Service request/response uninitialization.
 * @details Request in the heap, response on the stack.
 * @note    This macro should be placed after the @p UROS_HND_FINALLY label.
 *
 * @param[in] inctypename
 *          Mangled version of the request type name (@p in_srv_*).
 * @param[in] outctypename
 *          Mangled version of the response type name (@p out_srv_*).
 */
#define UROS_SRV_UNINIT_HISO(inctypename, outctypename) \
  UROS_MSG_UNINIT_H(UROS_SRV_INPTR_H, inctypename) \
  UROS_MSG_UNINIT_S(UROS_SRV_OUTPTR_S, outctypename)

/**
 * @brief   Service request/response uninitialization.
 * @details Request and response in the heap.
 * @note    This macro should be placed after the @p UROS_HND_FINALLY label.
 *
 * @param[in] inctypename
 *          Mangled version of the request type name (@p in_srv_*).
 * @param[in] outctypename
 *          Mangled version of the response type name (@p out_srv_*).
 */
#define UROS_SRV_UNINIT_HIHO(inctypename, outctypename) \
  UROS_MSG_UNINIT_H(UROS_SRV_INPTR_H, inctypename) \
  UROS_MSG_UNINIT_H(UROS_SRV_OUTPTR_H, outctypename)

/**
 * @brief   Sends the <em>OK byte</em>, and error string if necessary.
 * @details If the <em>OK byte</em> is @p 0, it sends the string from
 *          @p UROS_HND_TCPSTP->errstr error, and it goes to
 *          @p UROS_HND_FINALLY.
 */
#define UROS_SRV_SEND_OKBYTE_ERRSTR() \
  { urosTcpRosSendRaw(UROS_HND_TCPSTP, UROS_SRV_OKVAR); \
    if (UROS_SRV_OKVAR == 0) { \
      urosTcpRosSendString(UROS_HND_TCPSTP, &(UROS_HND_TCPSTP)->errstr); \
      urosStringObjectInit(&(UROS_HND_TCPSTP)->errstr); \
      goto UROS_HND_FINALLY; } }

/**
 * @brief   Sends the <em>OK byte</em>, and error string if necessary.
 * @details If the <em>OK byte</em> is @p 0, it sends the string from
 *          @p UROS_HND_TCPSTP->errstr error, and it goes to
 *          @p UROS_HND_FINALLY.
 */
#define UROS_SRV_RECV_OKBYTE() \
  { urosTcpRosRecvRaw(UROS_HND_TCPSTP, UROS_SRV_OKVAR); \
    urosError(UROS_SRV_OKVAR == 0, goto UROS_HND_FINALLY, \
              ("Service OK byte reveals an error (0)\n")); }

/**
 * @brief   Service call handler prologue.
 * @details This macro defines the following:
 *          -# declaration of the @p UROS_SRV_INDECL_* message and the
 *             @p UROS_HND_LENVAR variable, used by other macros;
 *          -# assertions about the @p UROS_HND_TCPSTP object
 *          -# initialization of the @p UROS_SRV_OUTPTR_H result object.
 * @note    This macro should be placed at the beginning of the handler
 *          function, just after variable declarations (if any).
 *
 * @param[in] inctypename
 *          Mangled version of the request type name (@p in_srv_*).
 * @param[in] outctypename
 *          Mangled version of the response type name (@p out_srv_*).
 */
#define UROS_SRVCALL_INIT(inctypename, outctypename) \
  struct inctypename indecl; \
  uint32_t UROS_HND_LENVAR; \
  uint8_t UROS_SRV_OKVAR; \
  urosAssert((UROS_HND_TCPSTP) != NULL); \
  urosAssert((UROS_HND_TCPSTP)->topicp != NULL); \
  urosAssert((UROS_HND_TCPSTP)->topicp->flags.service); \
  urosAssert(!(UROS_HND_TCPSTP)->topicp->flags.persistent); \
  urosAssert(urosConnIsValid((UROS_HND_TCPSTP)->csp)); \
  init_##outctypename(UROS_SRV_OUTPTR_H);

/** @} */

/** @} */

/*===========================================================================*/
/* GLOBAL PROTOTYPES                                                         */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

void urosTopicSubParamsDelete(uros_tcpcliargs_t *parp);

void urosTcpRosStatusObjectInit(UrosTcpRosStatus *tcpstp, UrosConn *csp);
void urosTcpRosStatusClean(UrosTcpRosStatus *tcpstp, uros_bool_t deep);
void urosTcpRosStatusDelete(UrosTcpRosStatus *tcpstp, uros_bool_t deep);
void urosTcpRosStatusIssueExit(UrosTcpRosStatus *tcpstp);
uros_bool_t urosTcpRosStatusCheckExit(UrosTcpRosStatus *tcpstp);

void urosTcpRosArrayObjectInit(UrosTcpRosArray *arrayp);
void urosTcpRosArrayClean(UrosTcpRosArray *arrayp);
void urosTcpRosArrayDelete(UrosTcpRosArray *arrayp, uros_bool_t deep);

uros_err_t urosTcpRosSkip(UrosTcpRosStatus *tcpstp, size_t length);
uros_err_t urosTcpRosExpect(UrosTcpRosStatus *tcpstp,
                            void *tokp, size_t toklen);
uros_err_t urosTcpRosRecv(UrosTcpRosStatus *tcpstp,
                          void *bufp, size_t buflen);
uros_err_t urosTcpRosRecvRev(UrosTcpRosStatus *tcpstp,
                             void *bufp, size_t buflen);
uros_err_t urosTcpRosRecvString(UrosTcpRosStatus *tcpstp,
                                UrosString *strp);
uros_err_t urosTcpRosSend(UrosTcpRosStatus *tcpstp,
                          const void *bufp, size_t buflen);
uros_err_t urosTcpRosSendRev(UrosTcpRosStatus *tcpstp,
                             const void *bufp, size_t buflen);
uros_err_t urosTcpRosSendString(UrosTcpRosStatus *tcpstp,
                                const UrosString *strp);
uros_err_t urosTcpRosSendStringSZ(UrosTcpRosStatus *tcpstp,
                                  const char *strp);
uros_err_t urosTcpRosSendError(UrosTcpRosStatus *tcpstp);
uros_err_t urosTcpRosSendHeader(UrosTcpRosStatus *tcpstp,
                                uros_bool_t isrequest);
uros_err_t urosTcpRosRecvHeader(UrosTcpRosStatus *tcpstp,
                                uros_bool_t isrequest,
                                uros_bool_t isservice);

uros_err_t urosTcpRosCallService(const UrosAddr *pubaddrp,
                                 const UrosTopic *servicep,
                                 void *resobjp);
uros_err_t urosTcpRosListenerThread(void *data);
uros_err_t urosTcpRosServerThread(UrosConn *csp);
uros_err_t urosTcpRosClientThread(uros_tcpcliargs_t *argsp);
void urosTcpRosTopicSubscriberDone(UrosTcpRosStatus *tcpstp);
void urosTcpRosTopicPublisherDone(UrosTcpRosStatus *tcpstp);
void urosTcpRosServiceDone(UrosTcpRosStatus *tcpstp);

#ifdef __cplusplus
}
#endif
#endif /* _UROSTCPROS_H_ */
