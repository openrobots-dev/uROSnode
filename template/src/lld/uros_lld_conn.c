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
 * @file    uros_lld_conn.c
 * @author  Andrea Zoppi <texzk@email.it>
 *
 * @brief   Low-level connectivity features implementation (TEMPLATE).
 */

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include "../../../include/urosBase.h"
#include "../../../include/urosUser.h"
#include "../../../include/urosConn.h"
#include "../../../include/lld/uros_lld_conn.h"

/*===========================================================================*/
/* LOCAL TYPES & MACROS                                                      */
/*===========================================================================*/

#if UROS_CONN_C_USE_ASSERT == UROS_FALSE && !defined(__DOXYGEN__)
#undef urosAssert
#define urosAssert(expr)
#endif

/*===========================================================================*/
/* GLOBAL FUNCTIONS                                                          */
/*===========================================================================*/

/** @addtogroup conn_lld_funcs */
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
uros_err_t uros_lld_hostnametoip(const UrosString *hostnamep,
                                 UrosIp *ipp) {

  /* TODO: Hostname to ip conversion.*/
  (void)hostnamep;
  (void)ipp;
}

/**
 * @brief   Initializes a connection object.
 * @details Invalidates the connection fields and initializes the
 *          low-level ones.
 *
 * @param[in,out] cp
 *          Pointer to an allocated @p UrosConn object.
 */
void uros_lld_conn_objectinit(UrosConn *cp) {

  urosAssert(cp != NULL);

  /* TODO: Low-level connection initialization (see UrosConn__LLD).*/
  (void)cp;
}

/**
 * @brief   Checks if pointing to a valid connection object.
 * @details @p cp points to a valid connection if it is not @p NULL and its
 *          low-level fields are in a valid state.
 *
 * @param[in] cp
 *          Pointer to a presumed valid @p UrosConn object.
 * @return
 *          @p true if @p cp addresses a valid @p UrosConn object.
 */
uros_bool_t uros_lld_conn_isvalid(UrosConn *cp) {

  /* TODO: Check if the @p cp points to a connected object.*/
  return cp != NULL /* && connection alive */;
}

/**
 * @brief   Creates a new connection.
 * @details The connection object is initialized for a new connection.
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
uros_err_t uros_lld_conn_create(UrosConn *cp, uros_connproto_t protocol) {

  urosAssert(cp != NULL);
  urosAssert(protocol == UROS_PROTO_TCP ||
             protocol == UROS_PROTO_UDP);
  urosAssert(cp->socket == -1);

  /* TODO: Create a connection object with the requested protocol.*/
  (void)cp;
  (void)protocol;
  return UROS_ERR_NOTIMPL;
}

/**
 * @brief   Binds to a local address.
 * @details The local side of a connection object is bound to the provided
 *          address.
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
uros_err_t uros_lld_conn_bind(UrosConn *cp, const UrosAddr *locaddrp) {

  urosAssert(urosConnIsValid(cp));
  urosAssert(locaddrp != NULL);

  /* TODO: Bind the connection object to the requeste local address.*/
  (void)cp;
  (void)locaddrp;
  return UROS_ERR_NOTIMPL;
}

/**
 * @brief   Accepts an incoming connection.
 * @details A listener connection waits until a remote connection is
 *          instantiated. A new dedicated connection channel is spawned
 *          and returned, and the spawning connection is put back into
 *          listening state.
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
uros_err_t uros_lld_conn_accept(UrosConn *cp, UrosConn *spawnedp) {

  urosAssert(urosConnIsValid(cp));
  urosAssert(spawnedp != NULL);
  urosAssert(spawnedp->socket == -1);

  /* TODO: Accept the incoming connection and setup the spawned object with
   *       the new connection information.*/
  (void)cp;
  (void)spawnedp;
  return UROS_ERR_NOTIMPL;
}

/**
 * @brief   Initializes the listening mode.
 * @details The connection object is initialized for listening.
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
uros_err_t uros_lld_conn_listen(UrosConn *cp, uros_cnt_t backlog) {

  urosAssert(urosConnIsValid(cp));

  /* TODO: Make the connection object listen to incoming connection requests.*/
  (void)cp;
  (void)backlog;
  return UROS_ERR_NOTIMPL;
}

/**
 * @brief   Connects to a remote address.
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
uros_err_t uros_lld_conn_connect(UrosConn *cp, const UrosAddr *remaddrp) {

  urosAssert(urosConnIsValid(cp));
  urosAssert(remaddrp != NULL);

  /* TODO: Connect to the requested remote address.*/
  (void)cp;
  (void)remaddrp;
  return UROS_ERR_NOTIMPL;
}

/**
 * @brief   Receives some data.
 * @details The connection object waits until some data is received from the
 *          remote address. The pointer to the received data is returned,
 *          with a length up to the requested one.
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
uros_err_t uros_lld_conn_recv(UrosConn *cp,
                              void **bufpp, size_t *buflenp) {

  urosAssert(urosConnIsValid(cp));
  urosAssert(bufpp != NULL);
  urosAssert(buflenp != NULL);

  /* TODO: Receive a new buffer from the remote address.*/
  (void)cp;
  (void)bufpp;
  (void)buflenp;
  return UROS_ERR_NOTIMPL;
}

/**
 * @brief   Receives some data from a remote address.
 * @details The connection object waits until some data is received from the
 *          remote address. The pointer to the received data is returned,
 *          with a length up to the requested one.
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
uros_err_t uros_lld_conn_recvfrom(UrosConn *cp,
                                  void **bufpp, size_t *buflenp,
                                  const UrosAddr *remaddrp) {

  urosAssert(urosConnIsValid(cp));
  urosAssert(bufpp != NULL);
  urosAssert(buflenp != NULL);
  urosAssert(remaddrp != NULL);

  /* TODO: Receive a new buffer from the remote address.*/
  (void)cp;
  (void)bufpp;
  (void)buflenp;
  (void)remaddrp;
  return UROS_ERR_NOTIMPL;
}

/**
 * @brief   Sends some data.
 * @details Sends the buffered data to the remote address. The data is copied
 *          to an internal buffer while streaming.
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
uros_err_t uros_lld_conn_send(UrosConn *cp,
                              const void *bufp, size_t buflen) {

  urosAssert(urosConnIsValid(cp));
  urosAssert(!(buflen > 0) || (bufp != NULL));

  /* TODO: Send the buffer data to the remote address.*/
  (void)cp;
  (void)bufp;
  (void)buflen;
  return UROS_ERR_NOTIMPL;
}

/**
 * @brief   Sends some data.
 * @details Sends the buffered data to the remote address. The data is not
 *          copied to internal buffers.
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
uros_err_t uros_lld_conn_sendconst(UrosConn *cp,
                                   const void *bufp, size_t buflen) {

  urosAssert(urosConnIsValid(cp));
  urosAssert(!(buflen > 0) || (bufp != NULL));

  /* TODO: Send the buffer data to the remote address.*/
  (void)cp;
  (void)bufp;
  (void)buflen;
  return UROS_ERR_NOTIMPL;
}

/**
 * @brief   Sends some data to a remote address.
 * @details Sends the buffered data to the remote address. The data is copied
 *          to an internal buffer while streaming.
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
uros_err_t uros_lld_conn_sendto(UrosConn *cp,
                                const void *bufp, size_t buflen,
                                const UrosAddr *remaddrp) {

  urosAssert(urosConnIsValid(cp));
  urosAssert(!(buflen > 0) || (bufp != NULL));
  urosAssert(remaddrp != NULL);

  /* TODO: Send the buffer data to the remote address.*/
  (void)cp;
  (void)bufp;
  (void)buflen;
  (void)remaddrp;
  return UROS_ERR_NOTIMPL;
}

/**
 * @brief   Sends some data to a remote address.
 * @details Sends the buffered data to the remote address. The data is not
 *          copied to internal buffers.
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
uros_err_t uros_lld_conn_sendtoconst(UrosConn *cp,
                                     const void *bufp, size_t buflen,
                                     const UrosAddr *remaddrp) {

  urosAssert(urosConnIsValid(cp));
  urosAssert(!(buflen > 0) || (bufp != NULL));
  urosAssert(remaddrp != NULL);

  /* TODO: Send the buffer data to the remote address.*/
  (void)cp;
  (void)bufp;
  (void)buflen;
  (void)remaddrp;
  return UROS_ERR_NOTIMPL;
}

/**
 * @brief   Terminates soem ends of a full-duplex channel.
 * @details The reception or the transission channels are closed.
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
uros_err_t uros_lld_conn_shutdown(UrosConn *cp,
                                  uros_bool_t rx, uros_bool_t tx) {

  urosAssert(urosConnIsValid(cp));

  /* TODO: Turn off the requested connection channels.*/
  (void)cp;
  (void)rx;
  (void)tx;
  return UROS_ERR_NOTIMPL;
}

/**
 * @brief   Closes a connection.
 * @details Terminates all connection channels and releases any internal
 *          buffers.
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
uros_err_t uros_lld_conn_close(UrosConn *cp) {

  urosAssert(cp != NULL);

  /* TODO: Close the connection and invalidate the object.*/
  (void)cp;
  return UROS_ERR_NOTIMPL;
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
uros_err_t uros_lld_conn_gettcpnodelay(UrosConn *cp, uros_bool_t *enablep) {

  urosAssert(urosConnIsValid(cp));
  urosAssert(enablep != NULL);

  /* TODO: Get the TCP_NODELAY flag value.*/
  (void)cp;
  (void)enablep;
  return UROS_ERR_NOTIMPL;
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
uros_err_t uros_lld_conn_settcpnodelay(UrosConn *cp, uros_bool_t enable) {

  urosAssert(urosConnIsValid(cp));

  /* TODO: Det the TCP_NODELAY flag value.*/
  (void)cp;
  (void)enable;
  return UROS_ERR_NOTIMPL;
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
uros_err_t uros_lld_conn_getrecvtimeout(UrosConn *cp, uint32_t *msp) {

  urosAssert(urosConnIsValid(cp));
  urosAssert(msp != NULL);

  /* TODO: Get the receiver timeout value.*/
  (void)cp;
  (void)msp;
  return UROS_ERR_NOTIMPL;
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
uros_err_t uros_lld_conn_setrecvtimeout(UrosConn *cp, uint32_t ms) {

  urosAssert(urosConnIsValid(cp));

  /* TODO: Set the receiver timeout value.*/
  (void)cp;
  (void)ms;
  return UROS_ERR_NOTIMPL;
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
uros_err_t uros_lld_conn_getsendtimeout(UrosConn *cp, uint32_t *msp) {

  urosAssert(urosConnIsValid(cp));
  urosAssert(msp != NULL);

  /* TODO: Get the transmitter timeout value.*/
  (void)cp;
  (void)msp;
  return UROS_ERR_NOTIMPL;
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
uros_err_t uros_lld_conn_setsendtimeout(UrosConn *cp, uint32_t ms) {

  urosAssert(urosConnIsValid(cp));

  /* TODO: Set the transmitter timeout value.*/
  (void)cp;
  (void)ms;
  return UROS_ERR_NOTIMPL;
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
const char *uros_lld_conn_lasterrortext(const UrosConn *cp) {

  urosAssert(cp != NULL);

  /* TODO: Return the last error text.*/
  (void)cp;
  return "UROS_ERR_NOTIMPL";
}

/** @} */
