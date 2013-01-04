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
 * @file    uros_lld_conn.c
 * @author  Andrea Zoppi <texzk@email.it>
 *
 * @brief   Low-level connectivity features implementation.
 */

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include "../../../include/lld/uros_lld_conn.h"

#include <lwip/opt.h>
#include <lwip/init.h>
#include <lwip/sys.h>
#include <lwip/mem.h>
#include <lwip/memp.h>
#include <lwip/api.h>
#include <lwip/inet_chksum.h>
#include <lwip/tcpip.h>
#include <lwip/tcp_impl.h>
#include <lwip/stats.h>

#include <string.h>

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

  err_t err;
  char *hostnameszp;
  struct ip_addr addr;

  urosAssert(urosStringNotEmpty(hostnamep));
  urosAssert(ipp != NULL);

  hostnameszp = urosAlloc(hostnamep->length + 1);
  if (hostnameszp == NULL) { return UROS_ERR_NOMEM; }
  memcpy(hostnameszp, hostnamep->datap, hostnamep->length);
  hostnameszp[hostnamep->length] = 0;
  err = netconn_gethostbyname(hostnameszp, &addr);
  urosFree(hostnameszp);
  ipp->dword = ntohl(addr.addr);
  return (err == ERR_OK) ? UROS_OK : UROS_ERR_BADPARAM;
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

  cp->netconnp = NULL;
  cp->lwiperr = ERR_OK;
  cp->recvnetbufp = NULL;
  cp->recvchkoff = 0;
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

  return cp != NULL && cp->netconnp != NULL;
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
  urosAssert(cp->netconnp == NULL);

  if (protocol == UROS_PROTO_TCP) {
    cp->netconnp = netconn_new(NETCONN_TCP);
  } else if (protocol == UROS_PROTO_UDP) {
    cp->netconnp = netconn_new(NETCONN_UDP);
  } else {
    return UROS_ERR_BADPARAM;
  }
  if (cp->netconnp != NULL) {
    cp->protocol = protocol;
    return UROS_OK;
  } else {
    return UROS_ERR_NOMEM;
  }
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

  struct ip_addr locaddr;

  urosAssert(urosConnIsValid(cp));
  urosAssert(locaddrp != NULL);

  locaddr.addr = htonl(locaddrp->ip.dword);
  cp->lwiperr = netconn_bind(cp->netconnp, &locaddr, locaddrp->port);
  return (cp->lwiperr == ERR_OK) ? UROS_OK : UROS_ERR_BADCONN;
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

  struct netconn *netconnp;

  urosAssert(urosConnIsValid(cp));
  urosAssert(cp->protocol == UROS_PROTO_TCP ||
             cp->protocol == UROS_PROTO_UDP);
  urosAssert(spawnedp != NULL);
  urosAssert(spawnedp->netconnp == NULL);

  uros_lld_conn_objectinit(spawnedp);
  cp->lwiperr = netconn_accept(cp->netconnp, &netconnp);
  if (cp->lwiperr == ERR_OK) {
    spawnedp->locaddr = cp->locaddr;
    spawnedp->protocol = cp->protocol;
    if (cp->protocol == UROS_PROTO_TCP) {
      spawnedp->remaddr.ip.dword = ntohl(netconnp->pcb.tcp->remote_ip.addr);
      spawnedp->remaddr.port = netconnp->pcb.tcp->remote_port;
    } else if (cp->protocol == UROS_PROTO_UDP) {
      /* TODO */
    }
    spawnedp->netconnp = netconnp;
    return UROS_OK;
  } else {
    return UROS_ERR_BADCONN;
  }
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

  cp->lwiperr = netconn_listen_with_backlog(cp->netconnp, (u8_t)backlog);
  return (cp->lwiperr == ERR_OK) ? UROS_OK : UROS_ERR_BADCONN;
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

  struct ip_addr addr;

  urosAssert(urosConnIsValid(cp));
  urosAssert(remaddrp != NULL);

  addr.addr = htonl(remaddrp->ip.dword);
  cp->lwiperr = netconn_connect(cp->netconnp, &addr, remaddrp->port);
  return (cp->lwiperr == ERR_OK) ? UROS_OK : UROS_ERR_BADPARAM;
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

  void *datap;
  size_t datalen, pending;
  uros_bool_t refill = UROS_FALSE;
  u16_t datalen16;

  urosAssert(urosConnIsValid(cp));
  urosAssert(bufpp != NULL);
  urosAssert(buflenp != NULL);

  if (cp->recvnetbufp == NULL) {
    refill = UROS_TRUE;
  } else {
    /* Refer to the current chunk.*/
    netbuf_data(cp->recvnetbufp, &datap, &datalen16);
    datalen = (size_t)datalen16;
    urosAssert(cp->recvchkoff <= datalen);
    if (cp->recvchkoff == datalen) {
      /* Free the previous buffer and request for a new one.*/
      netbuf_delete(cp->recvnetbufp);
      refill = UROS_TRUE;
    }
  }
  if (refill) {
    /* Receive a new buffer.*/
    cp->recvnetbufp = NULL;
    cp->recvchkoff = 0;
    cp->lwiperr = netconn_recv(cp->netconnp, &cp->recvnetbufp);
    if (cp->lwiperr != ERR_OK) {
      cp->recvnetbufp = NULL;
      switch (cp->lwiperr) {
      case ERR_RST:
      case ERR_CLSD:    { return UROS_ERR_EOF; }
      case ERR_TIMEOUT: { return UROS_ERR_TIMEOUT; }
      case ERR_MEM:     { return UROS_ERR_NOMEM; }
      default:          { return UROS_ERR_BADCONN; }
      }
    }
  }

  /* Refer to the current chunk position.*/
  netbuf_data(cp->recvnetbufp, &datap, &datalen16);
  datalen = (size_t)datalen16;
  *bufpp = datap + cp->recvchkoff;

  /* Check if the requested length fits and move the offset accordingly.*/
  pending = datalen - cp->recvchkoff;
  if (*buflenp >= pending) {
    *buflenp = pending;
    cp->recvchkoff += pending;
  } else {
    cp->recvchkoff += *buflenp;
  }
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

  urosAssert(urosConnIsValid(cp));
  urosAssert(!(buflen > 0) || (bufp != NULL));

  if (buflen == 0) { return UROS_OK; }
  if (cp->protocol == UROS_PROTO_TCP) {
    cp->lwiperr = netconn_write(cp->netconnp, bufp, buflen, NETCONN_COPY);
  } else if (cp->protocol == UROS_PROTO_UDP) {
    /* TODO */
  } else {
    return UROS_ERR_BADPARAM;
  }
  switch (cp->lwiperr) {
  case ERR_OK:      { return UROS_OK; }
  case ERR_RST:
  case ERR_CLSD:    { return UROS_ERR_NOCONN; }
  case ERR_TIMEOUT: { return UROS_ERR_TIMEOUT; }
  case ERR_MEM:     { return UROS_ERR_NOMEM; }
  default:          { return UROS_ERR_BADCONN; }
  }
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

  if (buflen == 0) { return UROS_OK; }
  if (cp->protocol == UROS_PROTO_TCP) {
    cp->lwiperr = netconn_write(cp->netconnp, bufp, buflen, NETCONN_NOCOPY);
  } else if (cp->protocol == UROS_PROTO_UDP) {
    /* TODO */
  } else {
    return UROS_ERR_BADPARAM;
  }
  switch (cp->lwiperr) {
  case ERR_OK:      { return UROS_OK; }
  case ERR_RST:
  case ERR_CLSD:    { return UROS_ERR_NOCONN; }
  case ERR_TIMEOUT: { return UROS_ERR_TIMEOUT; }
  case ERR_MEM:     { return UROS_ERR_NOMEM; }
  default:          { return UROS_ERR_BADCONN; }
  }
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

  cp->lwiperr = netconn_shutdown(cp->netconnp, (u8_t)rx, (u8_t)tx);
  return (cp->lwiperr == ERR_OK) ? UROS_OK : UROS_ERR_BADCONN;
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

  /* Free any space allocated by the receiver.*/
  if (cp->recvnetbufp != NULL) {
    netbuf_delete(cp->recvnetbufp);
    cp->recvnetbufp = NULL;
  }
  cp->recvchkoff = 0;

  /* Close the connection, if still existing.*/
  if (cp->netconnp != NULL) {
    cp->lwiperr = netconn_close(cp->netconnp);
    netconn_delete(cp->netconnp);
    cp->netconnp = NULL;
  } else {
    cp->lwiperr = ERR_OK;
  }
  return (cp->lwiperr == ERR_OK) ? UROS_OK : UROS_ERR_BADCONN;
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

  /* FIXME: Lock the PCB variable.*/
  *enablep = !tcp_nagle_disabled(cp->netconnp->pcb.tcp);
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

  urosAssert(urosConnIsValid(cp));

  /* FIXME: Lock the PCB variable.*/
  if (enable) {
    tcp_nagle_enable(cp->netconnp->pcb.tcp);
  } else {
    tcp_nagle_disable(cp->netconnp->pcb.tcp);
  }
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

  urosAssert(urosConnIsValid(cp));
  urosAssert(msp != NULL);

  /* Get the receiver timeout value.*/
  *msp = (uint32_t)netconn_get_recvtimeout(cp->netconnp);
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

  urosAssert(urosConnIsValid(cp));

  /* Set the receiver timeout value.*/
  netconn_set_recvtimeout(cp->netconnp, (int)ms);
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

  urosAssert(urosConnIsValid(cp));
  urosAssert(msp != NULL);

  /* Get the sender timeout value.*/
  *msp = (uint32_t)netconn_get_sendtimeout(cp->netconnp);
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

  urosAssert(urosConnIsValid(cp));

  /* Set the sender timeout value.*/
  netconn_set_sendtimeout(cp->netconnp, (int)ms);
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

  urosAssert(cp != NULL);

#ifdef LWIP_DEBUG
  return lwip_strerr(cp->lwiperr);
#else
  (void)cp;
  return "<LWIP_ERROR>";
#endif
}
