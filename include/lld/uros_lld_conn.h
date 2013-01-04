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
 * @brief   Low-level connectivity features of the middleware.
 */

#ifndef _UROS_LLD_CONN_H_
#define _UROS_LLD_CONN_H_

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include "../urosConn.h"

/*===========================================================================*/
/* GLOBAL PROTOTYPES                                                         */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

uros_err_t uros_lld_hostnametoip(const UrosString *hostnamep,
                                 UrosIp *ipp);
void uros_lld_conn_objectinit(UrosConn *cp);
uros_bool_t uros_lld_conn_isvalid(UrosConn *cp);
uros_err_t uros_lld_conn_create(UrosConn *cp, uros_connproto_t protocol);
uros_err_t uros_lld_conn_bind(UrosConn *cp, const UrosAddr *locaddrp);
uros_err_t uros_lld_conn_accept(UrosConn *cp, UrosConn *spawnedp);
uros_err_t uros_lld_conn_listen(UrosConn *cp, uros_cnt_t backlog);
uros_err_t uros_lld_conn_connect(UrosConn *cp, const UrosAddr *remaddrp);
uros_err_t  uros_lld_conn_recv(UrosConn *cp,
                               void **bufpp, size_t *buflenp);
uros_err_t  uros_lld_conn_recvfrom(UrosConn *cp,
                                   void **bufpp, size_t *buflenp,
                                   const UrosAddr *remaddrp);
uros_err_t  uros_lld_conn_send(UrosConn *cp,
                               const void *bufp, size_t buflen);
uros_err_t  uros_lld_conn_sendconst(UrosConn *cp,
                                    const void *bufp, size_t buflen);
uros_err_t  uros_lld_conn_sendto(UrosConn *cp,
                                 const void *bufp, size_t buflen,
                                 const UrosAddr *remaddrp);
uros_err_t  uros_lld_conn_sendtoconst(UrosConn *cp,
                                      const void *bufp, size_t buflen,
                                      const UrosAddr *remaddrp);
uros_err_t uros_lld_conn_shutdown(UrosConn *cp,
                                  uros_bool_t read, uros_bool_t write);
uros_err_t uros_lld_conn_close(UrosConn *cp);

uros_err_t uros_lld_conn_gettcpnodelay(UrosConn *cp, uros_bool_t *enablep);
uros_err_t uros_lld_conn_settcpnodelay(UrosConn *cp, uros_bool_t enable);
uros_err_t uros_lld_conn_getrecvtimeout(UrosConn *cp, uint32_t *msp);
uros_err_t uros_lld_conn_setrecvtimeout(UrosConn *cp, uint32_t ms);
uros_err_t uros_lld_conn_getsendtimeout(UrosConn *cp, uint32_t *msp);
uros_err_t uros_lld_conn_setsendtimeout(UrosConn *cp, uint32_t ms);

const char *uros_lld_conn_lasterrortext(const UrosConn *cp);

#ifdef __cplusplus
}
#endif
#endif /* _UROS_LLD_CONN_H_ */
