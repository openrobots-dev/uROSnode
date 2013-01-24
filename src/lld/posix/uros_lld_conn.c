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
 * @brief   Low-level connectivity features implementation.
 */

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include "../../../include/lld/uros_lld_conn.h"
#include "../../../include/urosUser.h"

#if !defined(_XOPEN_SOURCE)
#error "_XOPEN_SOURCE not defined, >= 600 expected"
#endif
#if _XOPEN_SOURCE < 600
#error "_XOPEN_SOURCE >= 600 required"
#endif

#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <fcntl.h>

/*===========================================================================*/
/* LOCAL TYPES & MACROS                                                      */
/*===========================================================================*/

#if UROS_CONN_C_USE_ASSERT == UROS_FALSE && !defined(__DOXYGEN__)
#undef urosAssert
#define urosAssert(expr)
#endif

/**
 * @brief   Receiving buffer length, in bytes.
 */
#if !defined(UROS_CONN_RECVBUFLEN) || defined(__DOXYGEN__)
#define UROS_CONN_RECVBUFLEN    256
#endif

/*===========================================================================*/
/* LOCAL FUNCTIONS                                                           */
/*===========================================================================*/

/**
 * @brief   Receive with timeout.
 * @details Works like @p recv(), but it also checks for timeout.
 *
 * @param fd
 *          Socket descriptor.
 * @param bufp
 *          Pointer to the buffer chunk.
 * @param buflen
 *          Length of the buffer chunk. Can be @p 0.
 * @param flags
 *          Flags for @p recv().
 * @param ms
 *          Timeout in milliseconds, @p 0 for blocking behavior.
 * @return
 *          Number of bytes received, or error code.
 * @retval > 0
 *          Number of bytes received.
 * @retval 0
 *          Connection closed by peer.
 * @retval -1
 *          Socket error, see @p errno. It @e will raise @p EAGAIN or
 *          @p EWOULDBLOCK when expected by a non-blocking socket.
 * @retval -2
 *          Operation timed out.
 */
ssize_t recv_to(int sock, void *bufp, size_t buflen, int flags,
                uint32_t ms) {

  ssize_t nb;
  int err, iof;
  struct timeval tv;
  fd_set fdset;

  if (ms == 0) {
    nb = recv(sock, bufp, buflen, flags);
    return (nb >= 0) ? nb : -1;
  }

  FD_ZERO(&fdset);
  FD_SET(sock, &fdset);
  tv.tv_sec = (time_t)(ms / 1000);
  tv.tv_usec = (time_t)((ms % 1000) * 1000);
  err = select(sock + 1, &fdset, NULL, NULL, &tv);
  if (err < 0) { return -1; }
  if (err > 0 && FD_ISSET(sock, &fdset)) {
    iof = fcntl(sock, F_GETFL, 0);
    if (iof != -1) { fcntl(sock, F_SETFL, iof | O_NONBLOCK); }
    nb = recv(sock, bufp, buflen, flags);
    if (iof != -1) { fcntl(sock, F_SETFL, iof); }
    return (nb >= 0) ? nb : -1;
  }
  return -2;
}

/**
 * @brief   Sends with timeout.
 * @details Works like @p send(), but it also checks for timeout.
 *
 * @param fd
 *          Socket descriptor.
 * @param bufp
 *          Pointer to the buffer chunk.
 * @param buflen
 *          Length of the buffer chunk. Can be @p 0.
 * @param flags
 *          Flags for @p send().
 * @param ms
 *          Timeout in milliseconds, @p 0 for blocking.
 * @return
 *          Number of bytes sent, or error code.
 * @retval >= 0
 *          Number of bytes sent.
 * @retval -1
 *          Socket error, see @p errno. It <i>will</i> raise @p EAGAIN or
 *          @p EWOULDBLOCK when expected by a non-blocking socket.
 * @retval -2
 *          Operation timed out.
 */
ssize_t send_to(int sock, const void *bufp, size_t buflen, int flags,
                uint32_t ms) {

  ssize_t nb;
  int err, iof;
  struct timeval tv;
  fd_set fdset;

  if (ms == 0) {
    nb = send(sock, bufp, buflen, flags);
    return (nb >= 0) ? nb : -1;
  }

  do {
    iof = fcntl(sock, F_GETFL, 0);
    if (iof != -1) { fcntl(sock, F_SETFL, iof | O_NONBLOCK); }
    nb = send(sock, bufp, buflen, flags);
    if (iof != -1) { fcntl(sock, F_SETFL, iof); }
    if (nb >= 0) { return nb; }
    else {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        FD_ZERO(&fdset);
        FD_SET(sock, &fdset);
        tv.tv_sec = (time_t)(ms / 1000);
        tv.tv_usec = (time_t)((ms % 1000) * 1000);
        err = select(sock + 1, &fdset, NULL, NULL, &tv);
        if (err < 0) { return -1; }
      } else {
        return -1;
      }
    }
  } while (err > 0 && FD_ISSET(sock, &fdset));
  return -2;
}

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

  struct addrinfo hints, *res;
  int status;
  char *namep;
  uros_err_t err;
  struct sockaddr_in *ipv4;

  urosAssert(urosStringNotEmpty(hostnamep));
  urosAssert(ipp != NULL);

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  namep = (char*)urosAlloc(NULL, hostnamep->length + 1);
  memcpy(namep, hostnamep->datap, hostnamep->length);
  namep[hostnamep->length] = 0;

  status = getaddrinfo(namep, NULL, &hints, &res);
  urosError(status != 0, { err = UROS_ERR_BADPARAM; goto _finally; },
            ("Socket error [%s] while getting address info\n",
             strerror(errno)));
  urosError(res == NULL, { err = UROS_ERR_BADPARAM; goto _finally; },
            ("Null getaddrinfo() result\n"));

  ipv4 = (struct sockaddr_in *)res->ai_addr;
  ipp->dword = ntohl(ipv4->sin_addr.s_addr);

  err = UROS_OK;
_finally:
  urosFree(namep);
  freeaddrinfo(res);
  return err;
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

  cp->socket = -1;
  cp->recvtimeout = 0;
  cp->sendtimeout = 0;
  cp->recvbufp = NULL;
  cp->recvbuflen = 0;
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

  return cp != NULL && cp->socket != -1;
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

  int sock, socktype;

  urosAssert(cp != NULL);
  urosAssert(protocol == UROS_PROTO_TCP ||
             protocol == UROS_PROTO_UDP);
  urosAssert(cp->socket == -1);

  /* Check for  avalid protocol.*/
  switch (protocol) {
  case UROS_PROTO_TCP: { socktype = SOCK_STREAM; break; }
  case UROS_PROTO_UDP: { socktype = SOCK_DGRAM; break; }
  default: {
    urosAssert(0 && "Unsupported protocol");
    return UROS_ERR_BADPARAM;
  }
  }

  /* Create the low-level socket.*/
  sock = socket(AF_INET, socktype, 0);
  urosError(sock < 0, return UROS_ERR_BADCONN,
            ("Cannot create a connection with protocol id %d\n",
             (int)protocol));

  /* Fill the connection record.*/
  cp->locaddr.ip.dword = UROS_ANY_IP;
  cp->locaddr.port = UROS_ANY_PORT;
  cp->remaddr.ip.dword = UROS_ANY_IP;
  cp->remaddr.port = UROS_ANY_PORT;

  cp->protocol = protocol;

  cp->socket = sock;
  return UROS_OK;
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

  struct sockaddr_in locaddr;
  int err, reuse = 1;

  urosAssert(urosConnIsValid(cp));
  urosAssert(locaddrp != NULL);

  err = setsockopt(cp->socket, SOL_SOCKET,  SO_REUSEADDR,
                   (char*)&reuse, sizeof(reuse));
  urosError(err != 0, return UROS_ERR_BADCONN,
            ("Socket error [%s] while reusing "UROS_ADDRFMT"\n",
             strerror(errno), UROS_ADDRARG(&cp->locaddr)));

  locaddr.sin_family = AF_INET;
  locaddr.sin_port = htons(locaddrp->port);
  locaddr.sin_addr.s_addr = htonl(locaddrp->ip.dword);
  memset(locaddr.sin_zero, 0, sizeof(locaddr.sin_zero));

  err = bind(cp->socket, (struct sockaddr *)&locaddr, sizeof(struct sockaddr_in));
  urosError(err != 0, return UROS_ERR_BADCONN,
            ("Socket error [%s] while binding as "UROS_ADDRFMT"\n",
             strerror(errno), UROS_ADDRARG(locaddrp)));

  cp->locaddr = *locaddrp;
  return UROS_OK;
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

  int remsock;
  struct sockaddr_in remaddr;
  socklen_t remsize = sizeof(struct sockaddr_in);

  urosAssert(urosConnIsValid(cp));
  urosAssert(spawnedp != NULL);
  urosAssert(spawnedp->socket == -1);

  remsock = accept(cp->socket, (struct sockaddr *)&remaddr, &remsize);
  urosError(remsock < 0, return UROS_ERR_BADCONN,
            ("Socket error [%s] while accepting as "UROS_ADDRFMT"\n",
             strerror(errno), UROS_ADDRARG(&cp->locaddr)));
  urosError(remsize != sizeof(struct sockaddr_in),
            { close(remsock); return UROS_ERR_BADCONN; },
            ("Wrong remote socket size (%u instead of %u)\n",
             (unsigned)remsize, (unsigned)sizeof(struct sockaddr_in)));

  spawnedp->locaddr = cp->locaddr;
  spawnedp->remaddr.port = ntohs(remaddr.sin_port);
  spawnedp->remaddr.ip.dword = ntohl(remaddr.sin_addr.s_addr);
  spawnedp->protocol = cp->protocol;
  spawnedp->socket = remsock;
  return UROS_OK;
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

  int err;

  urosAssert(urosConnIsValid(cp));

  err = listen(cp->socket, (int)backlog);
  urosError(err != 0, return UROS_ERR_BADCONN,
            ("Socket error [%s] while listening as "UROS_ADDRFMT"\n",
             strerror(errno), UROS_ADDRARG(&cp->locaddr)));
  return UROS_OK;
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

  int err;
  struct sockaddr_in remaddr;

  urosAssert(urosConnIsValid(cp));
  urosAssert(remaddrp != NULL);

  remaddr.sin_family = AF_INET;
  remaddr.sin_port = htons(remaddrp->port);
  remaddr.sin_addr.s_addr = htonl(remaddrp->ip.dword);
  memset(remaddr.sin_zero, 0, sizeof(remaddr.sin_zero));
  cp->remaddr = *remaddrp;

  err = connect(cp->socket, (struct sockaddr *)&remaddr, sizeof(remaddr));
  urosError(err == ETIMEDOUT, return UROS_ERR_NOCONN,
            ("Connection to "UROS_ADDRFMT" timed out\n",
             UROS_ADDRARG(remaddrp)));
  urosError(err == ECONNREFUSED, return UROS_ERR_NOCONN,
            ("Connection to "UROS_ADDRFMT" refused\n",
             UROS_ADDRARG(remaddrp)));
  urosError(err != 0, return UROS_ERR_NOCONN,
            ("Socket error [%s] while connecting to "UROS_ADDRFMT"\n",
             strerror(errno), UROS_ADDRARG(remaddrp)));

  return UROS_OK;
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

  ssize_t nb;

  urosAssert(urosConnIsValid(cp));
  urosAssert(bufpp != NULL);
  urosAssert(buflenp != NULL);

  if (*buflenp == 0) { return UROS_OK; }
  if (cp->recvbufp == NULL) {
    cp->recvbufp = urosAlloc(NULL, UROS_CONN_RECVBUFLEN);
    if (cp->recvbufp == NULL) { return UROS_ERR_NOMEM; }
    cp->recvbuflen = UROS_CONN_RECVBUFLEN;
  }
  if (*buflenp > cp->recvbuflen) { *buflenp = cp->recvbuflen; }
  do {
    nb = recv_to(cp->socket, cp->recvbufp, *buflenp, MSG_NOSIGNAL,
                 cp->recvtimeout);
  } while (nb == -1 && (errno == EAGAIN || errno == EWOULDBLOCK));
  urosError(nb == 0, return UROS_ERR_EOF,
            ("Socket closed by remote before receiving at most %u bytes from "
             UROS_ADDRFMT"\n",
             (unsigned)*buflenp, UROS_ADDRARG(&cp->remaddr)));
  urosError(nb == -1, return UROS_ERR_BADCONN,
            ("Socket error [%s] while receiving at most %u bytes from "
             UROS_ADDRFMT"\n",
             strerror(errno), (unsigned)*buflenp, UROS_ADDRARG(&cp->remaddr)));
  if (nb == -2) { return UROS_ERR_TIMEOUT; }

  *bufpp = cp->recvbufp;
  *buflenp = (size_t)nb;
  cp->recvlen += (size_t)nb;
  return UROS_OK;
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

  ssize_t nb;

  urosAssert(urosConnIsValid(cp));
  urosAssert(!(buflen > 0) || (bufp != NULL));

  while (buflen > 0) {
    do {
      nb = send_to(cp->socket, bufp, buflen, MSG_NOSIGNAL, cp->sendtimeout);
    } while (nb == -1 && (errno == EAGAIN || errno == EWOULDBLOCK));
    urosError(nb == -1, return UROS_ERR_BADCONN,
              ("Socket error [%s] while sending [%.*s] (%u bytes) to "
               UROS_ADDRFMT"\n",
               strerror(errno), (unsigned)buflen, (const char *)bufp,
               (unsigned)buflen, UROS_ADDRARG(&cp->remaddr)));
    if (nb == -2) { return UROS_ERR_TIMEOUT; }

    buflen -= (size_t)nb;
    bufp = (const void *)((const uint8_t *)bufp + (ptrdiff_t)nb);
    cp->sentlen += (size_t)nb;
  }
  return UROS_OK;
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

  return uros_lld_conn_send(cp, bufp, buflen);
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

  int err;

  urosAssert(urosConnIsValid(cp));

  if (rx && tx) { err = shutdown(cp->socket, SHUT_RDWR); }
  else if (rx)  { err = shutdown(cp->socket, SHUT_RD); }
  else if (tx)  { err = shutdown(cp->socket, SHUT_WR); }
  else          { err = 0; }
  urosError(err != 0, return UROS_ERR_BADCONN,
            ("Socket error [%s] while shutting down (RX=%d, TX=%d), self "UROS_ADDRFMT
             ", remote"UROS_ADDRFMT"\n", strerror(errno), (int)rx, (int)tx,
             UROS_ADDRARG(&cp->locaddr), UROS_ADDRARG(&cp->remaddr)));
  return UROS_OK;
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

  urosFree(cp->recvbufp);
  cp->recvbufp = NULL;
  cp->recvbuflen = 0;

  /* Close if not already closed by this library.*/
  if (cp->socket != -1) {
    int err = close(cp->socket);
    cp->socket = -1;
    urosError(err != 0, return UROS_ERR_BADCONN,
              ("Socket error [%s] while closing, self "UROS_ADDRFMT
               ", remote "UROS_ADDRFMT"\n", strerror(errno),
               UROS_ADDRARG(&cp->locaddr), UROS_ADDRARG(&cp->remaddr)));
  }
  return UROS_OK;
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

  int err, flag;
  socklen_t size = sizeof(int);

  urosAssert(urosConnIsValid(cp));
  urosAssert(enablep != NULL);

  urosError(cp->protocol != UROS_PROTO_TCP, return UROS_ERR_BADPARAM,
            ("Not a TCP/IP connection, self "UROS_ADDRFMT", remote "UROS_ADDRFMT"\n",
             UROS_ADDRARG(&cp->locaddr), UROS_ADDRARG(&cp->remaddr)));

  err = getsockopt(cp->socket, IPPROTO_TCP, TCP_NODELAY, &flag, &size);
  urosError(err != 0, return UROS_ERR_BADCONN,
            ("Socket error [%s] while getting TCP_NODELAY\n", strerror(errno)));
  urosError(size != sizeof(int), return UROS_ERR_BADCONN,
            ("Wrong <int> size, got %u, expected %u\n",
             (unsigned)size, (unsigned)sizeof(int)));

  *enablep = flag ? UROS_TRUE : UROS_FALSE;
  return UROS_OK;
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

  int err, flag = enable ? 1 : 0;

  urosAssert(urosConnIsValid(cp));

  urosError(cp->protocol != UROS_PROTO_TCP, return UROS_ERR_BADPARAM,
            ("Not a TCP/IP connection, self "UROS_ADDRFMT", remote "UROS_ADDRFMT"\n",
             UROS_ADDRARG(&cp->locaddr), UROS_ADDRARG(&cp->remaddr)));

  err = setsockopt(cp->socket, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int));
  urosError(err != 0, return UROS_ERR_BADCONN,
            ("Socket error [%s] while setting TCP_NODELAY to %d\n",
             strerror(errno), (int)enable));
  return UROS_OK;
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

  *msp = cp->recvtimeout;
  return UROS_OK;
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

  cp->recvtimeout = ms;
  return UROS_OK;
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

  *msp = cp->sendtimeout;
  return UROS_OK;
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

  cp->sendtimeout = ms;
  return UROS_OK;
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

  (void)cp;

  urosAssert(cp != NULL);

  return strerror(errno);
}

/** @} */
