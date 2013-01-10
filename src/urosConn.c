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
 * @file    urosConn.c
 * @author  Andrea Zoppi <texzk@email.it>
 *
 * @brief   Connectivity features of the middleware.
 */

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include "../include/urosBase.h"
#include "../include/urosUser.h"
#include "../include/urosConn.h"
#include "../include/lld/uros_lld_conn.h"

/*===========================================================================*/
/* LOCAL TYPES & MACROS                                                      */
/*===========================================================================*/

#if UROS_CONN_C_USE_ASSERT == UROS_FALSE && !defined(__DOXYGEN__)
#undef urosAssert
#define urosAssert(expr)
#endif

#if UROS_CONN_C_USE_ERROR_MSG == UROS_FALSE && !defined(__DOXYGEN__)
#undef urosError
#define urosError(when, action, msgargs) { if (when) { action; } }
#endif

/*===========================================================================*/
/* GLOBAL FUNCTIONS                                                          */
/*===========================================================================*/

/** @addtogroup conn_funcs */
/** @{ */

/**
 * @brief   Resolves an hostname to an IP address.
 * @details Executes a DNS call to resolve the hostname string to an IP address
 *          which can be used by the connectivity library.
 *
 * @param[in] hostnamep
 *          Pointer to a hostname string.
 * @param[out] ipp
 *          Pointer to an allocated @p UrosIp object.
 * @return
 *          Error code.
 */
uros_err_t urosHostnameToIp(const UrosString *hostnamep, UrosIp *ipp) {

  return uros_lld_hostnametoip(hostnamep, ipp);
}

/**
 * @brief   Resolves an URI to a connection address.
 * @details The provided URI is converted into a connection address. The URI
 *          has the following structure:
 *
 *          <pre><i>protocol</i>://<i>hostname</i>:<i>port</i>[/...]</pre>
 *
 *          where:
 *          - the <i>protocol</i> is ignored and supposed to be <tt>http</tt>
 *            (ROS XMLRPC over HTTP)
 *          - the <i>hostname</i> can also be an IPv4 address
 *          - the remainder after the <i>port</i> is simply ignored.
 *
 * @param[in] urip
 *          Pointer to the URI string.
 * @param[out] addrp
 *          Pointer to an annlocated @p UrosAddr descriptor.
 * @return
 *          Error code.
 */
uros_err_t urosUriToAddr(const UrosString *urip, UrosAddr *addrp) {

  const char *start, *end, *ptr;
  size_t rem;
  uint32_t field;
  UrosString hostname;
  uros_err_t err;

  urosAssert(urosStringNotEmpty(urip));
  urosAssert(addrp != NULL);

  /* Skip the "{protocol}://" prefix.*/
  rem = urip->length;
  for (ptr = urip->datap; ptr[0] != ':' && rem > 0; --rem, ++ptr) {}
  urosError(rem <= 3, return UROS_ERR_PARSE,
            ("URI [%.*s] does not start with [{protocol}://]\n",
             UROS_STRARG(urip)));
  urosError(ptr[1] != '/' || ptr[2] != '/', return UROS_ERR_PARSE,
            ("URI [%.*s] does not start with [{protocol}://]\n",
             UROS_STRARG(urip)));
  ptr += 3; rem -= 3;

  /* Get the hostname/IP.*/
  start = ptr;
  for (ptr = start; ptr[0] != ':' && rem > 0; --rem, ++ptr) {}
  end = ptr++; --rem;
  urosError(rem <= 1, return UROS_ERR_PARSE,
            ("Cannot find ':' separator for {IP}:{PORT} in URI [%.*s]\n",
             UROS_STRARG(urip)));
  hostname.length = (size_t)end - (size_t)start;
  hostname.datap = (char*)start;
  err = urosHostnameToIp(&hostname, &addrp->ip);
  urosError(err != UROS_OK, return err,
            ("Error %s while resolving hostname [%.*s]\n",
             urosErrorText(err), UROS_STRARG(&hostname)));

  /* Decode the port number.*/
  if (rem > 6) { rem = 6; }
  for (field = 0; rem > 0 && *ptr >= '0' && *ptr <= '9'; ++ptr, --rem) {
    field = field * 10 + *ptr - '0';
  }
  urosError(field > 65535, return UROS_ERR_PARSE,
            ("Port number %d > 65535\n", field));
  addrp->port = (uint16_t)field;

  /* Check for a final '/', and ignore the rest.*/
  urosError(rem > 0 && *ptr != '/', return UROS_ERR_PARSE,
            ("Missing '/' after {PORT} in URI [%.*s]\n", UROS_STRARG(urip)));
  return UROS_OK;
}

/**
 * @brief   Initializes a connection object.
 * @details Invalidates the connection fields and initializes the
 *          low-level ones.
 * @see     uros_lld_conn_objectinit()
 *
 * @param[in,out] cp
 *          Pointer to an allocated @p UrosConn object.
 */
void urosConnObjectInit(UrosConn *cp) {

  urosAssert(cp != NULL);

  cp->locaddr.ip.dword = UROS_ANY_IP;
  cp->locaddr.port = UROS_ANY_PORT;
  cp->remaddr.ip.dword = UROS_ANY_IP;
  cp->remaddr.port = UROS_ANY_PORT;

  cp->protocol = UROS_PROTO__LENGTH;

  cp->recvlen = 0;
  cp->sentlen = 0;

  uros_lld_conn_objectinit(cp);
}

/**
 * @brief   Checks if pointing to a valid connection object.
 * @details @p cp points to a valid connection if it is not @p NULL and its
 *          low-level fields are in a valid state.
 * @see     uros_lld_conn_isvalid()
 *
 * @param[in] cp
 *          Pointer to a presumed valid @p UrosConn object.
 * @return
 *          @p true if @p cp addresses a valid @p UrosConn object.
 */
uros_bool_t urosConnIsValid(UrosConn *cp) {

  return uros_lld_conn_isvalid(cp);
}

/**
 * @brief   Creates a new connection.
 * @details The connection object is initialized for a new connection.
 * @see     uros_lld_conn_create()
 * @note    Conceptually equivalent to POSIX @p create().
 *
 * @pre     The connection object is initialized but no connection has been
 *          created with it.
 *
 * @param[in,out] cp
 *          Pointer to an initialized @p UrosConn object.
 * @param[in] protocol
 *          Connection protocol.
 * @return
 *          Error code.
 */
uros_err_t urosConnCreate(UrosConn *cp, uros_connproto_t protocol) {

  return uros_lld_conn_create(cp, protocol);
}

/**
 * @brief   Binds to a local address.
 * @details The local side of a connection object is bound to the provided
 *          address.
 * @see     uros_lld_conn_bind()
 * @note    Conceptually equivalent to POSIX @p bind().
 *
 * @pre     The connection object is only created.
 *
 * @param[in,out] cp
 *          Pointer to an initialized @p UrosConn object.
 * @param[in] locaddrp
 *          Pointer to the local address descriptor to be bound.
 * @return
 *          Error code.
 */
uros_err_t urosConnBind(UrosConn *cp, const UrosAddr *locaddrp) {

  return uros_lld_conn_bind(cp, locaddrp);
}

/**
 * @brief   Accepts an incoming connection.
 * @details A listener connection waits until a remote connection is
 *          instantiated. A new dedicated connection channel is spawned
 *          and returned, and the spawning connection is put back into
 *          listening state.
 * @see     uros_lld_conn_accept()
 * @note    Conceptually equivalent to POSIX @p accept().
 *
 * @pre     The @p cp connection object is listening.
 *
 * @param[in,out] cp
 *          Pointer to the listener connection object.
 * @param[out] spawnedp
 *          Pointer to an allocated @p UrosConn object with which the new
 *          dedicated communication channel is created.
 * @return
 *          Error code.
 */
uros_err_t urosConnAccept(UrosConn *cp, UrosConn *spawnedp) {

  return uros_lld_conn_accept(cp, spawnedp);
}

/**
 * @brief   Initializes the listening mode.
 * @details The connection object is initialized for listening.
 * @see     uros_lld_conn_listen()
 * @note    Conceptually equivalent to POSIX @p listen().
 *
 * @pre     The connection object is only created.
 *
 * @param[in,out] cp
 *          Pointer to an initialized @p UrosConn object.
 * @param[in] backlog
 *          Maximum number of incoming connection simultaneously waiting to be
 *          accepdted with @p urosConnAccept().
 * @return
 *          Error code.
 */
uros_err_t urosConnListen(UrosConn *cp, uros_cnt_t backlog) {

  return uros_lld_conn_listen(cp, backlog);
}

/**
 * @brief   Connects to a remote address.
 * @see     uros_lld_conn_connect()
 * @note    Conceptually equivalent to POSIX @p connect().
 *
 * @pre     The connection object is only created.
 *
 * @param[in,out] cp
 *          Pointer to an initialized @p UrosConn object.
 * @param[in] remaddrp
 *          Pointer to the remote address descriptor to connect to.
 * @return
 *          Error code.
 */
uros_err_t urosConnConnect(UrosConn *cp, const UrosAddr *remaddrp) {

  return uros_lld_conn_connect(cp, remaddrp);
}

/**
 * @brief   Receives some data.
 * @details The connection object waits until some data is received from the
 *          remote address. The pointer to the received data is returned,
 *          with a length up to the requested one.
 * @see     uros_lld_conn_recv()
 * @warning Not conceptually equivalent to POSIX @p connect(), as the buffer
 *          is not provided by the user, but by the middleware.
 *
 * @pre     The connection must be open and working.
 * @post    Either data is received, or an @p UROS_ERR_EOF is returned when
 *          there is no more pending data (connection closed by remote).
 *
 * @param[in,out] cp
 *          Pointer to a communicating connection object.
 * @param[out] bufpp
 *          Indirect pointer to the received data buffer.
 * @param[in,out] buflenp
 *          Pointer to the buffer length in bytes, with the following meaning:
 *          - At call, it indicates the maximum length of the received data.
 *          - At return, it is the actual length of the received data.
 * @return
 *          Error code.
 */
uros_err_t  urosConnRecv(UrosConn *cp,
                         void **bufpp, size_t *buflenp) {

  return uros_lld_conn_recv(cp, bufpp, buflenp);
}

/**
 * @brief   Receives some data from a remote address.
 * @details The connection object waits until some data is received from the
 *          remote address. The pointer to the received data is returned,
 *          with a length up to the requested one.
 * @see     uros_lld_conn_recvfrom()
 * @note    Not available for TCP connections.
 * @warning Not conceptually equivalent to POSIX @p connect(), as the buffer
 *          is not provided by the user, but by the middleware.
 *
 * @param[in,out] cp
 *          Pointer to a communicating connection object.
 * @param[out] bufpp
 *          Indirect pointer to the received data buffer.
 * @param[in,out] buflenp
 *          Pointer to the buffer length in bytes, with the following meaning:
 *          - At call, it indicates the maximum length of the received data.
 *          - At return, it is the actual length of the received data.
 * @param[in] remaddrp
 *          Pointer to the remote address descriptor.
 * @return
 *          Error code.
 */
uros_err_t  urosConnRecvFrom(UrosConn *cp,
                             void **bufpp, size_t *buflenp,
                             const UrosAddr *remaddrp) {

  return uros_lld_conn_recvfrom(cp, bufpp, buflenp, remaddrp);
}

/**
 * @brief   Sends some data.
 * @details Sends the buffered data to the remote address. The data is copied
 *          to an internal buffer while streaming.
 * @see     uros_lld_conn_send()
 * @warning Not conceptually equivalent to POSIX send(), as all the data
 *          is granted to be sent with a single call to this function
 *          (if no connection errors occurred).
 *
 * @pre     The connection must be open and working.
 * @post    All the buffered data ise sent.
 *
 * @param[in,out] cp
 *          Pointer to a communicating connection object.
 * @param[in] bufp
 *          Pointer to the buffered data to be sent.
 * @param[in] buflen
 *          Length of the buffered data, in bytes.
 * @return
 *          Error code.
 */
uros_err_t  urosConnSend(UrosConn *cp,
                         const void *bufp, size_t buflen) {

  return uros_lld_conn_send(cp, bufp, buflen);
}

/**
 * @brief   Sends some data.
 * @details Sends the buffered data to the remote address. The data is not
 *          copied to internal buffers.
 * @see     uros_lld_conn_sendconst()
 * @note    If the low-level connectivity library does not support <i>no-copy</i>
 *          streaming, this is simply an alias to @p urosConnSend().
 * @warning Not conceptually equivalent to POSIX send(), as all the data
 *          is granted to be sent with a single call to this function
 *          (if no connection errors occurred).
 *
 * @pre     The buffered data is granted not to change while it is still being
 *          streamed by the asynchronous low-level library thread.
 * @pre     The connection must be open and working.
 * @post    All the buffered data ise sent.
 *
 * @param[in,out] cp
 *          Pointer to a communicating connection object.
 * @param[in] bufp
 *          Pointer to the buffered data to be sent.
 * @param[in] buflen
 *          Length of the buffered data, in bytes.
 * @return
 *          Error code.
 */
uros_err_t  urosConnSendConst(UrosConn *cp,
                              const void *bufp, size_t buflen) {

  return uros_lld_conn_sendconst(cp, bufp, buflen);
}

/**
 * @brief   Sends some data to a remote address.
 * @details Sends the buffered data to the remote address. The data is copied
 *          to an internal buffer while streaming.
 * @see     uros_lld_conn_sendto()
 * @note    Not available for TCP connections.
 * @warning Not conceptually equivalent to POSIX send(), as all the data
 *          is granted to be sent with a single call to this function
 *          (if no connection errors occurred).
 *
 * @pre     The connection must be open and working.
 * @post    All the buffered data ise sent.
 *
 * @param[in,out] cp
 *          Pointer to a communicating connection object.
 * @param[in] bufp
 *          Pointer to the buffered data to be sent.
 * @param[in] buflen
 *          Length of the buffered data, in bytes.
 * @param[in] remaddrp
 *          Pointer to the remote address descriptor.
 * @return
 *          Error code.
 */
uros_err_t  urosConnSendTo(UrosConn *cp,
                           const void *bufp, size_t buflen,
                           const UrosAddr *remaddrp) {

  return uros_lld_conn_sendto(cp, bufp, buflen, remaddrp);
}

/**
 * @brief   Sends some data to a remote address.
 * @details Sends the buffered data to the remote address. The data is not
 *          copied to internal buffers.
 * @see     uros_lld_conn_sendtoconst()
 * @note    Not available for TCP connections.
 * @note    If the low-level connectivity library does not support <i>no-copy</i>
 *          streaming, this is simply an alias to @p urosConnSend().
 * @warning Not conceptually equivalent to POSIX send(), as all the data
 *          is granted to be sent with a single call to this function
 *          (if no connection errors occurred).
 *
 * @pre     The buffered data is granted not to change while it is still being
 *          streamed by the asynchronous low-level library thread.
 * @pre     The connection must be open and working.
 * @post    All the buffered data ise sent.
 *
 * @param[in,out] cp
 *          Pointer to a communicating connection object.
 * @param[in] bufp
 *          Pointer to the buffered data to be sent.
 * @param[in] buflen
 *          Length of the buffered data, in bytes.
 * @param[in] remaddrp
 *          Pointer to the remote address descriptor.
 * @return
 *          Error code.
 */
uros_err_t  urosConnSendToConst(UrosConn *cp,
                                const void *bufp, size_t buflen,
                                const UrosAddr *remaddrp) {

  return uros_lld_conn_sendtoconst(cp, bufp, buflen, remaddrp);
}

/**
 * @brief   Terminates soem ends of a full-duplex channel.
 * @details The reception or the transission channels are closed.
 * @see     uros_lld_conn_shutdown
 * @note    Conceptually equivalent to POSIX @p shutdown().
 * @warning The connection is not actually closed after a call to this
 *          function, even with both @p rx and @p tx @p true.
 *
 * @pre     The connection must be open and working.
 * @post    All the buffered data ise sent.
 *
 * @param[in,out] cp
 *          Pointer to a communicating @p UrosConn object.
 * @param[in] rx
 *          Close the receiver channel.
 * @param[in] tx
 *          Close the reansmitter channel.
 * @return
 *          Error code.
 */
uros_err_t urosConnShutdown(UrosConn *cp,
                            uros_bool_t rx, uros_bool_t tx) {

  return uros_lld_conn_shutdown(cp, rx, tx);
}

/**
 * @brief   Closes a connection.
 * @details Terminates all connection channels and releases any internal
 *          buffers.
 * @see     uros_lld_conn_close()
 * @note    Conceptually equivalent to POSIX @p close().
 *
 * @pre     The connection must be open and working.
 * @post    The connection is closed.
 * @post    Any internal buffers are freed.
 *
 * @param[in,out] cp
 *          Pointer to a communicating @p UrosConn object.
 * @return
 *          Error code.
 */
uros_err_t urosConnClose(UrosConn *cp) {

  return uros_lld_conn_close(cp);
}

/**
 * @brief   Gets the <i>Nagle</i> algorithm state.
 * @details Checks if the <i>Nagle</i> algorithm is enabled on the provided
 *          connection.
 * @note    Available only for TCP connections.
 *
 * @param[in] cp
 *          Pointer to a valid @p UrosConn object.
 * @param[out] enablep
 *          Pointer to the check result.
 * @return
 *          Error code.
 */
uros_err_t urosConnGetTcpNoDelay(UrosConn *cp, uros_bool_t *enablep) {

  return uros_lld_conn_gettcpnodelay(cp, enablep);
}

/**
 * @brief   Sets the <i>Nagle</i> algorithm state.
 * @details When the <i>Nagle</i> algorithm is enabled, small packets are not
 *          immediately sent, but possibl enqueued, so that the TCP overhead
 *          is minimized. When it is disabled, even one-byte packets are
 *          immediately sent, to minimize latency despite the TCP overhead.
 * @note    Available only for TCP connections.
 *
 * @param[in,out] cp
 *          Pointer to a valid @p UrosConn object.
 * @param[in] enable
 *          <i>Nagle</i> algorithm activation switch.
 * @return
 *          Error code.
 */
uros_err_t urosConnSetTcpNoDelay(UrosConn *cp, uros_bool_t enable) {

  return uros_lld_conn_settcpnodelay(cp, enable);
}

/**
 * @brief   Gets the receiver timeout.
 * @warning May not be implemented on all platforms.
 *
 * @param[in] cp
 *          Pointer to a valid @p UrosConn object.
 * @param[out] msp
 *          Pointer to the timeout value, in milliseconds.
 * @return
 *          Error code.
 */
uros_err_t urosConnGetRecvTimeout(UrosConn *cp, uint32_t *msp) {

  return uros_lld_conn_getrecvtimeout(cp, msp);
}

/**
 * @brief   Sets the receiver timeout.
 * @warning May not be implemented on all platforms.
 *
 * @param[in] cp
 *          Pointer to a valid @p UrosConn object.
 * @param[out] ms
 *          Timeout value, in milliseconds.
 * @return
 *          Error code.
 */
uros_err_t urosConnSetRecvTimeout(UrosConn *cp, uint32_t ms) {

  return uros_lld_conn_setrecvtimeout(cp, ms);
}

/**
 * @brief   Gets the sender timeout.
 * @warning May not be implemented on all platforms.
 *
 * @param[in] cp
 *          Pointer to a valid @p UrosConn object.
 * @param[out] msp
 *          Pointer to the timeout value, in milliseconds.
 * @return
 *          Error code.
 */
uros_err_t urosConnGetSendTimeout(UrosConn *cp, uint32_t *msp) {

  return uros_lld_conn_getsendtimeout(cp, msp);
}

/**
 * @brief   Sets the sender timeout.
 * @warning May not be implemented on all platforms.
 *
 * @param[in] cp
 *          Pointer to a valid @p UrosConn object.
 * @param[out] ms
 *          Timeout value, in milliseconds.
 * @return
 *          Error code.
 */
uros_err_t urosConnSetSendTimeout(UrosConn *cp, uint32_t ms) {

  return uros_lld_conn_setsendtimeout(cp, ms);
}

/**
 * @brief   Gets the last low-level connectivity library error text.
 *
 * @param[in] cp
 *          Pointer to the connection which generated the error to be analyzed.
 * @return
 *          Textural description of the last low-level connectovity error,
 *          null-terminated string.
 */
const char *urosConnLastErrorText(const UrosConn *cp) {

  return uros_lld_conn_lasterrortext(cp);
}

/** @} */
