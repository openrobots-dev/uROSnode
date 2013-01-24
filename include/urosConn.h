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
 * @file    urosConn.h
 * @author  Andrea Zoppi <texzk@email.it>
 *
 * @brief   Connectivity features of the middleware.
 */

#ifndef _UROSCONN_H_
#define _UROSCONN_H_

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include "urosBase.h"

/*===========================================================================*/
/* TYPES & MACROS                                                            */
/*===========================================================================*/

/** @addtogroup conn_macros */
/** @{ */

/** @name Address values */
/** @{ */

/**
 * @brief   Makes an IP address packed @em dword.
 *
 * @param[in] f1
 *          IP address field 1 (byte 3).
 * @param[in] f2
 *          IP address field 2 (byte 2).
 * @param[in] f3
 *          IP address field 3 (byte 1).
 * @param[in] f4
 *          IP address field 4 (byte 0).
 * @return
 *          The packed little-endian @em dword IP address.
 *
 * @par     Example
 *          See @ref conn_ex_ipfmt "UROS_IPFMT example".
 */
#define urosIpDword(f1, f2, f3, f4) \
  ((((uint32_t)(f1) & 0xFFu) << 24u) | \
   (((uint32_t)(f2) & 0xFFu) << 16u) | \
   (((uint32_t)(f3) & 0xFFu) <<  8u) | \
   (((uint32_t)(f4) & 0xFFu) <<  0u))

/**
 * @brief   Binds to any IP addresses.
 * @note    Packed little-endian @em dword.
 */
#define UROS_ANY_IP     urosIpDword(0, 0, 0, 0)

/**
 * @brief   Binds to any ports.
 */
#define UROS_ANY_PORT   ((uint16_t)0)

/** @} */

/** @name Variable arguments macros */
/** @{ */

/**
 * @brief   @p printf() compatible format string for an IP address.
 *
 * @par     Example
 * @anchor  conn_ex_ipfmt
 *          @code{.c}
 *          UrosIp ip;
 *          ip.dword = urosIpAddr(192, 168, 1, 1);
 *          printf("The IP address is: "UROS_IPFMT, UROS_IPARG(&ip));
 *          @endcode
 *          prints
 *          @verbatim The IP address is: 192.168.1.1@endverbatim
 */
#define UROS_IPFMT      "%u.%u.%u.%u"

/**
 * @brief   @p printf() compatible format string for a connection address.
 *
 * @par     Example
 * @anchor  conn_ex_addrfmt
 *          @code{.c}
 *          UrosAddr addr;
 *          addr.ip.dword = urosIpAddr(192, 168, 1, 1);
 *          addr.port = 12345;
 *          printf("The TCP/IP address is: "UROS_ADDRFMT, UROS_ADDRARG(&addr));
 *          @endcode
 *          prints
 *          @verbatim The TCP/IP address is: 192.168.1.1:12345@endverbatim
 */
#define UROS_ADDRFMT    UROS_IPFMT":%u"

/**
 * @brief   Makes the formatted parameters for an IP address.
 * @details Generates a list of values to be passed to a variable arguments
 *          function.
 * @note    To be used with the @p UROS_IPFMT format string.
 *
 * @param[in] ipp
 *          Pointer to an @p UrosIp descriptor.
 * @return
 *          List of values for a variable arguments function.
 *
 * @par     Example
 *          See @ref conn_ex_ipfmt "UROS_IPFMT example".
 */
#define UROS_IPARG(ipp) \
  (unsigned)((ipp)->fields.field1), \
  (unsigned)((ipp)->fields.field2), \
  (unsigned)((ipp)->fields.field3), \
  (unsigned)((ipp)->fields.field4)

/**
 * @brief   Makes the formatted parameters for a connection address.
 * @details Generates a list of values to be passed to a variable arguments
 *          function.
 * @note    To be used with the @p UROS_ADDRFMT format string.
 *
 * @param[in] addrp
 *          Pointer to an @p UrosAddr descriptor.
 * @return
 *          List of values for a variable arguments function.
 *
 * @par     Example
 *          See @ref conn_ex_addrfmt "UROS_IPFMT example".
 */
#define UROS_ADDRARG(addrp) \
  UROS_IPARG(&(addrp)->ip), (unsigned)((addrp)->port)

/** @} */
/** @} */

/** @addtogroup conn_types */
/** @{ */

/**
 * @brief   Connection transport protocol identifier.
 */
typedef enum uros_connproto_t {
  UROS_PROTO_TCP = 0,   /**< @brief TCP/IP.*/
  UROS_PROTO_UDP,       /**< @brief UDP/IP.*/

  UROS_PROTO__LENGTH    /**< @brief Enumeration length.*/
} uros_connproto_t;

/**
 * @brief   Little-endian IP addres record.
 */
typedef union UrosIp {
  uint32_t  dword;      /**< @brief Packed dword.*/
  struct {
    uint8_t field4;     /**< @brief Address field 4.*/
    uint8_t field3;     /**< @brief Address field 3.*/
    uint8_t field2;     /**< @brief Address field 2.*/
    uint8_t field1;     /**< @brief Address field 1.*/
  }         fields;     /**< @brief Individual fields.*/
  uint8_t   bytes[4];   /**< @brief Individual bytes.*/
} UrosIp;

/**
 * @brief   Full address record (IP + port).
 */
typedef struct UrosAddr {
  UrosIp    ip;         /**< @brief Network address (IP).*/
  uint16_t  port;       /**< @brief Transport layer port.*/
} UrosAddr;

/**
 * @brief   Connection information record.
 * @pre     @p UrosConn_LLD defines additional platform-dependent fields.
 */
typedef struct UrosConn {
  UrosAddr          locaddr;    /**< @brief Local address.*/
  UrosAddr          remaddr;    /**< @brief Remote address.*/
  uros_connproto_t  protocol;   /**< @brief Connection protocol.*/
  size_t            recvlen;    /**< @brief Number of received bytes up to now.*/
  size_t            sentlen;    /**< @brief Number of sent bytes up up now.*/

  /* Implementation dependent.*/
  UrosConn__LLD
} UrosConn;

/** @} */

/*===========================================================================*/
/* GLOBAL PROTOTYPES                                                         */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

uros_err_t urosHostnameToIp(const UrosString *hostnamep, UrosIp *ipp);
uros_err_t urosUriToAddr(const UrosString *uri, UrosAddr *addrp);

void urosConnObjectInit(UrosConn *cp);

uros_bool_t urosConnIsValid(UrosConn *cp);
uros_err_t urosConnCreate(UrosConn *cp, uros_connproto_t protocol);
uros_err_t urosConnBind(UrosConn *cp, const UrosAddr *locaddrp);
uros_err_t urosConnAccept(UrosConn *cp, UrosConn *spawnedp);
uros_err_t urosConnListen(UrosConn *cp, uros_cnt_t backlog);
uros_err_t urosConnConnect(UrosConn *cp, const UrosAddr *remaddrp);
uros_err_t urosConnRecv(UrosConn *cp,
                        void **bufpp, size_t *buflenp);
uros_err_t urosConnRecvFrom(UrosConn *cp,
                            void **bufpp, size_t *buflenp,
                            const UrosAddr *remaddrp);
uros_err_t urosConnSend(UrosConn *cp,
                        const void *bufp, size_t buflen);
uros_err_t urosConnSendConst(UrosConn *cp,
                             const void *bufp, size_t buflen);
uros_err_t urosConnSendTo(UrosConn *cp,
                          const void *bufp, size_t buflen,
                          const UrosAddr *remaddrp);
uros_err_t urosConnSendToConst(UrosConn *cp,
                               const void *bufp, size_t buflen,
                               const UrosAddr *remaddrp);
uros_err_t urosConnShutdown(UrosConn *cp,
                            uros_bool_t read, uros_bool_t write);
uros_err_t urosConnClose(UrosConn *cp);

uros_err_t urosConnGetTcpNoDelay(UrosConn *cp, uros_bool_t *enablep);
uros_err_t urosConnSetTcpNoDelay(UrosConn *cp, uros_bool_t enable);
uros_err_t urosConnGetRecvTimeout(UrosConn *cp, uint32_t *msp);
uros_err_t urosConnSetRecvTimeout(UrosConn *cp, uint32_t ms);
uros_err_t urosConnGetSendTimeout(UrosConn *cp, uint32_t *msp);
uros_err_t urosConnSetSendTimeout(UrosConn *cp, uint32_t ms);

const char *urosConnLastErrorText(const UrosConn *cp);

#ifdef __cplusplus
}
#endif
#endif /* _UROSCONN_H_ */
