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
 * @file    urosRpcSlave.c
 * @author  Andrea Zoppi <texzk@email.it>
 *
 * @brief   XMLRPC Slave API functions.
 */

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include "../include/urosBase.h"
#include "../include/urosUser.h"
#include "../include/urosRpcSlave.h"
#include "../include/urosRpcParser.h"
#include "../include/urosRpcStreamer.h"
#include "../include/urosNode.h"

#include <string.h>

/*===========================================================================*/
/* LOCAL TYPES & MACROS                                                      */
/*===========================================================================*/

#if UROS_RPCSLAVE_C_USE_ASSERT == UROS_FALSE && !defined(__DOXYGEN__)
#undef urosAssert
#define urosAssert(expr)
#endif

#if UROS_RPCSLAVE_C_USE_ERROR_MSG == UROS_FALSE && !defined(__DOXYGEN__)
#undef urosError
#define urosError(when, action, msgargs) { if (when) { action; } }
#endif

/*===========================================================================*/
/* LOCAL FUNCTIONS                                                           */
/*===========================================================================*/

uros_err_t uros_rpcslave_methodresponse_prologue(UrosRpcStreamer *sp) {

  urosAssert(sp != NULL);
#define _CHKOK   { if (sp->err != UROS_OK) { return sp->err; } }

  /* Print the HTTP status headers.*/
  sp->chunked = UROS_FALSE;
  urosRpcStreamerHttpStatus(sp, 200); _CHKOK

  /* Using XML content (XMLRPC).*/
  urosRpcStreamerHttpHeader(sp, "Content-Type", 12, "text/xml", 8); _CHKOK

  /* Content-Length default value (fixed).*/
  urosRpcStreamerHttpContentLength(sp); _CHKOK
  urosRpcStreamerHttpEnd(sp); _CHKOK

  /* XML header.*/
  urosRpcStreamerXmlHeader(sp); _CHKOK

  /* XMLRPC response prologue. */
  urosRpcStreamerXmlTagOpen(sp, "methodResponse", 14); _CHKOK
  urosRpcStreamerXmlTagOpen(sp, "params", 6); _CHKOK
  urosRpcStreamerXmlTagOpen(sp, "param", 5); _CHKOK
  urosRpcStreamerXmlTagOpen(sp, "value", 5); _CHKOK
  urosRpcStreamerXmlTagOpen(sp, "array", 5); _CHKOK
  urosRpcStreamerXmlTagOpen(sp, "data", 4); _CHKOK

  return sp->err = UROS_OK;
#undef _CHKOK
}

uros_err_t uros_rpcslave_methodresponse_epilogue(UrosRpcStreamer *sp) {

  urosAssert(sp != NULL);
#define _CHKOK   { if (sp->err != UROS_OK) { return sp->err; } }

  /* XMLRPC response epilogue. */
  urosRpcStreamerXmlTagClose(sp, "data", 4); _CHKOK
  urosRpcStreamerXmlTagClose(sp, "array", 5); _CHKOK
  urosRpcStreamerXmlTagClose(sp, "value", 5); _CHKOK
  urosRpcStreamerXmlTagClose(sp, "param", 5); _CHKOK
  urosRpcStreamerXmlTagClose(sp, "params", 6); _CHKOK
  urosRpcStreamerXmlTagClose(sp, "methodResponse", 14); _CHKOK
  urosRpcStreamerWrite(sp, "\r\n", 2); _CHKOK

  /* Fix the actual Content-Length.*/
  urosRpcStreamerXmlEndHack(sp); _CHKOK
  return urosRpcStreamerFlush(sp);
#undef _CHKOK
}

uros_err_t uros_rpcslave_value_int(UrosRpcStreamer *sp, int32_t code) {

  urosAssert(sp != NULL);
#define _CHKOK   { if (sp->err != UROS_OK) { return sp->err; } }

  urosRpcStreamerXmlTagOpen(sp, "value", 5); _CHKOK
  urosRpcStreamerXmlTagOpen(sp, "i4", 2); _CHKOK
  urosRpcStreamerInt32(sp, code); _CHKOK
  urosRpcStreamerXmlTagClose(sp, "i4", 2); _CHKOK
  urosRpcStreamerXmlTagClose(sp, "value", 5); _CHKOK

  return sp->err = UROS_OK;
#undef _CHKOK
}

uros_err_t uros_rpcslave_value_string(UrosRpcStreamer *sp,
                                      const char *txtp, size_t txtlen) {

  urosAssert(sp != NULL);
#define _CHKOK   { if (sp->err != UROS_OK) { return sp->err; } }

  urosRpcStreamerXmlTagOpen(sp, "value", 5); _CHKOK
#if UROS_RPCSTREAMER_USE_STRING_TAG != UROS_FALSE
  urosRpcStreamerXmlTagOpen(sp, "string", 6); _CHKOK
#endif
  if (txtp != NULL) {
    urosRpcStreamerWrite(sp, txtp, txtlen); _CHKOK
  }
#if UROS_RPCSTREAMER_USE_STRING_TAG != UROS_FALSE
  urosRpcStreamerXmlTagClose(sp, "string", 6); _CHKOK
#endif
  urosRpcStreamerXmlTagClose(sp, "value", 5); _CHKOK

  return sp->err = UROS_OK;
#undef _CHKOK
}

uros_err_t uros_rpcslave_value_array_begin(UrosRpcStreamer *sp) {

  urosAssert(sp != NULL);
#define _CHKOK   { if (sp->err != UROS_OK) { return sp->err; } }

  urosRpcStreamerXmlTagOpen(sp, "value", 5); _CHKOK
  urosRpcStreamerXmlTagOpen(sp, "array", 5); _CHKOK
  urosRpcStreamerXmlTagOpen(sp, "data", 4); _CHKOK

  return sp->err = UROS_OK;
#undef _CHKOK
}

uros_err_t uros_rpcslave_value_array_end(UrosRpcStreamer *sp) {

  urosAssert(sp != NULL);
#define _CHKOK   { if (sp->err != UROS_OK) { return sp->err; } }

  urosRpcStreamerXmlTagClose(sp, "data", 4); _CHKOK
  urosRpcStreamerXmlTagClose(sp, "array", 5); _CHKOK
  urosRpcStreamerXmlTagClose(sp, "value", 5); _CHKOK

  return sp->err = UROS_OK;
#undef _CHKOK
}

uros_err_t uros_rpcslave_value_string_ip(UrosRpcStreamer *sp, UrosIp ip) {

  urosAssert(sp != NULL);
#define _CHKOK   { if (sp->err != UROS_OK) { return sp->err; } }

  urosRpcStreamerXmlTagOpen(sp, "value", 5); _CHKOK
#if UROS_RPCSTREAMER_USE_STRING_TAG != UROS_FALSE
  urosRpcStreamerXmlTagOpen(sp, "string", 6); _CHKOK
#endif
  urosRpcStreamerIp(sp, ip);
#if UROS_RPCSTREAMER_USE_STRING_TAG != UROS_FALSE
  urosRpcStreamerXmlTagClose(sp, "string", 6); _CHKOK
#endif
  urosRpcStreamerXmlTagClose(sp, "value", 5); _CHKOK

  return sp->err = UROS_OK;
#undef _CHKOK
}

uros_err_t uros_rpcslave_process_publisherupdate(const UrosString *topic,
                                                 const UrosRpcParam *publishers) {

  static UrosNodeStatus *const stp = &urosNode.status;

  const UrosListNode *topicnodep;
  UrosList newpubs;
  uros_err_t err;

  urosAssert(urosStringNotEmpty(topic));
  urosAssert(publishers != NULL);
  urosAssert(publishers->pclass == UROS_RPCP_ARRAY);
  urosAssert(publishers->value.listp != NULL);

  /* Check if the topic is actually subscribed.*/
  urosMutexLock(&stp->subTopicListLock);
  topicnodep = urosTopicListFindByName(&stp->subTopicList, topic);
  urosMutexUnlock(&stp->subTopicListLock);
  urosError(topicnodep == NULL, return UROS_ERR_BADPARAM,
            ("Topic [%.*s] not found\n", UROS_STRARG(topic)));

  /* Get new publishers and connect.*/
  urosListObjectInit(&newpubs);
  err = urosNodeFindNewTopicPublishers(topic, publishers, &newpubs);
  urosError(err != UROS_OK, return err,
            ("Error %s while looking for new publishers\n",
             urosErrorText(err)));
  urosRpcSlaveConnectToPublishers(topic, &newpubs);

  urosListClean(&newpubs, (uros_delete_f)urosFree);
  return UROS_OK;
}

uros_err_t uros_rpcslave_xmlmethodname(UrosRpcParser *pp,
                                       uros_rpcslave_methodid_t *methodp) {

  urosAssert(pp != NULL);
  urosAssert(methodp != NULL);
#define _GOT(tokp, toklen) \
    (urosRpcParserExpectQuiet(pp, tokp, toklen) == UROS_OK)

  urosRpcParserXmlTagOpen(pp, "methodName", 10);
  if (pp->err != UROS_OK) { return pp->err; }

  if (_GOT("get", 3)) {
    if (_GOT("Bus", 3)) {
      if (_GOT("Info", 4)) {
        *methodp = UROS_RPCSM_GET_BUS_INFO;
      } else if (_GOT("Status", 6)) {
        *methodp = UROS_RPCSM_GET_BUS_STATS;
      }
    } else if (_GOT("MasterUri", 9)) {
      *methodp = UROS_RPCSM_GET_MASTER_URI;
    } else if (_GOT("P", 1)) {
      if (_GOT("id", 2)) {
        *methodp = UROS_RPCSM_GET_PID;
      } else if (_GOT("ublications", 12)) {
        *methodp = UROS_RPCSM_GET_PUBLICATIONS;
      }
    } else if (_GOT("Subscriptions", 13)) {
      *methodp = UROS_RPCSM_GET_SUBSCRIPTIONS;
    }
  } else if (_GOT("p", 1)) {
    if (_GOT("aramUpdate", 10)) {
      *methodp = UROS_RPCSM_PARAM_UPDATE;
    } else if (_GOT("ublisherUpdate", 14)) {
      *methodp = UROS_RPCSM_PUBLISHER_UPDATE;
    }
  } else if (_GOT("requestTopic", 12)) {
    *methodp = UROS_RPCSM_REQUEST_TOPIC;
  } else if (_GOT("shutdown", 8)) {
    *methodp = UROS_RPCSM_SHUTDOWN;
  }

  if (pp->err == UROS_OK) {
    urosRpcParserXmlTagClose(pp, "methodName", 10);
  }
  return pp->err;
#undef _GOT
}

uros_err_t uros_rpcslave_receive_parambyclass(UrosRpcParser *pp,
                                              UrosRpcParam *paramp) {

  urosAssert(pp != NULL);
  urosAssert(paramp != NULL);
#define _CHKOK   { if (pp->err != UROS_OK) { return pp->err; } }

  urosRpcParserXmlTagOpen(pp, "param", 5); _CHKOK
  urosRpcParserSkipWs(pp); _CHKOK
  urosRpcParserParamByClass(pp, paramp); _CHKOK
  urosRpcParserSkipWs(pp); _CHKOK
  urosRpcParserXmlTagClose(pp, "param", 5); _CHKOK
  return pp->err = UROS_OK;

#undef _CHKOK
}

uros_err_t uros_rpcslave_receive_params(UrosRpcParser *pp,
                                        uros_rpcslave_methodid_t methodid,
                                        UrosRpcParamList *parlistp) {

  UrosRpcParamNode *str1;

  urosAssert(pp != NULL);
  urosAssert(parlistp != NULL);
  urosAssert(parlistp->length == 0);
  urosAssert(parlistp->headp == NULL);
  urosAssert(parlistp->tailp == NULL);
#define _CHKOK   { if (pp->err != UROS_OK) { goto _error; } }

  urosRpcParserXmlTagOpen(pp, "params", 6); _CHKOK
  urosRpcParserSkipWs(pp); _CHKOK

  /* str caller_id always as first parameter.*/
  str1 = urosNew(NULL, UrosRpcParamNode);
  if (str1 == NULL) { return UROS_ERR_NOMEM; }
  urosRpcParamNodeObjectInit(str1, UROS_RPCP_STRING);
  urosRpcParamListAppendNode(parlistp, str1);
  uros_rpcslave_receive_parambyclass(pp, &str1->param); _CHKOK

  switch (methodid) {
  case UROS_RPCSM_GET_BUS_INFO:
  case UROS_RPCSM_GET_BUS_STATS:
  case UROS_RPCSM_GET_MASTER_URI:
  case UROS_RPCSM_GET_PID:
  case UROS_RPCSM_GET_PUBLICATIONS:
  case UROS_RPCSM_GET_SUBSCRIPTIONS: {
    /* caller_id already read.*/
    break;
  }
  case UROS_RPCSM_PARAM_UPDATE: {
    UrosRpcParamNode *str2 = urosNew(NULL, UrosRpcParamNode);
    UrosRpcParamNode *any3 = urosNew(NULL, UrosRpcParamNode);
    if (str2 == NULL || any3 == NULL) {
      urosFree(str2); urosFree(any3);
      pp->err = UROS_ERR_NOMEM; goto _error;
    }
    urosRpcParamNodeObjectInit(str2, UROS_RPCP_STRING);
    urosRpcParamNodeObjectInit(any3, UROS_RPCP__LENGTH);
    urosRpcParamListAppendNode(parlistp, str2);
    urosRpcParamListAppendNode(parlistp, any3);

    urosRpcParserSkipWs(pp); _CHKOK
    uros_rpcslave_receive_parambyclass(pp, &str2->param); _CHKOK
    urosRpcParserSkipWs(pp); _CHKOK

    urosRpcParserXmlTagOpen(pp, "param", 5); _CHKOK
    urosRpcParserSkipWs(pp); _CHKOK
    urosRpcParserParamByTag(pp, &any3->param); _CHKOK
    urosRpcParserSkipWs(pp); _CHKOK
    urosRpcParserXmlTagClose(pp, "param", 5); _CHKOK
    break;
  }
  case UROS_RPCSM_PUBLISHER_UPDATE:
  case UROS_RPCSM_REQUEST_TOPIC: {
    UrosRpcParamNode *str2 = urosNew(NULL, UrosRpcParamNode);
    UrosRpcParamNode *array3 = urosNew(NULL, UrosRpcParamNode);
    if (str2 == NULL || array3 == NULL) {
      urosFree(str2); urosFree(array3);
      pp->err = UROS_ERR_NOMEM; goto _error;
    }
    urosRpcParamNodeObjectInit(str2, UROS_RPCP_STRING);
    urosRpcParamNodeObjectInit(array3, UROS_RPCP_ARRAY);
    urosRpcParamListAppendNode(parlistp, str2);
    urosRpcParamListAppendNode(parlistp, array3);

    urosRpcParserSkipWs(pp); _CHKOK
    uros_rpcslave_receive_parambyclass(pp, &str2->param); _CHKOK
    urosRpcParserSkipWs(pp); _CHKOK
    uros_rpcslave_receive_parambyclass(pp, &array3->param); _CHKOK
    break;
  }
  case UROS_RPCSM_SHUTDOWN: {
    UrosRpcParamNode *str2 = urosNew(NULL, UrosRpcParamNode);
    if (str2 == NULL) { pp->err = UROS_ERR_NOMEM; goto _error; }
    urosRpcParamNodeObjectInit(str2, UROS_RPCP_STRING);
    urosRpcParamListAppendNode(parlistp, str2);

    urosRpcParserSkipWs(pp); _CHKOK
    uros_rpcslave_receive_parambyclass(pp, &str2->param); _CHKOK
    break;
  }
  default: {
    urosError(UROS_ERR_PARSE, UROS_NOP,
              ("Unknown XMLRPC Slave method id %d\n", (int)methodid));
    pp->err = UROS_ERR_PARSE;
    goto _error;
  }
  }

  urosRpcParserSkipWs(pp); _CHKOK
  urosRpcParserXmlTagClose(pp, "params", 6); _CHKOK
  return pp->err = UROS_OK;

_error:
  urosRpcParamListClean(parlistp, UROS_TRUE);
  return pp->err;
#undef _CHKOK
}

uros_err_t uros_rpcslave_xmlmethodcall(UrosRpcParser *pp,
                                       uros_rpcslave_methodid_t *methodidp,
                                       UrosRpcParamList *parlistp) {

  urosAssert(pp != NULL);
  urosAssert(parlistp != NULL);
  urosAssert(parlistp->length == 0);
  urosAssert(parlistp->headp == NULL);
  urosAssert(parlistp->tailp == 0);
#define _CHKOK   { if (pp->err != UROS_OK) { return pp->err; } }

  urosRpcParserXmlTagOpen(pp, "methodCall", 10); _CHKOK
  urosRpcParserSkipWs(pp); _CHKOK
  uros_rpcslave_xmlmethodname(pp, methodidp); _CHKOK
  urosRpcParserSkipWs(pp); _CHKOK
  uros_rpcslave_receive_params(pp, *methodidp, parlistp); _CHKOK
  urosRpcParserSkipWs(pp); _CHKOK
  urosRpcParserXmlTagClose(pp, "methodCall", 10); _CHKOK

  return pp->err = UROS_OK;
#undef _CHKOK
}

uros_err_t uros_rpcslave_method_getbusinfo(UrosRpcStreamer *sp,
                                           UrosRpcParamList *parlistp) {

  (void)parlistp;

  urosAssert(sp != NULL);
  urosAssert(parlistp != NULL);
#define _CHKOK   { if (sp->err != UROS_OK) { return sp->err; } }

  /* Generate the HTTP response.*/
  uros_rpcslave_methodresponse_prologue(sp); _CHKOK

  /* int code */
  uros_rpcslave_value_int(sp, UROS_RPCC_SUCCESS); _CHKOK

  /* str statusMessage */
  uros_rpcslave_value_string(sp, NULL, 0); _CHKOK

  /* FIXME: Fake empty response: [] because stats are too
   * cumbersome to be tracked.*/
  uros_rpcslave_value_array_begin(sp); _CHKOK
  uros_rpcslave_value_array_end(sp); _CHKOK

  uros_rpcslave_methodresponse_epilogue(sp); _CHKOK
  return sp->err = UROS_OK;
#undef _CHKOK
}

uros_err_t uros_rpcslave_method_getbusstats(UrosRpcStreamer *sp,
                                            UrosRpcParamList *parlistp) {

  (void)parlistp;

  urosAssert(sp != NULL);
  urosAssert(parlistp != NULL);
#define _CHKOK   { if (sp->err != UROS_OK) { return sp->err; } }

  /* Generate the HTTP response.*/
  uros_rpcslave_methodresponse_prologue(sp); _CHKOK

  /* int code */
  uros_rpcslave_value_int(sp, UROS_RPCC_SUCCESS); _CHKOK

  /* str statusMessage */
  uros_rpcslave_value_string(sp, NULL, 0); _CHKOK

  /* FIXME: Fake empty response: [[], [], []] because stats are too
   * cumbersome to be tracked.*/
  uros_rpcslave_value_array_begin(sp); _CHKOK
  uros_rpcslave_value_array_begin(sp); _CHKOK
  uros_rpcslave_value_array_end(sp); _CHKOK
  uros_rpcslave_value_array_begin(sp); _CHKOK
  uros_rpcslave_value_array_end(sp); _CHKOK
  uros_rpcslave_value_array_begin(sp); _CHKOK
  uros_rpcslave_value_array_end(sp); _CHKOK
  uros_rpcslave_value_array_end(sp); _CHKOK

  uros_rpcslave_methodresponse_epilogue(sp); _CHKOK
  return sp->err = UROS_OK;
#undef _CHKOK
}

uros_err_t uros_rpcslave_method_getmasteruri(UrosRpcStreamer *sp,
                                             UrosRpcParamList *parlistp) {

  static const UrosNodeConfig *const cfgp = &urosNode.config;

  const UrosString *masterURI;
  (void)parlistp;

  urosAssert(sp != NULL);
  urosAssert(parlistp != NULL);
#define _CHKOK   { if (sp->err != UROS_OK) { return sp->err; } }

  /* Generate the HTTP response.*/
  uros_rpcslave_methodresponse_prologue(sp); _CHKOK

  /* int code */
  uros_rpcslave_value_int(sp, UROS_RPCC_SUCCESS); _CHKOK

  /* str statusMessage */
  uros_rpcslave_value_string(sp, NULL, 0); _CHKOK

  /* str masterURI.*/
  masterURI = &cfgp->masterUri;
  uros_rpcslave_value_string(sp, masterURI->datap, masterURI->length);

  uros_rpcslave_methodresponse_epilogue(sp); _CHKOK
  return sp->err;
#undef _CHKOK
}

uros_err_t uros_rpcslave_method_getpid(UrosRpcStreamer *sp,
                                       UrosRpcParamList *parlistp) {

  static UrosNodeStatus *const stp = &urosNode.status;

  int32_t pid;
  (void)parlistp;

  urosAssert(sp != NULL);
  urosAssert(parlistp != NULL);
#define _CHKOK   { if (sp->err != UROS_OK) { return sp->err; } }

  urosMutexLock(&stp->xmlrpcPidLock);
  pid = stp->xmlrpcPid;
  urosMutexUnlock(&stp->xmlrpcPidLock);

  /* Generate the HTTP response.*/
  uros_rpcslave_methodresponse_prologue(sp); _CHKOK

  /* int code */
  uros_rpcslave_value_int(sp, UROS_RPCC_SUCCESS); _CHKOK

  /* str statusMessage */
  uros_rpcslave_value_string(sp, NULL, 0); _CHKOK

  /* int serverProcessPID.*/
  uros_rpcslave_value_int(sp, pid); _CHKOK

  uros_rpcslave_methodresponse_epilogue(sp); _CHKOK
  return sp->err = UROS_OK;
#undef _CHKOK
}

uros_err_t uros_rpcslave_method_getpublications(UrosRpcStreamer *sp,
                                                UrosRpcParamList *parlistp) {

  static UrosNodeStatus *const stp = &urosNode.status;

  const UrosListNode *curp;
  const UrosString *strp;
  (void)parlistp;

  urosAssert(sp != NULL);
  urosAssert(parlistp != NULL);
#define _CHKOK   { if (sp->err != UROS_OK) { goto _finally; } }

  urosMutexLock(&stp->pubTopicListLock);

  /* Generate the HTTP response.*/
  uros_rpcslave_methodresponse_prologue(sp); _CHKOK

  /* int code */
  uros_rpcslave_value_int(sp, UROS_RPCC_SUCCESS); _CHKOK

  /* str statusMessage */
  uros_rpcslave_value_string(sp, NULL, 0); _CHKOK

  /* [[str topic, str type]*] */
  uros_rpcslave_value_array_begin(sp); _CHKOK
  for (curp = stp->pubTopicList.headp; curp != NULL; curp = curp->nextp) {
    uros_rpcslave_value_array_begin(sp); _CHKOK
    strp = &((const UrosTopic *)curp->datap)->name;
    uros_rpcslave_value_string(sp, strp->datap, strp->length); _CHKOK
    strp = &((const UrosTopic *)curp->datap)->typep->name;
    uros_rpcslave_value_string(sp, strp->datap, strp->length); _CHKOK
    uros_rpcslave_value_array_end(sp); _CHKOK
  }
  uros_rpcslave_value_array_end(sp); _CHKOK

  uros_rpcslave_methodresponse_epilogue(sp); _CHKOK
  sp->err = UROS_OK;

_finally:
  urosMutexUnlock(&stp->pubTopicListLock);
  return sp->err;
#undef _CHKOK
}

uros_err_t uros_rpcslave_method_getsubscriptions(UrosRpcStreamer *sp,
                                                 UrosRpcParamList *parlistp) {

  static UrosNodeStatus *const stp = &urosNode.status;

  const UrosListNode *curp;
  const UrosString *strp;
  (void)parlistp;

  urosAssert(sp != NULL);
  urosAssert(parlistp != NULL);
#define _CHKOK   { if (sp->err != UROS_OK) { goto _finally; } }

  urosMutexLock(&stp->subTopicListLock);

  /* Generate the HTTP response.*/
  uros_rpcslave_methodresponse_prologue(sp); _CHKOK

  /* int code */
  uros_rpcslave_value_int(sp, UROS_RPCC_SUCCESS); _CHKOK

  /* str statusMessage */
  uros_rpcslave_value_string(sp, NULL, 0); _CHKOK

  /* [[str topic, str type]*] */
  uros_rpcslave_value_array_begin(sp); _CHKOK
  for (curp = stp->subTopicList.headp; curp != NULL; curp = curp->nextp) {
    uros_rpcslave_value_array_begin(sp); _CHKOK
    strp = &((const UrosTopic *)curp->datap)->name;
    uros_rpcslave_value_string(sp, strp->datap, strp->length); _CHKOK
    strp = &((const UrosTopic *)curp->datap)->typep->name;
    uros_rpcslave_value_string(sp, strp->datap, strp->length); _CHKOK
    uros_rpcslave_value_array_end(sp); _CHKOK
  }
  uros_rpcslave_value_array_end(sp); _CHKOK

  uros_rpcslave_methodresponse_epilogue(sp); _CHKOK
  sp->err = UROS_OK;

_finally:
  urosMutexUnlock(&stp->subTopicListLock);
  return sp->err;
#undef _CHKOK
}

uros_err_t uros_rpcslave_method_paramupdate(UrosRpcStreamer *sp,
                                            UrosRpcParamList *parlistp) {

  const UrosRpcParamNode *paramnodep;
  const UrosRpcParam *caller_id, *parameter_key, *parameter_value;
  UrosString keystr;
  uros_err_t err;

  urosAssert(sp != NULL);
  urosAssert(parlistp != NULL);
#define _CHKOK   { if (sp->err != UROS_OK) { return sp->err; } }

  /* Get parameters and check.*/
  paramnodep = parlistp->headp;
  urosError(paramnodep == NULL, return sp->err = UROS_ERR_BADPARAM,
            ("Expecting a further parameter\n"));
  caller_id = &paramnodep->param;
  urosError(caller_id->pclass != UROS_RPCP_STRING,
            return sp->err = UROS_ERR_BADPARAM,
            ("Class id of [caller_id] is %d, expected %d (UROS_RPCP_STRING)\n",
             (int)caller_id->pclass, (int)UROS_RPCP_STRING));

  paramnodep = paramnodep->nextp;
  urosError(paramnodep == NULL, return sp->err = UROS_ERR_BADPARAM,
            ("Expecting a further parameter\n"));
  parameter_key = (UrosRpcParam*)&paramnodep->param;
  urosError(parameter_key->pclass != UROS_RPCP_STRING,
            return sp->err = UROS_ERR_BADPARAM,
            ("Class id of [parameter_key] is %d, expected %d "
             "(UROS_RPCP_STRING)\n",
             (int)parameter_key->pclass, (int)UROS_RPCP_STRING));

  paramnodep = paramnodep->nextp;
  urosError(paramnodep == NULL, return sp->err = UROS_ERR_BADPARAM,
            ("Expecting a further parameter\n"));
  parameter_value = &paramnodep->param;

  /* Fix the key length, if there is a trailing '/'.*/
  keystr = parameter_key->value.string;
  if (keystr.length > 0 && keystr.datap[keystr.length - 1] == '/') {
    --keystr.length;
  }

  /* Update the parameter value.*/
  err = urosUserParamUpdate(&keystr, parameter_value);
  urosError(err != UROS_OK, UROS_NOP,
            ("Cannot update param [%.*s]\n", UROS_STRARG(&keystr)));

  /* Generate the HTTP response.*/
  uros_rpcslave_methodresponse_prologue(sp); _CHKOK

  /* int code */
  err = (err == UROS_OK) ? UROS_RPCC_SUCCESS : UROS_RPCC_FAILURE;
  uros_rpcslave_value_int(sp, (int32_t)err); _CHKOK

  /* str statusMessage */
  uros_rpcslave_value_string(sp, NULL, 0); _CHKOK

  /* int ignore */
  uros_rpcslave_value_int(sp, 0); _CHKOK

  uros_rpcslave_methodresponse_epilogue(sp); _CHKOK
  return sp->err = UROS_OK;
#undef _CHKOK
}

uros_err_t uros_rpcslave_method_publisherupdate(UrosRpcStreamer *sp,
                                                UrosRpcParamList *parlistp) {

  const UrosRpcParamNode *paramnodep;
  const UrosRpcParam *caller_id, *topic, *publishers;
  const UrosString *topicstrp;
  uros_err_t err;

  urosAssert(sp != NULL);
  urosAssert(parlistp != NULL);
#define _CHKOK   { if (sp->err != UROS_OK) { return sp->err; } }

  /* Get parameters and check.*/
  paramnodep = parlistp->headp;
  urosError(paramnodep == NULL, return sp->err = UROS_ERR_BADPARAM,
            ("Expecting a further parameter\n"));
  caller_id = &paramnodep->param;
  urosError(caller_id->pclass != UROS_RPCP_STRING,
            return sp->err = UROS_ERR_BADPARAM,
            ("Class id of [caller_id] is %d, expected %d (UROS_RPCP_STRING)\n",
             (int)caller_id->pclass, (int)UROS_RPCP_STRING));

  paramnodep = paramnodep->nextp;
  urosError(paramnodep == NULL, return sp->err = UROS_ERR_BADPARAM,
            ("Expecting a further parameter\n"));
  topic = &paramnodep->param;
  urosError(topic->pclass != UROS_RPCP_STRING,
            return sp->err = UROS_ERR_BADPARAM,
            ("Class id of [topic] is %d, expected %d (UROS_RPCP_STRING)\n",
             (int)topic->pclass, (int)UROS_RPCP_STRING));
  topicstrp = &topic->value.string;

  paramnodep = paramnodep->nextp;
  urosError(paramnodep == NULL, return sp->err = UROS_ERR_BADPARAM,
            ("Expecting a further parameter\n"));
  publishers = &paramnodep->param;
  urosError(publishers->pclass != UROS_RPCP_ARRAY,
            return sp->err = UROS_ERR_BADPARAM,
            ("Class id of [publishers] is %d, expected %d (UROS_RPCP_ARRAY)\n",
             (int)publishers->pclass, (int)UROS_RPCP_ARRAY));

  /* Process new publishers and dispose the old ones.*/
  err = uros_rpcslave_process_publisherupdate(topicstrp, publishers);
  urosError(err != UROS_OK, UROS_NOP,
            ("Cannot process publisherUpdate() for topic [%.*s]\n",
             UROS_STRARG(topicstrp)));

  /* Generate the HTTP response.*/
  uros_rpcslave_methodresponse_prologue(sp); _CHKOK

  /* int code */
  err = (err == UROS_OK) ? UROS_RPCC_SUCCESS : UROS_RPCC_FAILURE;
  uros_rpcslave_value_int(sp, (int32_t)err); _CHKOK

  /* str statusMessage */
  uros_rpcslave_value_string(sp, NULL, 0); _CHKOK

  /* int ignore */
  uros_rpcslave_value_int(sp, 0); _CHKOK

  uros_rpcslave_methodresponse_epilogue(sp); _CHKOK
  return sp->err = UROS_OK;
  return sp->err = UROS_OK;
#undef _CHKOK
}

uros_err_t uros_rpcslave_method_requesttopic(UrosRpcStreamer *sp,
                                             UrosRpcParamList *parlistp) {

  static const UrosNodeConfig *const cfgp = &urosNode.config;
  static UrosNodeStatus *const stp = &urosNode.status;
  static const UrosString tcprosstr = { 6, "TCPROS" };

  const UrosRpcParamNode *paramnodep;
  const UrosRpcParam *caller_id, *topic, *protocols;
  const UrosRpcParamList *protolistp;
  UrosListNode *topicnodep;
  uros_bool_t tcpros = UROS_FALSE;

  urosAssert(sp != NULL);
  urosAssert(parlistp != NULL);
#define _CHKOK   { if (sp->err != UROS_OK) { return sp->err; } }

  /* Get parameters and check.*/
  paramnodep = parlistp->headp;
  urosError(paramnodep == NULL, return sp->err = UROS_ERR_BADPARAM,
            ("Expecting a further parameter\n"));
  caller_id = &paramnodep->param;
  urosError(caller_id->pclass != UROS_RPCP_STRING,
            return sp->err = UROS_ERR_BADPARAM,
            ("Class id of [caller_id] is %d, expected %d (UROS_RPCP_STRING)\n",
             (int)caller_id->pclass, (int)UROS_RPCP_STRING));

  paramnodep = paramnodep->nextp;
  urosError(paramnodep == NULL, return sp->err = UROS_ERR_BADPARAM,
            ("Expecting a further parameter\n"));
  topic = &paramnodep->param;
  urosError(topic->pclass != UROS_RPCP_STRING,
            return sp->err = UROS_ERR_BADPARAM,
            ("Class id of [topic] is %d, expected %d (UROS_RPCP_STRING)\n",
             (int)topic->pclass, (int)UROS_RPCP_STRING));

  paramnodep = paramnodep->nextp;
  urosError(paramnodep == NULL, return sp->err = UROS_ERR_BADPARAM,
            ("Expecting a further parameter\n"));
  protocols = &paramnodep->param;
  if (protocols->pclass != UROS_RPCP_ARRAY) {
    return sp->err = UROS_ERR_BADPARAM;
  }

  /* Check if the topic is actually published.*/
  urosMutexLock(&stp->pubTopicListLock);
  topicnodep = urosTopicListFindByName(&stp->pubTopicList, &topic->value.string);
  urosMutexUnlock(&stp->pubTopicListLock);
  urosError(topicnodep == NULL, return UROS_ERR_BADPARAM,
            ("Topic [%.*s] not found\n", UROS_STRARG(&topic->value.string)));

  /* Check if TCPROS is supported by the caller.*/
  urosAssert(protocols->value.listp != NULL);
  protolistp = protocols->value.listp;
  if (protolistp->length == 0) { return sp->err = UROS_ERR_BADPARAM; }
  for (paramnodep = protolistp->headp; paramnodep != NULL; paramnodep = paramnodep->nextp) {
    const UrosRpcParamList *childlistp;
    const UrosRpcParam *nameparamp;
    const UrosString *namestrp;

    urosError(paramnodep->param.pclass != UROS_RPCP_ARRAY,
              return sp->err = UROS_ERR_BADPARAM,
              ("Class id of parameter is %d, expected %d (UROS_RPCP_ARRAY)\n",
               (int)paramnodep->param.pclass, (int)UROS_RPCP_ARRAY));
    childlistp = paramnodep->param.value.listp;
    urosAssert(childlistp != NULL);
    urosError(childlistp->length == 0, return sp->err = UROS_ERR_BADPARAM,
              ("Expecting a non-empty array\n"));
    nameparamp = &childlistp->headp->param;
    urosError(nameparamp->pclass != UROS_RPCP_STRING,
              return sp->err = UROS_ERR_BADPARAM,
              ("Class id of array value is %d, expected %d (UROS_RPCP_STRING)\n",
               (int)nameparamp->pclass, (int)UROS_RPCP_STRING));
    namestrp = &nameparamp->value.string;
    if (0 == urosStringCmp(namestrp, &tcprosstr)) {
      /* "TCPROS" found.*/
      tcpros = UROS_TRUE; break;
    }
  }
  urosError(tcpros == UROS_FALSE, UROS_NOP,
            ("Caller [%.*s] does not support TCPROS for topic [%.*s]\n",
             UROS_STRARG(&caller_id->value.string),
             UROS_STRARG(&topic->value.string)));

  /* Generate the HTTP response.*/
  uros_rpcslave_methodresponse_prologue(sp); _CHKOK

  /* int code */
  uros_rpcslave_value_int(sp, UROS_RPCC_SUCCESS); _CHKOK

  /* str statusMessage */
  uros_rpcslave_value_string(sp, NULL, 0); _CHKOK

  /* [str name, any param*] */
  uros_rpcslave_value_array_begin(sp); _CHKOK
  if (tcpros) {
    /* ["TCPROS", node_ip, node_port] */
    uros_rpcslave_value_string(sp, "TCPROS", 6); _CHKOK
    uros_rpcslave_value_string_ip(sp, cfgp->tcprosAddr.ip);
    if (sp->err != UROS_OK) {
      return sp->err;
    }
    uros_rpcslave_value_int(sp, (int32_t)cfgp->tcprosAddr.port);
  }
  uros_rpcslave_value_array_end(sp); _CHKOK

  uros_rpcslave_methodresponse_epilogue(sp); _CHKOK
  return sp->err = UROS_OK;
#undef _CHKOK
}

uros_err_t uros_rpcslave_method_shutdown(UrosRpcStreamer *sp,
                                         UrosRpcParamList *parlistp) {

  static UrosNodeStatus *const stp = &urosNode.status;
  UrosRpcParam *msgparamp;

  urosAssert(sp != NULL);
  urosAssert(parlistp != NULL);
#define _CHKOK   { if (sp->err != UROS_OK) { return sp->err; } }

  /* Check for valid parameters.*/
  urosError(parlistp->length != 2, return sp->err = UROS_ERR_BADPARAM,
            ("Expecting 2 parameters, got %d\n", (int)parlistp->length));
  urosAssert(parlistp->headp != NULL);
  msgparamp = &parlistp->headp->nextp->param;
  urosError(msgparamp->pclass != UROS_RPCP_STRING,
            return sp->err = UROS_ERR_BADPARAM,
            ("Class id of [msg] is %d, expected %d (UROS_RPCP_STRING)\n",
             (int)msgparamp->pclass, (int)UROS_RPCP_STRING));

  /* Set the shudtown flag, handled by the Node thread.*/
  urosMutexLock(&stp->stateLock);
  stp->exitFlag = UROS_TRUE;
  stp->exitMsg = msgparamp->value.string;
  msgparamp->value.string = urosStringAssignZ(NULL);
  urosMutexUnlock(&stp->stateLock);

  /* Generate the HTTP response.*/
  uros_rpcslave_methodresponse_prologue(sp); _CHKOK

  /* int code */
  uros_rpcslave_value_int(sp, UROS_RPCC_SUCCESS); _CHKOK

  /* str statusMessage */
  uros_rpcslave_value_string(sp, NULL, 0); _CHKOK

  /* int ignore */
  uros_rpcslave_value_int(sp, 0); _CHKOK

  uros_rpcslave_methodresponse_epilogue(sp); _CHKOK

  return sp->err = UROS_OK;
#undef _CHKOK
}

/*===========================================================================*/
/* GLOBAL FUNCTIONS                                                          */
/*===========================================================================*/

/** @addtogroup rpc_slave_funcs */
/** @{ */

/**
 * @brief   Connects to a list of publishers.
 * @details Given the topic name and a list of publishers, tries to connect to
 *          all of them.
 *
 * @pre     There are no duplicates inside the publishers list.
 * @pre     The publishers exist.
 *
 * @param[in] namep
 *          Pointer to a non-empty string which names the related topic or
 *          service.
 * @param[in] addrlstp
 *          Pointer to a valid list of publisher URIs.
 * @return
 *          Error code.
 */
uros_err_t urosRpcSlaveConnectToPublishers(const UrosString *namep,
                                           const UrosList *addrlstp) {

  static UrosNodeStatus *const stp = &urosNode.status;
  static const uros_topicflags_t flags = {
    UROS_FALSE, UROS_FALSE, UROS_FALSE, UROS_FALSE, UROS_FALSE, UROS_FALSE
  };

  const UrosListNode *nodep;

  urosAssert(urosStringNotEmpty(namep));
  urosAssert(addrlstp != NULL);

  /* Connect to each publisher.*/
  for (nodep = addrlstp->headp; nodep != NULL; nodep = nodep->nextp) {
    uros_err_t err;
    uros_tcpcliargs_t *argsp;

    argsp = urosNew(NULL, uros_tcpcliargs_t);
    if (argsp == NULL) { return UROS_ERR_NOMEM; }
    argsp->topicName = urosStringClone(namep);
    argsp->topicFlags = flags;
    argsp->remoteAddr = *(const UrosAddr *)nodep->datap;

    /* Start the TCPROS Client thread.*/
    err = urosThreadPoolStartWorker(&stp->tcpcliThdPool, (void*)argsp);

    /* Check if anything went wrong.*/
    urosError(err != UROS_OK, urosTopicSubParamsDelete(argsp),
              ("Error %s while connecting to publisher of topic [%.*s]",
               urosErrorText(err), UROS_STRARG(namep)));
  }
  return UROS_OK;
}

/**
 * @brief   XMLRPC Slave API listener thread.
 * @details This thread listens for any incoming XMLRPC calls to the Slave API.
 *
 *          When a new connection request is received, a dedicated
 *          communication channel is spawned and handled by a new thread. This
 *          thread is allocated from the Slave thread pool. If no threads are
 *          free, it waits until one becomes available again.
 * @see     urosRpcSlaveServerThread()
 *
 * @pre     There are no other XMLRPC listener threads with the same connection
 *          port.
 *
 * @param[in] data
 *          Ignored.
 * @return
 *          Error code.
 */
uros_err_t urosRpcSlaveListenerThread(void *data) {

  static const UrosNodeConfig *const cfgp = &urosNode.config;
  static UrosNodeStatus *const stp = &urosNode.status;

  uros_err_t err;
  UrosAddr locaddr;
  UrosConn conn;
  (void)err;
  (void)data;

  /* Setup the local address.*/
  locaddr.port = cfgp->xmlrpcAddr.port;
  locaddr.ip.dword = UROS_ANY_IP;

  /* Setup the listening socket.*/
  urosConnObjectInit(&conn);
  err = urosConnCreate(&conn, UROS_PROTO_TCP);
  urosError(err != UROS_OK, UROS_NOP,
            ("Cannot create XMLRPC Slave server TCP connection.\n"));
  urosAssert(err == UROS_OK);
  err = urosConnBind(&conn, &locaddr);
  urosError(err != UROS_OK, UROS_NOP,
            ("Cannot bind XMLRPC Slave server to "UROS_ADDRFMT"\n",
             UROS_ADDRARG(&locaddr)));
  urosAssert(err == UROS_OK);

  /* Save this thread ID as the server PID.*/
  urosMutexLock(&stp->xmlrpcPidLock);
  stp->xmlrpcPid = (int32_t)urosThreadSelf();
  urosMutexUnlock(&stp->xmlrpcPidLock);

  /* Start listening.*/
  err = urosConnListen(&conn, UROS_XMLRPC_LISTENER_BACKLOG);
  urosError(err != UROS_OK, UROS_NOP,
            ("Cannot make XMLRPC Slave server listen as "UROS_ADDRFMT"\n",
             UROS_ADDRARG(&locaddr)));
  urosAssert(err == UROS_OK);
  while (UROS_TRUE) {
    UrosConn *spawnedp;

    /* Accept the incoming connection.*/
    spawnedp = urosNew(NULL, UrosConn);
    urosAssert(spawnedp != NULL);
    urosConnObjectInit(spawnedp);
    err = urosConnAccept(&conn, spawnedp);
    urosError(err != UROS_OK, UROS_NOP,
              ("Error %s while accepting an incoming XMLRPC Slave connection "
               UROS_ADDRFMT"\n",
               urosErrorText(err), UROS_ADDRARG(&spawnedp->remaddr)));
    urosMutexLock(&stp->stateLock);
    if (stp->exitFlag) {
      /* Refuse the connection if the listener has to exit.*/
      urosMutexUnlock(&stp->stateLock);
      if (err == UROS_OK) {
        urosConnClose(spawnedp);
      }
      urosFree(spawnedp);
      break;
    }
    urosMutexUnlock(&stp->stateLock);
    if (err != UROS_OK) {
      urosFree(spawnedp);
      continue;
    }

    /* Set timeouts for the spawned connection.*/
    err = urosConnSetRecvTimeout(spawnedp, UROS_XMLRPC_RECVTIMEOUT);
    urosAssert(err == UROS_OK);
    err = urosConnSetSendTimeout(spawnedp, UROS_XMLRPC_SENDTIMEOUT);
    urosAssert(err == UROS_OK);

    /* Create the XMLRPC Server worker thread.*/
    err = urosThreadPoolStartWorker(&stp->slaveThdPool,
                                    (void*)spawnedp);

    /* Check if anything went wrong.*/
    urosError(err != UROS_OK, UROS_NOP,
              ("Error %s while running the XMLRPC Server worker thread\n",
               urosErrorText(err)));
    if (err != UROS_OK) {
      urosConnClose(spawnedp);
      urosFree(spawnedp);
    }
  }

  /* Close the listening connection.*/
  urosConnClose(&conn);

  return UROS_OK;
}

/**
 * @brief   XMLRPC Slave API processing thread.
 * @details This worker thread processes an incoming XMLRPC connection
 *          accepted by the XMLRPC listener.
 * @see     urosRpcSlaveListenerThread()
 *
 * @pre     The provided connection was accepted by the XMLRPC Listener.
 * @pre     The connection was alloacted with @p urosAlloc().
 * @post    The connection is closed and deallocated.
 *
 * @param[in] csp
 *          Pointer to an @p UrosConn object, which is the connection accepted
 *          by the XMLRPC Slave API listener, and @p csp points to an
 *          invalid address.
 * @return
 *          Error code.
 */
uros_err_t urosRpcSlaveServerThread(UrosConn *csp) {

  union eps {
    uros_err_t      err;
    UrosRpcParser   parser;
    UrosRpcStreamer streamer;
  } *x;
  UrosRpcParamList parlist;
  uros_rpcslave_methodid_t methodid = UROS_RPCSM__LENGTH;
  char *bufp;
  uros_err_t err;

  urosAssert(csp != NULL);
#define _CHKOK  { if (x->err != UROS_OK) { goto _finally; } }

  x = urosNew(NULL, union eps);
  if (x == NULL) { return UROS_ERR_NOMEM; }
  urosRpcParamListObjectInit(&parlist);

  /* Create the buffer.*/
  bufp = (char*)urosAlloc(NULL, UROS_MTU_SIZE);
  if (bufp == NULL) { return UROS_ERR_NOMEM; }

  /* Initialize the parser object.*/
  urosRpcParserObjectInit(&x->parser, csp, bufp, UROS_MTU_SIZE);

  /* Check if it is a valid HTTP transfer.*/
  urosRpcParserHttpRequest(&x->parser); _CHKOK

  /* Check if the XML header is valid */
  urosRpcParserSkipWs(&x->parser); _CHKOK
  urosRpcParserXmlHeader(&x->parser); _CHKOK

  /* Decode the method call.*/
  urosRpcParserSkipWs(&x->parser); _CHKOK
  uros_rpcslave_xmlmethodcall(&x->parser, &methodid, &parlist); _CHKOK

  /* Dispose the parser object.*/
  urosRpcParserClean(&x->parser, UROS_FALSE);

  /* Initialize the streamer object.*/
  urosRpcStreamerObjectInit(&x->streamer, csp, bufp, UROS_MTU_SIZE);

  /* Reply to the method call.*/
  switch(methodid) {
#define _DISPATCH(id, proc) \
  case id: { proc(&x->streamer, &parlist); _CHKOK; break; }

  _DISPATCH(UROS_RPCSM_GET_BUS_INFO,
            uros_rpcslave_method_getbusinfo)
  _DISPATCH(UROS_RPCSM_GET_BUS_STATS,
            uros_rpcslave_method_getbusstats)
  _DISPATCH(UROS_RPCSM_GET_MASTER_URI,
            uros_rpcslave_method_getmasteruri)
  _DISPATCH(UROS_RPCSM_GET_PID,
            uros_rpcslave_method_getpid)
  _DISPATCH(UROS_RPCSM_GET_PUBLICATIONS,
            uros_rpcslave_method_getpublications)
  _DISPATCH(UROS_RPCSM_GET_SUBSCRIPTIONS,
            uros_rpcslave_method_getsubscriptions)
  _DISPATCH(UROS_RPCSM_PARAM_UPDATE,
            uros_rpcslave_method_paramupdate)
  _DISPATCH(UROS_RPCSM_PUBLISHER_UPDATE,
            uros_rpcslave_method_publisherupdate)
  _DISPATCH(UROS_RPCSM_REQUEST_TOPIC,
            uros_rpcslave_method_requesttopic)
  _DISPATCH(UROS_RPCSM_SHUTDOWN,
            uros_rpcslave_method_shutdown)
  default: {
    urosError(UROS_ERR_BADPARAM, UROS_NOP,
              ("Unknown XMLRPC Slave method id %d\n", (int)methodid));
    x->err = UROS_ERR_BADPARAM;
    goto _finally;
  }
#undef _DISPATCH
  }

  /* Finalize any output messages.*/
  urosRpcStreamerFlush(&x->streamer); _CHKOK

  /* Dispose the streamer object.*/
  urosRpcStreamerClean(&x->streamer, UROS_FALSE);

_finally:
  /* Free any allocated objects.*/
  urosRpcParamListClean(&parlist, UROS_TRUE);
  urosFree(bufp);
  err = x->err;
  urosFree(x);

  /* Gentle connection close.*/
  urosConnClose(csp);
  urosFree(csp);
  return err;
#undef _CHKOK
}

/** @} */
