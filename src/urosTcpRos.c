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
 * @file    urosTcpRos.c
 * @author  Andrea Zoppi <texzk@email.it>
 *
 * @brief   TCPROS features of the middleware.
 */

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include "../include/urosBase.h"
#include "../include/urosUser.h"
#include "../include/urosTcpRos.h"
#include "../include/urosNode.h"

#include <string.h>

/*===========================================================================*/
/* LOCAL TYPES & MACROS                                                      */
/*===========================================================================*/

#if UROS_TCPROS_C_USE_ASSERT == UROS_FALSE && !defined(__DOXYGEN__)
#undef urosAssert
#define urosAssert(expr)
#endif

/*===========================================================================*/
/* LOCAL VARIABLES                                                           */
/*===========================================================================*/

static const UrosString calleridfield =   {  8, "callerid" };
static const UrosString topicfield =      {  5, "topic" };
static const UrosString servicefield =    {  7, "service" };
static const UrosString md5field =        {  6, "md5sum" };
static const UrosString typefield =       {  4, "type" };
static const UrosString reqtypefield =    { 12, "request_type" };
static const UrosString restypefield =    { 13, "response_type" };
static const UrosString persistentfield = { 10, "persistent" };
static const UrosString latchingfield =   {  8, "latching" };
static const UrosString tcpnodelayfield = { 11, "tcp_nodelay" };
static const UrosString errfield =        {  5, "error" };

static const UrosMsgType dummytype = {
  { 0, NULL },
  { 0, NULL },
  { 0, NULL }
};

static const UrosTopic dummytopic = {
  { 0, NULL },
  &dummytype,
  NULL,
  { UROS_FALSE, UROS_FALSE, UROS_FALSE, UROS_FALSE, UROS_FALSE, UROS_FALSE },
  0
};

/*===========================================================================*/
/* LOCAL FUNCTIONS                                                           */
/*===========================================================================*/

uros_err_t uros_tcpros_sendfieldstring(UrosTcpRosStatus *tcpstp,
                                        const UrosString *namep,
                                        const UrosString *valuep) {

  uint32_t hdrlen;

  urosAssert(tcpstp != NULL);
  urosAssert(urosStringNotEmpty(namep));
  urosAssert(urosStringIsValid(valuep));
#define _CHKOK  { if (tcpstp->err != UROS_OK) { return tcpstp->err; } }

  hdrlen = (uint32_t)(namep->length + 1 + valuep->length);
  urosTcpRosSendRaw(tcpstp, hdrlen); _CHKOK
  urosTcpRosSend(tcpstp, namep->datap, namep->length); _CHKOK
  urosTcpRosSend(tcpstp, "=", 1); _CHKOK
  urosTcpRosSend(tcpstp, valuep->datap, valuep->length); _CHKOK

  return tcpstp->err = UROS_OK;
#undef _CHKOK
}

uros_err_t uros_tcpros_sendfieldbool(UrosTcpRosStatus *tcpstp,
                                      const UrosString *namep,
                                      uros_bool_t value) {

  uint32_t hdrlen;

  urosAssert(tcpstp != NULL);
  urosAssert(urosStringNotEmpty(namep));
#define _CHKOK  { if (tcpstp->err != UROS_OK) { return tcpstp->err; } }

  hdrlen = (uint32_t)(namep->length + 2);
  urosTcpRosSendRaw(tcpstp, hdrlen); _CHKOK
  urosTcpRosSend(tcpstp, namep->datap, namep->length); _CHKOK
  urosTcpRosSend(tcpstp, value ? "=1" : "=0", 2); _CHKOK

  return tcpstp->err = UROS_OK;
#undef _CHKOK
}

uros_err_t uros_tcpros_sendheader(UrosTcpRosStatus *tcpstp,
                                  uros_bool_t thisisclient) {

  size_t hdrlen = 0;
  uint32_t hdrlen32;
  const UrosString *calleridstrp, *namestrp, *typestrp, *md5strp;

  urosAssert(tcpstp != NULL);
  urosAssert(tcpstp->topicp != NULL);
  urosAssert(tcpstp->topicp->typep != NULL);
#define _CHKOK  { if (tcpstp->err != UROS_OK) { return tcpstp->err; } }

  calleridstrp = &urosNode.config.nodeName;
  typestrp = &tcpstp->topicp->typep->name;
  md5strp = &tcpstp->topicp->typep->md5str;

  hdrlen += 4 + calleridfield.length + 1 + calleridstrp->length;
  hdrlen += 4 + md5field.length + 1 + md5strp->length;
  hdrlen += 4 + typefield.length + 1 + typestrp->length;
  if (thisisclient) {
    namestrp = &tcpstp->topicp->name;
    if (tcpstp->flags.service) {
      hdrlen += 4 + servicefield.length + 1 + namestrp->length;
      hdrlen += 4 + persistentfield.length + 2;
    } else {
      hdrlen += 4 + topicfield.length + 1 + namestrp->length;
      hdrlen += 4 + tcpnodelayfield.length + 2;
    }
  } else {
    namestrp = NULL;
    if (tcpstp->flags.service) {
      hdrlen += 4 + reqtypefield.length + 1 + typestrp->length + 7;
      hdrlen += 4 + restypefield.length + 1 + typestrp->length + 8;
    } else {
      hdrlen += 4 + latchingfield.length + 2;
    }
  }

  /* uint32 header_length */
  hdrlen32 = (uint32_t)hdrlen;
  urosTcpRosSendRaw(tcpstp, hdrlen32); _CHKOK

  /* uint32 field_length, callerid={str} */
  uros_tcpros_sendfieldstring(tcpstp, &calleridfield, calleridstrp); _CHKOK

  if (thisisclient) {
    if (tcpstp->flags.service) {
      /* uint32 field_length, service={str} */
      uros_tcpros_sendfieldstring(tcpstp, &servicefield, namestrp); _CHKOK
    } else {
      /* uint32 field_length, topic={str} */
      uros_tcpros_sendfieldstring(tcpstp, &topicfield, namestrp); _CHKOK
    }
  }

  /* uint32 field_length, md5sum={str} */
  uros_tcpros_sendfieldstring(tcpstp, &md5field, md5strp); _CHKOK

  if (tcpstp->flags.service) {
    /* uint32 field_length, request_type={str} */
    hdrlen32 = reqtypefield.length + 1 + typestrp->length + 7;
    urosTcpRosSendRaw(tcpstp, hdrlen32); _CHKOK
    urosTcpRosSend(tcpstp, reqtypefield.datap, reqtypefield.length); _CHKOK
    urosTcpRosSend(tcpstp, "=", 1); _CHKOK
    urosTcpRosSend(tcpstp, typestrp->datap, typestrp->length); _CHKOK
    urosTcpRosSend(tcpstp, "Request", 7); _CHKOK

    /* uint32 field_length, response_type={str} */
    hdrlen32 = restypefield.length + 1 + typestrp->length + 8;
    urosTcpRosSendRaw(tcpstp, hdrlen32); _CHKOK
    urosTcpRosSend(tcpstp, restypefield.datap, restypefield.length); _CHKOK
    urosTcpRosSend(tcpstp, "=", 1); _CHKOK
    urosTcpRosSend(tcpstp, typestrp->datap, typestrp->length); _CHKOK
    urosTcpRosSend(tcpstp, "Response", 8); _CHKOK
  }

  /* uint32 field_length, type={str} */
  uros_tcpros_sendfieldstring(tcpstp, &typefield, typestrp); _CHKOK

  if (thisisclient) {
    if (tcpstp->flags.service) {
      /* uint32 field_length, persistent=(0|1) */
      uros_tcpros_sendfieldbool(tcpstp, &persistentfield,
                                tcpstp->flags.persistent); _CHKOK
    } else {
      /* uint32 field_length, tcp_nodelay=(0|1) */
      uros_tcpros_sendfieldbool(tcpstp, &tcpnodelayfield,
                                tcpstp->flags.noDelay); _CHKOK
    }
  } else {
    if (!tcpstp->flags.service) {
      /* uint32 field_length, latching=(0|1) */
      uros_tcpros_sendfieldbool(tcpstp, &latchingfield,
                                tcpstp->flags.latching); _CHKOK
    }
  }

  return tcpstp->err = UROS_OK;
#undef _CHKOK
}

uros_err_t uros_tcpros_recvheader(UrosTcpRosStatus *tcpstp,
                                  uros_bool_t thisisclient) {

  uint32_t hdrlen, remlen;
  uint32_t vallen, fieldlen;
  char *valp = NULL;
  UrosTopic *topicp;
  UrosMsgType *typep;
  char buf[4];  /* Length matches the shortest field ("type").*/

  urosAssert(tcpstp != NULL);
#define _CHKOK  { if (tcpstp->err != UROS_OK) { goto _error; } }
#define _GOT(tokp, toklen) \
    ((toklen <= 4) \
     ? (0 == memcmp(buf, tokp, toklen)) \
     : (0 == memcmp(buf, tokp, 4) && \
        UROS_OK == urosTcpRosExpect(tcpstp, tokp + 4, toklen - 4)))

  if (!thisisclient) {
    urosAssert(tcpstp->topicp == NULL);
    typep = urosNew(UrosMsgType);
    topicp = urosNew(UrosTopic);
    if (typep == NULL || topicp == NULL) {
      urosFree(typep); urosFree(topicp);
      return tcpstp->err = UROS_ERR_NOMEM;
    }
    urosMsgTypeObjectInit(typep);
    urosTopicObjectInit(topicp);
    topicp->typep = typep;
  } else {
    urosAssert(tcpstp->topicp != NULL);
    urosAssert(tcpstp->topicp->typep != NULL);
    topicp = (UrosTopic*)tcpstp->topicp;
    typep = (UrosMsgType*)topicp->typep;
  }

  /* Read the header length.*/
  urosTcpRosRecvRaw(tcpstp, hdrlen); _CHKOK

  /* Get each header field.*/
  for (remlen = hdrlen; remlen > 0; remlen -= fieldlen) {
    UrosString *strp = NULL;

    /* Get the field length and check for size consistency.*/
    if (remlen < 4) { tcpstp->err = UROS_ERR_BADCONN; goto _error; }
    remlen -= 4;
    urosTcpRosRecvRaw(tcpstp, fieldlen); _CHKOK
    if (remlen < fieldlen) { tcpstp->err = UROS_ERR_BADCONN; goto _error; }
    vallen = fieldlen;

    /* Decode the field name.*/
    urosTcpRosRecv(tcpstp, buf, 4);    /* Minimum-length field.*/
    if (_GOT("callerid=", 9)) {
      /* callerid={str} */
      strp = &tcpstp->callerId;
      vallen -= 9;
    } else if (_GOT("error=", 6)) {
      /* error={str} */
      strp = &tcpstp->errstr;
      vallen -= 6;
    } else if (_GOT("latching=", 9)) {
      if (!thisisclient) { tcpstp->err = UROS_ERR_PARSE; goto _error; }
      if (tcpstp->flags.service) { tcpstp->err = UROS_ERR_PARSE; goto _error; }
      /* latching=(0|1) */
      if (vallen == 10) {
        urosTcpRosRecv(tcpstp, buf, 1);
        if      (buf[0] == '1') { tcpstp->flags.latching = UROS_TRUE; continue; }
        else if (buf[0] == '0') { tcpstp->flags.latching = UROS_FALSE; continue; }
        else { tcpstp->err = UROS_ERR_PARSE; goto _error; }
      } else { tcpstp->err = UROS_ERR_PARSE; goto _error; }
    } else if (_GOT("md5sum=", 7)) {
      /* md5sum={str} */
      strp = &typep->md5str;
      vallen -= 7;
      if (thisisclient) {
        /* Check if it matches the referenced one.*/
        if (vallen != strp->length) { tcpstp->err = UROS_ERR_PARSE; goto _error; }
        urosTcpRosExpect(tcpstp, strp->datap, strp->length);
        if (tcpstp->err == UROS_OK) { continue; }
        else { goto _error; }
      }
    } else if (_GOT("message_definition=", 19)) {
      /* message_definition={str} */
      vallen -= 19;
#if UROS_TCPROS_USE_MESSAGE_DEFINITION
      strp = &typep->desc;
#else
      urosTcpRosSkip(tcpstp, vallen);
      continue;
#endif
    } else if (_GOT("persistent=", 11)) {
      if (thisisclient) { tcpstp->err = UROS_ERR_PARSE; goto _error; }
      if (!tcpstp->flags.service) { tcpstp->err = UROS_ERR_PARSE; goto _error; }
      /* persistent=(0|1) */
      if (vallen == 12) {
        urosTcpRosRecv(tcpstp, buf, 1);
        if      (buf[0] == '1') { tcpstp->flags.persistent = UROS_TRUE; continue; }
        else if (buf[0] == '0') { tcpstp->flags.persistent = UROS_FALSE; continue; }
        else { tcpstp->err = UROS_ERR_PARSE; goto _error; }
      } else { tcpstp->err = UROS_ERR_PARSE; goto _error; }
    } else if (_GOT("probe=", 6)) {
      /* probe=(0|1) */
      if (vallen == 7) {
        urosTcpRosRecv(tcpstp, buf, 1);
        if      (buf[0] == '1') { tcpstp->flags.probe = UROS_TRUE; continue; }
        else if (buf[0] == '0') { tcpstp->flags.probe = UROS_FALSE; continue; }
        else { tcpstp->err = UROS_ERR_PARSE; goto _error; }
      } else { tcpstp->err = UROS_ERR_PARSE; goto _error; }
    } else if (_GOT("request_type=", 13)) {
      /* request_type={str} */
      /* TODO: Type checking (ignoring it now, assuming it is correct).*/
      vallen -= 13;
      if (urosTcpRosSkip(tcpstp, vallen) != UROS_OK) { goto _error; }
      continue;
    } else if (_GOT("response_type=", 14)) {
      /* response_type={str} */
      /* TODO: Type checking (ignoring it now, assuming it is correct).*/
      vallen -= 14;
      if (urosTcpRosSkip(tcpstp, vallen) != UROS_OK) { goto _error; }
      continue;
    } else if (_GOT("service=", 8)) {
      if (thisisclient) {
        /* Check if it matches the referenced one.*/
        urosTcpRosExpect(tcpstp, topicp->name.datap, topicp->name.length);
        if (tcpstp->err == UROS_OK) { continue; }
        else { goto _error; }
      } else {
        /* service={str} */
        strp = &topicp->name;
        tcpstp->flags.service = UROS_TRUE;
        vallen -= 8;
      }
    } else if (_GOT("topic=", 6)) {
      if (thisisclient) {
        /* Check if it matches the referenced one.*/
        urosTcpRosExpect(tcpstp, topicp->name.datap, topicp->name.length);
        if (tcpstp->err == UROS_OK) { continue; }
        else { goto _error; }
      } else {
        /* topic={str} */
        strp = &topicp->name;
        tcpstp->flags.service = UROS_FALSE;
        vallen -= 6;
      }
    } else if (_GOT("type=", 5)) { /* <<< Minimum-length field.*/
      /* type={str} */
      strp = &typep->name;
      vallen -= 5;
      if (thisisclient) {
        /* Check if it matches the referenced one.*/
        if (vallen != strp->length) { tcpstp->err = UROS_ERR_PARSE; goto _error; }
        urosTcpRosExpect(tcpstp, strp->datap, strp->length);
        if (tcpstp->err == UROS_OK) { continue; }
        else { goto _error; }
      }
    } else if (_GOT("tcp_nodelay=", 12)) {
      if (thisisclient) { tcpstp->err = UROS_ERR_PARSE; goto _error; }
      if (tcpstp->flags.service) { tcpstp->err = UROS_ERR_PARSE; goto _error; }
      /* tcp_nodelay=(0|1) */
      if (vallen == 13) {
        urosTcpRosRecv(tcpstp, buf, 1);
        if      (buf[0] == '1') { tcpstp->flags.noDelay = UROS_TRUE; continue; }
        else if (buf[0] == '0') { tcpstp->flags.noDelay = UROS_FALSE; continue; }
        else { tcpstp->err = UROS_ERR_PARSE; goto _error; }
      } else { tcpstp->err = UROS_ERR_PARSE; goto _error; }
    }
    _CHKOK

    /* Check if the field is already populated.*/
    urosAssert(strp != NULL);
    urosError(strp->length > 0 || strp->datap != NULL,
              { tcpstp->err = UROS_ERR_BADPARAM; goto _error; },
              ("Field already populated as [%.*s]\n", UROS_STRARG(strp)));

    /* Allocate and read the value.*/
    if (vallen > 0) {
      valp = (char*)urosAlloc(vallen);
      if (valp == NULL) { tcpstp->err = UROS_ERR_NOMEM; goto _error; }
      strp->length = vallen;
      strp->datap = valp;
      urosTcpRosRecv(tcpstp, valp, vallen); _CHKOK
    } else {
      strp->length = 0;
      strp->datap = NULL;
    }
  }

  if (!thisisclient) {
    /* Temporarily save the topic information inside the TCPROS status.*/
    tcpstp->topicp = topicp;
  }

  return tcpstp->err = UROS_OK;

_error:
  if (!thisisclient) {
    urosMsgTypeDelete(typep);
    urosTopicDelete(topicp);
  }
  return tcpstp->err;
#undef _CHKOK
#undef _GOTPREFIX
}

uros_err_t uros_tcpserver_processtopicheader(UrosTcpRosStatus *tcpstp) {

  const UrosListNode *topicnodep;
  UrosListNode *tcpnodep;
  UrosTopic *topicp;
  const UrosMsgType *reftypep;

  urosAssert(tcpstp != NULL);
  urosAssert(tcpstp->topicp != NULL);

  /* The reference topic was temporarily saved inside the TCPROS status.*/
  topicp = (UrosTopic*)tcpstp->topicp;
  tcpstp->topicp = NULL;

  /* Check if the topic is actually published.*/
  topicnodep = urosTopicListFindByName(&urosNode.status.pubTopicList,
                                       &topicp->name);
  urosError(topicnodep == NULL,
            { tcpstp->err = UROS_ERR_BADPARAM; goto _finally; },
            ("Topic [%.*s] not found\n", UROS_STRARG(&topicp->name)));
  reftypep = ((const UrosTopic *)topicnodep->datap)->typep;
  urosError(0 != urosStringCmp(&topicp->typep->name, &reftypep->name),
            { tcpstp->err = UROS_ERR_BADPARAM; goto _finally; },
            ("Found type [%.*s], expected [%.*s]\n",
             UROS_STRARG(&topicp->typep->name), UROS_STRARG(&reftypep->name)));
  urosError(0 != urosStringCmp(&topicp->typep->md5str, &reftypep->md5str),
            { tcpstp->err = UROS_ERR_BADPARAM; goto _finally; },
            ("Found MD5 [%.*s], expected [%.*s]\n",
             UROS_STRARG(&topicp->typep->md5str),
             UROS_STRARG(&reftypep->md5str)));
  tcpstp->topicp = (UrosTopic*)topicnodep->datap;

  /* Add this connection to the active publisher connections list.*/
  tcpnodep = urosNew(UrosListNode);
  if (tcpnodep == NULL) { tcpstp->err = UROS_ERR_NOMEM; goto _finally; }
  tcpnodep->datap = tcpstp;
  tcpnodep->nextp = NULL;
  urosMutexLock(&urosNode.status.pubTcpListLock);
  urosListAdd(&urosNode.status.pubTcpList, tcpnodep);
  urosMutexUnlock(&urosNode.status.pubTcpListLock);

  tcpstp->err = UROS_OK;
_finally:
  urosMsgTypeDelete((UrosMsgType*)topicp->typep);
  urosTopicDelete(topicp);
  return tcpstp->err;
}

uros_err_t uros_tcpserver_processserviceheader(UrosTcpRosStatus *tcpstp) {

  const UrosListNode *servicenodep;
  UrosListNode *tcpnodep;
  UrosTopic *servicep;
  const UrosMsgType *reftypep;

  urosAssert(tcpstp != NULL);
  urosAssert(tcpstp->topicp != NULL);

  /* The reference service was temporarily saved inside the TCPROS status.*/
  servicep = (UrosTopic*)tcpstp->topicp;
  tcpstp->topicp = NULL;

  /* Check if the topic is actually published.*/
  servicenodep = urosTopicListFindByName(&urosNode.status.pubServiceList,
                                         &servicep->name);
  if (servicenodep != NULL) {
    urosTopicRefInc((UrosTopic*)servicenodep->datap);
  }

  urosError(servicenodep == NULL,
            { tcpstp->err = UROS_ERR_BADPARAM; goto _finally; },
            ("Service [%.*s] not found\n", UROS_STRARG(&servicep->name)));
  reftypep = ((const UrosTopic *)servicenodep->datap)->typep;
  if (servicep->typep->name.length > 0) {
    urosError(0 != urosStringCmp(&servicep->typep->name, &reftypep->name),
              { tcpstp->err = UROS_ERR_BADPARAM; goto _finally; },
              ("Found type [%.*s], expected [%.*s]\n",
               UROS_STRARG(&servicep->typep->name),
               UROS_STRARG(&reftypep->name)));
  }
  if (servicep->typep->md5str.length > 1 &&
      servicep->typep->md5str.datap[0] != '*') {
    urosError(0 != urosStringCmp(&servicep->typep->md5str, &reftypep->md5str),
              { tcpstp->err = UROS_ERR_BADPARAM; goto _finally; },
              ("Found MD5 [%.*s], expected [%.*s]\n",
               UROS_STRARG(&servicep->typep->md5str),
               UROS_STRARG(&reftypep->md5str)));
  }
  tcpstp->topicp = (UrosTopic*)servicenodep->datap;

  /* Add this connection to the active publisher connections list.*/
  tcpnodep = urosNew(UrosListNode);
  if (tcpnodep == NULL) { tcpstp->err = UROS_ERR_NOMEM; goto _finally; }
  tcpnodep->datap = tcpstp;
  tcpnodep->nextp = NULL;
  urosMutexLock(&urosNode.status.pubTcpListLock);
  urosListAdd(&urosNode.status.pubTcpList, tcpnodep);
  urosMutexUnlock(&urosNode.status.pubTcpListLock);

_finally:
  if (tcpstp->err != UROS_OK && servicenodep != NULL) {
    urosTopicRefDec((UrosTopic*)servicenodep->datap);
  }
  urosMsgTypeDelete((UrosMsgType*)servicep->typep);
  urosTopicDelete(servicep);
  return tcpstp->err;
}

uros_err_t uros_tcpcli_topicsubscription(const UrosString *topicnamep,
                                         const UrosAddr *pubaddrp) {

  uros_err_t err;
  UrosConn conn;
  UrosTcpRosStatus tcpst;
  UrosListNode *topicnodep;
  UrosTopic *topicp;
  uros_proc_f handler;

  urosAssert(urosStringNotEmpty(topicnamep));
  urosAssert(pubaddrp != NULL);
#define _CHKOK  { if (err != UROS_OK) { goto _error; } }

  /* Get topic features.*/
  urosMutexLock(&urosNode.status.subTopicListLock);
  topicnodep = urosTopicListFindByName(&urosNode.status.subTopicList,
                                       topicnamep);
  if (topicnodep != NULL) {
    topicp = (UrosTopic*)topicnodep->datap;
    urosTopicRefInc(topicp);
    handler = topicp->procf;
  }
  urosMutexUnlock(&urosNode.status.subTopicListLock);
  urosError(topicnodep == NULL, return UROS_ERR_BADPARAM,
            ("Topic [%.*s] not found\n", UROS_STRARG(topicnamep)));

  /* Connect to the publisher.*/
  urosConnObjectInit(&conn);
  urosConnCreate(&conn, UROS_PROTO_TCP);
  err = urosConnConnect(&conn, pubaddrp);
  urosError(err != UROS_OK, goto _error,
            ("Error %s while connecting to "UROS_ADDRFMT"\n",
             urosErrorText(err), UROS_ADDRARG(pubaddrp)));
  urosTpcRosStatusObjectInit(&tcpst, &conn);
  tcpst.topicp = topicp;
  tcpst.flags = topicp->flags;

  /* Send the TCPROS conenction header.*/
  err = uros_tcpros_sendheader(&tcpst, UROS_TRUE); _CHKOK

  /* Receive the TCPROS connection header.*/
  err = uros_tcpros_recvheader(&tcpst, UROS_TRUE); _CHKOK

  /* Handle the incoming published stream, if not probing.*/
  tcpst.err = UROS_OK;
  urosAssert(handler != NULL);
  if (!tcpst.flags.probe) {
    err = handler(&tcpst);
  }

  /* Release the topic/service descriptor reference.*/
  urosTcpRosTopicSubscriberDone(&tcpst);

  urosError(err != UROS_OK, goto _error,
            ("Topic [%.*s] client handler returned %s\n",
             UROS_STRARG(topicnamep), urosErrorText(err)));
  return urosConnClose(&conn);

_error:
  urosConnClose(&conn);
  return err;
#undef _CHKOK
}

uros_err_t uros_tcpros_resolvepublisher(const uros_tcpcliargs_t *parp,
                                        UrosAddr *pubaddrp) {

  static const UrosRpcParamNode tcprosnode = {
    { UROS_RPCP_STRING, {{ 6, "TCPROS" }} }, NULL
  };
  static const UrosRpcParamList tcproslist = {
    (UrosRpcParamNode*)&tcprosnode, (UrosRpcParamNode*)&tcprosnode, 1
  };
  static const UrosRpcParamNode protonode = {
    { UROS_RPCP_ARRAY, {{ (size_t)&tcproslist, NULL }} }, NULL
  };
  static const UrosRpcParamList protolist = {
    (UrosRpcParamNode*)&protonode, (UrosRpcParamNode*)&protonode, 1
  };

  uros_err_t err;
  UrosRpcParamList *parlistp;
  UrosRpcParamNode *nodep;
  UrosRpcParam *str1, *str2, *int3;
  UrosRpcResponse response;

  urosAssert(parp != NULL);
  urosAssert(parp->topicName.length > 0);
  urosAssert(parp->topicName.datap != NULL);
  urosAssert(pubaddrp != NULL);
#define _ERR    { err = UROS_ERR_BADPARAM; goto _finally; }

  /* Request the topic to the publisher.*/
  urosRpcResponseObjectInit(&response);
  err = urosRpcCallRequestTopic(
    &parp->remoteAddr,
    &urosNode.config.nodeName,
    &parp->topicName,
    &protolist,
    &response
  );

  /* Check for valid values. */
  if (err != UROS_OK) { goto _finally; }
  urosError(response.httpcode != 200, _ERR,
            ("The HTTP response code is %lu, expecting 200\n",
             response.httpcode));
  if (response.code != UROS_RPCC_SUCCESS) { _ERR };
  urosError(response.valuep->class != UROS_RPCP_ARRAY, _ERR,
            ("Response calue class is %d, expecting %d (UROS_RPCP_ARRAY)\n",
             (int)response.valuep->class, (int)UROS_RPCP_ARRAY));
  parlistp = response.valuep->value.listp;
  urosAssert(parlistp != NULL);
  if (parlistp->length != 3) { _ERR }
  nodep = parlistp->headp;
  str1 = &nodep->param; nodep = nodep->nextp;
  str2 = &nodep->param; nodep = nodep->nextp;
  int3 = &nodep->param;
  urosError(str1->class != UROS_RPCP_STRING, _ERR,
            ("Response calue class is %d, expecting %d (UROS_RPCP_STRING)\n",
             (int)str1->class, (int)UROS_RPCP_STRING));
  urosError(str2->class != UROS_RPCP_STRING, _ERR,
            ("Response calue class is %d, expecting %d (UROS_RPCP_STRING)\n",
             (int)str2->class, (int)UROS_RPCP_STRING));
  urosError(int3->class != UROS_RPCP_INT, _ERR,
            ("Response calue class is %d, expecting %d (UROS_RPCP_INT)\n",
             (int)int3->class, (int)UROS_RPCP_INT));
  if (0 != urosStringCmp(&tcprosnode.param.value.string,
                         &str1->value.string)) { _ERR }
  err = urosHostnameToIp(&str2->value.string, &pubaddrp->ip);
  if (err != UROS_OK) { goto _finally; }
  if (int3->value.int32 < 0 || int3->value.int32 > 65535) { _ERR }
  pubaddrp->port = (uint16_t)int3->value.int32;

  err = UROS_OK;
_finally:
  urosRpcResponseClean(&response);
  return err;
#undef _ERR
}

/*===========================================================================*/
/* GLOBAL FUNCTIONS                                                          */
/*===========================================================================*/

/** @addtogroup tcpros_funcs */
/** @{ */

/**
 * @brief   Deletes topic subscription parameters.
 * @details Deallocates the memory chunk of the string data, and the object
 *          itself.
 * @see     urosFree()
 *
 * @pre     The parameters must have been allocated with @p urosAlloc().
 * @pre     The parameter data must have been allocated with @p urosAlloc().
 * @post    @p parp points to an invalid address.
 *
 * @param[in] parp
 *          Pointer to an allocated @p uros_tcpcliargs_t object.
 */
void urosTopicSubParamsDelete(uros_tcpcliargs_t *parp) {

  if (parp != NULL) {
    urosStringClean(&parp->topicName);
    urosFree(parp);
  }
}

/**
 * @brief   Initializes a TCPROS status record.
 *
 * @pre     The record is not initialized.
 * @post    The record is initialized.
 *
 * @param[in,out] tcpstp
 *          Pointer to an allocated @p UrosTcpRosStatus object.
 * @param[in] csp
 *          Pointer to an allocated connection.
 */
void urosTpcRosStatusObjectInit(UrosTcpRosStatus *tcpstp, UrosConn *csp) {

  urosAssert(tcpstp != NULL);
  urosAssert(csp != NULL);

  memset(tcpstp, 0, sizeof(UrosTcpRosStatus));
  tcpstp->csp = csp;

  /* Initialize the thread exit request mutex.*/
  urosMutexObjectInit(&tcpstp->threadExitMtx);

  tcpstp->err = UROS_OK;
}

/**
 * @brief   Cleans a TCPROS status record.
 * @details Invalidates the private members. Optionally, deallocates their data.
 *
 * @pre     The record is initialized.
 * @post    @p tcpstp points to an uninitialized @p UrosTcpRosStatus object.
 * @post    If desidred so, private members are deallocated. They must have
 *          been allocated with @p urosAlloc().
 *
 * @param[in,out] tcpstp
 *          Pointer to an initialized @p UrosTcpRosStatus object.
 * @param[in] deep
 *          Performs deallocation of private members.
 */
void urosTpcRosStatusClean(UrosTcpRosStatus *tcpstp, uros_bool_t deep) {

  urosAssert(tcpstp != NULL);

  if (deep) {
    urosFree(tcpstp->csp);
    urosStringClean(&tcpstp->callerId);
    urosStringClean(&tcpstp->errstr);
  }
  memset(tcpstp, 0, sizeof(UrosTcpRosStatus));
}

/**
 * @brief   Deallocates a TCPROS status record.
 * @details Invalidates the private members. Optionally, deallocates their data.
 *
 * @pre     The record is initialized.
 * @post    @p tcpstp points to an invalid address.
 * @post    If desidred so, private members are deallocated. They must have
 *          been allocated with @p urosAlloc().
 *
 * @param[in,out] tcpstp
 *          Pointer to an initialized @p UrosTcpRosStatus object.
 * @param[in] deep
 *          Performs deallocation of private members.
 */
void urosTpcRosStatusDelete(UrosTcpRosStatus *tcpstp, uros_bool_t deep) {

  if (tcpstp != NULL) {
    urosTpcRosStatusClean(tcpstp, deep);
    urosFree(tcpstp);
  }
}

/**
 * @brief   Raises the exit flag.
 * @details Sets the @p exitFlag to @p true, so that the user handler will exit
 *          the loop and therefore the TCPROS worker thread can exit.
 *
 * @param[in,out] tcpstp
 *          Pointer to an initialized @p UrosTcpRosStatus object.
 */
void urosTcpRosStatusIssueExit(UrosTcpRosStatus *tcpstp) {

  urosAssert(tcpstp != NULL);

  urosMutexLock(&tcpstp->threadExitMtx);
  tcpstp->threadExit = UROS_TRUE;
  urosMutexUnlock(&tcpstp->threadExitMtx);
}

/**
 * @brief   Checks if the exit flag is raised.
 *
 * @param[in] tcpstp
 *          Pointer to an initialized @p UrosTcpRosStatus object.
 * @return
 *          Exit flag status.
 */
uros_bool_t urosTcpRosStatusCheckExit(UrosTcpRosStatus *tcpstp) {

  uros_bool_t flag;

  urosAssert(tcpstp != NULL);

  urosMutexLock(&tcpstp->threadExitMtx);
  flag = tcpstp->threadExit;
  urosMutexUnlock(&tcpstp->threadExitMtx);
  return flag;
}

/**
 * @brief   Initializes a TCPROS array descriptor.
 * @details The array is initialized as empty.
 *
 * @pre     The array is not initialized.
 * @post    The array is empty.
 *
 * @param[in,out] arrayp
 *          Pointer to an allocated @p UrosTcpRosArray object.
 */
void urosTcpRosArrayObjectInit(UrosTcpRosArray *arrayp) {

  urosAssert(arrayp != NULL);

  arrayp->length = 0;
  arrayp->entriesp = NULL;
}

/**
 * @brief   Cleans a TCPROS array.
 * @details Invalidates the private members, and deallocates their data.
 *
 * @pre     The array is initialized.
 * @post    @p arrayp points to an empty array descriptor.
 * @post    Private members are deallocated. They must have been allocated with
 *          @p urosAlloc().
 *
 * @param[in,out] arrayp
 *          Pointer to an initialized @p UrosTcpRosArray object.
 */
void urosTcpRosArrayClean(UrosTcpRosArray *arrayp) {

  urosAssert(arrayp != NULL);

  arrayp->length = 0;
  urosFree(arrayp->entriesp);
  arrayp->entriesp = NULL;
}

/**
 * @brief   Deallocates a TCPROS array descriptor.
 * @details Invalidates the private members. Optionally, deallocates their data.
 *
 * @pre     The array is initialized.
 * @post    @p arrayp points to an invalid address.
 * @post    If desidred so, private members are deallocated. They must have
 *          been allocated with @p urosAlloc().
 *
 * @param[in,out] arrayp
 *          Pointer to an initialized @p UrosTcpRosArray object.
 * @param[in] deep
 *          Performs deallocation of private members.
 */
void urosTcpRosArrayDelete(UrosTcpRosArray *arrayp, uros_bool_t deep) {

  if (arrayp != NULL) {
    if (deep) {
      urosTcpRosArrayClean(arrayp);
    }
    urosFree(arrayp);
  }
}

/**
 * @brief   Skips a number of bytes from the incoming TCPROS stream.
 *
 * @param[in,out] tcpstp
 *          Pointer to a TCPROS status with a working connection.
 * @param[in] length
 *          Length of the data to skip, in bytes. Can be @p 0.
 * @return
 *          Error code.
 */
uros_err_t urosTcpRosSkip(UrosTcpRosStatus *tcpstp, size_t length) {

  void *bufp;

  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));

  while (length > 0) {
    size_t nb = length;
    tcpstp->err = urosConnRecv(tcpstp->csp, &bufp, &nb);
    urosError(tcpstp->err != UROS_OK, return tcpstp->err,
              ("Error %s while skipping %zu bytes from the current position\n",
               urosErrorText(tcpstp->err), length));
    length -= nb;
  }
  return tcpstp->err = UROS_OK;
}

/**
 * @brief   Expects a token from the incoming TCPROS stream.
 *
 * @param[in,out] tcpstp
 *          Pointer to a TCPROS status with a working connection.
 * @param[in] tokp
 *          Pointer to the token string. Can be @p NULL only if
 *          @p toklen is @p 0.
 * @param[in] toklen
 *          Token length, in bytes. Can be @p 0.
 * @return
 *          Error code.
 * @retval UROS_ERR_PARSE
 *          Token not found.
 */
uros_err_t urosTcpRosExpect(UrosTcpRosStatus *tcpstp, void *tokp, size_t toklen) {

  uint8_t *curp, *bufp;
  size_t pending, nb;

  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(!(toklen > 0) || tokp != NULL);

  curp = (uint8_t*)tokp;
  pending = toklen;
  while (pending > 0) {
    nb = pending;
    tcpstp->err = urosConnRecv(tcpstp->csp, (void**)&bufp, &nb);
    urosError(tcpstp->err != UROS_OK, return tcpstp->err,
              ("Error %s while receiving %zu bytes after [%.*s]\n",
               urosErrorText(tcpstp->err), nb,
               (int)((ptrdiff_t)curp - (ptrdiff_t)tokp), tokp));
    urosError(0 != memcmp(curp, bufp, nb),
              return tcpstp->err = UROS_ERR_PARSE,
              ("Found [%.*s], expected [%.*s] after [%.*s] in token [%.*s])\n",
               (int)nb, curp,
               (int)nb, bufp,
               (int)((ptrdiff_t)curp - (ptrdiff_t)tokp), tokp,
               (int)toklen, tokp));
    pending -= nb;
    curp += nb;
  }
  return tcpstp->err = UROS_OK;
}

/**
 * @brief   Reads some data from the incoming TCPROS stream.
 *
 * @param[in,out] tcpstp
 *          Pointer to a TCPROS status with a working connection.
 * @param[in] bufp
 *          Pointer to the read buffer.
 * @param[in] buflen
 *          Length of the data to be read, in bytes. Can be @p 0.
 * @return
 *          Error code.
 */
uros_err_t urosTcpRosRecv(UrosTcpRosStatus *tcpstp,
                          void *bufp, size_t buflen) {

  uint8_t *recvp, *curp;
  size_t pending;

  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(!(buflen > 0) || bufp != NULL);

  curp = (uint8_t*)bufp;
  pending = buflen;
  while (pending > 0) {
    size_t nb = pending;
    tcpstp->err = urosConnRecv(tcpstp->csp, (void**)&recvp, &nb);
    urosError(tcpstp->err != UROS_OK, return tcpstp->err,
              ("Error %s while receiving %zu bytes after [%.*s]\n",
               urosErrorText(tcpstp->err), nb,
               (int)((ptrdiff_t)curp - (ptrdiff_t)bufp), bufp));
    memcpy(curp, recvp, nb);
    pending -= nb;
    curp += nb;
  }
  return tcpstp->err = UROS_OK;
}

/**
 * @brief   Reads a string from the incoming TCPROS stream.
 *
 * @param[in,out] tcpstp
 *          Pointer to a TCPROS status with a working connection.
 * @param[in] strp
 *          Pointer to an allcoated @p UrosString object to be read.
 * @return
 *          Error code.
 */
uros_err_t urosTcpRosRecvString(UrosTcpRosStatus *tcpstp,
                                UrosString *strp) {

  uint32_t length;

  urosAssert(tcpstp != NULL);
  urosAssert(strp != NULL);

  /* Initialize the string.*/
  urosStringObjectInit(strp);

  /* Read the string length.*/
  urosTcpRosRecvRaw(tcpstp, length);
  if (tcpstp->err != UROS_OK) { return tcpstp->err; }

  /* Read the string data.*/
  strp->datap = (char*)urosAlloc(strp->length);
  if (strp->datap == NULL) {
    strp->length = 0;
    return tcpstp->err = UROS_ERR_NOMEM;
  }
  urosTcpRosRecv(tcpstp, strp->datap, strp->length);
  if (tcpstp->err != UROS_OK) {
    urosStringClean(strp);
    return tcpstp->err;
  }

  return UROS_OK;
}

/**
 * @brief   Writes some data to the outgoing TCPROS stream.
 *
 * @param[in,out] tcpstp
 *          Pointer to a TCPROS status with a working connection.
 * @param[in] bufp
 *          Pointer to the buffered data to be written.
 * @param[in] buflen
 *          Length of the data to be written, in bytes. Can be @p 0.
 * @return
 *          Error code.
 */
uros_err_t urosTcpRosSend(UrosTcpRosStatus *tcpstp,
                          const void *bufp, size_t buflen) {

  urosAssert(tcpstp != NULL);
  urosAssert(tcpstp->csp != NULL);
  urosAssert(!(buflen > 0) || bufp != NULL);

  if (buflen > 0) {
    tcpstp->err = urosConnSend(tcpstp->csp, bufp, buflen);
    urosError(tcpstp->err != UROS_OK, return tcpstp->err,
              ("Error %s while sending [%.*s]\n",
               urosErrorText(tcpstp->err), (int)buflen, bufp));
  }
  return tcpstp->err = UROS_OK;
}

/**
 * @brief   Writes a string to the outgoing TCPROS stream.
 *
 * @param[in,out] tcpstp
 *          Pointer to a TCPROS status with a working connection.
 * @param[in] strp
 *          Pointer to a valid @p UrosString object to be written.
 * @return
 *          Error code.
 */
uros_err_t urosTcpRosSendString(UrosTcpRosStatus *tcpstp,
                                const UrosString *strp) {

  uint32_t size;

  urosAssert(tcpstp != NULL);
  urosAssert(urosStringIsValid(strp));

  /* Write the string length.*/
  size = (uint32_t)strp->length;
  urosTcpRosSend(tcpstp, &size, sizeof(uint32_t));
  if (tcpstp->err != UROS_OK) { return tcpstp->err; }

  /* Write the string data.*/
  urosTcpRosSend(tcpstp, strp->datap, strp->length);
  if (tcpstp->err != UROS_OK) { return tcpstp->err; }

  return UROS_OK;
}

/**
 * @brief   Writes a string to the outgoing TCPROS stream.
 *
 * @param[in,out] tcpstp
 *          Pointer to a TCPROS status with a working connection.
 * @param[in] strp
 *          Pointer to a null-terminated string. Can be @p NULL.
 * @return
 *          Error code.
 */
uros_err_t urosTcpRosSendStringSZ(UrosTcpRosStatus *tcpstp,
                                  const char *strp) {

  uint32_t size;

  urosAssert(tcpstp != NULL);

  /* Write the string length.*/
  size = (strp != NULL) ? (uint32_t)strlen(strp) : 0;
  urosTcpRosSend(tcpstp, &size, sizeof(uint32_t));
  if (tcpstp->err != UROS_OK) { return tcpstp->err; }

  /* Write the string data.*/
  urosTcpRosSend(tcpstp, strp, (size_t)size);
  if (tcpstp->err != UROS_OK) { return tcpstp->err; }

  return UROS_OK;
}

/**
 * @brief   Sends the error string
 * @details The error string is <tt>tcpstp->errstr</tt>.
 *
 * @param[in,out] tcpstp
 *          Pointer to an initialized @p UrosTcpRosStatus object.
 * @return
 *          Error code.
 */
uros_err_t urosTcpRosSendError(UrosTcpRosStatus *tcpstp) {

  size_t hdrlen = 0;
  uint32_t hdrlen32;
  const UrosString *errstrp, *typestrp, *md5strp;

  urosAssert(tcpstp != NULL);
#define _CHKOK  { if (tcpstp->err != UROS_OK) { return tcpstp->err; } }

  if (tcpstp->topicp != NULL) {
    typestrp = &dummytype.name;
    md5strp = &dummytype.md5str;
    (void)dummytopic;
  } else {
    typestrp = &tcpstp->topicp->typep->name;
    md5strp = &tcpstp->topicp->typep->md5str;
  }
  errstrp = &tcpstp->errstr;

  hdrlen += errfield.length + 1 + errstrp->length;
  hdrlen += typefield.length + 1 + typestrp->length;
  hdrlen += md5field.length + 1 + md5strp->length;

  /* uint32 header_length */
  hdrlen32 = (uint32_t)hdrlen;
  urosTcpRosSend(tcpstp, &hdrlen32, sizeof(hdrlen32)); _CHKOK

  /* uint32 field_length, error={str} */
  uros_tcpros_sendfieldstring(tcpstp, &errfield, errstrp); _CHKOK

  /* uint32 field_length, type={str} */
  uros_tcpros_sendfieldstring(tcpstp, &typefield, typestrp); _CHKOK

  /* uint32 field_length, md5sum={str} */
  uros_tcpros_sendfieldstring(tcpstp, &md5field, md5strp); _CHKOK

  return tcpstp->err = UROS_OK;
#undef _CHKOK
}

/**
 * @brief   TCPROS Listener thread.
 * @details This thread listens for incoming TCPROS connections by TCPROS
 *          clients.
 *
 *          An incoming connection is accepted by he Listener, and then
 *          handled by a new TCPROS Server thread, allocated from a dedicated
 *          thread pool. If no threads are free, it waits until one becomes
 *          available again.
 * @see     urosThreadPoolStartWorker()
 *
 * @pre     There are no other TCPROS listener threads with the same connection
 *          port.
 *
 * @param[in] data
 *          Ignored.
 * @return
 *          Error code.
 */
uros_err_t urosTcpRosListenerThread(void *data) {

  uros_err_t err;
  UrosAddr locaddr;
  UrosConn conn;
  (void)data;
  (void)err;

  /* Setup the local address.*/
  locaddr.port = urosNode.config.tcprosAddr.port;
  locaddr.ip.dword = UROS_ANY_IP;

  /* Setup the listening socket.*/
  urosConnObjectInit(&conn);
  err = urosConnCreate(&conn, UROS_PROTO_TCP);
  urosAssert(err == UROS_OK);
  err = urosConnBind(&conn, &locaddr);
  urosAssert(err == UROS_OK);

  /* Start listening.*/
  err = urosConnListen(&conn, UROS_TCPROS_LISTENER_BACKLOG);
  urosAssert(err == UROS_OK);
  while (UROS_TRUE) {
    UrosConn *spawnedp;

    /* Accept the incoming connection.*/
    spawnedp = urosNew(UrosConn);
    urosAssert(spawnedp != NULL);
    urosConnObjectInit(spawnedp);
    err = urosConnAccept(&conn, spawnedp);
    urosAssert(err == UROS_OK);
#if 0
    err = urosConnSetSendTimeout(&conn, UROS_TCPROS_SENDTIMEOUT);
    urosAssert(err == UROS_OK);
#endif
    /* Create the TCPROS Server worker thread.*/
    err = urosThreadPoolStartWorker(&urosNode.status.tcpsvrThdPool,
                                    (void*)spawnedp);

    /* Check if anything went wrong.*/
    if (err != UROS_OK && urosConnIsValid(spawnedp)) {
      urosConnClose(spawnedp);
      urosFree(spawnedp);
    }
  }
  return UROS_OK;
}

/**
 * @brief   TCPROS Server worker thread.
 * @details This thread processes an incoming TCPROS topic or service request.
 *
 *          The TCPROS header is received, and the toipc/service handler is
 *          called for processing by the user-defined routine inside the
 *          related descriptor.
 *
 * @pre     The provided connection was accepted by the TCPROS Listener.
 * @pre     The connection was alloacted with @p urosAlloc().
 * @post    The connection is closed and deallocated, and @p csp points to an
 *          invalid address.
 *
 * @param[in] csp
 *          Pointer to the incoming connection.
 * @return
 *          Error code.
 */
uros_err_t urosTcpRosServerThread(UrosConn *csp) {

  UrosTcpRosStatus *tcpstp;
  uros_err_t err;
  uros_proc_f handler = NULL;

  urosAssert(csp != NULL);

  tcpstp = urosNew(UrosTcpRosStatus);
  if (tcpstp == NULL) { return UROS_ERR_NOMEM; }
  urosTpcRosStatusObjectInit(tcpstp, csp);

  /* Receive the connection header.*/
  err = uros_tcpros_recvheader(tcpstp, UROS_FALSE);
  if (err != UROS_OK) {
    /* Send the error message.*/
    tcpstp->errstr = urosStringCloneZ(urosErrorText(err));
    urosTcpRosSendError(tcpstp);
    urosStringClean(&tcpstp->errstr);
    goto _finally;
  }

  /* Process the received header, to link to the actual topic/service.*/
  if (tcpstp->flags.service) {
    /* TCPROS service connection header.*/
    urosMutexLock(&urosNode.status.pubServiceListLock);
    err = uros_tcpserver_processserviceheader(tcpstp);
    if (err == UROS_OK) {
      handler = tcpstp->topicp->procf;
    }
    urosMutexUnlock(&urosNode.status.pubServiceListLock);
    if (err != UROS_OK) {
      /* Write the error byte and string.*/
      uint32_t length = 0;
      tcpstp->errstr = urosStringCloneZ(urosErrorText(err));
      urosTcpRosSend(tcpstp, &length, 1);
      length = (uint32_t)tcpstp->errstr.length;
      urosTcpRosSend(tcpstp, &length, sizeof(length));
      urosTcpRosSend(tcpstp, tcpstp->errstr.datap, length);
      goto _finally;
    }
  } else {
    /* TCPROS topic connection header.*/
    urosMutexLock(&urosNode.status.pubTopicListLock);
    err = uros_tcpserver_processtopicheader(tcpstp);
    if (err == UROS_OK) {
      handler = tcpstp->topicp->procf;
    }
    urosMutexUnlock(&urosNode.status.pubTopicListLock);
    if (err != UROS_OK) {
      /* Send an error message.*/
      tcpstp->errstr = urosStringCloneZ(urosErrorText(err));
      urosTcpRosSendError(tcpstp);
      goto _finally;
    }
  }

  /* Send the response header.*/
  err = uros_tcpros_sendheader(tcpstp, UROS_FALSE);
  if (err != UROS_OK) { goto _finally; }

  /* Call the connection handler.*/
  tcpstp->err = UROS_OK;
  urosAssert(handler != NULL);
  err = handler(tcpstp);

  /* Release the topic/service descriptor reference.*/
  if (tcpstp->flags.service) {
    urosTcpRosServiceDone(tcpstp);
  } else {
    urosTcpRosTopicPublisherDone(tcpstp);
  }

_finally:
  /* Close the connection and free the allocated memory.*/
  urosConnClose(csp);
  urosTpcRosStatusDelete(tcpstp, UROS_TRUE);
  return err;
}

/**
 * @brief   TCPROS Client worker thread.
 * @details This thread routine subscribes to remote TCPROS topics or
 *          services.
 *
 *          The TCPROS header is sent, and the toipc/service handler is called
 *          for processing by the user-defined routine inside the related
 *          descriptor.
 *
 * @pre     The thread arguments addressed by @p argsp are allocated with
 *          @p urosAlloc().
 * @post    The thread arguments are deallocated, and @p argsp points to an
 *          invalid address.
 *
 * @param[in] argsp
 *          Pointer to the TCPROS Client worker thread arguments.
 * @return
 *          Error code.
 */
uros_err_t urosTcpRosClientThread(uros_tcpcliargs_t *argsp) {

  uros_err_t err;
  UrosAddr pubaddr;

  urosAssert(argsp != NULL);
  urosAssert(urosStringNotEmpty(&argsp->topicName));

  /* Resolve the publisher address.*/
  err = uros_tcpros_resolvepublisher(argsp, &pubaddr);
  if (err != UROS_OK) { goto _finally; }

  /* TODO: Dispatch between topics and services.*/
  /* Start a new TCPROS topic subscriber connection.*/
  err = uros_tcpcli_topicsubscription(&argsp->topicName, &pubaddr);
  if (err != UROS_OK) { goto _finally; }

  err = UROS_OK;
_finally:
  urosTopicSubParamsDelete(argsp);
  return err;
}

/**
 * @brief   Notifies that a TCPROS topic subscriber thread has terminated.
 * @details Releases a topic reference. If the topic was deleted and this was
 *          its last reference, free its descriptor.
 *
 * @pre     Called by a TCPROS topic subscriber handler thread which has
 *          finished its job.
 * @post    The topic has one reference less.
 * @post    If this is the last topic reference, the topic descriptor is
 *          deleted.
 *
 * @param[in,out] tcpstp
 *          Pointer to an initialized @p UrosTcpRosStatus object.
 */
void urosTcpRosTopicSubscriberDone(UrosTcpRosStatus *tcpstp) {

  urosAssert(tcpstp != NULL);
  urosAssert(tcpstp->topicp != NULL);

  /* Decrement the topic reference count.*/
  urosMutexLock(&urosNode.status.subTopicListLock);
  if (0 == urosTopicRefDec(tcpstp->topicp)) {
    /* The topic has been deleted an this is its last reference, free it.*/
    if (tcpstp->flags.deleted) {
      urosTopicDelete(tcpstp->topicp);
      tcpstp->topicp = NULL;
    }
  }
  urosMutexUnlock(&urosNode.status.subTopicListLock);
}

/**
 * @brief   Notifies that a TCPROS topic publisher thread has terminated.
 * @details Releases a topic reference. If the topic was deleted and this was
 *          its last reference, free its descriptor.
 *
 * @pre     Called by a TCPROS topic publisher handler thread which has
 *          finished its job.
 * @post    The topic has one reference less.
 * @post    If this is the last topic reference, the topic descriptor is
 *          deleted.
 *
 * @param[in,out] tcpstp
 *          Pointer to an initialized @p UrosTcpRosStatus object.
 */
void urosTcpRosTopicPublisherDone(UrosTcpRosStatus *tcpstp) {

  urosAssert(tcpstp != NULL);
  urosAssert(tcpstp->topicp != NULL);

  /* Decrement the topic reference count.*/
  urosMutexLock(&urosNode.status.pubTopicListLock);
  if (0 == urosTopicRefDec(tcpstp->topicp)) {
    /* The topic has been deleted an this is its last reference, free it.*/
    if (tcpstp->flags.deleted) {
      urosTopicDelete(tcpstp->topicp);
      tcpstp->topicp = NULL;
    }
  }
  urosMutexUnlock(&urosNode.status.pubTopicListLock);
}

/**
 * @brief   Notifies that a TCPROS service thread has terminated.
 * @details Releases a service reference. If the ervice was deleted and this
 *          was its last reference, free its descriptor.
 *
 * @pre     Called by a TCPROS service thread which has finished its job.
 * @post    The service has one reference less.
 * @post    If this is the last service reference, the service descriptor is
 *          deleted.
 *
 * @param[in,out] tcpstp
 *          Pointer to an initialized @p UrosTcpRosStatus object.
 */
void urosTcpRosServiceDone(UrosTcpRosStatus *tcpstp) {

  urosAssert(tcpstp != NULL);
  urosAssert(tcpstp->topicp != NULL);

  /* Decrement the service reference count.*/
  urosMutexLock(&urosNode.status.pubServiceListLock);
  if (0 == urosTopicRefDec(tcpstp->topicp)) {
    /* The service has been deleted an this is its last reference, free it.*/
    if (tcpstp->flags.deleted) {
      urosTopicDelete(tcpstp->topicp);
      tcpstp->topicp = NULL;
    }
  }
  urosMutexUnlock(&urosNode.status.pubServiceListLock);
}

/** @} */
