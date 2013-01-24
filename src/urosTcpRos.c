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

#if UROS_TCPROS_C_USE_ERROR_MSG == UROS_FALSE && !defined(__DOXYGEN__)
#undef urosError
#define urosError(when, action, msgargs) { if (when) { action; } }
#endif

/*===========================================================================*/
/* LOCAL VARIABLES                                                           */
/*===========================================================================*/

static const UrosString calleridfield   = {  8, "callerid" };
static const UrosString topicfield      = {  5, "topic" };
static const UrosString servicefield    = {  7, "service" };
static const UrosString md5field        = {  6, "md5sum" };
static const UrosString typefield       = {  4, "type" };
static const UrosString reqtypefield    = { 12, "request_type" };
static const UrosString restypefield    = { 13, "response_type" };
static const UrosString persistentfield = { 10, "persistent" };
static const UrosString latchingfield   = {  8, "latching" };
static const UrosString tcpnodelayfield = { 11, "tcp_nodelay" };
static const UrosString errfield        = {  5, "error" };

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

uros_err_t uros_tcpserver_processtopicheader(UrosTcpRosStatus *tcpstp) {

  static UrosNodeStatus *const stp = &urosNode.status;

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
  topicnodep = urosTopicListFindByName(&stp->pubTopicList,
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
  urosMutexLock(&stp->pubTcpListLock);
  urosListAdd(&stp->pubTcpList, tcpnodep);
  urosMutexUnlock(&stp->pubTcpListLock);

  tcpstp->err = UROS_OK;
_finally:
  urosMsgTypeDelete((UrosMsgType*)topicp->typep);
  urosTopicDelete(topicp);
  return tcpstp->err;
}

uros_err_t uros_tcpserver_processserviceheader(UrosTcpRosStatus *tcpstp) {

  static UrosNodeStatus *const stp = &urosNode.status;

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
  servicenodep = urosTopicListFindByName(&stp->pubServiceList,
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
  urosMutexLock(&stp->pubTcpListLock);
  urosListAdd(&stp->pubTcpList, tcpnodep);
  urosMutexUnlock(&stp->pubTcpListLock);

_finally:
  if (tcpstp->err != UROS_OK && servicenodep != NULL) {
    urosTopicRefDec((UrosTopic*)servicenodep->datap);
  }
  urosMsgTypeDelete((UrosMsgType*)servicep->typep);
  urosTopicDelete(servicep);
  return tcpstp->err;
}

uros_err_t uros_tcpcli_topicsubscription(const UrosString *namep,
                                         const UrosAddr *pubaddrp) {

  static UrosNodeStatus *const stp = &urosNode.status;

  uros_err_t err;
  UrosConn conn;
  UrosTcpRosStatus tcpst;
  UrosListNode *topicnodep;

  urosAssert(urosStringNotEmpty(namep));
  urosAssert(pubaddrp != NULL);
#define _CHKOK  { if (err != UROS_OK) { goto _error; } }

  /* Get topic features.*/
  urosTcpRosStatusObjectInit(&tcpst, &conn);
  urosMutexLock(&stp->subTopicListLock);
  topicnodep = urosTopicListFindByName(&stp->subTopicList,
                                       namep);
  if (topicnodep != NULL) {
    tcpst.topicp = (UrosTopic*)topicnodep->datap;
    urosTopicRefInc(tcpst.topicp);
  }
  urosMutexUnlock(&stp->subTopicListLock);
  urosError(topicnodep == NULL, return UROS_ERR_BADPARAM,
            ("Topic [%.*s] not found\n", UROS_STRARG(namep)));

  /* Connect to the publisher.*/
  urosConnObjectInit(&conn);
  urosConnCreate(&conn, UROS_PROTO_TCP);
  err = urosConnConnect(&conn, pubaddrp);
  urosError(err != UROS_OK, goto _error,
            ("Error %s while connecting to "UROS_ADDRFMT"\n",
             urosErrorText(err), UROS_ADDRARG(pubaddrp)));

  /* Set timeouts for the spawned connection.*/
  err = urosConnSetRecvTimeout(&conn, UROS_TCPROS_RECVTIMEOUT);
  urosAssert(err == UROS_OK);
  err = urosConnSetSendTimeout(&conn, UROS_TCPROS_SENDTIMEOUT);
  urosAssert(err == UROS_OK);

  /* Send the TCPROS conenction header.*/
  err = urosTcpRosSendHeader(&tcpst, UROS_TRUE); _CHKOK

  /* Receive the TCPROS connection header.*/
  err = urosTcpRosRecvHeader(&tcpst, UROS_FALSE, UROS_FALSE); _CHKOK

  /* Handle the incoming published stream, if not probing.*/
  tcpst.err = UROS_OK;
  urosAssert(tcpst.topicp->procf != NULL);
  if (!tcpst.topicp->flags.probe) {
    err = tcpst.topicp->procf(&tcpst);
  }

  /* Release the topic/service descriptor reference.*/
  urosTcpRosTopicSubscriberDone(&tcpst);

  urosError(err != UROS_OK, goto _error,
            ("Topic [%.*s] client handler returned %s\n",
             UROS_STRARG(namep), urosErrorText(err)));
  return urosConnClose(&conn);

_error:
  urosConnClose(&conn);
  return err;
#undef _CHKOK
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
void urosTcpRosStatusObjectInit(UrosTcpRosStatus *tcpstp, UrosConn *csp) {

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
void urosTcpRosStatusClean(UrosTcpRosStatus *tcpstp, uros_bool_t deep) {

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
void urosTcpRosStatusDelete(UrosTcpRosStatus *tcpstp, uros_bool_t deep) {

  if (tcpstp != NULL) {
    urosTcpRosStatusClean(tcpstp, deep);
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
              ("Error %s while skipping %u bytes from the current position\n",
               urosErrorText(tcpstp->err), (unsigned)length));
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
uros_err_t urosTcpRosExpect(UrosTcpRosStatus *tcpstp,
                            void *tokp, size_t toklen) {

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
              ("Error %s while receiving %u bytes after [%.*s]\n",
               urosErrorText(tcpstp->err), (unsigned)nb,
               (unsigned)((ptrdiff_t)curp - (ptrdiff_t)tokp),
               (const char *)tokp));
    urosError(0 != memcmp(curp, bufp, nb),
              return tcpstp->err = UROS_ERR_PARSE,
              ("Found [%.*s], expected [%.*s] after [%.*s] in token [%.*s])\n",
               (unsigned)nb, curp,
               (unsigned)nb, bufp,
               (unsigned)((ptrdiff_t)curp - (ptrdiff_t)tokp),
               (const char *)tokp,
               (unsigned)toklen, (const char *)tokp));
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
    if (tcpstp->err != UROS_OK) {
      if (tcpstp->err == UROS_ERR_EOF && nb < pending) {
        urosError(tcpstp->err == UROS_ERR_EOF && nb < pending,
                  return tcpstp->err,
                  ("Error %s while receiving %u bytes after [%.*s]\n",
                   urosErrorText(tcpstp->err), (unsigned)nb,
                   (unsigned)(buflen - pending), (const char *)bufp));
      } else {
        urosError(tcpstp->err != UROS_OK && tcpstp->err != UROS_ERR_TIMEOUT,
                  return tcpstp->err,
                  ("Error %s while receiving %u bytes after [%.*s]\n",
                   urosErrorText(tcpstp->err), (unsigned)nb,
                   (unsigned)(buflen - pending), (const char *)bufp));
        return UROS_ERR_TIMEOUT;
      }
    }
    memcpy(curp, recvp, nb);
    pending -= nb;
    curp += nb;
  }
  return tcpstp->err = UROS_OK;
}

/**
 * @brief   Reads some data from the incoming TCPROS stream.
 * @details Data is read in a reversed (per-byte) fashion.
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
uros_err_t urosTcpRosRecvRev(UrosTcpRosStatus *tcpstp,
                             void *bufp, size_t buflen) {

  uint8_t *recvp, *curp;
  size_t pending;

  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(!(buflen > 0) || bufp != NULL);

  curp = (uint8_t*)bufp + buflen;
  pending = buflen;
  while (pending > 0) {
    size_t nb = pending;
    tcpstp->err = urosConnRecv(tcpstp->csp, (void**)&recvp, &nb);
    if (tcpstp->err != UROS_OK) {
      if (tcpstp->err == UROS_ERR_EOF && nb < pending) {
        urosError(tcpstp->err == UROS_ERR_EOF && nb < pending,
                  return tcpstp->err,
                  ("Error %s while receiving %u bytes\n",
                   urosErrorText(tcpstp->err), (unsigned)nb));
      } else {
        urosError(tcpstp->err != UROS_OK && tcpstp->err != UROS_ERR_TIMEOUT,
                  return tcpstp->err,
                  ("Error %s while receiving %u bytes\n",
                   urosErrorText(tcpstp->err), (unsigned)nb));
        return UROS_ERR_TIMEOUT;
      }
    }
    while (nb-- > 0) {
      *--curp = *recvp++;
      --pending;
    }
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
  strp->length = (size_t)length;
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
    urosError(tcpstp->err != UROS_OK && tcpstp->err != UROS_ERR_TIMEOUT,
              return tcpstp->err,
              ("Error %s while sending [%.*s]\n",
               urosErrorText(tcpstp->err), (int)buflen, (const char *)bufp));
    if (tcpstp->err == UROS_ERR_TIMEOUT) { return UROS_ERR_TIMEOUT; }
  }
  return tcpstp->err = UROS_OK;
}

/**
 * @brief   Writes some data to the outgoing TCPROS stream.
 * @details Data is written in a reversed (per-byte) fashion.
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
uros_err_t urosTcpRosSendRev(UrosTcpRosStatus *tcpstp,
                             const void *bufp, size_t buflen) {

  urosAssert(tcpstp != NULL);
  urosAssert(tcpstp->csp != NULL);
  urosAssert(!(buflen > 0) || bufp != NULL);

  for (; buflen > 0; --buflen) {
    tcpstp->err = urosConnSend(tcpstp->csp, bufp, 1);
    urosError(tcpstp->err != UROS_OK && tcpstp->err != UROS_ERR_TIMEOUT,
              return tcpstp->err,
              ("Error %s while sending [%*.s]\n",
               urosErrorText(tcpstp->err), (int)buflen, (const char *)bufp));
    if (tcpstp->err == UROS_ERR_TIMEOUT) { return UROS_ERR_TIMEOUT; }
    bufp = (const void *)((const uint8_t *)bufp + 1);
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
  urosTcpRosSendRaw(tcpstp, size);
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
  urosTcpRosSendRaw(tcpstp, size);
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

  uint32_t hdrlen = 0;
  const UrosString *errstrp, *typestrp, *md5strp;

  urosAssert(tcpstp != NULL);
#define _CHKOK  { if (tcpstp->err != UROS_OK) { return tcpstp->err; } }

#define _FIELDSTRING(tcpstp, namep, valuep) { \
  urosAssert(urosStringNotEmpty(namep)); \
  urosAssert(valuep != NULL); \
  hdrlen = (uint32_t)((namep)->length + 1 + (valuep)->length); \
  urosTcpRosSendRaw(tcpstp, hdrlen); _CHKOK \
  urosTcpRosSend(tcpstp, (namep)->datap, (namep)->length); _CHKOK \
  urosTcpRosSend(tcpstp, "=", 1); _CHKOK \
  urosTcpRosSend(tcpstp, (valuep)->datap, (valuep)->length); _CHKOK }

  if (tcpstp->topicp == NULL) {
    typestrp = &dummytype.name;
    md5strp = &dummytype.md5str;
    (void)dummytopic;
  } else {
    typestrp = &tcpstp->topicp->typep->name;
    md5strp = &tcpstp->topicp->typep->md5str;
  }
  errstrp = &tcpstp->errstr;

  hdrlen += (uint32_t)(1 + errfield.length + errstrp->length);
  hdrlen += (uint32_t)(1 + typefield.length + typestrp->length);
  hdrlen += (uint32_t)(1 + md5field.length + md5strp->length);

  /* uint32 header_length */
  urosTcpRosSendRaw(tcpstp, hdrlen); _CHKOK

  /* uint32 field_length, error={str} */
  _FIELDSTRING(tcpstp, &errfield, errstrp);

  /* uint32 field_length, type={str} */
  _FIELDSTRING(tcpstp, &typefield, typestrp);

  /* uint32 field_length, md5sum={str} */
  _FIELDSTRING(tcpstp, &md5field, md5strp);

  return tcpstp->err = UROS_OK;
#undef _CHKOK
#undef _FIELDSTRING
}

/**
 * @brief   Sends a TCPROS handshake header.
 * @details The @p isrequest flag tells wether a request or a response header
 *          will be generated. The topic flags of the topic referenced by
 *          @p tcpstp will be used. In particular, the @p service flag will
 *          switch between the @e topic or the @e service header format.
 *
 * @param[in,out] tcpstp
 *          Pointer to a TCPROS status with a working connection.
 * @param[in] isrequest
 *          Tells if sending a request handshake header, otherwise a response
 *          one will be generated.
 * @return
 *          Error code.
 */
uros_err_t urosTcpRosSendHeader(UrosTcpRosStatus *tcpstp,
                                uros_bool_t isrequest) {

  uint32_t hdrlen = 0;
  const UrosString *calleridstrp, *namestrp, *typestrp, *md5strp;

  urosAssert(tcpstp != NULL);
  urosAssert(tcpstp->topicp != NULL);
  urosAssert(tcpstp->topicp->typep != NULL);
#define _CHKOK  { if (tcpstp->err != UROS_OK) { return tcpstp->err; } }

#define _FIELDSTRING(tcpstp, namep, valuep) { \
  urosAssert(urosStringNotEmpty(namep)); \
  urosAssert(valuep != NULL); \
  hdrlen = (uint32_t)((namep)->length + 1 + (valuep)->length); \
  urosTcpRosSendRaw(tcpstp, hdrlen); _CHKOK \
  urosTcpRosSend(tcpstp, (namep)->datap, (namep)->length); _CHKOK \
  urosTcpRosSend(tcpstp, "=", 1); _CHKOK \
  urosTcpRosSend(tcpstp, (valuep)->datap, (valuep)->length); _CHKOK }

#define _FIELDBOOL(tcpstp, namep, value) { \
  urosAssert(urosStringNotEmpty(namep)); \
  hdrlen = (uint32_t)((namep)->length + 2); \
  urosTcpRosSendRaw(tcpstp, hdrlen); _CHKOK \
  urosTcpRosSend(tcpstp, (namep)->datap, (namep)->length); _CHKOK \
  urosTcpRosSend(tcpstp, (value) ? "=1" : "=0", 2); _CHKOK }

  calleridstrp = &urosNode.config.nodeName;
  typestrp = &tcpstp->topicp->typep->name;
  md5strp = &tcpstp->topicp->typep->md5str;

  hdrlen += (uint32_t)(5 + calleridfield.length + calleridstrp->length);
  hdrlen += (uint32_t)(5 + md5field.length + md5strp->length);
  hdrlen += (uint32_t)(5 + typefield.length + typestrp->length);
  if (isrequest) {
    namestrp = &tcpstp->topicp->name;
    if (tcpstp->topicp->flags.service) {
      hdrlen += (uint32_t)(5 + servicefield.length + namestrp->length);
      hdrlen += (uint32_t)(6 + persistentfield.length);
    } else {
      hdrlen += (uint32_t)(5 + topicfield.length + namestrp->length);
      hdrlen += (uint32_t)(6 + tcpnodelayfield.length);
    }
  } else {
    namestrp = NULL;
    if (tcpstp->topicp->flags.service) {
      hdrlen += (uint32_t)(12 + reqtypefield.length + typestrp->length);
      hdrlen += (uint32_t)(13 + restypefield.length + typestrp->length);
    } else {
      hdrlen += (uint32_t)(6 + latchingfield.length);
    }
  }

  /* uint32 header_length */
  urosTcpRosSendRaw(tcpstp, hdrlen); _CHKOK

  /* uint32 field_length, callerid={str} */
  _FIELDSTRING(tcpstp, &calleridfield, calleridstrp);

  if (isrequest) {
    if (tcpstp->topicp->flags.service) {
      /* uint32 field_length, service={str} */
      _FIELDSTRING(tcpstp, &servicefield, namestrp);
    } else {
      /* uint32 field_length, topic={str} */
      _FIELDSTRING(tcpstp, &topicfield, namestrp);
    }
  }

  /* uint32 field_length, md5sum={str} */
  _FIELDSTRING(tcpstp, &md5field, md5strp);

  if (tcpstp->topicp->flags.service) {
    /* uint32 field_length, request_type={str} */
    hdrlen = 8 + reqtypefield.length + typestrp->length;
    urosTcpRosSendRaw(tcpstp, hdrlen); _CHKOK
    urosTcpRosSend(tcpstp, reqtypefield.datap, reqtypefield.length); _CHKOK
    urosTcpRosSend(tcpstp, "=", 1); _CHKOK
    urosTcpRosSend(tcpstp, typestrp->datap, typestrp->length); _CHKOK
    urosTcpRosSend(tcpstp, "Request", 7); _CHKOK

    /* uint32 field_length, response_type={str} */
    hdrlen = 9 + restypefield.length + typestrp->length;
    urosTcpRosSendRaw(tcpstp, hdrlen); _CHKOK
    urosTcpRosSend(tcpstp, restypefield.datap, restypefield.length); _CHKOK
    urosTcpRosSend(tcpstp, "=", 1); _CHKOK
    urosTcpRosSend(tcpstp, typestrp->datap, typestrp->length); _CHKOK
    urosTcpRosSend(tcpstp, "Response", 8); _CHKOK
  }

  /* uint32 field_length, type={str} */
  _FIELDSTRING(tcpstp, &typefield, typestrp);

  if (isrequest) {
    if (tcpstp->topicp->flags.service) {
      /* uint32 field_length, persistent=(0|1) */
      _FIELDBOOL(tcpstp, &persistentfield, tcpstp->topicp->flags.persistent);
    } else {
      /* uint32 field_length, tcp_nodelay=(0|1) */
      _FIELDBOOL(tcpstp, &tcpnodelayfield, tcpstp->topicp->flags.noDelay);
    }
  } else {
    if (!tcpstp->topicp->flags.service) {
      /* uint32 field_length, latching=(0|1) */
      _FIELDBOOL(tcpstp, &latchingfield, tcpstp->topicp->flags.latching);
    }
  }

  return tcpstp->err = UROS_OK;
#undef _CHKOK
#undef _FIELDSTRING
#undef _FIELDBOOL
}

/**
 * @brief   Receives a TCPROS handshake header.
 * @details The @p isrequest flag tells wether a request or a response header
 *          is expected to be received. The topic flags of the topic referenced
 *          by @p tcpstp will be updated or checked. In particular, the
 *          @p service flag will switch between the @e topic or the @e service
 *          header format.
 *          Additionally, there is a different use of the @p topicp pointer of
 *          the @p tcpstp object, depending on the @p isrequest flag:
 *          - when @p true, the received topic information is written into a
 *            newly allocated @ UrosTopic object, which is returned by the
 *            @p tcpstp->topicp pointer (must be @p NULL at call time);
 *          - when @p false, the received topic information is compared with
 *            that of the @p tcpstp->topicp object for correctness.
 *
 * @param[in,out] tcpstp
 *          Pointer to a TCPROS status with a working connection.
 * @param[in] isrequest
 *          Tells if receiving a request handshake header, otherwise a response
 *          one will be expected.
 * @param[in] isservice
 *          Tells if the transaction is done by a ROS Service.
 * @return
 *          Error code.
 */
uros_err_t urosTcpRosRecvHeader(UrosTcpRosStatus *tcpstp,
                                uros_bool_t isrequest,
                                uros_bool_t isservice) {

  uint32_t hdrlen, remlen;
  uint32_t vallen, fieldlen;
  char *valp = NULL;
  UrosTopic *topicp;
  UrosMsgType *typep;
  char buf[4];

  urosAssert(tcpstp != NULL);
#define _ERRPARSE   { tcpstp->err = UROS_ERR_PARSE; goto _error; }
#define _CHKOK      { if (tcpstp->err != UROS_OK) { goto _error; } }
#define _GOT(tokp, toklen) \
    ((toklen <= 4) \
     ? (0 == memcmp(buf, tokp, toklen)) \
     : (0 == memcmp(buf, tokp, 4) && \
        UROS_OK == urosTcpRosExpect(tcpstp, tokp + 4, toklen - 4)))

  if (isrequest) {
    /* The server will receive the remote topic descriptor.*/
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
    /* The client will check for correctness of the remote descriptor.*/
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
    urosError(remlen < 4, _ERRPARSE,
              ("Premature end of header (no field length, %u bytes left)\n",
               (unsigned)remlen));
    remlen -= 4;
    urosTcpRosRecvRaw(tcpstp, fieldlen); _CHKOK
    urosError(remlen < fieldlen, _ERRPARSE,
              ("Premature end of header (%u bytes left, expected %u)\n",
               (unsigned)remlen, (unsigned)fieldlen));
    vallen = fieldlen;

    /* Decode the field name.*/
    urosTcpRosRecv(tcpstp, buf, 4);
    if (_GOT("callerid=", 9)) {
      /* callerid={str} */
      strp = &tcpstp->callerId;
      vallen -= 9;
    } else if (_GOT("error=", 6)) {
      /* error={str} */
      strp = &tcpstp->errstr;
      vallen -= 6;
    } else if (_GOT("latching=", 9)) {
      if (isrequest) { _ERRPARSE }
      if (isservice) {
        tcpstp->err = UROS_ERR_PARSE; goto _error;
      }
      /* latching=(0|1) */
      if (vallen == 10) {
        urosTcpRosRecv(tcpstp, buf, 1);
        if (buf[0] == '1') {
          tcpstp->remoteFlags.latching = UROS_TRUE; continue;
        } else if (buf[0] == '0') {
          tcpstp->remoteFlags.latching = UROS_FALSE; continue;
        } else { _ERRPARSE }
      } else { _ERRPARSE }
    } else if (_GOT("md5sum=", 7)) {
      /* md5sum={str} */
      strp = &typep->md5str;
      vallen -= 7;
      if (!isrequest) {
        /* Check if it matches the referenced one.*/
        if (vallen != strp->length) { _ERRPARSE }
        urosTcpRosExpect(tcpstp, strp->datap, strp->length);
        if (tcpstp->err == UROS_OK) { continue; }
        else { goto _error; }
      }
    } else if (_GOT("message_definition=", 19)) {
      /* message_definition={str} */
      vallen -= 19;
#if UROS_TCPROS_USE_MSGDEF
      strp = &typep->desc;
#else
      urosTcpRosSkip(tcpstp, vallen);
      continue;
#endif
    } else if (_GOT("persistent=", 11)) {
      if (!isrequest) { _ERRPARSE }
      if (!tcpstp->remoteFlags.service) {
        tcpstp->err = UROS_ERR_PARSE; goto _error;
      }
      /* persistent=(0|1) */
      if (vallen == 12) {
        urosTcpRosRecv(tcpstp, buf, 1);
        if (buf[0] == '1') {
          tcpstp->remoteFlags.persistent = UROS_TRUE; continue;
        } else if (buf[0] == '0') {
          tcpstp->remoteFlags.persistent = UROS_FALSE; continue;
        } else { _ERRPARSE }
      } else { _ERRPARSE }
    } else if (_GOT("probe=", 6)) {
      /* probe=(0|1) */
      if (vallen == 7) {
        urosTcpRosRecv(tcpstp, buf, 1);
        if (buf[0] == '1') {
          tcpstp->remoteFlags.probe = UROS_TRUE; continue;
        } else if (buf[0] == '0') {
          tcpstp->remoteFlags.probe = UROS_FALSE; continue;
        } else { _ERRPARSE }
      } else { _ERRPARSE }
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
      if (!isrequest) {
        /* Check if it matches the referenced one.*/
        urosTcpRosExpect(tcpstp, topicp->name.datap, topicp->name.length);
        if (tcpstp->err == UROS_OK) { continue; }
        else { goto _error; }
      } else {
        /* service={str} */
        strp = &topicp->name;
        tcpstp->remoteFlags.service = UROS_TRUE;
        vallen -= 8;
      }
    } else if (_GOT("topic=", 6)) {
      if (!isrequest) {
        /* Check if it matches the referenced one.*/
        urosTcpRosExpect(tcpstp, topicp->name.datap, topicp->name.length);
        if (tcpstp->err == UROS_OK) { continue; }
        else { goto _error; }
      } else {
        /* topic={str} */
        strp = &topicp->name;
        tcpstp->remoteFlags.service = UROS_FALSE;
        vallen -= 6;
      }
    } else if (_GOT("type=", 5)) {
      /* type={str} */
      strp = &typep->name;
      vallen -= 5;
      if (!isrequest) {
        /* Check if it matches the referenced one.*/
        if (vallen != strp->length) { _ERRPARSE }
        urosTcpRosExpect(tcpstp, strp->datap, strp->length);
        if (tcpstp->err == UROS_OK) { continue; }
        else { goto _error; }
      }
    } else if (_GOT("tcp_nodelay=", 12)) {
      if (!isrequest) { _ERRPARSE }
      /* tcp_nodelay=(0|1) */
      if (vallen == 13) {
        urosTcpRosRecv(tcpstp, buf, 1);
        if (buf[0] == '1') {
          tcpstp->remoteFlags.noDelay = UROS_TRUE; continue;
        } else if (buf[0] == '0') {
          tcpstp->remoteFlags.noDelay = UROS_FALSE; continue;
        } else { _ERRPARSE }
      } else { _ERRPARSE }
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

  if (isrequest) {
    /* Temporarily save the remote topic descriptor inside the TCPROS status.*/
    topicp->flags =  tcpstp->remoteFlags;
    tcpstp->topicp = topicp;
  }
  return tcpstp->err = UROS_OK;

_error:
  if (isrequest) {
    urosMsgTypeDelete(typep);
    urosTopicDelete(topicp);
  }
  return tcpstp->err;
#undef _ERRPARSE
#undef _CHKOK
#undef _GOT
}

/**
 * @brief   Executes a service call.
 * @details Gets the service URI from the Master node. If found, it executes
 *          the service call once, and the result is returned.
 * @note    Only a @e single call will be executed. Persistent TCPROS service
 *          connections need custom handlers.
 *
 * @pre     @p servicep->procf must address a @p uros_tcpsrvcall_t function.
 * @pre     The TPCORS @p service flag must be set, @p persistent clear.
 *
 * @param[in] pubaddrp
 *          Pointer to the service publisher address.
 * @param[in] servicep
 *          Pointer to the service descriptor.
 * @param[out] resobjp
 *          Pointer to the allocated response object. The service result will
 *          be written there only if the call is successful.
 * @return
 *          Error code.
 */
uros_err_t urosTcpRosCallService(const UrosAddr *pubaddrp,
                                 const UrosTopic *servicep,
                                 void *resobjp) {

  UrosConn conn;
  UrosTcpRosStatus tcpst;

  urosAssert(pubaddrp != NULL);
  urosAssert(servicep != NULL);
  urosAssert(urosStringNotEmpty(&servicep->name));
  urosAssert(servicep->typep != NULL);
  urosAssert(urosStringNotEmpty(&servicep->typep->name));
  urosAssert(servicep->procf != NULL);
  urosAssert(servicep->flags.service);
  urosAssert(!servicep->flags.persistent);
  urosAssert(resobjp != NULL);
#define _CHKOK  { if (tcpst.err != UROS_OK) { goto _error; } }

  /* Connect to the remote service host.*/
  urosConnObjectInit(&conn);
  urosTcpRosStatusObjectInit(&tcpst, &conn);
  tcpst.err = urosConnCreate(&conn, UROS_PROTO_TCP); _CHKOK
  tcpst.err = urosConnConnect(&conn, pubaddrp); _CHKOK

  /* Send the TCPROS connection header, and its response.*/
  urosTcpRosSendHeader(&tcpst, UROS_TRUE); _CHKOK
  urosTcpRosRecvHeader(&tcpst, UROS_FALSE, UROS_FALSE); _CHKOK

  /* Call the service handler.*/
  tcpst.err = ((uros_tcpsrvcall_t)servicep->procf)(&tcpst, resobjp); _CHKOK
  return urosConnClose(&conn);

_error:
  urosConnClose(&conn);
  return tcpst.err;
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

  static UrosNodeStatus *const stp = &urosNode.status;

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
    urosError(err != UROS_OK, UROS_NOP,
              ("Error %s while accepting an incoming TCPROS connection "
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
    err = urosConnSetRecvTimeout(spawnedp, UROS_TCPROS_RECVTIMEOUT);
    urosAssert(err == UROS_OK);
    err = urosConnSetSendTimeout(spawnedp, UROS_TCPROS_SENDTIMEOUT);
    urosAssert(err == UROS_OK);

    /* Create the TCPROS Server worker thread.*/
    err = urosThreadPoolStartWorker(&stp->tcpsvrThdPool, (void*)spawnedp);

    /* Check if anything went wrong.*/
    if (err != UROS_OK && urosConnIsValid(spawnedp)) {
      urosConnClose(spawnedp);
      urosFree(spawnedp);
    }
  }

  /* Close the listening connection.*/
  urosConnClose(&conn);

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

  static UrosNodeStatus *const stp = &urosNode.status;

  UrosTcpRosStatus *tcpstp;
  uros_err_t err;
  uros_proc_f handler = NULL;

  urosAssert(csp != NULL);

  tcpstp = urosNew(UrosTcpRosStatus);
  if (tcpstp == NULL) { return UROS_ERR_NOMEM; }
  urosTcpRosStatusObjectInit(tcpstp, csp);

  /* Receive the connection header.*/
  err = urosTcpRosRecvHeader(tcpstp, UROS_TRUE, UROS_TRUE);
  if (err != UROS_OK) {
    /* Send the error message.*/
    tcpstp->errstr = urosStringCloneZ(urosErrorText(err));
    urosTcpRosSendError(tcpstp);
    urosStringClean(&tcpstp->errstr);
    goto _finally;
  }

  /* Process the received header, to link to the actual topic/service.*/
  if (tcpstp->topicp->flags.service) {
    /* TCPROS service connection header.*/
    urosMutexLock(&stp->pubServiceListLock);
    err = uros_tcpserver_processserviceheader(tcpstp);
    if (err == UROS_OK) {
      handler = tcpstp->topicp->procf;
    }
    urosMutexUnlock(&stp->pubServiceListLock);
  } else {
    /* TCPROS topic connection header.*/
    urosMutexLock(&stp->pubTopicListLock);
    err = uros_tcpserver_processtopicheader(tcpstp);
    if (err == UROS_OK) {
      handler = tcpstp->topicp->procf;
    }
    urosMutexUnlock(&stp->pubTopicListLock);
  }
  if (err != UROS_OK) {
    /* Send an error message.*/
    tcpstp->errstr = urosStringCloneZ(urosErrorText(err));
    urosTcpRosSendError(tcpstp);
    goto _finally;
  }

  /* Send the response header.*/
  err = urosTcpRosSendHeader(tcpstp, UROS_FALSE);
  if (err != UROS_OK) { goto _finally; }

  /* Call the connection handler.*/
  tcpstp->err = UROS_OK;
  urosAssert(handler != NULL);
  err = handler(tcpstp);

  /* Release the topic/service descriptor reference.*/
  if (tcpstp->topicp->flags.service) {
    urosTcpRosServiceDone(tcpstp);
  } else {
    urosTcpRosTopicPublisherDone(tcpstp);
  }

_finally:
  /* Close the connection and free the allocated memory.*/
  urosConnClose(csp);
  urosTcpRosStatusDelete(tcpstp, UROS_TRUE);
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
  urosAssert(!argsp->topicFlags.service);

  /* Resolve the publisher address.*/
  err = urosNodeResolveTopicPublisher(&argsp->remoteAddr,
                                      &argsp->topicName,
                                      &pubaddr);
  if (err != UROS_OK) { goto _finally; }

  /* Start a new TCPROS topic subscripion.*/
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

  static UrosNodeStatus *const stp = &urosNode.status;

  urosAssert(tcpstp != NULL);
  urosAssert(tcpstp->topicp != NULL);

  /* Decrement the topic reference count.*/
  urosMutexLock(&stp->subTopicListLock);
  if (0 == urosTopicRefDec(tcpstp->topicp)) {
    /* The topic has been deleted an this is its last reference, free it.*/
    if (tcpstp->topicp->flags.deleted) {
      urosTopicDelete(tcpstp->topicp);
      tcpstp->topicp = NULL;
    }
  }
  urosMutexUnlock(&stp->subTopicListLock);
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

  static UrosNodeStatus *const stp = &urosNode.status;

  urosAssert(tcpstp != NULL);
  urosAssert(tcpstp->topicp != NULL);

  /* Decrement the topic reference count.*/
  urosMutexLock(&stp->pubTopicListLock);
  if (0 == urosTopicRefDec(tcpstp->topicp)) {
    /* The topic has been deleted an this is its last reference, free it.*/
    if (tcpstp->topicp->flags.deleted) {
      urosTopicDelete(tcpstp->topicp);
      tcpstp->topicp = NULL;
    }
  }
  urosMutexUnlock(&stp->pubTopicListLock);
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

  static UrosNodeStatus *const stp = &urosNode.status;

  urosAssert(tcpstp != NULL);
  urosAssert(tcpstp->topicp != NULL);

  /* Decrement the service reference count.*/
  urosMutexLock(&stp->pubServiceListLock);
  if (0 == urosTopicRefDec(tcpstp->topicp)) {
    /* The service has been deleted an this is its last reference, free it.*/
    if (tcpstp->topicp->flags.deleted) {
      urosTopicDelete(tcpstp->topicp);
      tcpstp->topicp = NULL;
    }
  }
  urosMutexUnlock(&stp->pubServiceListLock);
}

/** @} */
