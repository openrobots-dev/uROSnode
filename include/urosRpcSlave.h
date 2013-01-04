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
 * @file    urosRpcSlave.h
 * @author  Andrea Zoppi <texzk@email.it>
 *
 * @brief   XMLRPC Slave API functions.
 */

#ifndef _UROSSLAVE_H_
#define _UROSSLAVE_H_

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include "urosBase.h"
#include "urosRpcCall.h"
#include "urosTcpRos.h"

/*===========================================================================*/
/* TYPES & MACROS                                                            */
/*===========================================================================*/

/** @addtogroup rpc_types */
/** @{ */

/**
 * @brief   XMLRPC Slave API methods.
 */
typedef enum uros_rpcslave_methodid_t {

  UROS_RPCSM_GET_BUS_INFO,      /**< @brief @p getBusInfo() command.*/
  UROS_RPCSM_GET_BUS_STATS,     /**< @brief @p getBusStatus() command.*/
  UROS_RPCSM_GET_MASTER_URI,    /**< @brief @p getMasterUri() command.*/
  UROS_RPCSM_GET_PID,           /**< @brief @p getPid() command.*/
  UROS_RPCSM_GET_PUBLICATIONS,  /**< @brief @p getPublications() command.*/
  UROS_RPCSM_GET_SUBSCRIPTIONS, /**< @brief @p getSubscriptions() command.*/
  UROS_RPCSM_PARAM_UPDATE,      /**< @brief @p paramUpdate() command.*/
  UROS_RPCSM_PUBLISHER_UPDATE,  /**< @brief @p publisherUpdate() command.*/
  UROS_RPCSM_REQUEST_TOPIC,     /**< @brief @p requestTopic() command.*/
  UROS_RPCSM_SHUTDOWN,          /**< @brief @p shutdown() command.*/

  UROS_RPCSM__LENGTH
} uros_rpcslave_methodid_t;

/** @} */

/*===========================================================================*/
/* GLOBAL PROTOTYPES                                                         */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

uros_err_t urosRpcSlaveConnectToPublishers(const UrosString *topicp,
                                           const UrosList *addrlstp);
uros_err_t urosRpcSlaveListenerThread(void *data);
uros_err_t urosRpcSlaveServerThread(UrosConn *csp);

#ifdef __cplusplus
}
#endif
#endif /* _UROSSLAVE_H_ */
