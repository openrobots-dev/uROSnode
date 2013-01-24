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
 * @file    urosRpcParser.h
 * @author  Andrea Zoppi <texzk@email.it>
 *
 * @brief   XMLRPC parser methods.
 */

#ifndef _UROSRPCPARSER_H_
#define _UROSRPCPARSER_H_

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include "urosBase.h"
#include "urosConn.h"
#include "urosRpcCall.h"

/*===========================================================================*/
/* TYPES & MACROS                                                            */
/*===========================================================================*/

/** @addtogroup rpc_types */
/** @{ */

/**
 * @brief   XMLRPC parser object.
 */
typedef struct UrosRpcParser {
  uros_err_t    err;            /**< @brief Last error message.*/
  UrosConn      *csp;           /**< @brief Connection status.*/
  char          *rdbufp;        /**< @brief Reading buffer pointer.*/
  size_t        rdbuflen;       /**< @brief Reading buffer length.*/
  char          *curp;          /**< @brief Current parsing pointer.*/
  size_t        pending;        /**< @brief Remaining buffer characters to be parsed.*/
  size_t        total;          /**< @brief Parsed characters counter.*/
  size_t        mark;           /**< @brief Position mark, for user purposes.*/
  char          *bufp;          /**< @brief Pointer to the refill buffer.*/
  size_t        buflen;         /**< @brief Refill buffer length.*/
  size_t        contentLength;  /**< @brief Content-Length of XMLRPC message.*/
} UrosRpcParser;

/** @} */

/*===========================================================================*/
/* GLOBAL PROTOTYPES                                                         */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

void urosRpcParserObjectInit(UrosRpcParser *pp,
                             UrosConn *csp,
                             char *rdbufp, size_t rdbuflen);
void urosRpcParserClean(UrosRpcParser *pp, uros_bool_t freeBuffer);
uros_err_t urosRpcParserRefill(UrosRpcParser *pp);
uros_err_t urosRpcParserRead(UrosRpcParser *pp,
                             void *chunkp, size_t chunklen);
uros_err_t urosRpcParserExpect(UrosRpcParser *pp,
                               const char *tokp, size_t toklen);
uros_err_t urosRpcParserExpectQuiet(UrosRpcParser *pp,
                                    const char *tokp, size_t toklen);
uros_err_t urosRpcParserExpectNoCase(UrosRpcParser *pp,
                                     const char *tokp, size_t toklen);
uros_err_t urosRpcParserExpectNoCaseQuiet(UrosRpcParser *pp,
                                         const char *tokp, size_t toklen);
uros_err_t urosRpcParserLookAhead(UrosRpcParser *pp, char c);
uros_err_t urosRpcParserLookAheadQuiet(UrosRpcParser *pp, char c);
uros_err_t urosRpcParserSkipUntil(UrosRpcParser *pp, char c);
uros_err_t urosRpcParserSkip(UrosRpcParser *pp, size_t length);
uros_err_t urosRpcParserSkipAfter(UrosRpcParser *pp,
                                  const char *tokp, size_t toklen);
uros_err_t urosRpcParserSkipWs(UrosRpcParser *pp);
uros_err_t urosRpcParserExpectWs(UrosRpcParser *pp);
uros_err_t urosRpcParserUint32(UrosRpcParser *pp, uint32_t *valuep);
uros_err_t urosRpcParserInt32(UrosRpcParser *pp, int32_t *valuep);
uros_err_t urosRpcParserDouble(UrosRpcParser *pp, double *valuep);
uros_err_t urosRpcParserFixStringChars(UrosString *strp);

uros_err_t urosRpcParserHttpRequest(UrosRpcParser *pp);
uros_err_t urosRpcParserHttpResponse(UrosRpcParser *pp, uint32_t *codep);

uros_err_t urosRpcParserXmlAttrWVal(UrosRpcParser *pp,
                                    const char *namep, size_t namelen,
                                    const char *valp, size_t vallen);
uros_err_t urosRpcParserXmlTagBeginNoName(UrosRpcParser *pp);
uros_err_t urosRpcParserXmlTagBegin(UrosRpcParser *pp,
                                    const char *tagp, size_t taglen);
uros_err_t urosRpcParserXmlTagEnd(UrosRpcParser *pp);
uros_err_t urosRpcParserXmlTagSlashEnd(UrosRpcParser *pp);
uros_err_t urosRpcParserXmlTagOpen(UrosRpcParser *pp,
                                   const char *tagp, size_t taglen);
uros_err_t urosRpcParserXmlTagClose(UrosRpcParser *pp,
                                    const char *tagp, size_t taglen);
uros_err_t urosRpcParserXmlHeader(UrosRpcParser *pp);

uros_err_t urosRpcParserParamValueInt(UrosRpcParser *pp,
                                      UrosRpcParam *paramp);
uros_err_t urosRpcParserParamValueBoolean(UrosRpcParser *pp,
                                          UrosRpcParam *paramp);
uros_err_t urosRpcParserParamValueString(UrosRpcParser *pp,
                                         UrosRpcParam *paramp);
uros_err_t urosRpcParserParamValueDouble(UrosRpcParser *pp,
                                         UrosRpcParam *paramp);
uros_err_t urosRpcParserParamValueBase64(UrosRpcParser *pp,
                                         UrosRpcParam *paramp);
uros_err_t urosRpcParserParamValueStruct(UrosRpcParser *pp,
                                         UrosRpcParam *paramp);
uros_err_t urosRpcParserParamValueArray(UrosRpcParser *pp,
                                        UrosRpcParam *paramp);
uros_err_t urosRpcParserParamByTag(UrosRpcParser *pp,
                                   UrosRpcParam *paramp);
uros_err_t urosRpcParserParamByTagQuiet(UrosRpcParser *pp,
                                       UrosRpcParam *paramp);
uros_err_t urosRpcParserParamByClass(UrosRpcParser *pp,
                                     UrosRpcParam *paramp);
uros_err_t urosRpcParserParam(UrosRpcParser *pp,
                              UrosRpcParam *paramp,
                              uros_rpcparamclass_t paramclass);

uros_err_t urosRpcParserMethodResponse(UrosRpcParser *pp,
                                       UrosRpcResponse *resp);

#ifdef __cplusplus
}
#endif
#endif /* _UROSRPCPARSER_H_ */
