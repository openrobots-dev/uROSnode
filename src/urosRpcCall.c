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
 * @file    urosRpcCall.c
 * @author  Andrea Zoppi <texzk@email.it>
 *
 * @brief   XMLRPC call functions.
 */

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include "../include/urosBase.h"
#include "../include/urosUser.h"
#include "../include/urosRpcCall.h"
#include "../include/urosRpcParser.h"
#include "../include/urosRpcStreamer.h"
#include "../include/urosNode.h"

#include <string.h>

/*===========================================================================*/
/* LOCAL TYPES & MACROS                                                      */
/*===========================================================================*/

#if UROS_RPCCALL_C_USE_ASSERT == UROS_FALSE && !defined(__DOXYGEN__)
#undef urosAssert
#define urosAssert(expr)
#endif

/** @addtogroup rpc_types */
/** @{ */

/**
 * @brief   XMLRPC call context.
 */
typedef struct uros_rpcpcallctx_t {
  const UrosAddr    *addrp;     /**< @brief Target address.*/
  UrosConn          conn;       /**< @brief Connection handle.*/
  union {
    uros_err_t      err;        /**< @brief Last error message (aliased).*/
    UrosRpcParser   parser;     /**< @brief XMLRPC parser.*/
    UrosRpcStreamer streamer;   /**< @brief XMLRPC streamer.*/
  } x;                          /**< @brief Parser/Streamer, mutually exclusive.*/
} uros_rpcpcallctx_t;

/** @} */

#define _CHKOKE { if (ctxp->x.err != UROS_OK) { goto _finally; } }

/*===========================================================================*/
/* LOCAL FUNCTIONS                                                           */
/*===========================================================================*/

uros_err_t uros_rpccall_httprequest(UrosRpcStreamer *sp) {

  urosAssert(sp != NULL);
#define _CHKOK  { if (sp->err != UROS_OK) { return sp->err; } }

  /* Send a POST request.*/
  sp->chunked = UROS_FALSE;
  urosRpcStreamerHttpPost(sp); _CHKOK

  /* Using XML content (XMLRPC).*/
  urosRpcStreamerHttpHeader(sp, "Content-Type", 12, "text/xml", 8); _CHKOK

  /* Content-Length default value (fixed).*/
  urosRpcStreamerHttpContentLength(sp); _CHKOK
  urosRpcStreamerHttpEnd(sp); _CHKOK

  return sp->err;
#undef _CHKOK
}

uros_err_t uros_rpcpcall_buildctx(uros_rpcpcallctx_t *ctxp,
                                  const UrosAddr *addrp) {

  char *bufp;

  urosAssert(ctxp != NULL);
  urosAssert(addrp != NULL);

  ctxp->addrp = addrp;

  /* Connect to the target interface.*/
  urosConnObjectInit(&ctxp->conn);
  ctxp->x.err = urosConnCreate(&ctxp->conn, UROS_PROTO_TCP);
  if (ctxp->x.err != UROS_OK) { return ctxp->x.err; }
  ctxp->x.err = urosConnConnect(&ctxp->conn, addrp);
  if (ctxp->x.err != UROS_OK) { return ctxp->x.err; }
#if 0
  ctxp->x.err = urosConnSetRecvTimeout(&ctxp->conn, UROS_XMLRPC_RECVTIMEOUT);
  if (ctxp->x.err != UROS_OK) { return ctxp->x.err; }
  ctxp->x.err = urosConnSetSendTimeout(&ctxp->conn, UROS_XMLRPC_SENDTIMEOUT);
  if (ctxp->x.err != UROS_OK) { return ctxp->x.err; }
#endif

  /* Initialize the streamer.*/
  bufp = (char*)urosAlloc(UROS_MTU_SIZE);
  if (bufp == NULL) { return ctxp->x.err = UROS_ERR_NOMEM; }
  urosRpcStreamerObjectInit(&ctxp->x.streamer, &ctxp->conn,
                            bufp, UROS_MTU_SIZE);

  return ctxp->x.err = UROS_OK;
}

uros_err_t uros_rpcpcall_cleanctx(uros_rpcpcallctx_t *ctxp) {

  urosAssert(ctxp != NULL);

  /* Dispose the parser.*/
  urosRpcParserClean(&ctxp->x.parser, UROS_TRUE);

  /* Close the connection.*/
  urosConnClose(&ctxp->conn);

  return ctxp->x.err;
}

uros_err_t uros_rpccall_waitresponsestart(uros_rpcpcallctx_t *ctxp) {

  char *rdbufp;

  urosAssert(ctxp != NULL);

  /* Dispose the streamer.*/
  urosRpcStreamerClean(&ctxp->x.streamer, UROS_TRUE);

  /* Initialize the parser.*/
  rdbufp = (char*)urosAlloc(UROS_RPCPARSER_RDBUFLEN);
  if (rdbufp == NULL) { return ctxp->x.err = UROS_ERR_NOMEM; }
  urosRpcParserObjectInit(&ctxp->x.parser, &ctxp->conn,
                          rdbufp, UROS_RPCPARSER_RDBUFLEN);

  /* Wait until some data is read.*/
  urosRpcParserRefill(&ctxp->x.parser);
  return ctxp->x.err;
}

uros_err_t uros_rpccall_methodcall_prologue(UrosRpcStreamer *sp,
                                            const char *methodp,
                                            size_t methodlen) {

  urosAssert(sp != NULL);
  urosAssert(methodp != NULL);
  urosAssert(methodlen > 0);
#define _CHKOK  { if (sp->err != UROS_OK) { return sp->err; } }

  urosRpcStreamerXmlHeader(sp); _CHKOK
  urosRpcStreamerXmlTagOpen(sp, "methodCall", 10); _CHKOK
  urosRpcStreamerXmlTagOpen(sp, "methodName", 10); _CHKOK
  urosRpcStreamerWrite(sp, methodp, methodlen); _CHKOK
  urosRpcStreamerXmlTagClose(sp, "methodName", 10); _CHKOK
  urosRpcStreamerXmlTagOpen(sp, "params", 6); _CHKOK
  return UROS_OK;
#undef _CHKOK
}

uros_err_t uros_rpccall_methodcall_epilogue(UrosRpcStreamer *sp) {

  urosAssert(sp != NULL);
#define _CHKOK  { if (sp->err != UROS_OK) { return sp->err; } }

  urosRpcStreamerXmlTagClose(sp, "params", 6); _CHKOK
  urosRpcStreamerXmlTagClose(sp, "methodCall", 10); _CHKOK
  urosRpcStreamerWrite(sp, "\r\n", 2); _CHKOK

  /* Fix the actual Content-Length.*/
  urosRpcStreamerXmlEndHack(sp); _CHKOK
  return urosRpcStreamerFlush(sp);
#undef _CHKOK
}

uros_err_t uros_rpccall_param_value_int(UrosRpcStreamer *sp,
                                        int32_t value) {

  urosAssert(sp != NULL);
#define _CHKOK  { if (sp->err != UROS_OK) { return sp->err; } }

  urosRpcStreamerXmlTagOpen(sp, "param", 5); _CHKOK
  urosRpcStreamerXmlTagOpen(sp, "value", 5); _CHKOK
  urosRpcStreamerXmlTagOpen(sp, "int", 3); _CHKOK
  urosRpcStreamerInt32(sp, value); _CHKOK
  urosRpcStreamerXmlTagClose(sp, "int", 3); _CHKOK
  urosRpcStreamerXmlTagClose(sp, "value", 5); _CHKOK
  urosRpcStreamerXmlTagClose(sp, "param", 5); _CHKOK

  return UROS_OK;
#undef _CHKOK
}

uros_err_t uros_rpccall_param_value_string(UrosRpcStreamer *sp,
                                           const UrosString *strp) {

  urosAssert(sp != NULL);
#define _CHKOK  { if (sp->err != UROS_OK) { return sp->err; } }

  urosRpcStreamerXmlTagOpen(sp, "param", 5); _CHKOK
  urosRpcStreamerXmlTagOpen(sp, "value", 5); _CHKOK
#if UROS_RPCSTREAMER_USE_STRING_TAG != UROS_FALSE
  urosRpcStreamerXmlTagOpen(sp, "string", 6); _CHKOK
#endif
  if (strp != NULL) {
    urosRpcStreamerWrite(sp, strp->datap, strp->length); _CHKOK
  }
#if UROS_RPCSTREAMER_USE_STRING_TAG != UROS_FALSE
  urosRpcStreamerXmlTagClose(sp, "string", 6); _CHKOK
#endif
  urosRpcStreamerXmlTagClose(sp, "value", 5); _CHKOK
  urosRpcStreamerXmlTagClose(sp, "param", 5); _CHKOK

  return UROS_OK;
#undef _CHKOK
}

uros_err_t uros_rpccall_param_value_array(UrosRpcStreamer *sp,
                                          const UrosRpcParamList *lstp) {

  UrosRpcParam wrapper;

  urosAssert(sp != NULL);
  urosAssert(lstp != NULL);
#define _CHKOK  { if (sp->err != UROS_OK) { return sp->err; } }

  wrapper.class = UROS_RPCP_ARRAY;
  wrapper.value.listp = (UrosRpcParamList*)lstp;

  urosRpcStreamerXmlTagOpen(sp, "param", 5); _CHKOK
  urosRpcStreamerXmlTagOpen(sp, "value", 5); _CHKOK
  urosRpcStreamerXmlTagOpen(sp, "array", 5); _CHKOK
  urosRpcStreamerParamValueArray(sp, &wrapper); _CHKOK
  urosRpcStreamerXmlTagClose(sp, "array", 5); _CHKOK
  urosRpcStreamerXmlTagClose(sp, "value", 5); _CHKOK
  urosRpcStreamerXmlTagClose(sp, "param", 5); _CHKOK

  return UROS_OK;
#undef _CHKOK
}

uros_err_t uros_rpccall_param(UrosRpcStreamer *sp,
                              const UrosRpcParam *paramp) {

  urosAssert(sp != NULL);
#define _CHKOK  { if (sp->err != UROS_OK) { return sp->err; } }

  urosRpcStreamerXmlTagOpen(sp, "param", 5); _CHKOK
  urosRpcStreamerParam(sp, paramp); _CHKOK
  urosRpcStreamerXmlTagClose(sp, "param", 5); _CHKOK

  return UROS_OK;
#undef _CHKOK
}

uros_err_t uros_rpccall_methodresponse(UrosRpcParser *pp,
                                       UrosRpcResponse *resp) {

  uint32_t httpcode;

  urosAssert(pp != NULL);
  urosAssert(resp != NULL);

  urosRpcParserHttpResponse(pp, &httpcode);
  resp->httpcode = httpcode;
  if (pp->err != UROS_OK) { return pp->err; }
  if (resp->httpcode != 200) { return UROS_ERR_PARSE; }
  urosRpcParserMethodResponse(pp, resp);
  resp->httpcode = httpcode;    /* Object cleaning workaround.*/
  if (pp->err != UROS_OK) { return pp->err; }
  if (resp->code != UROS_RPCC_SUCCESS) {
    return pp->err = UROS_ERR_BADCONN;
  }

  return UROS_OK;
}

uros_err_t uros_rpccall_registercall(
  const char        *methodp,
  size_t            methdolen,
  const UrosAddr    *addrp,
  const UrosString  *caller_id,
  const UrosString  *what,
  const UrosString  *type,
  const UrosString  *caller_api,
  UrosRpcResponse   *resp) {

  uros_rpcpcallctx_t *ctxp;
  uros_err_t err;

  urosAssert(methodp != NULL);
  urosAssert(methdolen > 0);
  urosAssert(addrp != NULL);
  urosAssert(urosStringNotEmpty(caller_id));
  urosAssert(urosStringNotEmpty(what));
  urosAssert(urosStringNotEmpty(type));
  urosAssert(urosStringNotEmpty(caller_api));
  urosAssert(resp != NULL);

  ctxp = urosNew(uros_rpcpcallctx_t);
  if (ctxp == NULL) { return UROS_ERR_NOMEM; }

  urosRpcResponseObjectInit(resp);
  uros_rpcpcall_buildctx(ctxp, addrp); _CHKOKE

  /* Send the request message.*/
  uros_rpccall_httprequest(&ctxp->x.streamer); _CHKOKE
  uros_rpccall_methodcall_prologue(&ctxp->x.streamer, methodp, methdolen); _CHKOKE
  uros_rpccall_param_value_string(&ctxp->x.streamer, caller_id); _CHKOKE
  uros_rpccall_param_value_string(&ctxp->x.streamer, what); _CHKOKE
  uros_rpccall_param_value_string(&ctxp->x.streamer, type); _CHKOKE
  uros_rpccall_param_value_string(&ctxp->x.streamer, caller_api); _CHKOKE
  uros_rpccall_methodcall_epilogue(&ctxp->x.streamer); _CHKOKE

  /* Receive the response message.*/
  uros_rpccall_waitresponsestart(ctxp); _CHKOKE
  uros_rpccall_methodresponse(&ctxp->x.parser, resp); _CHKOKE
  if (resp->valuep->class == UROS_RPCP_ARRAY ||
      resp->valuep->class == UROS_RPCP_INT) {
    ctxp->x.err = UROS_OK;
  } else {
    ctxp->x.err = UROS_ERR_BADPARAM;
  }

_finally:
  err = ctxp->x.err;
  uros_rpcpcall_cleanctx(ctxp);
  urosFree(ctxp);
  return err;
}

uros_err_t uros_rpccall_unregistercall(
  const char        *methodp,
  size_t            methdolen,
  const UrosAddr    *addrp,
  const UrosString  *caller_id,
  const UrosString  *what,
  const UrosString  *api,
  UrosRpcResponse   *resp) {

  uros_rpcpcallctx_t *ctxp;
  uros_err_t err;

  urosAssert(methodp != NULL);
  urosAssert(methdolen > 0);
  urosAssert(addrp != NULL);
  urosAssert(urosStringNotEmpty(caller_id));
  urosAssert(urosStringNotEmpty(what));
  urosAssert(urosStringNotEmpty(api));
  urosAssert(resp != NULL);

  ctxp = urosNew(uros_rpcpcallctx_t);
  if (ctxp == NULL) { return UROS_ERR_NOMEM; }

  urosRpcResponseObjectInit(resp);
  uros_rpcpcall_buildctx(ctxp, addrp); _CHKOKE

  /* Send the request message.*/
  uros_rpccall_httprequest(&ctxp->x.streamer); _CHKOKE
  uros_rpccall_methodcall_prologue(&ctxp->x.streamer, methodp, methdolen); _CHKOKE
  uros_rpccall_param_value_string(&ctxp->x.streamer, caller_id); _CHKOKE
  uros_rpccall_param_value_string(&ctxp->x.streamer, what); _CHKOKE
  uros_rpccall_param_value_string(&ctxp->x.streamer, api); _CHKOKE
  uros_rpccall_methodcall_epilogue(&ctxp->x.streamer); _CHKOKE

  /* Receive the response message.*/
  uros_rpccall_waitresponsestart(ctxp); _CHKOKE
  uros_rpccall_methodresponse(&ctxp->x.parser, resp); _CHKOKE
  if (resp->valuep->class != UROS_RPCP_INT) {
    ctxp->x.err = UROS_ERR_BADPARAM;
  } else { ctxp->x.err = UROS_OK; }

_finally:
  err = ctxp->x.err;
  uros_rpcpcall_cleanctx(ctxp);
  urosFree(ctxp);
  return err;
}

uros_err_t uros_rpccall_simplecall(
  const char            *methodp,
  size_t                methodlen,
  uros_rpcparamclass_t  resclass,
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  UrosRpcResponse       *resp) {

  uros_rpcpcallctx_t *ctxp;
  uros_err_t err;

  urosAssert(methodp != NULL);
  urosAssert(methodlen > 0);
  urosAssert(addrp != NULL);
  urosAssert(urosStringNotEmpty(caller_id));
  urosAssert(resp != NULL);

  ctxp = urosNew(uros_rpcpcallctx_t);
  if (ctxp == NULL) { return UROS_ERR_NOMEM; }

  urosRpcResponseObjectInit(resp);
  uros_rpcpcall_buildctx(ctxp, addrp); _CHKOKE

  /* Send the request message.*/
  uros_rpccall_httprequest(&ctxp->x.streamer); _CHKOKE
  uros_rpccall_methodcall_prologue(&ctxp->x.streamer, methodp, methodlen); _CHKOKE
  uros_rpccall_param_value_string(&ctxp->x.streamer, caller_id); _CHKOKE
  uros_rpccall_methodcall_epilogue(&ctxp->x.streamer); _CHKOKE

  /* Receive the response message.*/
  uros_rpccall_waitresponsestart(ctxp); _CHKOKE
  uros_rpccall_methodresponse(&ctxp->x.parser, resp); _CHKOKE
  if (resclass != UROS_RPCP__LENGTH && resp->valuep->class != resclass) {
      ctxp->x.err = UROS_ERR_BADPARAM;
  } else { ctxp->x.err = UROS_OK; }

_finally:
  err = ctxp->x.err;
  uros_rpcpcall_cleanctx(ctxp);
  urosFree(ctxp);
  return err;
}

uros_err_t uros_rpccall_stringcall(
  const char            *methodp,
  size_t                methodlen,
  uros_rpcparamclass_t  resclass,
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  const UrosString      *string,
  UrosRpcResponse       *resp) {

  uros_rpcpcallctx_t *ctxp;
  uros_err_t err;

  urosAssert(methodp != NULL);
  urosAssert(methodlen > 0);
  urosAssert(addrp != NULL);
  urosAssert(urosStringNotEmpty(caller_id));
  urosAssert(resp != NULL);

  ctxp = urosNew(uros_rpcpcallctx_t);
  if (ctxp == NULL) { return UROS_ERR_NOMEM; }

  urosRpcResponseObjectInit(resp);
  uros_rpcpcall_buildctx(ctxp, addrp); _CHKOKE

  /* Send the request message.*/
  uros_rpccall_httprequest(&ctxp->x.streamer); _CHKOKE
  uros_rpccall_methodcall_prologue(&ctxp->x.streamer, methodp, methodlen); _CHKOKE
  uros_rpccall_param_value_string(&ctxp->x.streamer, caller_id); _CHKOKE
  uros_rpccall_param_value_string(&ctxp->x.streamer, string); _CHKOKE
  uros_rpccall_methodcall_epilogue(&ctxp->x.streamer); _CHKOKE

  /* Receive the response message.*/
  uros_rpccall_waitresponsestart(ctxp); _CHKOKE
  uros_rpccall_methodresponse(&ctxp->x.parser, resp); _CHKOKE
  if (resclass != UROS_RPCP__LENGTH && resp->valuep->class != resclass) {
      ctxp->x.err = UROS_ERR_BADPARAM;
  } else { ctxp->x.err = UROS_OK; }

_finally:
  err = ctxp->x.err;
  uros_rpcpcall_cleanctx(ctxp);
  urosFree(ctxp);
  return err;
}

/*===========================================================================*/
/* GLOBAL FUNCTIONS                                                          */
/*===========================================================================*/

/** @addtogroup rpc_funcs */
/** @{ */

/** @name XMLRPC parameter */
/** @{ */

/**
 * @brief   Cleans a XMLRPC parameter.
 * @details Deallocates any memory chunks allocated by the parameter, and
 *          invalidates it.
 *
 * @pre     The parameter is initialized.
 * @pre     The parameter data must have been allocated with @p urosAlloc().
 * @post    @p paramp points to an invalidated @p UrosRpcParam object.
 * @post    The parameter class is unchanged.
 * @post    If desidred so, private members are deallocated. They must have
 *          been allocated with @p urosAlloc().
 *
 * @param[in,out] paramp
 *          Pointer to an initialized @p UrosRpcParam object.
 * @param[in] deep
 *          Performs deallocation of private members.
 */
void urosRpcParamClean(UrosRpcParam *paramp, uros_bool_t deep) {

  urosAssert(paramp != NULL);

  switch (paramp->class) {
  case UROS_RPCP_INT:
  case UROS_RPCP_BOOLEAN:
  case UROS_RPCP_DOUBLE:
  case UROS_RPCP__LENGTH: {
    /* Nothing to clean.*/
    break;
  }
  case UROS_RPCP_BASE64: {
    paramp->value.base64.length = 0;
    if (paramp->value.base64.datap != NULL) {
      urosFree(paramp->value.base64.datap);
      paramp->value.base64.datap = NULL;
    }
    break;
  }
  case UROS_RPCP_STRING: {
    if (deep) {
      urosStringClean(&paramp->value.string);
    } else {
      paramp->value.string.length = 0;
    }
    break;
  }
  case UROS_RPCP_STRUCT: {
    /* TODO */
    break;
  }
  case UROS_RPCP_ARRAY: {
    urosRpcParamListDelete(paramp->value.listp, deep);
    paramp->value.listp = NULL;
    break;
  }
  default: {
    urosAssert(0);
    break;
  }
  }
}

/**
 * @brief   Deallocates a XMLRPC parameter.
 * @details Invalidates the private members. Optionally, deallocates their data.
 *
 * @pre     The parameter is initilized.
 * @post    @p paramp points to an invalid address.
 * @post    If desidred so, private members are deallocated. They must have
 *          been allocated with @p urosAlloc().
 *
 * @param[in,out] paramp
 *          Pointer to an initialized @p UrosRpcParam object, or @p NULL.
 * @param[in] deep
 *          Performs deallocation of private members.
 */
void urosRpcParamDelete(UrosRpcParam *paramp, uros_bool_t deep) {

  if (paramp != NULL) {
    if (deep) {
      urosRpcParamClean(paramp, UROS_TRUE);
    }
    urosFree(paramp);
  }
}

/**
 * @brief   Initializes a XMLRPC parameter object.
 * @details Initializes the object to the desired class, with invalidated safe
 *          values.
 *
 * @param[in,out] paramp
 *          Pointer to an allocated @p UrosRpcParam object.
 * @param[in] class
 *          Parameter class.
 */
void urosRpcParamObjectInit(UrosRpcParam *paramp,
                            uros_rpcparamclass_t class) {

  urosAssert(paramp != NULL);

  paramp->class = class;
  switch (paramp->class) {
    case UROS_RPCP_INT:
    case UROS_RPCP_BOOLEAN:
    case UROS_RPCP_DOUBLE:
    case UROS_RPCP__LENGTH: {
      /* Nothing to invalidate.*/
      break;
    }
    case UROS_RPCP_BASE64: {
      paramp->value.base64.length = 0;
      paramp->value.base64.datap = NULL;
      break;
    }
    case UROS_RPCP_STRING: {
      paramp->value.string.length = 0;
      paramp->value.string.datap = NULL;
      break;
    }
    case UROS_RPCP_STRUCT: {
      /* TODO */
      break;
    }
    case UROS_RPCP_ARRAY: {
      paramp->value.listp = NULL;
      break;
    }
    default: {
      urosAssert(0);
      break;
    }
    }
}

/** @} */

/** @name XMLRPC parameter list */
/** @{ */

/**
 * @brief   Cleans a XMLRPC parameter list node.
 * @details Deallocates any memory chunks allocated by the node, and
 *          invalidates it.
 *
 * @pre     The parameter node is initialized.
 * @pre     The parameter data must have been allocated with @p urosAlloc().
 * @post    @p nodep points to an invalidated @p UrosRpcParam object.
 * @post    The parameter class is unchanged.
 * @post    If desidred so, private members are deallocated. They must have
 *          been allocated with @p urosAlloc().
 *
 * @param[in,out] nodep
 *          Pointer to an initialized @p UrosRpcParamNode object.
 * @param[in] deep
 *          Performs deallocation of private members.
 */
void urosRpcParamNodeClean(UrosRpcParamNode *nodep, uros_bool_t deep) {

  urosAssert(nodep != NULL);

  urosRpcParamClean(&nodep->param, deep);
  nodep->nextp = NULL;
}

/**
 * @brief   Deallocates a XMLRPC parameter list node.
 * @details Invalidates the private members. Optionally, deallocates their data.
 *
 * @pre     The parameter node is initialized.
 * @post    @p nodep points to an invalid address.
 * @post    If desidred so, private members are deallocated. They must have
 *          been allocated with @p urosAlloc().
 *
 * @param[in,out] nodep
 *          Pointer to an initialized @p UrosRpcParamNode object, or @p NULL.
 * @param[in] deep
 *          Performs deallocation of private members.
 */
void urosRpcParamNodeDelete(UrosRpcParamNode *nodep, uros_bool_t deep) {

  if (nodep != NULL) {
    if (deep) {
      urosRpcParamNodeClean(nodep, UROS_TRUE);
    }
    urosFree(nodep);
  }
}

/**
 * @brief   Initializes a XMLRPC parameter list node object.
 * @details Initializes the parameter to the desired class, with invalidated
 *          safe values.
 *
 * @param[in,out] nodep
 *          Pointer to an allocated @p UrosRpcParamNode object.
 * @param[in] class
 *          Parameter class.
 */
void urosRpcParamNodeObjectInit(UrosRpcParamNode *nodep,
                                uros_rpcparamclass_t class) {

  urosAssert(nodep != NULL);

  urosRpcParamObjectInit(&nodep->param, class);
  nodep->nextp = NULL;
}

/**
 * @brief   Cleans a XMLRPC parameter list.
 * @details Removes all the nodes from a list. If desired, also all the nodes
 *          are deallcoated.
 *
 * @pre     The object and its fields must have been allocated with
 *          @p urosAlloc().
 * @post    @p listp points to an empty list.
 * @post    If desidred so, all the nodes are deallocated. They must have
 *          been allocated with @p urosAlloc().
 *
 * @param[in,out] listp
 *          Pointer to an initialized @p UrosRpcParamList object.
 * @param[in] deep
 *          Performs deallocation of al the nodes.
 */
void urosRpcParamListClean(UrosRpcParamList *listp, uros_bool_t deep) {

  UrosRpcParamNode *nodep, *nextp, *unlinkedp;
  (void)unlinkedp;

  urosAssert(listp != NULL);

  for (nodep = listp->headp; nodep != NULL; nodep = nextp) {
    nextp = nodep->nextp;
    unlinkedp = urosRpcParamListUnlinkNode(listp, nodep);
    urosAssert(unlinkedp == nodep);
    urosRpcParamNodeDelete(nodep, deep);
  }
  urosAssert(listp->headp == NULL);
  urosAssert(listp->tailp == NULL);
  urosAssert(listp->length == 0);
}

/**
 * @brief   Deletes a XMLRPC parameter list object.
 * @details Removes all the nodes from a list. If desired, also all the nodes
 *          are deallcoated. The list object is the deallocated.
 *
 * @pre     The list is allocated with @p urosAlloc() and initialized.
 * @post    @p listp points to an invalid address.
 * @post    If desidred so, all the nodes are deallocated. They must have
 *          been allocated with @p urosAlloc().
 *
 * @param[in,out] listp
 *          Pointer to an initialized @p UrosRpcParamList object, or @p NULL.
 * @param[in] deep
 *          Performs deallocation of all the nodes.
 */
void urosRpcParamListDelete(UrosRpcParamList *listp, uros_bool_t deep) {

  if (listp != NULL) {
    if (deep) {
      urosRpcParamListClean(listp, UROS_TRUE);
    }
    urosFree(listp);
  }
}

/**
 * @brief   Initializes a XMLRPC parameter list.
 * @details Initializes to an empty list.
 *
 * @post    The list is empty.
 *
 * @param[in,out] listp
 *          Pointer to an allocated @p UrosRpcParamList object.
 */
void urosRpcParamListObjectInit(UrosRpcParamList *listp) {

  urosAssert(listp != NULL);

  listp->length = 0;
  listp->headp = NULL;
  listp->tailp = NULL;
}

/**
 * @brief   Appends a node to the XMLRPC parameter list.
 *
 * @pre     The node does not belong to the list.
 * @pre     The node has no successors.
 * @post    The provided node is at the end of the list.
 * @post    The list length is incremented by one.
 *
 * @param[in,out] listp
 *          Pointer to an initialized @p UrosRpcParamList object.
 * @param[in,out] nodep
 *          Pointer to an initialized @p UrosRpcParamNode object with no
 *          successors.
 */
void urosRpcParamListAppendNode(UrosRpcParamList *listp,
                                UrosRpcParamNode *nodep) {

  urosAssert(nodep != NULL);
  urosAssert(nodep->nextp == NULL);

  if (listp->tailp != NULL) {
    ++listp->length;
    listp->tailp->nextp = nodep;
    listp->tailp = nodep;
  } else {
    listp->length = 1;
    listp->headp = listp->tailp = nodep;
  }
}

/**
 * @brief   Removes a node from the XMLRPC parameter list.
 * @details The node, if found, is unlinked from the list and returned.
 *
 * @post    The node does not belong to the list.
 * @post    The node has no successors.
 * @post    The list length is decremented by one.
 * @post    The list is consistent.
 *
 * @param[in,out] listp
 *          Pointer to an initialized @p UrosRpcParamList object.
 * @param[in] nodep
 *          Pointer to the node to be removed from the list.
 * @return
 *          Pointer to the removed (unlinked) node.
 * @retval nodep
 *          The addressed node, if found inside the list.
 * @retval NULL
 *          No such node found inside the list.
 */
UrosRpcParamNode *urosRpcParamListUnlinkNode(UrosRpcParamList *listp,
                                             UrosRpcParamNode *nodep) {

  UrosRpcParamNode *curp, *prevp;

  urosAssert(listp != NULL);
  urosAssert(nodep != NULL);

  for (curp = listp->headp, prevp = NULL;
       curp != NULL;
       prevp = curp, curp = curp->nextp) {
    if (curp == nodep) {
      if (curp == listp->tailp) {
        listp->tailp = prevp;
      }
      if (prevp != NULL) {
        prevp->nextp = curp->nextp;
      } else {
        listp->headp = curp->nextp;
      }
      curp->nextp = NULL;
      --listp->length;
      return curp;
    }
  }
  return NULL;
}

/** @} */

/** @name XMLRPC response */
/** @{ */

/**
 * @brief   Initializes a XMLRPC response.
 * @details Initializes to an empty response.
 *
 * @param[in,out] rp
 *          Pointer to an allocated @p UrosRpcResponse object.
 */
void urosRpcResponseObjectInit(UrosRpcResponse *rp) {

  urosAssert(rp != NULL);

  rp->httpcode = 0;
  rp->code = UROS_RPCC_ERROR;
  rp->statusMsgp = NULL;
  rp->valuep = NULL;
}

/**
 * @brief   Cleans a XMLRPC resposnse.
 * @details Deallocates the memory chunk of the object and all its fields.
 *
 * @pre     The object and its fields must have been allocated with
 *          @p urosAlloc().
 * @post    @p rp points to an invalid address.
 *
 * @param[in] rp
 *          Pointer to an initialized @p UrosRpcResponse object.
 */
void urosRpcResponseClean(UrosRpcResponse *rp) {

  urosAssert(rp != NULL);

  rp->httpcode = 0;
  rp->code = UROS_RPCC_ERROR;
  urosStringDelete(rp->statusMsgp);
  rp->statusMsgp = NULL;
  urosRpcParamDelete(rp->valuep, UROS_TRUE);
  rp->valuep = NULL;
}

/** @} */
/** @} */

/** @addtogroup rpc_call_funcs */
/** @{ */

/** @name Master API calls */
/** @{ */

/**
 * @brief   Registers the caller as a provider of the specified service.
 * @see     http://www.ros.org/wiki/ROS/Master_API
 *
 * @param[in] addrp
 *          Pointer to the Master API connection address.
 * @param[in] caller_id
 *          ROS caller ID. Non-empty string pointer.
 * @param[in] service
 *          Fully-qualified name of service. Non-empty string pointer.
 * @param[in] service_api
 *          ROSRPC Service URI. Non-empty string pointer.
 * @param[in] caller_api
 *          XML-RPC URI of caller node. Non-empty string pointer.
 * @param[out] resp
 *          Pointer to the @p UrosRpcResponse response object. Format:
 *          @verbatim [ int code, str statusMessage, any ignore ]@endverbatim
 * @return
 *          Error code.
 */
uros_err_t urosRpcCallRegisterService(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  const UrosString      *service,
  const UrosString      *service_api,
  const UrosString      *caller_api,
  UrosRpcResponse       *resp) {

  return uros_rpccall_registercall(
    "registerService", 15,
    addrp, caller_id, service, service_api, caller_api, resp);
}

/**
 * @brief   Unregister the caller as a provider of the specified service.
 * @see     http://www.ros.org/wiki/ROS/Master_API
 *
 * @param[in] addrp
 *          Pointer to the Master API connection address.
 * @param[in] caller_id
 *          ROS caller ID. Non-empty string pointer.
 * @param[in] service
 *          Fully-qualified name of service. Non-empty string pointer.
 * @param[in] service_api
 *          API URI of service to unregister. Unregistration will only occur
 *          if current registration matches. Non-empty string pointer.
 * @param[out] resp
 *          Pointer to the @p UrosRpcResponse response object. Format:
 *          @verbatim [ int code, str statusMessage, int numUnregistered ]@endverbatim
 *
 *          Number of unregistrations (either 0 or 1). If this is zero it means
 *          that the caller was not registered as a service provider. The call
 *          still succeeds as the intended final state is reached.
 * @return
 *          Error code.
 */
uros_err_t urosRpcCallUnregisterService(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  const UrosString      *service,
  const UrosString      *service_api,
  UrosRpcResponse       *resp) {

  return uros_rpccall_unregistercall(
    "unregisterService", 17,
    addrp, caller_id, service, service_api, resp);
}

/**
 * @brief   Subscribe the caller to the specified topic.
 * @details In addition to receiving a list of current publishers, the
 *          subscriber will also receive notifications of new publishers via
 *          the @p publisherUpdate() API.
 * @see     http://www.ros.org/wiki/ROS/Master_API
 *
 * @param[in] addrp
 *          Pointer to the Master API connection address.
 * @param[in] caller_id
 *          ROS caller ID. Non-empty string pointer.
 * @param[in] topic
 *          Fully-qualified name of topic. Non-empty string pointer.
 * @param[in] topic_type
 *          Datatype for topic. Must be a package-resource name, i.e. the
 *          <tt>.msg</tt> name. Non-empty string pointer.
 * @param[in] caller_api
 *          API URI of subscriber to register. Will be used for new publisher
 *          notifications. Non-empty string pointer.
 * @param[out] resp
 *          Pointer to the @p UrosRpcResponse response object. Format:
 *          @verbatim [ int code, str statusMessage, [str publisherN] ]@endverbatim
 * @return
 *          Error code.
 */
uros_err_t urosRpcCallRegisterSubscriber(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  const UrosString      *topic,
  const UrosString      *topic_type,
  const UrosString      *caller_api,
  UrosRpcResponse       *resp) {

  return uros_rpccall_registercall(
    "registerSubscriber", 18,
    addrp, caller_id, topic, topic_type, caller_api, resp);
}

/**
 * @brief   Unregister the caller as a publisher of the topic.
 * @see     http://www.ros.org/wiki/ROS/Master_API
 *
 * @param[in] addrp
 *          Pointer to the Master API connection address.
 * @param[in] caller_id
 *          ROS caller ID. Non-empty string pointer.
 * @param[in] topic
 *          Fully-qualified name of topic. Non-empty string pointer.
 * @param[in] caller_api
 *          API URI of service to unregister. Unregistration will only occur
 *          if current registration matches. Non-empty string pointer.
 * @param[out] resp
 *          Pointer to the @p UrosRpcResponse response object. Format:
 *          @verbatim [ int code, str statusMessage, int numUnsubscribed ]@endverbatim
 *
 *          If @p numUnsubscribed is zero it means that the caller was not
 *          registered as a subscriber. The call still succeeds as the intended
 *          final state is reached.
 * @return
 *          Error code.
 */
uros_err_t urosRpcCallUnregisterSubscriber(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  const UrosString      *topic,
  const UrosString      *caller_api,
  UrosRpcResponse       *resp) {

  return uros_rpccall_unregistercall(
    "unregisterSubscriber", 20,
    addrp, caller_id, topic, caller_api, resp);
}

/**
 * @brief   Register the caller as a publisher the topic.
 * @see     http://www.ros.org/wiki/ROS/Master_API
 *
 * @param[in] addrp
 *          Pointer to the Master API connection address.
 * @param[in] caller_id
 *          ROS caller ID. Non-empty string pointer.
 * @param[in] topic
 *          Fully-qualified name of topic to register. Non-empty string
 *          pointer.
 * @param[in] topic_type
 *          Datatype for topic. Must be a package-resource name, i.e. the
 *          <tt>.msg</tt> name. Non-empty string pointer.
 * @param[in] caller_api
 *          API URI of publisher to register. Non-empty string pointer.
 * @param[out] resp
 *          Pointer to the @p UrosRpcResponse response object. Format:
 *          @verbatim [ int code, str statusMessage, [str subscriberApiN] ]@endverbatim
 * @return
 *          Error code.
 */
uros_err_t urosRpcCallRegisterPublisher(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  const UrosString      *topic,
  const UrosString      *topic_type,
  const UrosString      *caller_api,
  UrosRpcResponse       *resp) {

  return uros_rpccall_registercall(
    "registerPublisher", 17,
    addrp, caller_id, topic, topic_type, caller_api, resp);
}

/**
 * @brief   Unregister the caller as a publisher of the topic.
 * @see     http://www.ros.org/wiki/ROS/Master_API
 *
 * @param[in] addrp
 *          Pointer to the Master API connection address.
 * @param[in] caller_id
 *          ROS caller ID. Non-empty string pointer.
 * @param[in] topic
 *          Fully-qualified name of topic to unregister. Non-empty string
 *          pointer.
 * @param[in] caller_api
 *          API URI of publisher to unregister. Unregistration will only
 *          occur if current registration matches. Non-empty string pointer.
 * @param[out] resp
 *          Pointer to the @p UrosRpcResponse response object. Format:
 *          @verbatim [ int code, str statusMessage, int numUnregistered ]@endverbatim
 *
 *          If @p numUnregistered is zero it means that the caller was not
 *          registered as a publisher. The call still succeeds as the intended
 *          final state is reached.
 * @return
 *          Error code.
 */
uros_err_t urosRpcCallUnregisterPublisher(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  const UrosString      *topic,
  const UrosString      *caller_api,
  UrosRpcResponse       *resp) {

  return uros_rpccall_unregistercall(
    "unregisterPublisher", 19,
    addrp, caller_id, topic, caller_api, resp);
}

/**
 * @brief   Get the XML-RPC URI of the node with the associated name.
 * @details This API is for looking information about publishers and
 *          subscribers. Use @p lookupService() instead to lookup ROS-RPC URIs.
 * @see     http://www.ros.org/wiki/ROS/Master_API
 *
 * @param[in] addrp
 *          Pointer to the Master API connection address.
 * @param[in] caller_id
 *          ROS caller ID. Non-empty string pointer.
 * @param[in] node
 *          Name of node to lookup. Non-empty string pointer.
 * @param[out] resp
 *          Pointer to the @p UrosRpcResponse response object. Format:
 *          @verbatim [ int code, str statusMessage, str URI ]@endverbatim
 * @return
 *          Error code.
 */
uros_err_t urosRpcCallLookupNode(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  const UrosString      *node,
  UrosRpcResponse       *resp) {

  urosAssert(node != NULL);
  urosAssert(node->length > 0);
  urosAssert(node->datap != NULL);

  return uros_rpccall_stringcall(
    "lookupNode", 10, UROS_RPCP_STRING,
    addrp, caller_id, node, resp);
}

/**
 * @brief   Get list of topics that can be subscribed to.
 * @details This does not return topics that have no publishers.
 * @see     http://www.ros.org/wiki/ROS/Master_API
 * @see     urosRpcCallGetSystemState()
 *
 * @param[in] addrp
 *          Pointer to the Master API connection address.
 * @param[in] caller_id
 *          ROS caller ID. Non-empty string pointer.
 * @param[in] subgraph
 *          Restrict topic names to match within the specified subgraph.
 *          Subgraph namespace is resolved relative to the caller's
 *          namespace. Use emptry string to specify all names. Non-empty
 *          string pointer.
 * @param[out] resp
 *          Pointer to the @p UrosRpcResponse response object. Format:
 *          @verbatim [ int code, str statusMessage, [[str topicN, str typeN]] ]@endverbatim
 * @return
 *          Error code.
 */
uros_err_t urosRpcCallGetPublishedTopics(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  const UrosString      *subgraph,
  UrosRpcResponse       *resp) {

  return uros_rpccall_stringcall(
    "getPublishedTopics", 18, UROS_RPCP_ARRAY,
    addrp, caller_id, subgraph, resp);
}

/**
 * @brief   Retrieve list topic names and their types.
 * @see     http://www.ros.org/wiki/ROS/Master_API
 *
 * @param[in] addrp
 *          Pointer to the Master API connection address.
 * @param[in] caller_id
 *          ROS caller ID. Non-empty string pointer.
 * @param[out] resp
 *          Pointer to the @p UrosRpcResponse response object. Format:
 *          @verbatim [ int code, str statusMessage, [[str topicN, str typeN]] ]@endverbatim
 * @return
 *          Error code.
 */
uros_err_t urosRpcCallGetTopicTypes(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  UrosRpcResponse       *resp) {

  return uros_rpccall_simplecall(
    "getTopicTypes", 13, UROS_RPCP_ARRAY,
    addrp, caller_id, resp);
}

/**
 * @brief   Retrieve list representation of system state.
 * @see     http://www.ros.org/wiki/ROS/Master_API
 *
 * @param[in] addrp
 *          Pointer to the Master API connection address.
 * @param[in] caller_id
 *          ROS caller ID. Non-empty string pointer.
 * @param[out] resp
 *          Pointer to the @p UrosRpcResponse response object. Format:
@verbatim
[ int code, str statusMessage,
  [
    [ str topicN, [str topicN_publisherM] ],
    [ str topicN, [str topicN_subscriberM] ],
    [ str serviceN, [str serviceN_providerM] ]
  ]
]
@endverbatim
 * @return
 *          Error code.
 */
uros_err_t urosRpcCallGetSystemState(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  UrosRpcResponse       *resp) {

  return uros_rpccall_simplecall(
    "getSystemState", 14, UROS_RPCP_ARRAY,
    addrp, caller_id, resp);
}

/**
 * @brief   Get the URI of the the Master.
 * @see     http://www.ros.org/wiki/ROS/Master_API
 *
 * @param[in] addrp
 *          Pointer to the Master API connection address.
 * @param[in] caller_id
 *          ROS caller ID. Non-empty string pointer.
 * @param[out] resp
 *          Pointer to the @p UrosRpcResponse response object. Format:
 *          @verbatim [ int code, str statusMessage, str masterURI ]@endverbatim
 * @return
 *          Error code.
 */
uros_err_t urosRpcCallGetUri(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  UrosRpcResponse       *resp) {

  return uros_rpccall_simplecall(
    "getUri", 6, UROS_RPCP_STRING,
    addrp, caller_id, resp);
}

/**
 * @brief   Lookup all provider of a particular service.
 * @see     http://www.ros.org/wiki/ROS/Master_API
 *
 * @param[in] addrp
 *          Pointer to the Master API connection address.
 * @param[in] caller_id
 *          ROS caller ID. Non-empty string pointer.
 * @param[in] service
 *          Fully-qualified name of service. Non-empty string pointer.
 * @param[out] resp
 *          Pointer to the @p UrosRpcResponse response object. Format:
 *          @verbatim [ int code, str statusMessage, str serviceURI ]@endverbatim
 * @return
 *          Error code.
 */
uros_err_t urosRpcCallLookupService(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  const UrosString      *service,
  UrosRpcResponse       *resp) {

  urosAssert(urosStringNotEmpty(service));

  return uros_rpccall_stringcall(
    "lookupService", 13, UROS_RPCP_STRING,
    addrp, caller_id, service, resp);
}

/** @} */

/** @name Parameter server API calls */
/** @{ */

/**
 * @brief   Delete parameter.
 * @see     http://www.ros.org/wiki/ROS/Master_API
 *
 * @param[in] addrp
 *          Pointer to the Master API connection address.
 * @param[in] caller_id
 *          ROS caller ID. Non-empty string pointer.
 * @param[in] key
 *          Parameter name. Non-empty string pointer.
 * @param[out] resp
 *          Pointer to the @p UrosRpcResponse response object. Format:
 *          @verbatim [ int code, str statusMessage, any ignore ]@endverbatim
 * @return
 *          Error code.
 */
uros_err_t urosRpcCallDeleteParam(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  const UrosString      *key,
  UrosRpcResponse       *resp) {

  urosAssert(urosStringNotEmpty(key));

  return uros_rpccall_stringcall(
    "deleteParam", 11, UROS_RPCP__LENGTH,
    addrp, caller_id, key, resp);
}

/**
 * @brief   Set parameter.
 * @note    If value is a dictionary it will be treated as a parameter tree,
 *          where key is the parameter namespace.
 * @see     http://www.ros.org/wiki/ROS/Parameter%20Server%20API
 *
 * @param[in] addrp
 *          Pointer to the Master API connection address.
 * @param[in] caller_id
 *          ROS caller ID. Non-empty string pointer.
 * @param[in] key
 *          Parameter name. Non-empty string pointer.
 * @param[in] value
 *          Parameter value.
 * @param[out] resp
 *          Pointer to the @p UrosRpcResponse response object. Format:
 *          @verbatim [ int code, str statusMessage, any ignore ]@endverbatim
 * @return
 *          Error code.
 */
uros_err_t urosRpcCallSetParam(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  const UrosString      *key,
  const UrosRpcParam    *value,
  UrosRpcResponse       *resp) {

  uros_rpcpcallctx_t *ctxp;
  uros_err_t err;

  urosAssert(addrp != NULL);
  urosAssert(urosStringNotEmpty(caller_id));
  urosAssert(urosStringNotEmpty(key));
  urosAssert(value != NULL);
  urosAssert(resp != NULL);

  ctxp = urosNew(uros_rpcpcallctx_t);
  if (ctxp == NULL) { return UROS_ERR_NOMEM; }

  urosRpcResponseObjectInit(resp);
  uros_rpcpcall_buildctx(ctxp, addrp); _CHKOKE

  /* Send the request message.*/
  uros_rpccall_httprequest(&ctxp->x.streamer); _CHKOKE
  uros_rpccall_methodcall_prologue(&ctxp->x.streamer, "setParam", 8); _CHKOKE
  uros_rpccall_param_value_string(&ctxp->x.streamer, caller_id); _CHKOKE
  uros_rpccall_param_value_string(&ctxp->x.streamer, key); _CHKOKE
  uros_rpccall_param(&ctxp->x.streamer, value); _CHKOKE
  uros_rpccall_methodcall_epilogue(&ctxp->x.streamer); _CHKOKE

  /* Receive the response message.*/
  uros_rpccall_waitresponsestart(ctxp); _CHKOKE
  uros_rpccall_methodresponse(&ctxp->x.parser, resp); _CHKOKE
  if (resp->valuep->class != value->class) {
    ctxp->x.err = UROS_ERR_BADPARAM;
  } else { ctxp->x.err = UROS_OK; }

_finally:
  err = ctxp->x.err;
  uros_rpcpcall_cleanctx(ctxp);
  urosFree(ctxp);
  return err;
}

/**
 * @brief   Retrieve parameter value from server.
 * @see     http://www.ros.org/wiki/ROS/Parameter%20Server%20API
 *
 * @param[in] addrp
 *          Pointer to the Master API connection address.
 * @param[in] caller_id
 *          ROS caller ID. Non-empty string pointer.
 * @param[in] key
 *          Parameter name. If key is a namespace, it will return a parameter
 *          tree. Non-empty string pointer.
 * @param[out] resp
 *          Pointer to the @p UrosRpcResponse response object. Format:
 *          @verbatim [ int code, str statusMessage, any parameterValue ]@endverbatim
 * @return
 *          Error code.
 */
uros_err_t urosRpcCallGetParam(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  const UrosString      *key,
  UrosRpcResponse       *resp) {

  urosAssert(urosStringNotEmpty(key));

  return uros_rpccall_stringcall(
    "getParam", 8, UROS_RPCP__LENGTH,
    addrp, caller_id, key, resp);
}

/**
 * @brief   Search for parameter key on the Parameter Server.
 * @details Search starts in caller's namespace and proceeds upwards through
 *          parent namespaces until Parameter Server finds a matching key.
 *
 *          Its behavior is to search for the first partial match. For example,
 *          imagine that there are two <tt>robot_description</tt> parameters:
 *          - /robot_description
 *            - /arm
 *            - /base
 *          - /pr2
 *            - /robot_description
 *              - /base
 *
 *          If starting in the namespace <tt>/pr2/foo</tt> and search for
 *          <tt>robot_description</tt>, searchParam() will match
 *          <tt>/pr2/robot_description</tt>.
 *          If searching for <tt>robot_description/arm</tt> it will return
 *          <tt>/pr2/robot_description/arm</tt>, even though that parameter
 *          does not exist (yet).
 * @see     http://www.ros.org/wiki/ROS/Parameter%20Server%20API
 *
 * @param[in] addrp
 *          Pointer to the Master API connection address.
 * @param[in] caller_id
 *          ROS caller ID. Non-empty string pointer.
 * @param[in] key
 *          Parameter name to search for. Non-empty string pointer.
 * @param[out] resp
 *          Pointer to the @p UrosRpcResponse response object. Format:
 *          @verbatim [ int code, str statusMessage, str foundKey ]@endverbatim
 * @return
 *          Error code.
 */
uros_err_t urosRpcCallSearchParam(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  const UrosString      *key,
  UrosRpcResponse       *resp) {

  urosAssert(urosStringNotEmpty(key));

  return uros_rpccall_stringcall(
    "searchParam", 11, UROS_RPCP_STRING,
    addrp, caller_id, key, resp);
}

/**
 * @brief   Retrieve parameter value from server and subscribe to updates to
 *          that param.
 * @see     http://www.ros.org/wiki/ROS/Parameter%20Server%20API
 * @see     urosUserParamUpdate()
 *
 * @param[in] addrp
 *          Pointer to the Master API connection address.
 * @param[in] caller_id
 *          ROS caller ID. Non-empty string pointer.
 * @param[in] caller_api
 *          Node API URI of subscriber for @p urosUserParamUpdate() callbacks.
 * @param[in] key
 *          Parameter name. Non-empty string pointer.
 *          Non-empty string pointer.
 * @param[out] resp
 *          Pointer to the @p UrosRpcResponse response object. Format:
 *          @verbatim [ int code, str statusMessage, any parameterValue ]@endverbatim
 *
 *          If @p code is not 1, parameterValue should be ignored.
 *          @p parameterValue is an empty dictionary if the parameter has not
 *          been set yet.
 * @return
 *          Error code.
 */
uros_err_t urosRpcCallSubscribeParam(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  const UrosString      *caller_api,
  const UrosString      *key,
  UrosRpcResponse       *resp) {

  uros_rpcpcallctx_t *ctxp;
  uros_err_t err;

  urosAssert(addrp != NULL);
  urosAssert(urosStringNotEmpty(caller_id));
  urosAssert(urosStringNotEmpty(key));
  urosAssert(urosStringNotEmpty(caller_api));
  urosAssert(resp != NULL);

  ctxp = urosNew(uros_rpcpcallctx_t);
  if (ctxp == NULL) { return UROS_ERR_NOMEM; }

  urosRpcResponseObjectInit(resp);
  uros_rpcpcall_buildctx(ctxp, addrp); _CHKOKE

  /* Send the request message.*/
  uros_rpccall_httprequest(&ctxp->x.streamer); _CHKOKE
  uros_rpccall_methodcall_prologue(&ctxp->x.streamer, "subscribeParam", 14); _CHKOKE
  uros_rpccall_param_value_string(&ctxp->x.streamer, caller_id); _CHKOKE
  uros_rpccall_param_value_string(&ctxp->x.streamer, caller_api); _CHKOKE
  uros_rpccall_param_value_string(&ctxp->x.streamer, key); _CHKOKE
  uros_rpccall_methodcall_epilogue(&ctxp->x.streamer); _CHKOKE

  /* Receive the response message.*/
  uros_rpccall_waitresponsestart(ctxp); _CHKOKE
  uros_rpccall_methodresponse(&ctxp->x.parser, resp); _CHKOKE

_finally:
  err = ctxp->x.err;
  uros_rpcpcall_cleanctx(ctxp);
  urosFree(ctxp);
  return err;
}

/**
 * @brief   Retrieve parameter value from server and subscribe to updates to
 *          that param.
 * @see     http://www.ros.org/wiki/ROS/Parameter%20Server%20API
 * @see     urosUserParamUpdate()
 *
 * @param[in] addrp
 *          Pointer to the Master API connection address.
 * @param[in] caller_id
 *          ROS caller ID. Non-empty string pointer.
 * @param[in] caller_api
 *          Node API URI of subscriber. Non-empty string pointer.
 * @param[in] key
 *          Parameter name. Non-empty string pointer.
 * @param[out] resp
 *          Pointer to the @p UrosRpcResponse response object. Format:
 *          @verbatim [ int code, str statusMessage, int numUnsubscribed ]@endverbatim
 *
 *          If <tt>numUnsubscribed</tt> is zero, it means that the caller was
 *          not subscribed to the parameter
 * @return
 *          Error code.
 */
uros_err_t urosRpcCallUnsubscribeParam(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  const UrosString      *caller_api,
  const UrosString      *key,
  UrosRpcResponse       *resp) {

  return uros_rpccall_unregistercall(
    "unsubscribeParam", 16,
    addrp, caller_id, caller_api, key, resp);
}

/**
 * @brief   Check if parameter is stored on server.
 * @see     http://www.ros.org/wiki/ROS/Parameter%20Server%20API
 *
 * @param[in] addrp
 *          Pointer to the Master API connection address.
 * @param[in] caller_id
 *          ROS caller ID. Non-empty string pointer.
 * @param[in] key
 *          Parameter name. Non-empty string pointer.
 * @param[out] resp
 *          Pointer to the @p UrosRpcResponse response object. Format:
 *          @verbatim [ int code, str statusMessage, bool hasParam ]@endverbatim
 * @return
 *          Error code.
 */
uros_err_t urosRpcCallHasParam(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  const UrosString      *key,
  UrosRpcResponse       *resp) {

  urosAssert(key != NULL);
  urosAssert(key->length > 0);
  urosAssert(key->datap != NULL);

  return uros_rpccall_stringcall(
    "hasParam", 8, UROS_RPCP_BOOLEAN,
    addrp, caller_id, key, resp);
}

/**
 * @brief   Get list of all parameter names stored on this server.
 * @see     http://www.ros.org/wiki/ROS/Parameter%20Server%20API
 *
 * @param[in] addrp
 *          Pointer to the Master API connection address.
 * @param[in] caller_id
 *          ROS caller ID. Non-empty string pointer.
 * @param[out] resp
 *          Pointer to the @p UrosRpcResponse response object. Format:
 *          @verbatim [ int code, str statusMessage, [str paramaterNameN] ]@endverbatim
 * @return
 *          Error code.
 */
uros_err_t urosRpcCallGetParamNames(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  UrosRpcResponse       *resp) {

  return uros_rpccall_simplecall(
    "getParamNames", 13, UROS_RPCP_ARRAY,
    addrp, caller_id, resp);
}

/** @} */

/** @name Slave API calls */
/** @{ */

/**
 * @brief   Retrieve transport/topic statistics.
 * @see     http://www.ros.org/wiki/ROS/Slave_API
 *
 * @param[in] addrp
 *          Pointer to the Master API connection address.
 * @param[in] caller_id
 *          ROS caller ID. Non-empty string pointer.
 * @param[out] resp
 *          Pointer to the @p UrosRpcResponse response object. Format:
@verbatim
[ int code, str statusMessage,
  [ publishStats, subscribeStats, serviceStats ]
]
@endverbatim
            where
@verbatim
publishStats:   [ [ topicName, messageDataSent, pubConnectionData ]... ]
subscribeStats: [ [ topicName, subConnectionData ]... ]
serviceStats:   [ numRequests, bytesReceived, bytesSent ] (proposed)

pubConnectionData: [ connectionId, bytesSent, numSent, connected ]*
subConnectionData: [ connectionId, bytesReceived, dropEstimate, connected ]*
@endverbatim
            <tt>dropEstimate</tt> is <tt>-1</tt> for no estimate.
 * @return
 *          Error code.
 */
uros_err_t urosRpcCallGetBusStats(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  UrosRpcResponse       *resp) {

  return uros_rpccall_simplecall(
    "getBusStats", 11, UROS_RPCP_ARRAY,
    addrp, caller_id, resp);
}

/**
 * @brief   Retrieve transport/topic connection information.
 * @see     http://www.ros.org/wiki/ROS/Slave_API
 *
 * @param[in] addrp
 *          Pointer to the Master API connection address.
 * @param[in] caller_id
 *          ROS caller ID. Non-empty string pointer.
 * @param[out] resp
 *          Pointer to the @p UrosRpcResponse response object. Format:
 *          @verbatim [ int code, str statusMessage, busInfo ]@endverbatim
 *          where <tt>busInfo</tt> is in the form:
@verbatim
[ [
    connectionIdN,
    destinationIdN,
    directionN,
    transportN,
    topicN,
    connectedN
] ]
@endverbatim
 *          - <tt>connectionId</tt> is defined by the node and is opaque,
 *          - <tt>destinationId</tt> is the XMLRPC URI of the destination,
 *          - <tt>direction</tt> is one of <tt>'i'</tt>, <tt>'o'</tt>, or
 *            <tt>'b'</tt> (in, out, both),
 *          - <tt>transport</tt> is the transport type (e.g.
 *            <tt>'TCPROS'</tt>),
 *          - <tt>topic</tt> is the topic name,
 *          - <tt>connected</tt> indicates connection status.
 * @return
 *          Error code.
 */
uros_err_t urosRpcCallGetBusInfo(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  UrosRpcResponse       *resp) {

  return uros_rpccall_simplecall(
    "getBusInfo", 10, UROS_RPCP_ARRAY,
    addrp, caller_id, resp);
}

/**
 * @brief   Get the URI of the Master node.
 * @see     http://www.ros.org/wiki/ROS/Slave_API
 *
 * @param[in] addrp
 *          Pointer to the Master API connection address.
 * @param[in] caller_id
 *          ROS caller ID. Non-empty string pointer.
 * @param[out] resp
 *          Pointer to the @p UrosRpcResponse response object. Format:
 *          @verbatim [ int code, str statusMessage, str masterURI ]@endverbatim
 * @return
 *          Error code.
 */
uros_err_t urosRpcCallGetMasterUri(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  UrosRpcResponse       *resp) {

  return uros_rpccall_simplecall(
    "getMasterUri", 12, UROS_RPCP_STRING,
    addrp, caller_id, resp);
}

/**
 * @brief   Stop this server.
 * @see     http://www.ros.org/wiki/ROS/Slave_API
 *
 * @param[in] addrp
 *          Pointer to the Master API connection address.
 * @param[in] caller_id
 *          ROS caller ID. Non-empty string pointer.
 * @param[in] msg
 *          A message describing why the node is being shutdown. Valid string
 *          pointer.
 * @param[out] resp
 *          Pointer to the @p UrosRpcResponse response object. Format:
 *          @verbatim [ int code, str statusMessage, any ignore ]@endverbatim
 * @return
 *          Error code.
 */
uros_err_t urosRpcCallShutdown(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  const UrosString      *msg,
  UrosRpcResponse       *resp) {

  urosAssert(urosStringIsValid(msg));

  return uros_rpccall_stringcall(
    "shutdown", 8, UROS_RPCP__LENGTH,
    addrp, caller_id, msg, resp);
}

/**
 * @brief   Get the PID of this server.
 * @see     http://www.ros.org/wiki/ROS/Slave_API
 *
 * @param[in] addrp
 *          Pointer to the Master API connection address.
 * @param[in] caller_id
 *          ROS caller ID. Non-empty string pointer.
 * @param[out] resp
 *          Pointer to the @p UrosRpcResponse response object. Format:
 *          @verbatim [ int code, str statusMessage, int serverProcessPID ]@endverbatim
 * @return
 *          Error code.
 */
uros_err_t urosRpcCallGetPid(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  UrosRpcResponse       *resp) {

  return uros_rpccall_simplecall(
    "getPid", 6, UROS_RPCP_INT,
    addrp, caller_id, resp);
}

/**
 * @brief   Retrieve a list of topics that this node subscribes to.
 * @see     http://www.ros.org/wiki/ROS/Slave_API
 *
 * @param[in] addrp
 *          Pointer to the Master API connection address.
 * @param[in] caller_id
 *          ROS caller ID. Non-empty string pointer.
 * @param[out] resp
 *          Pointer to the @p UrosRpcResponse response object. Format:
 *          @verbatim [ int code, str statusMessage, [[str topicN, str topicTypeN]] ]@endverbatim
 * @return
 *          Error code.
 */
uros_err_t urosRpcCallGetSubscriptions(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  UrosRpcResponse       *resp) {

  return uros_rpccall_simplecall(
    "getSubscriptions", 16, UROS_RPCP_ARRAY,
    addrp, caller_id, resp);
}

/**
 * @brief   Retrieve a list of topics that this node publishes.
 * @see     http://www.ros.org/wiki/ROS/Slave_API
 *
 * @param[in] addrp
 *          Pointer to the Master API connection address.
 * @param[in] caller_id
 *          ROS caller ID. Non-empty string pointer.
 * @param[out] resp
 *          Pointer to the @p UrosRpcResponse response object. Format:
 *          @verbatim [ int code, str statusMessage, [[str topicN, str topicTypeN]] ]@endverbatim
 * @return
 *          Error code.
 */
uros_err_t urosRpcCallGetPublications(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  UrosRpcResponse       *resp) {

  return uros_rpccall_simplecall(
    "getPublications", 15, UROS_RPCP_ARRAY,
    addrp, caller_id, resp);
}

/**
 * @brief   Callback from master with updated value of subscribed parameter.
 * @see     http://www.ros.org/wiki/ROS/Slave_API
 * @see     urosUserParamUpdate()
 *
 * @param[in] addrp
 *          Pointer to the Master API connection address.
 * @param[in] caller_id
 *          ROS caller ID. Non-empty string pointer.
 * @param[in] parameter_key
 *          Parameter name, globally resolved. Non-empty string pointer.
 * @param[in] parameter_value
 *          New parameter value.
 * @param[out] resp
 *          Pointer to the @p UrosRpcResponse response object. Format:
 *          @verbatim [ int code, str statusMessage, any ignore ]@endverbatim
 * @return
 *          Error code.
 */
uros_err_t urosRpcCallParamUpdate(
  const UrosAddr        *addrp,
  const UrosString      *caller_id,
  const UrosString      *parameter_key,
  const UrosRpcParam    *parameter_value,
  UrosRpcResponse       *resp) {

  uros_rpcpcallctx_t *ctxp;
  uros_err_t err;

  urosAssert(addrp != NULL);
  urosAssert(urosStringNotEmpty(caller_id));
  urosAssert(urosStringNotEmpty(parameter_key));
  urosAssert(parameter_value != NULL);
  urosAssert(resp != NULL);

  ctxp = urosNew(uros_rpcpcallctx_t);
  if (ctxp == NULL) { return UROS_ERR_NOMEM; }

  urosRpcResponseObjectInit(resp);
  uros_rpcpcall_buildctx(ctxp, addrp); _CHKOKE

  /* Send the request message.*/
  uros_rpccall_httprequest(&ctxp->x.streamer); _CHKOKE
  uros_rpccall_methodcall_prologue(&ctxp->x.streamer, "paramUpdate", 11); _CHKOKE
  uros_rpccall_param_value_string(&ctxp->x.streamer, caller_id); _CHKOKE
  uros_rpccall_param_value_string(&ctxp->x.streamer, parameter_key); _CHKOKE
  uros_rpccall_param(&ctxp->x.streamer, parameter_value); _CHKOKE
  uros_rpccall_methodcall_epilogue(&ctxp->x.streamer); _CHKOKE

  /* Receive the response message.*/
  uros_rpccall_waitresponsestart(ctxp); _CHKOKE
  uros_rpccall_methodresponse(&ctxp->x.parser, resp); _CHKOKE

_finally:
  err = ctxp->x.err;
  uros_rpcpcall_cleanctx(ctxp);
  urosFree(ctxp);
  return err;
}

/**
 * @brief   Callback from master of current publisher list for specified topic.
 * @see     http://www.ros.org/wiki/ROS/Slave_API
 *
 * @param[in] addrp
 *          Pointer to the Master API connection address.
 * @param[in] caller_id
 *          ROS caller ID. Non-empty string pointer.
 * @param[in] topic
 *          Topic name. Non-empty string pointer.
 * @param[in] publishers
 *          List of current publishers for topic in the form of XMLRPC URIs.
 * @param[out] resp
 *          Pointer to the @p UrosRpcResponse response object. Format:
 *          @verbatim [ int code, str statusMessage, any ignore ]@endverbatim
 * @return
 *          Error code.
 */
uros_err_t urosRpcCallPublisherUpdate(
  const UrosAddr            *addrp,
  const UrosString          *caller_id,
  const UrosString          *topic,
  const UrosRpcParamList    *publishers,
  UrosRpcResponse           *resp) {

  uros_rpcpcallctx_t *ctxp;
  uros_err_t err;

  urosAssert(addrp != NULL);
  urosAssert(urosStringNotEmpty(caller_id));
  urosAssert(urosStringNotEmpty(topic));
  urosAssert(publishers != NULL);
  urosAssert(resp != NULL);

  ctxp = urosNew(uros_rpcpcallctx_t);
  if (ctxp == NULL) { return UROS_ERR_NOMEM; }

  urosRpcResponseObjectInit(resp);
  uros_rpcpcall_buildctx(ctxp, addrp); _CHKOKE

  /* Send the request message.*/
  uros_rpccall_httprequest(&ctxp->x.streamer); _CHKOKE
  uros_rpccall_methodcall_prologue(&ctxp->x.streamer, "publisherUpdate", 15); _CHKOKE
  uros_rpccall_param_value_string(&ctxp->x.streamer, caller_id); _CHKOKE
  uros_rpccall_param_value_string(&ctxp->x.streamer, topic); _CHKOKE
  uros_rpccall_param_value_array(&ctxp->x.streamer, publishers); _CHKOKE
  uros_rpccall_methodcall_epilogue(&ctxp->x.streamer); _CHKOKE

  /* Receive the response message.*/
  uros_rpccall_waitresponsestart(ctxp); _CHKOKE
  uros_rpccall_methodresponse(&ctxp->x.parser, resp); _CHKOKE

_finally:
  err = ctxp->x.err;
  uros_rpcpcall_cleanctx(ctxp);
  urosFree(ctxp);
  return err;
}

/**
 * @brief   Publisher node API method called by a subscriber node.
 * @details This requests that source allocate a channel for communication.
 *          Subscriber provides a list of desired protocols for communication.
 *          Publisher returns the selected protocol along with any additional
 *          params required for establishing connection. For example, for a
 *          TCP/IP-based connection, the source node may return a port number
 *          of TCP/IP server.
 * @see     http://www.ros.org/wiki/ROS/Slave_API
 *
 * @param[in] addrp
 *          Pointer to the Master API connection address.
 * @param[in] caller_id
 *          ROS caller ID. Non-empty string pointer.
 * @param[in] topic
 *          Topic name. Non-empty string pointer.
 * @param[in] protocols
 *          List of desired protocols for communication in order of preference.
 *          Each protocol is a list of the form:
 *          @verbatim [ protocolName, protocolParam1, protocolParam2, ... ]@endverbatim
 * @param[out] resp
 *          Pointer to the @p UrosRpcResponse response object. Format:
 *          @verbatim [ int code, str statusMessage, [str protocolParamN] ]@endverbatim
 * @return
 *          Error code.
 */
uros_err_t urosRpcCallRequestTopic(
  const UrosAddr            *addrp,
  const UrosString          *caller_id,
  const UrosString          *topic,
  const UrosRpcParamList    *protocols,
  UrosRpcResponse           *resp) {

  uros_rpcpcallctx_t *ctxp;
  uros_err_t err;

  urosAssert(addrp != NULL);
  urosAssert(urosStringNotEmpty(caller_id));
  urosAssert(urosStringNotEmpty(topic));
  urosAssert(protocols != NULL);
  urosAssert(resp != NULL);

  ctxp = urosNew(uros_rpcpcallctx_t);
  if (ctxp == NULL) { return UROS_ERR_NOMEM; }

  urosRpcResponseObjectInit(resp);
  uros_rpcpcall_buildctx(ctxp, addrp); _CHKOKE

  /* Send the request message.*/
  uros_rpccall_httprequest(&ctxp->x.streamer); _CHKOKE
  uros_rpccall_methodcall_prologue(&ctxp->x.streamer, "requestTopic", 12); _CHKOKE
  uros_rpccall_param_value_string(&ctxp->x.streamer, caller_id); _CHKOKE
  uros_rpccall_param_value_string(&ctxp->x.streamer, topic); _CHKOKE
  uros_rpccall_param_value_array(&ctxp->x.streamer, protocols); _CHKOKE
  uros_rpccall_methodcall_epilogue(&ctxp->x.streamer); _CHKOKE

  /* Receive the response message.*/
  uros_rpccall_waitresponsestart(ctxp); _CHKOKE
  uros_rpccall_methodresponse(&ctxp->x.parser, resp); _CHKOKE
  if (resp->valuep->class != UROS_RPCP_ARRAY) {
    ctxp->x.err = UROS_ERR_BADPARAM;
  } else { ctxp->x.err = UROS_OK; }

_finally:
  err = ctxp->x.err;
  uros_rpcpcall_cleanctx(ctxp);
  urosFree(ctxp);
  return err;
}

/** @} */
/** @} */
