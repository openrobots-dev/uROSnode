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
 * @file    urosRpcStreamer.h
 * @author  Andrea Zoppi <texzk@email.it>
 *
 * @brief   XMLRPC streamer functions.
 */

#ifndef _UROSRPCSTREAMER_H_
#define _UROSRPCSTREAMER_H_

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include "urosBase.h"
#include "urosConn.h"
#include "urosRpcCall.h"

/*===========================================================================*/
/* TYPES & MACROS                                                            */
/*===========================================================================*/

/**
 * @brief  Use the <tt>\<string\></tt> tag for string values.
 */
#if !defined(UROS_RPCSTREAMER_USE_STRING_TAG) || defined(__DOXYGEN__)
#define UROS_RPCSTREAMER_USE_STRING_TAG     UROS_FALSE
#endif

/** @addtogroup rpc_types */
/** @{ */

/**
 * @brief   XMLRPC streamer object.
 */
typedef struct UrosRpcStreamer {
  uros_err_t    err;        /**< @brief Last error message.*/
  UrosConn      *csp;       /**< @brief Connection status.*/
  char          *bufp;      /**< @brief Pointer to the refill buffer.*/
  size_t        buflen;     /**< @brief Refill buffer length.*/
  char          *curp;      /**< @brief Current parsing pointer.*/
  size_t        free;       /**< @brief Remaining free buffer characters.*/
  size_t        total;      /**< @brief Total streamed characters counter.*/
  size_t        mark;       /**< @brief Position mark, for user purposes.*/
  uros_bool_t   chunked;    /**< @brief Use HTTP chunked Transfer-Encoding.*/
} UrosRpcStreamer;

/** @} */

/*===========================================================================*/
/* GLOBAL PROTOTYPES                                                         */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

void urosRpcStreamerObjectInit(UrosRpcStreamer *sp,
                               UrosConn *csp,
                               char *bufp, size_t buflen);
void urosRpcStreamerClean(UrosRpcStreamer *sp, uros_bool_t freeBuffer);
uros_err_t urosRpcStreamerFlush(UrosRpcStreamer *sp);
uros_err_t urosRpcStreamerWrite(UrosRpcStreamer *sp,
                                const void *chunkp, size_t chunklen);
uros_err_t urosRpcStreamerUint32(UrosRpcStreamer *sp, uint32_t value);
uros_err_t urosRpcStreamerInt32(UrosRpcStreamer *sp, int32_t value);
uros_err_t urosRpcStreamerIp(UrosRpcStreamer *sp, UrosIp ip);
uros_err_t urosRpcStreamerHttpPost(UrosRpcStreamer *sp);
uros_err_t urosRpcStreamerHttpStatus(UrosRpcStreamer *sp, uint32_t code);
uros_err_t urosRpcStreamerHttpHeader(UrosRpcStreamer *sp,
                                     const char *keyp, size_t keylen,
                                     const char *valp, size_t vallen);
uros_err_t urosRpcStreamerHttpEnd(UrosRpcStreamer *sp);
uros_err_t urosRpcStreamerHttpContentLength(UrosRpcStreamer *sp);
uros_err_t urosRpcStreamerXmlEndHack(UrosRpcStreamer *sp);

uros_err_t urosRpcStreamerXmlAttrWVal(UrosRpcStreamer *sp,
                                      const char *namep, size_t namelen,
                                      const char *valp, size_t vallen,
                                      const char quotec);
uros_err_t urosRpcStreamerXmlTagBegin(UrosRpcStreamer *sp,
                                      const char *tagp, size_t taglen);
uros_err_t urosRpcStreamerXmlTagEnd(UrosRpcStreamer *sp);
uros_err_t urosRpcStreamerXmlTagSlashEnd(UrosRpcStreamer *sp);
uros_err_t urosRpcStreamerXmlTagOpen(UrosRpcStreamer *sp,
                                     const char *tagp, size_t taglen);
uros_err_t urosRpcStreamerXmlTagClose(UrosRpcStreamer *sp,
                                      const char *tagp, size_t taglen);
uros_err_t urosRpcStreamerXmlHeader(UrosRpcStreamer *sp);

uros_err_t urosRpcStreamerParamValueInt(UrosRpcStreamer *sp,
                                        const UrosRpcParam *paramp);
uros_err_t urosRpcStreamerParamValueBoolean(UrosRpcStreamer *sp,
                                            const UrosRpcParam *paramp);
uros_err_t urosRpcStreamerParamValueString(UrosRpcStreamer *sp,
                                           const UrosRpcParam *paramp);
uros_err_t urosRpcStreamerParamValueDouble(UrosRpcStreamer *sp,
                                           const UrosRpcParam *paramp);
uros_err_t urosRpcStreamerParamValueBase64(UrosRpcStreamer *sp,
                                           const UrosRpcParam *paramp);
uros_err_t urosRpcStreamerParamValueStruct(UrosRpcStreamer *sp,
                                           const UrosRpcParam *paramp);
uros_err_t urosRpcStreamerParamValueArray(UrosRpcStreamer *sp,
                                          const UrosRpcParam *paramp);
uros_err_t urosRpcStreamerParam(UrosRpcStreamer *sp,
                                const UrosRpcParam *paramp);

#ifdef __cplusplus
}
#endif
#endif /* _UROSRPCSTREAMER_H_ */
