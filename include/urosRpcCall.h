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
 * @file    urosRpcCall.h
 * @author  Andrea Zoppi <texzk@email.it>
 *
 * @brief   XMLRPC call functions.
 */

#ifndef _UROSRPC_H_
#define _UROSRPC_H_

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include "urosBase.h"
#include "urosConn.h"

/*===========================================================================*/
/* TYPES & MACROS                                                            */
/*===========================================================================*/

/** @addtogroup rpc_types */
/** @{ */

/**
 * @brief   XMLRPC status code.
 */
enum uros_rpccode_t {
  UROS_RPCC_ERROR       = -1,   /**< @brief Caller error, action not executed.*/
  UROS_RPCC_FAILURE     =  0,   /**< @brief Method error, possible side effects.*/
  UROS_RPCC_SUCCESS     =  1    /**< @brief Method completed successfully.*/
};

/**
 * @brief   XMLRPC parameter pclass id.
 */
typedef enum uros_rpcparamclass_t {

  UROS_RPCP_INT,                /**< @brief Signed 32-bits value.*/
  UROS_RPCP_BOOLEAN,            /**< @brief Boolean value (either @p 1 or @p 0).*/
  UROS_RPCP_STRING,             /**< @brief String value.*/
  UROS_RPCP_DOUBLE,             /**< @brief Double-precision floating point value.*/
  UROS_RPCP_BASE64,             /**< @brief Base64 data.*/
  UROS_RPCP_STRUCT,             /**< @brief Associative map.*/
  UROS_RPCP_ARRAY,              /**< @brief Generic array.*/

  UROS_RPCP__LENGTH
} uros_rpcparamclass_t;

struct uros_rpcparamlist_t;
/**
 * @brief   XMLRPC parameter value.
 * @details
 */
typedef struct UrosRpcParam {
  uros_rpcparamclass_t      pclass;      /**< @brief Parameter pclass.*/
  union {
    UrosString              string;     /**< @brief XMLRPC string.*/
    int32_t                 int32;      /**< @brief XMLRPC int/i4.*/
    uros_bool_t             boolean;    /**< @brief XMLRPC boolean.*/
    double                  real;       /**< @brief XMLRPC double.*/
    UrosString              base64;     /**< @brief XMLRPC base64.*/
    void                    *mapp;      /**< @brief XMLRPC struct (map pointer).*/ /* TODO */
    struct UrosRpcParamList *listp;     /**< @brief XMLRPC array (list pointer).*/
  }                         value;      /**< @brief Parameter value.*/
} UrosRpcParam;

/**
 * @brief   Parameter list node, forward only.
 */
typedef struct UrosRpcParamNode {
  UrosRpcParam              param;      /**< @brief Parameter value.*/
  struct UrosRpcParamNode   *nextp;     /**< @brief Pointer to the next node.*/
} UrosRpcParamNode;

/**
 * @brief   Parameter list, double ended.
 */
typedef struct UrosRpcParamList {
  UrosRpcParamNode  *headp;         /**< @brief Pointer to the first parameter.*/
  UrosRpcParamNode  *tailp;         /**< @brief Pointer to the last parameter.*/
  uros_cnt_t        length;         /**< @brief Number of list elements.*/
} UrosRpcParamList;

/**
 * @brief   XMLRPC call response object.
 */
typedef struct UrosRpcResponse {
  uint32_t          httpcode;       /**< @brief HTTP status code.*/
  int32_t           code;           /**< @brief Response code (@see @p UROS_RPCC_*).*/
  UrosString        *statusMsgp;    /**< @brief Status message.*/
  UrosRpcParam      *valuep;        /**< @brief Response value.*/
} UrosRpcResponse;

/** @} */

/*===========================================================================*/
/* GLOBAL PROTOTYPES                                                         */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

void urosRpcParamClean(UrosRpcParam *paramp, uros_bool_t deep);
void urosRpcParamDelete(UrosRpcParam *paramp, uros_bool_t deep);
void urosRpcParamObjectInit(UrosRpcParam *paramp,
                            uros_rpcparamclass_t pclass);

void urosRpcParamNodeClean(UrosRpcParamNode *nodep, uros_bool_t deep);
void urosRpcParamNodeDelete(UrosRpcParamNode *nodep, uros_bool_t deep);
void urosRpcParamNodeObjectInit(UrosRpcParamNode *nodep,
                                uros_rpcparamclass_t pclass);

void urosRpcParamListClean(UrosRpcParamList *listp, uros_bool_t deep);
void urosRpcParamListDelete(UrosRpcParamList *listp, uros_bool_t deep);
void urosRpcParamListObjectInit(UrosRpcParamList *listp);
void urosRpcParamListAppendNode(UrosRpcParamList *listp,
                                UrosRpcParamNode *nodep);
UrosRpcParamNode *urosRpcParamListUnlinkNode(UrosRpcParamList *listp,
                                             UrosRpcParamNode *nodep);

void urosRpcResponseObjectInit(UrosRpcResponse *rp);
void urosRpcResponseClean(UrosRpcResponse *rp);

uros_err_t urosRpcCallRegisterService(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  const UrosString      *service,
  const UrosString      *service_api,
  const UrosString      *caller_api,
  UrosRpcResponse       *resp);
uros_err_t urosRpcCallUnregisterService(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  const UrosString      *service,
  const UrosString      *service_api,
  UrosRpcResponse       *resp);
uros_err_t urosRpcCallRegisterSubscriber(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  const UrosString      *topic,
  const UrosString      *topic_type,
  const UrosString      *caller_api,
  UrosRpcResponse       *resp);
uros_err_t urosRpcCallUnregisterSubscriber(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  const UrosString      *topic,
  const UrosString      *caller_api,
  UrosRpcResponse       *resp);
uros_err_t urosRpcCallRegisterPublisher(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  const UrosString      *topic,
  const UrosString      *topic_type,
  const UrosString      *caller_api,
  UrosRpcResponse       *resp);
uros_err_t urosRpcCallUnregisterPublisher(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  const UrosString      *topic,
  const UrosString      *caller_api,
  UrosRpcResponse       *resp);
uros_err_t urosRpcCallLookupNode(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  const UrosString      *node,
  UrosRpcResponse       *resp);
uros_err_t urosRpcCallGetPublishedTopics(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  const UrosString      *subgraph,
  UrosRpcResponse       *resp);
uros_err_t urosRpcCallGetTopicTypes(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  UrosRpcResponse       *resp);
uros_err_t urosRpcCallGetSystemState(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  UrosRpcResponse       *resp);
uros_err_t urosRpcCallGetUri(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  UrosRpcResponse       *resp);
uros_err_t urosRpcCallLookupService(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  const UrosString      *service,
  UrosRpcResponse       *resp);
uros_err_t urosRpcCallDeleteParam(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  const UrosString      *key,
  UrosRpcResponse       *resp);
uros_err_t urosRpcCallSetParam(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  const UrosString      *key,
  const UrosRpcParam    *value,
  UrosRpcResponse       *resp);
uros_err_t urosRpcCallGetParam(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  const UrosString      *key,
  UrosRpcResponse       *resp);
uros_err_t urosRpcCallSearchParam(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  const UrosString      *key,
  UrosRpcResponse       *resp);
uros_err_t urosRpcCallSubscribeParam(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  const UrosString      *caller_api,
  const UrosString      *key,
  UrosRpcResponse       *resp);
uros_err_t urosRpcCallUnsubscribeParam(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  const UrosString      *caller_api,
  const UrosString      *key,
  UrosRpcResponse       *resp);
uros_err_t urosRpcCallHasParam(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  const UrosString      *key,
  UrosRpcResponse       *resp);
uros_err_t urosRpcCallGetParamNames(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  UrosRpcResponse       *resp);
uros_err_t urosRpcCallGetBusStats(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  UrosRpcResponse       *resp);
uros_err_t urosRpcCallGetBusInfo(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  UrosRpcResponse       *resp);
uros_err_t urosRpcCallGetMasterUri(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  UrosRpcResponse       *resp);
uros_err_t urosRpcCallShutdown(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  const UrosString      *msg,
  UrosRpcResponse       *resp);
uros_err_t urosRpcCallGetPid(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  UrosRpcResponse       *resp);
uros_err_t urosRpcCallGetSubscriptions(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  UrosRpcResponse       *resp);
uros_err_t urosRpcCallGetPublications(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  UrosRpcResponse       *resp);
uros_err_t urosRpcCallParamUpdate(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  const UrosString      *parameter_key,
  const UrosRpcParam    *parameter_value,
  UrosRpcResponse       *resp);
uros_err_t urosRpcCallPublisherUpdate(
  const UrosAddr            *addrp,
  const UrosString          *caller_id,
  const UrosString          *topic,
  const UrosRpcParamList    *publishers,
  UrosRpcResponse           *resp);
uros_err_t urosRpcCallRequestTopic(
  const UrosAddr            *addrp,
  const UrosString          *caller_id,
  const UrosString          *topic,
  const UrosRpcParamList    *protocols,
  UrosRpcResponse           *resp);

#ifdef __cplusplus
}
#endif
#endif /* _UROSRPC_H_ */
