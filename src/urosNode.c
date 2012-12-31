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
 * @file    urosNode.c
 * @author  Andrea Zoppi <texzk@email.it>
 *
 * @brief   Node features of the middleware.
 */

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include "../include/urosBase.h"
#include "../include/urosUser.h"
#include "../include/urosNode.h"
#include "../include/urosRpcCall.h"
#include "../include/urosRpcParser.h"
#include "../include/lld/uros_lld_node.h"

#include <string.h>

/*===========================================================================*/
/* LOCAL TYPES & MACROS                                                      */
/*===========================================================================*/

#if UROS_NODE_C_USE_ASSERT == UROS_FALSE && !defined(__DOXYGEN__)
#undef urosAssert
#define urosAssert(expr)
#endif

/*===========================================================================*/
/* LOCAL VARIABLES                                                           */
/*===========================================================================*/

/** @brief Node thread stack.*/
static UROS_STACK(urosNodeThreadStack, UROS_NODE_THREAD_STKSIZE);

/** @brief XMLRPC Listener thread stack.*/
static UROS_STACK(xmlrpcListenerStack, UROS_XMLRPC_LISTENER_STKSIZE);

/** @brief TCPROS Listener thread stack.*/
static UROS_STACK(tcprosListenerStack, UROS_TCPROS_LISTENER_STKSIZE);

/** @brief XMLRPC Slave server worker thread stacks.*/
static UROS_STACKPOOL(slaveMemPoolChunk, UROS_XMLRPC_SLAVE_STKSIZE,
                      UROS_XMLRPC_SLAVE_POOLSIZE);

/** @brief TCPROS Client worker thread stacks.*/
static UROS_STACKPOOL(tcpcliMemPoolChunk, UROS_TCPROS_CLIENT_STKSIZE,
                      UROS_TCPROS_CLIENT_POOLSIZE);

/** @brief TCPROS Server worker thread stacks.*/
static UROS_STACKPOOL(tcpsvrMemPoolChunk, UROS_TCPROS_SERVER_STKSIZE,
                      UROS_TCPROS_SERVER_POOLSIZE);

/*===========================================================================*/
/* GLOBAL VARIABLES                                                          */
/*===========================================================================*/

/**
 * @brief   Node singleton.
 */
UrosNode urosNode;

/*===========================================================================*/
/* LOCAL FUNCTIONS                                                           */
/*===========================================================================*/

void uros_node_createthreads(void) {

  static UrosNodeStatus *const stp = &urosNode.status;

  uros_err_t err;
  (void)err;

  urosAssert(stp->xmlrpcListenerId == UROS_NULL_THREADID);
  urosAssert(stp->tcprosListenerId == UROS_NULL_THREADID);

  /* Fill the worker thread pools.*/
  err = urosThreadPoolCreateAll(&stp->tcpcliThdPool);
  urosAssert(err == UROS_OK);
  err = urosThreadPoolCreateAll(&stp->tcpsvrThdPool);
  urosAssert(err == UROS_OK);
  err = urosThreadPoolCreateAll(&stp->slaveThdPool);
  urosAssert(err == UROS_OK);

  /* Spawn the XMLRPC Slave listener threads.*/
  err = urosThreadCreateStatic(&stp->xmlrpcListenerId,
                               "RpcSlaveListener",
                               UROS_XMLRPC_LISTENER_PRIO,
                               (uros_proc_f)urosRpcSlaveListenerThread, NULL,
                               xmlrpcListenerStack,
                               UROS_XMLRPC_LISTENER_STKSIZE);
  urosAssert(err == UROS_OK);

  /* Spawn the TCPROS listener thread.*/
  err = urosThreadCreateStatic(&stp->tcprosListenerId,
                               "TcpRosListener",
                               UROS_TCPROS_LISTENER_PRIO,
                               (uros_proc_f)urosTcpRosListenerThread, NULL,
                               tcprosListenerStack,
                               UROS_TCPROS_LISTENER_STKSIZE);
  urosAssert(err == UROS_OK);
}

void uros_node_jointhreads(void) {

  static const UrosNodeConfig *const cfgp = &urosNode.config;
  static UrosNodeStatus *const stp = &urosNode.status;

  UrosConn conn;
  uros_err_t err;
  (void)err;

  urosAssert(stp->xmlrpcListenerId != UROS_NULL_THREADID);
  urosAssert(stp->tcprosListenerId != UROS_NULL_THREADID);

  /* Join the XMLRPC Slave listener threads.*/
  urosConnObjectInit(&conn);
  urosConnCreate(&conn, UROS_PROTO_TCP);
  urosConnConnect(&conn, &cfgp->xmlrpcAddr);
  urosConnClose(&conn);
  err = urosThreadJoin(stp->xmlrpcListenerId);
  urosAssert(err == UROS_OK);
  stp->xmlrpcListenerId = UROS_NULL_THREADID;

  /* Join the TCPROS listener thread.*/
  urosConnObjectInit(&conn);
  urosConnCreate(&conn, UROS_PROTO_TCP);
  urosConnConnect(&conn, &cfgp->tcprosAddr);
  urosConnClose(&conn);
  err = urosThreadJoin(stp->tcprosListenerId);
  urosAssert(err == UROS_OK);
  stp->tcprosListenerId = UROS_NULL_THREADID;

  /* Join the worker thread pools.*/
  err = urosThreadPoolJoinAll(&stp->tcpcliThdPool);
  urosAssert(err == UROS_OK);
  err = urosThreadPoolJoinAll(&stp->tcpsvrThdPool);
  urosAssert(err == UROS_OK);
  err = urosThreadPoolJoinAll(&stp->slaveThdPool);
  urosAssert(err == UROS_OK);
}

uros_err_t uros_node_pollmaster(void) {

  static const UrosNodeConfig *const cfgp = &urosNode.config;

  UrosRpcResponse res;
  uros_err_t err;

  /* Check if the Master can reply to a getPid() request.*/
  urosRpcResponseObjectInit(&res);
  err = urosRpcCallGetPid(
    &cfgp->masterAddr,
    &cfgp->xmlrpcUri,
    &res
  );
  urosRpcResponseClean(&res);
  return err;
}

void uros_node_registerall(void) {

  /* Register topics.*/
  urosUserPublishTopics();
  urosUserSubscribeTopics();

  /* Register services.*/
  urosUserPublishServices();

  /* Register parameters.*/
  urosUserSubscribeParams();
}

void uros_node_unregisterall(void) {

  /* Unregister topics.*/
  urosUserUnpublishTopics();
  urosUserUnsubscribeTopics();

  /* Unregister services.*/
  urosUserUnpublishServices();

  /* Unregister parameters.*/
  urosUserUnsubscribeParams();
}

/*===========================================================================*/
/* GLOBAL FUNCTIONS                                                          */
/*===========================================================================*/

/** @addtogroup node_funcs */
/** @{ */

/**
 * @brief   Initializes a node object.
 * @details This procedure:
 *          - loads a nullified configuration
 *          - initializes the status lists
 *          - initializes the status locks
 *          - initializes and allocates memory pools
 *          - initializes thread pools
 *
 * @pre     The node has not been initialized yet.
 *
 * @param[in,out] np
 *          Pointer to the @p UrosNode to be initialized.
 */
void urosNodeObjectInit(UrosNode *np) {

  UrosNodeStatus *stp;

  urosAssert(np != NULL);

  /* Invalidate configuration variables.*/
  memset((void*)&np->config, 0, sizeof(UrosNodeConfig));

  /* Initialize status variables.*/
  stp = &np->status;
  stp->xmlrpcPid = ~0;
  urosListObjectInit(&stp->subTopicList);
  urosListObjectInit(&stp->pubTopicList);
  urosListObjectInit(&stp->pubServiceList);
  urosListObjectInit(&stp->subParamList);
  urosListObjectInit(&stp->subTcpList);
  urosListObjectInit(&stp->pubTcpList);
  stp->xmlrpcListenerId = UROS_NULL_THREADID;
  stp->tcprosListenerId = UROS_NULL_THREADID;

  urosMutexObjectInit(&stp->xmlrpcPidLock);
  urosMutexObjectInit(&stp->subTopicListLock);
  urosMutexObjectInit(&stp->pubTopicListLock);
  urosMutexObjectInit(&stp->pubServiceListLock);
  urosMutexObjectInit(&stp->subParamListLock);
  urosMutexObjectInit(&stp->subTcpListLock);
  urosMutexObjectInit(&stp->pubTcpListLock);
  stp->exitFlag = UROS_FALSE;
  urosMutexObjectInit(&stp->exitLock);

  /* Initialize mempools with their description.*/
  urosMemPoolObjectInit(&stp->slaveMemPool,
                        UROS_STACKPOOL_BLKSIZE(UROS_XMLRPC_SLAVE_STKSIZE),
                        NULL);
  urosMemPoolObjectInit(&stp->tcpcliMemPool,
                        UROS_STACKPOOL_BLKSIZE(UROS_TCPROS_CLIENT_STKSIZE),
                        NULL);
  urosMemPoolObjectInit(&stp->tcpsvrMemPool,
                        UROS_STACKPOOL_BLKSIZE(UROS_TCPROS_SERVER_STKSIZE),
                        NULL);

  /* Load the actual memory chunks for worker thread stacks.*/
  urosMemPoolLoadArray(&stp->slaveMemPool, slaveMemPoolChunk,
                       UROS_XMLRPC_SLAVE_POOLSIZE);
  urosMemPoolLoadArray(&stp->tcpcliMemPool, tcpcliMemPoolChunk,
                       UROS_TCPROS_CLIENT_POOLSIZE);
  urosMemPoolLoadArray(&stp->tcpsvrMemPool, tcpsvrMemPoolChunk,
                       UROS_TCPROS_SERVER_POOLSIZE);

  /* Initialize thread pools.*/
  urosThreadPoolObjectInit(&stp->tcpcliThdPool, &stp->tcpcliMemPool,
                           (uros_proc_f)urosTcpRosClientThread,
                           "TcpRosClient",
                           UROS_TCPROS_CLIENT_PRIO);

  urosThreadPoolObjectInit(&stp->tcpsvrThdPool, &stp->tcpsvrMemPool,
                           (uros_proc_f)urosTcpRosServerThread,
                           "TcpRosServer",
                           UROS_TCPROS_SERVER_PRIO);

  urosThreadPoolObjectInit(&stp->slaveThdPool, &stp->slaveMemPool,
                           (uros_proc_f)urosRpcSlaveServerThread,
                           "RpcSlaveServer",
                           UROS_XMLRPC_SLAVE_PRIO);
}

/**
 * @brief   Creates the main Node thread.
 * @note    Should be called at system startup, after @p urosInit().
 *
 * @return
 *          Error code.
 */
uros_err_t urosNodeCreateThread(void) {

  return urosThreadCreateStatic(
    &urosNode.status.nodeThreadId, "urosNode",
    UROS_NODE_THREAD_PRIO,
    (uros_proc_f)urosNodeThread, NULL,
    urosNodeThreadStack, UROS_NODE_THREAD_STKSIZE
  );
}

/**
 * @brief   Node thread.
 * @details This thread handles the whole life of the local node, including
 *          (un)registration (from)to the Master, thread pool magnagement,
 *          node shutdown, and so on.
 *
 * @param[in] argp
 *          Ignored.
 * @return
 *          Error code.
 */
uros_err_t urosNodeThread(void *argp) {

  static UrosNodeStatus *const stp = &urosNode.status;

  uros_bool_t exitFlag;
  (void)argp;

  /* Create the listener and pool threads.*/
  uros_node_createthreads();

  urosMutexLock(&stp->exitLock);
  exitFlag = stp->exitFlag;
  urosMutexUnlock(&stp->exitLock);
  while (!exitFlag) {
    /* Check if the Master is alive.*/
    if (uros_node_pollmaster() != UROS_OK) {
      /* Add a delay not to flood in case of short timeouts.*/
      urosThreadSleepSec(3);
      urosMutexLock(&stp->exitLock);
      exitFlag = stp->exitFlag;
      urosMutexUnlock(&stp->exitLock);
      continue;
    }

    /* Register to the Master.*/
    uros_node_registerall();

    /* Check if the Master is alive every 3 seconds.*/
    urosMutexLock(&stp->exitLock);
    exitFlag = stp->exitFlag;
    urosMutexUnlock(&stp->exitLock);
    while (!exitFlag) {
      urosError(uros_node_pollmaster() != UROS_OK, break,
                ("Master node "UROS_IPFMT" lost\n",
                 UROS_IPARG(&urosNode.config.masterAddr.ip)));

      urosThreadSleepSec(3);
      urosMutexLock(&stp->exitLock);
      exitFlag = stp->exitFlag;
      urosMutexUnlock(&stp->exitLock);
    }

    /* Unregister from the Master*/
    uros_node_unregisterall();

    urosMutexLock(&stp->exitLock);
    exitFlag = stp->exitFlag;
    urosMutexUnlock(&stp->exitLock);
  }

  /* Join listener and pool threads.*/
  uros_node_jointhreads();

  return UROS_OK;
}

/**
 * @brief   Loads the default node configuration.
 * @details Any previously allocated data is freed, then the default node
 *          configuration is loaded.
 *
 * @pre     The related @p UrosNode is initialized.
 *
 * @param[in,out] cfgp
 *          Pointer to the target configuration descriptor.
 */
void urosNodeConfigLoadDefaults(UrosNodeConfig *cfgp) {

  urosAssert(cfgp != NULL);

  /* Deallocate any previously allocated data.*/
  urosStringClean(&cfgp->nodeName);
  urosStringClean(&cfgp->xmlrpcUri);
  urosStringClean(&cfgp->tcprosUri);
  urosStringClean(&cfgp->masterUri);

  /* Load default values.*/
  cfgp->nodeName = urosStringCloneZ("/turtlesim");
  cfgp->xmlrpcAddr.ip.dword = UROS_XMLRPC_LISTENER_IP;
  cfgp->xmlrpcAddr.port = UROS_XMLRPC_LISTENER_PORT;
  cfgp->xmlrpcUri = urosStringCloneZ(
    "http://"UROS_XMLRPC_LISTENER_IP_SZ
    ":"UROS_STRINGIFY2(UROS_XMLRPC_LISTENER_PORT));
  cfgp->tcprosAddr.ip.dword = UROS_TCPROS_LISTENER_IP;
  cfgp->tcprosAddr.port = UROS_TCPROS_LISTENER_PORT;
  cfgp->tcprosUri = urosStringCloneZ(
    "rosrpc://"UROS_TCPROS_LISTENER_IP_SZ
    ":"UROS_STRINGIFY2(UROS_TCPROS_LISTENER_PORT));
  cfgp->masterAddr.ip.dword = UROS_XMLRPC_MASTER_IP;
  cfgp->masterAddr.port = UROS_XMLRPC_MASTER_PORT;
  cfgp->masterUri = urosStringCloneZ(
    "http://"UROS_XMLRPC_MASTER_IP_SZ
    ":"UROS_STRINGIFY2(UROS_XMLRPC_MASTER_PORT));
}

/**
 * @brief   Loads node configuration.
 * @details Any previously allocated data is freed, then the configuration is
 *          loaded from a static non-volatile memory chunk.
 * @see     uros_lld_nodeconfig_load()
 *
 * @pre     The related @p UrosNode is initialized.
 *
 * @param[in,out] cfgp
 *          Pointer to the target configuration descriptor.
 */
void urosNodeConfigLoad(UrosNodeConfig *cfgp) {

  uros_lld_nodeconfig_load(cfgp);
}

/**
 * @brief   Saves the node configuration.
 * @details The node configuration is saved to a static non-volatile memory
 *          chunk.
 * @see     uros_lld_nodeconfig_save()
 *
 * @pre     The related @p UrosNode is initialized.
 *
 * @param[in] cfgp
 *          Pointer to the configuration descriptor to be saved.
 */
void urosNodeConfigSave(const UrosNodeConfig *cfgp) {

  uros_lld_nodeconfig_save(cfgp);
}

/**
 * @brief   Publishes a topic.
 * @details Issues a @p publishTopic() call to the XMLRPC Master.
 * @see     urosRpcCallRegisterPublisher()
 * @see     urosNodePublishTopicByDesc()
 * @warning The access to the topic registry is thread-safe, but delays of the
 *          XMLRPC communication will delay also any other threads trying to
 *          publish/unpublish any topics.
 *
 * @pre     The topic is not published.
 *
 * @param[in] namep
 *          Pointer to the topic name string.
 * @param[in] typep
 *          Pointer to the topic message type name string.
 * @param[in] procf
 *          Topic handler function.
 * @return
 *          Error code.
 */
uros_err_t urosNodePublishTopic(const UrosString *namep,
                                const UrosString *typep,
                                uros_proc_f procf) {

  static UrosNode *const np = &urosNode;

  UrosTopic *topicp;
  const UrosMsgType *statictypep;
  UrosListNode *topicnodep;
  uros_err_t err;

  urosAssert(urosStringNotEmpty(namep));
  urosAssert(urosStringNotEmpty(typep));
  urosAssert(procf != NULL);

  /* Get the registered message type.*/
  statictypep = urosFindStaticMsgType(typep);
  urosError(statictypep == NULL, return UROS_ERR_BADPARAM,
            ("Unknown message type [%.*s]\n", UROS_STRARG(typep)));

  /* Check if the topic already exists.*/
  urosMutexLock(&np->status.pubTopicListLock);
  topicnodep = urosTopicListFindByName(&np->status.pubTopicList, namep);
  urosMutexUnlock(&np->status.pubTopicListLock);
  urosError(topicnodep != NULL, return UROS_ERR_BADPARAM,
            ("Topic [%.*s] already published\n", UROS_STRARG(namep)));

  /* Create a new topic descriptor.*/
  topicp = urosNew(UrosTopic);
  if (topicp == NULL) { return UROS_ERR_NOMEM; }
  urosTopicObjectInit(topicp);
  topicp->name = urosStringClone(namep);
  topicp->typep = statictypep;
  topicp->procf = procf;

  /* Publish the topic.*/
  err = urosNodePublishTopicByDesc(topicp);
  if (err != UROS_OK) { urosTopicDelete(topicp); }
  return err;
}

/**
 * @brief   Publishes a topic.
 * @details Issues a @p publishTopic() call to the XMLRPC Master.
 * @see     urosRpcCallRegisterPublisher()
 * @see     urosNodePublishTopicByDesc()
 * @warning The access to the topic registry is thread-safe, but delays of the
 *          XMLRPC communication will delay also any other threads trying to
 *          publish/unpublish any topics.
 *
 * @pre     The topic is not published.
 *
 * @param[in] namep
 *          Pointer to the topic name null-terminated string.
 * @param[in] typep
 *          Pointer to the topic message type name null-terminated string.
 * @param[in] procf
 *          Topic handler function.
 * @return
 *          Error code.
 */
uros_err_t urosNodePublishTopicSZ(const char *namep,
                                  const char *typep,
                                  uros_proc_f procf) {

  UrosString namestr, typestr;

  urosAssert(namep != NULL);
  urosAssert(namep[0] != 0);
  urosAssert(typep != NULL);
  urosAssert(typep[0] != 0);
  urosAssert(procf != NULL);

  namestr = urosStringAssignZ(namep);
  typestr = urosStringAssignZ(typep);
  return urosNodePublishTopic(&namestr, &typestr, procf);
}

/**
 * @brief   Publishes a topic by its descriptor.
 * @details Issues a @p publishTopic() call to the XMLRPC Master.
 * @see     urosRpcCallRegisterPublisher()
 * @warning The access to the topic registry is thread-safe, but delays of the
 *          XMLRPC communication will delay also any other threads trying to
 *          publish/unpublish any topics.
 *
 * @pre     The topic is not published.
 * @pre     The topic descriptor must have the @p service flag set to @p 0.
 * @post    - If successful, the topic descriptor is referenced by the topic
 *            registry, and is no longer modifiable by the caller function.
 *          - If unsuccessful, the topic descriptor can be deallocated by the
 *            caller function.
 *
 * @param[in] topicp
 *          Pointer to the topic descriptor to be published and registered.
 * @return
 *          Error code.
 */
uros_err_t urosNodePublishTopicByDesc(UrosTopic *topicp) {

  static UrosNode *const np = &urosNode;

  UrosRpcResponse response;
  uros_err_t err;
  UrosListNode *nodep;

  urosAssert(topicp != NULL);
  urosAssert(urosStringNotEmpty(&topicp->name));
  urosAssert(topicp->typep != NULL);
  urosAssert(urosStringNotEmpty(&topicp->typep->name));
  urosAssert(topicp->refcnt == 0);

  urosRpcResponseObjectInit(&response);
  urosMutexLock(&np->status.pubTopicListLock);

  /* Master XMLRPC registerPublisher() */
  err = urosRpcCallRegisterPublisher(
    &np->config.masterAddr,
    &np->config.nodeName,
    &topicp->name,
    &topicp->typep->name,
    &np->config.xmlrpcUri,
    &response
  );
  urosError(err != UROS_OK, goto _finally,
            ("Error %s while registering as publisher of topic [%.*s]\n",
             urosErrorText(err), UROS_STRARG(&topicp->name)));

  /* Check for valid codes.*/
  urosError(response.code != UROS_RPCC_SUCCESS,
            { err = UROS_ERR_BADPARAM; goto _finally; },
            ("Response code %d, expected %d\n",
             response.code, UROS_RPCC_SUCCESS));
  urosError(response.httpcode != 200,
            { err = UROS_ERR_BADPARAM; goto _finally; },
            ("Response HTTP code %d, expected 200\n", response.httpcode));

  /* Add to the published topics list.*/
  nodep = urosNew(UrosListNode);
  if (nodep == NULL) { err = UROS_ERR_NOMEM; goto _finally; }
  urosListNodeObjectInit(nodep);
  nodep->datap = (void*)topicp;
  urosListAdd(&np->status.pubTopicList, nodep);

  err = UROS_OK;
_finally:
  /* Cleanup and return.*/
  urosMutexUnlock(&np->status.pubTopicListLock);
  urosRpcResponseClean(&response);
  return err;
}

/**
 * @brief   Unpublishes a topic.
 * @details Issues an @p unpublishTopic() call to the XMLRPC Master.
 * @see     urosRpcCallUnregisterPublisher()
 * @warning The access to the topic registry is thread-safe, but delays of the
 *          XMLRPC communication will delay also any other threads trying to
 *          publish/unpublish any topics.
 *
 * @pre     The topic is published.
 * @post    If successful, the topic descriptor is dereferenced by the topic
 *          registry, and will be freed:
 *          - by this function, if there are no publishing TCPROS threads, or
 *          - by the last publishing TCPROS thread which references the topic.
 *
 * @param[in] namep
 *          Pointer to a string which names the topic.
 * @return
 *          Error code.
 */
uros_err_t urosNodeUnpublishTopic(const UrosString *namep) {

  static UrosNode *const np = &urosNode;

  UrosListNode *tcprosnodep, *topicnodep;
  UrosTopic *topicp;
  uros_err_t err;
  UrosRpcResponse res;

  urosAssert(urosStringNotEmpty(namep));

  /* Find the topic descriptor.*/
  urosMutexLock(&np->status.pubTopicListLock);
  topicnodep = urosTopicListFindByName(&np->status.pubTopicList, namep);
  if (topicnodep == NULL) {
    urosError(topicnodep == NULL,
              { err = UROS_ERR_BADPARAM; goto _finally; },
              ("Topic [%.*s] not published\n", UROS_STRARG(namep)));
  }
  topicp = (UrosTopic*)topicnodep->datap;

  /* Unregister the topic on the Master node.*/
  err = urosRpcCallUnregisterPublisher(
    &np->config.masterAddr,
    &np->config.nodeName,
    namep,
    &np->config.xmlrpcUri,
    &res
  );
  urosError(err != UROS_OK, UROS_NOP,
            ("Error %s while unregistering as publisher of topic [%.*s]\n",
             urosErrorText(err), UROS_STRARG(namep)));

  /* Unregister the topic locally.*/
  topicp->flags.deleted = UROS_TRUE;
  tcprosnodep = urosListRemove(&np->status.pubTopicList, topicnodep);
  urosAssert(tcprosnodep == topicnodep);

  if (topicp->refcnt > 0) {
    /* Tell each publishing TCPROS thread to exit.*/
    urosMutexLock(&np->status.pubTcpListLock);
    for (tcprosnodep = np->status.pubTcpList.headp;
         tcprosnodep != NULL;
         tcprosnodep = tcprosnodep->nextp) {

      UrosTcpRosStatus *tcpstp = (UrosTcpRosStatus*)tcprosnodep->datap;
      if (tcpstp->topicp == topicp && !tcpstp->flags.service) {
        urosMutexLock(&tcpstp->threadExitMtx);
        tcpstp->threadExit = UROS_TRUE;
        urosMutexUnlock(&tcpstp->threadExitMtx);
      }
    }
    urosMutexUnlock(&np->status.pubTcpListLock);
    /* NOTE: The last exiting thread freeds the topic descriptor.*/
  } else {
    /* No TCPROS connections, just free the descriptor immediately.*/
    urosListNodeDelete(topicnodep, (uros_delete_f)urosTopicDelete);
  }

_finally:
  urosMutexUnlock(&np->status.pubTopicListLock);
  return err;
}

/**
 * @brief   Unpublishes a topic.
 * @details Issues an @p unpublishTopic() call to the XMLRPC Master.
 * @see     urosRpcCallUnregisterPublisher()
 * @warning The access to the topic registry is thread-safe, but delays of the
 *          XMLRPC communication will delay also any other threads trying to
 *          publish/unpublish any topics.
 *
 * @pre     The topic is published.
 * @post    If successful, the topic descriptor is dereferenced by the topic
 *          registry, and will be freed:
 *          - by this function, if there are no publishing TCPROS threads, or
 *          - by the last publishing TCPROS thread which references the topic.
 *
 * @param[in] namep
 *          Pointer to a null-terminated string which names the topic.
 * @return
 *          Error code.
 */
uros_err_t urosNodeUnpublishTopicSZ(const char *namep) {

  UrosString namestr;

  urosAssert(namep != NULL);
  urosAssert(namep[0] != 0);

  namestr = urosStringAssignZ(namep);
  return urosNodeUnpublishTopic(&namestr);
}

/**
 * @brief   Subscribes to a topic.
 * @details Issues a @p registerSubscriber() call to the XMLRPC Master, and
 *          connects to known publishers.
 * @see     urosNodeSubscribeTopicByDesc()
 * @see     urosRpcCallRegisterSubscriber()
 * @see     urosNodeFindNewPublishers()
 * @see     urosRpcSlaveConnectToPublishers()
 * @warning The access to the topic registry is thread-safe, but delays of the
 *          XMLRPC communication will delay also any other threads trying to
 *          subscribe/unsubscribe to any topics.
 *
 * @pre     The topic is not subscribed.
 * @post    Connects to known publishers listed by a successful response.
 *
 * @param[in] namep
 *          Pointer to the topic name string.
 * @param[in] typep
 *          Pointer to the topic message type name string.
 * @param[in] procf
 *          Topic handler function.
 * @return
 *          Error code.
 */
uros_err_t urosNodeSubscribeTopic(const UrosString *namep,
                                  const UrosString *typep,
                                  uros_proc_f procf) {

  static UrosNode *const np = &urosNode;

  UrosTopic *topicp;
  const UrosMsgType *statictypep;
  UrosListNode *topicnodep;
  uros_err_t err;

  urosAssert(urosStringNotEmpty(namep));
  urosAssert(urosStringNotEmpty(typep));
  urosAssert(procf != NULL);

  /* Get the registered message type.*/
  statictypep = urosFindStaticMsgType(typep);
  urosError(statictypep == NULL, return UROS_ERR_BADPARAM,
            ("Unknown message type [%.*s]\n", UROS_STRARG(typep)));

  /* Check if the topic already exists.*/
  urosMutexLock(&np->status.subTopicListLock);
  topicnodep = urosTopicListFindByName(&np->status.subTopicList, namep);
  urosMutexUnlock(&np->status.subTopicListLock);
  urosError(topicnodep != NULL, return UROS_ERR_BADPARAM,
            ("Topic [%.*s] already subscribed\n", UROS_STRARG(namep)));

  /* Create a new topic descriptor.*/
  topicp = urosNew(UrosTopic);
  if (topicp == NULL) { return UROS_ERR_NOMEM; }
  urosTopicObjectInit(topicp);
  topicp->name = urosStringClone(namep);
  topicp->typep = statictypep;
  topicp->procf = procf;

  /* Subscribe to the topic.*/
  err = urosNodeSubscribeTopicByDesc(topicp);
  if (err != UROS_OK) { urosTopicDelete(topicp); }
  return err;
}

/**
 * @brief   Subscribes to a topic.
 * @details Issues a @p registerSubscriber() call to the XMLRPC Master, and
 *          connects to known publishers.
 * @see     urosNodeSubscribeTopicByDesc()
 * @see     urosRpcCallRegisterSubscriber()
 * @see     urosNodeFindNewPublishers()
 * @see     urosRpcSlaveConnectToPublishers()
 * @warning The access to the topic registry is thread-safe, but delays of the
 *          XMLRPC communication will delay also any other threads trying to
 *          subscribe/unsubscribe to any topics.
 *
 * @pre     The topic is not subscribed.
 * @post    Connects to known publishers listed by a successful response.
 *
 * @param[in] namep
 *          Pointer to the topic name null-terminated string.
 * @param[in] typep
 *          Pointer to the topic message type name null-terminated string.
 * @param[in] procf
 *          Topic handler function.
 * @return
 *          Error code.
 */
uros_err_t urosNodeSubscribeTopicSZ(const char *namep,
                                    const char *typep,
                                    uros_proc_f procf) {

  UrosString namestr, typestr;

  urosAssert(namep != NULL);
  urosAssert(namep[0] != 0);
  urosAssert(typep != NULL);
  urosAssert(typep[0] != 0);
  urosAssert(procf != NULL);

  namestr = urosStringAssignZ(namep);
  typestr = urosStringAssignZ(typep);
  return urosNodeSubscribeTopic(&namestr, &typestr, procf);
}

/**
 * @brief   Subscribes to a topic by its descriptor.
 * @details Issues a @p registerSubscriber() call to the XMLRPC Master, and
 *          connects to known publishers.
 * @see     urosRpcCallRegisterSubscriber()
 * @see     urosNodeFindNewPublishers()
 * @see     urosRpcSlaveConnectToPublishers()
 * @warning The access to the topic registry is thread-safe, but delays of the
 *          XMLRPC communication will delay also any other threads trying to
 *          subscribe/unsubscribe to any topics.
 *
 * @pre     The topic is not subscribed.
 * @pre     The topic descriptor must have the @p service flag set to @p 0.
 * @post    - If successful, the topic descriptor is referenced by the topic
 *            registry, and is no longer modifiable by the caller function.
 *          - If unsuccessful, the topic descriptor can be deallocated by the
 *            caller function.
 * @post    Connects to known publishers listed by a successful response.
 *
 * @param[in] topicp
 *          Pointer to the topic descriptor to be subscribed to and registered.
 * @return
 *          Error code.
 */
uros_err_t urosNodeSubscribeTopicByDesc(UrosTopic *topicp) {

  static UrosNode *const np = &urosNode;

  UrosRpcResponse response;
  uros_err_t err;
  UrosList newpubs;
  UrosListNode *nodep;

  urosAssert(topicp != NULL);
  urosAssert(urosStringNotEmpty(&topicp->name));
  urosAssert(topicp->typep != NULL);
  urosAssert(urosStringNotEmpty(&topicp->typep->name));
  urosAssert(topicp->refcnt == 0);

  urosRpcResponseObjectInit(&response);
  urosListObjectInit(&newpubs);
  urosMutexLock(&np->status.subTopicListLock);

  /* Master XMLRPC registerSubscriber() */
  err = urosRpcCallRegisterSubscriber(
    &np->config.masterAddr,
    &np->config.nodeName,
    &topicp->name,
    &topicp->typep->name,
    &np->config.xmlrpcUri,
    &response
  );
  urosError(err != UROS_OK, goto _finally,
            ("Cannot register as subscriber of topic [%.*s]\n",
             UROS_STRARG(&topicp->name)));

  /* Check for valid codes.*/
  urosError(response.code != UROS_RPCC_SUCCESS,
            { err = UROS_ERR_BADPARAM; goto _finally; },
            ("Response code %d, expected %d\n",
             response.code, UROS_RPCC_SUCCESS));
  urosError(response.httpcode != 200,
            { err = UROS_ERR_BADPARAM; goto _finally; },
            ("Response HTTP code %d, expected 200\n", response.httpcode));

  /* Connect to registered publishers.*/
  err = urosNodeFindNewPublishers(&topicp->name, response.valuep, &newpubs);
  urosError(err != UROS_OK, goto _finally,
            ("Error %s while finding new publishers of topic [%.*s]\n",
             urosErrorText(err), UROS_STRARG(&topicp->name)));
  urosRpcResponseClean(&response);
  err = urosRpcSlaveConnectToPublishers(&topicp->name, &newpubs);
  urosError(err != UROS_OK, goto _finally,
            ("Error %s while connecting to new publishers of topic [%.*s]\n",
             urosErrorText(err), UROS_STRARG(&topicp->name)));

  /* Add to the subscribed topics list.*/
  nodep = urosNew(UrosListNode);
  if (nodep == NULL) { err = UROS_ERR_NOMEM; goto _finally; }
  urosListNodeObjectInit(nodep);
  nodep->datap = (void*)topicp;
  urosListAdd(&np->status.subTopicList, nodep);

  err = UROS_OK;
_finally:
  /* Cleanup and return.*/
  urosMutexUnlock(&np->status.subTopicListLock);
  urosListClean(&newpubs, (uros_delete_f)urosFree);
  urosRpcResponseClean(&response);
  return err;
}

/**
 * @brief   Unsubscribes to a topic.
 * @details Issues an @p unregisterSubscriber() call to the XMLRPC Master.
 * @see     urosRpcCallUnregisterSubscriber()
 * @warning The access to the topic registry is thread-safe, but delays of the
 *          XMLRPC communication will delay also any other threads trying to
 *          publish/unpublish any topics.
 *
 * @pre     The topic is published.
 * @post    If successful, the topic descriptor is dereferenced by the topic
 *          registry, and will be freed:
 *          - by this function, if there are no subscribing TCPROS threads, or
 *          - by the last subscribing TCPROS thread which references the topic.
 *
 * @param[in] namep
 *          Pointer to a string which names the topic.
 * @return
 *          Error code.
 */
uros_err_t urosNodeUnsubscribeTopic(const UrosString *namep) {

  static UrosNode *const np = &urosNode;

  UrosListNode *tcprosnodep, *topicnodep;
  UrosTopic *topicp;
  UrosTcpRosStatus *tcpstp;
  uros_err_t err;
  UrosRpcResponse res;

  urosAssert(urosStringNotEmpty(namep));

  /* Find the topic descriptor.*/
  urosMutexLock(&np->status.subTopicListLock);
  topicnodep = urosTopicListFindByName(&np->status.subTopicList, namep);
  if (topicnodep == NULL) {
    urosError(topicnodep == NULL,
              { err = UROS_ERR_BADPARAM; goto _finally; },
              ("Topic [%.*s] not subscribed\n", UROS_STRARG(namep)));
  }
  topicp = (UrosTopic*)topicnodep->datap;

  /* Unregister the topic on the Master node.*/
  err = urosRpcCallUnregisterSubscriber(
    &np->config.masterAddr,
    &np->config.nodeName,
    namep,
    &np->config.xmlrpcUri,
    &res
  );
  urosError(err != UROS_OK, UROS_NOP,
            ("Error %s while unregistering as subscriber of topic [%.*s]\n",
             urosErrorText(err), UROS_STRARG(namep)));

  /* Unregister the topic locally.*/
  topicp->flags.deleted = UROS_TRUE;
  tcprosnodep = urosListRemove(&np->status.subTopicList, topicnodep);
  urosAssert(tcprosnodep == topicnodep);

  if (topicp->refcnt > 0) {
    /* Tell each subscribing TCPROS thread to exit.*/
    urosMutexLock(&np->status.subTcpListLock);
    for (tcprosnodep = np->status.subTcpList.headp;
         tcprosnodep != NULL;
         tcprosnodep = tcprosnodep->nextp) {

      tcpstp = (UrosTcpRosStatus*)tcprosnodep->datap;
      if (tcpstp->topicp == topicp && !tcpstp->flags.service) {
        urosMutexLock(&tcpstp->threadExitMtx);
        tcpstp->threadExit = UROS_TRUE;
        urosMutexUnlock(&tcpstp->threadExitMtx);
      }
    }
    urosMutexUnlock(&np->status.subTcpListLock);
    /* NOTE: The last exiting thread freeds the topic descriptor.*/
  } else {
    /* No TCPROS connections, just free the descriptor immediately.*/
    urosListNodeDelete(topicnodep, (uros_delete_f)urosTopicDelete);
  }

_finally:
  urosMutexUnlock(&np->status.subTopicListLock);
  return err;
}

/**
 * @brief   Unsubscribes to a topic.
 * @details Issues an @p unregisterSubscriber() call to the XMLRPC Master.
 * @see     urosRpcCallUnregisterSubscriber()
 * @warning The access to the topic registry is thread-safe, but delays of the
 *          XMLRPC communication will delay also any other threads trying to
 *          publish/unpublish any topics.
 *
 * @pre     The topic is published.
 * @post    If successful, the topic descriptor is dereferenced by the topic
 *          registry, and will be freed:
 *          - by this function, if there are no subscribing TCPROS threads, or
 *          - by the last subscribing TCPROS thread which references the topic.
 *
 * @param[in] namep
 *          Pointer to a null-terminated string which names the topic.
 * @return
 *          Error code.
 */
uros_err_t urosNodeUnsubscribeTopicSZ(const char *namep) {

  UrosString namestr;

  urosAssert(namep != NULL);
  urosAssert(namep[0] != 0);

  namestr = urosStringAssignZ(namep);
  return urosNodeUnsubscribeTopic(&namestr);
}

/**
 * @brief   Publishes a service.
 * @details Issues a @p registerService() call to the XMLRPC Master.
 * @warning The access to the service registry is thread-safe, but delays of
 *          the XMLRPC communication will delay also any other threads trying
 *          to publish/unpublish any services.
 * @see     urosNodePublishServiceByDesc()
 * @see     urosRpcCallRegisterService()
 *
 * @pre     The service is not published.
 *
 * @param[in] namep
 *          Pointer to the service name string.
 * @param[in] typep
 *          Pointer to the service type name string.
 * @param[in] procf
 *          Service handler function.
 * @return
 *          Error code.
 */
uros_err_t urosNodePublishService(const UrosString *namep,
                                  const UrosString *typep,
                                  uros_proc_f procf) {

  static UrosNode *const np = &urosNode;

  UrosTopic *servicep;
  const UrosMsgType *statictypep;
  UrosListNode *servicenodep;
  uros_err_t err;

  urosAssert(urosStringNotEmpty(namep));
  urosAssert(urosStringNotEmpty(typep));
  urosAssert(procf != NULL);

  /* Get the registered service type.*/
  statictypep = urosFindStaticSrvType(typep);
  urosError(statictypep == NULL, return UROS_ERR_BADPARAM,
            ("Unknown message type [%.*s]\n", UROS_STRARG(typep)));

  /* Check if the service already exists.*/
  urosMutexLock(&np->status.pubServiceListLock);
  servicenodep = urosTopicListFindByName(&np->status.pubServiceList,
                                         namep);
  urosMutexUnlock(&np->status.pubServiceListLock);
  urosError(servicenodep != NULL, return UROS_ERR_BADPARAM,
            ("Service [%.*s] already published\n", UROS_STRARG(namep)));

  /* Create a new topic descriptor.*/
  servicep = urosNew(UrosTopic);
  if (servicep == NULL) { return UROS_ERR_NOMEM; }
  urosTopicObjectInit(servicep);
  servicep->name = urosStringClone(namep);
  servicep->typep = statictypep;
  servicep->procf = procf;
  servicep->flags.service = UROS_TRUE;

  /* Try to register the topic.*/
  err = urosNodePublishServiceByDesc(servicep);
  if (err != UROS_OK) { urosTopicDelete(servicep); }
  return err;
}

/**
 * @brief   Publishes a service.
 * @details Issues a @p registerService() call to the XMLRPC Master.
 * @warning The access to the service registry is thread-safe, but delays of
 *          the XMLRPC communication will delay also any other threads trying
 *          to publish/unpublish any services.
 * @see     urosNodePublishServiceByDesc()
 * @see     urosRpcCallRegisterService()
 *
 * @pre     The service is not published.
 *
 * @param[in] namep
 *          Pointer to the service name null-terminated string.
 * @param[in] typep
 *          Pointer to the service type name null-terminated string.
 * @param[in] procf
 *          Service handler function.
 * @return
 *          Error code.
 */
uros_err_t urosNodePublishServiceSZ(const char *namep,
                                    const char *typep,
                                    uros_proc_f procf) {

  UrosString namestr, typestr;

  urosAssert(namep != NULL);
  urosAssert(namep[0] != 0);
  urosAssert(typep != NULL);
  urosAssert(typep[0] != 0);
  urosAssert(procf != NULL);

  namestr = urosStringAssignZ(namep);
  typestr = urosStringAssignZ(typep);
  return urosNodePublishService(&namestr, &typestr, procf);
}

/**
 * @brief   Publishes a service by its descriptor.
 * @details Issues a @p registerService() call to the XMLRPC Master.
 * @warning The access to the service registry is thread-safe, but delays of
 *          the XMLRPC communication will delay also any other threads trying
 *          to publish/unpublish any services.
 * @see     urosRpcCallRegisterService()
 *
 * @pre     The service is not published.
 * @pre     The service descriptor must have the @p service flag set to @p 1.
 * @post    - If successful, the service descriptor is referenced by the
 *            service registry, and is no longer modifiable by the caller
 *            function.
 *          - If unsuccessful, the service descriptor can be deallocated by the
 *            caller function.
 *
 * @param[in] servicep
 *          Pointer to the service descriptor to be published and registered.
 * @return
 *          Error code.
 */
uros_err_t urosNodePublishServiceByDesc(const UrosTopic *servicep) {

  static UrosNode *const np = &urosNode;

  UrosRpcResponse response;
  uros_err_t err;
  UrosListNode *nodep;

  urosAssert(servicep != NULL);
  urosAssert(urosStringNotEmpty(&servicep->name));
  urosAssert(servicep->typep != NULL);
  urosAssert(urosStringNotEmpty(&servicep->typep->name));

  urosRpcResponseObjectInit(&response);
  urosMutexLock(&np->status.pubServiceListLock);

  /* Master XMLRPC registerPublisher() */
  err = urosRpcCallRegisterService(
    &np->config.masterAddr,
    &np->config.nodeName,
    &servicep->name,
    &np->config.tcprosUri,
    &np->config.xmlrpcUri,
    &response
  );
  urosError(err != UROS_OK, goto _finally,
            ("Cannot register service [%.*s]\n",
             UROS_STRARG(&servicep->name)));

  /* Check for valid codes.*/
  urosError(response.code != UROS_RPCC_SUCCESS,
            { err = UROS_ERR_BADPARAM; goto _finally; },
            ("Response code %d, expected %d\n",
             response.code, UROS_RPCC_SUCCESS));
  urosError(response.httpcode != 200,
            { err = UROS_ERR_BADPARAM; goto _finally; },
            ("Response HTTP code %d, expected 200\n", response.httpcode));

  /* Add to the published topics list.*/
  nodep = urosNew(UrosListNode);
  if (nodep == NULL) { err = UROS_ERR_NOMEM; goto _finally; }
  urosListNodeObjectInit(nodep);
  nodep->datap = (void*)servicep;
  urosListAdd(&np->status.pubServiceList, nodep);

  err = UROS_OK;
_finally:
  /* Cleanup and return.*/
  urosMutexUnlock(&np->status.pubServiceListLock);
  urosRpcResponseClean(&response);
  return err;
}

/**
 * @brief   Unpublishes a service.
 * @details Issues an @p unregisterService() call to the XMLRPC Master.
 * @see     urosRpcCallUnregisterService()
 * @warning The access to the service registry is thread-safe, but delays of
 *          the XMLRPC communication will delay also any other threads trying
 *          to publish/unpublish any services.
 *
 * @pre     The service is published.
 * @post    If successful, the service descriptor is dereferenced by the
 *          service registry, and will be freed:
 *          - by this function, if there are no publishing TCPROS threads, or
 *          - by the last publishing TCPROS thread which references the
 *            service.
 *
 * @param[in] namep
 *          Pointer to a string which names the service.
 * @return
 *          Error code.
 */
uros_err_t urosNodeUnpublishService(const UrosString *namep) {

  static UrosNode *const np = &urosNode;

  UrosListNode *tcprosnodep, *servicenodep;
  UrosTopic *servicep;
  uros_err_t err;
  UrosRpcResponse res;

  urosAssert(urosStringNotEmpty(namep));

  /* Find the service descriptor.*/
  urosMutexLock(&np->status.pubServiceListLock);
  servicenodep = urosTopicListFindByName(&np->status.pubServiceList,
                                         namep);
  urosError(servicenodep == NULL,
            { err = UROS_ERR_BADPARAM; goto _finally; },
            ("Service [%.*s] not published\n", UROS_STRARG(namep)));
  servicep = (UrosTopic*)servicenodep->datap;

  /* Unregister the service on the Master node.*/
  err = urosRpcCallUnregisterService(
    &np->config.masterAddr,
    &np->config.nodeName,
    namep,
    &np->config.tcprosUri,
    &res
  );
  urosError(err != UROS_OK, goto _finally,
            ("Error %s while unregistering as publisher of service [%.*s]\n",
             urosErrorText(err), UROS_STRARG(namep)));

  /* Unregister the service locally.*/
  servicep->flags.deleted = UROS_TRUE;
  tcprosnodep = urosListRemove(&np->status.pubServiceList, servicenodep);
  urosAssert(tcprosnodep == servicenodep);

  if (np->status.pubTcpList.length > 0) {
    /* Tell each publishing TCPROS thread to exit.*/
    urosMutexLock(&np->status.pubTcpListLock);
    for (tcprosnodep = np->status.pubTcpList.headp;
         tcprosnodep != NULL;
         tcprosnodep = tcprosnodep->nextp) {

      UrosTcpRosStatus *tcpstp = (UrosTcpRosStatus*)tcprosnodep->datap;
      if (tcpstp->topicp == servicep && tcpstp->flags.service) {
        urosMutexLock(&tcpstp->threadExitMtx);
        tcpstp->threadExit = UROS_TRUE;
        urosMutexUnlock(&tcpstp->threadExitMtx);
      }
    }
    urosMutexUnlock(&np->status.pubTcpListLock);
    /* NOTE: The last exiting thread freeds the service descriptor.*/
  } else {
    /* No TCPROS connections, just free the descriptor immediately.*/
    urosListNodeDelete(servicenodep, (uros_delete_f)urosTopicDelete);
  }

_finally:
  urosMutexUnlock(&np->status.pubServiceListLock);
  return err;
}

/**
 * @brief   Unpublishes a service.
 * @details Issues an @p unregisterService() call to the XMLRPC Master.
 * @see     urosRpcCallUnregisterService()
 * @warning The access to the service registry is thread-safe, but delays of
 *          the XMLRPC communication will delay also any other threads trying
 *          to publish/unpublish any services.
 *
 * @pre     The service is published.
 * @post    If successful, the service descriptor is dereferenced by the
 *          service registry, and will be freed:
 *          - by this function, if there are no publishing TCPROS threads, or
 *          - by the last publishing TCPROS thread which references the
 *            service.
 *
 * @param[in] namep
 *          Pointer to a null-terminated string which names the service.
 * @return
 *          Error code.
 */
uros_err_t urosNodeUnpublishServiceSZ(const char *namep) {

  UrosString namestr;

  urosAssert(namep != NULL);
  urosAssert(namep[0] != 0);

  namestr = urosStringAssignZ(namep);
  return urosNodeUnpublishService(&namestr);
}

/**
 * @brief   Subscribes to a parameter by its descriptor.
 * @details Issues a @p subscribeParam() call to the XMLRPC Master, and
 *          connects to known publishers.
 * @see     urosRpcCallSubscribeParam()
 * @see     urosNodeSubscribeParamByDesc()
 * @warning The access to the parameter registry is thread-safe, but delays of
 *          the XMLRPC communication will delay also any other threads trying
 *          to subscribe/unsubscribe to any parameters.
 *
 * @pre     The parameter has not been registered yet.
 *
 * @param[in] namep
 *          Pointer to the parameter name string.
 * @return
 *          Error code.
 */
uros_err_t urosNodeSubscribeParam(const UrosString *namep) {

  static UrosNode *const np = &urosNode;

  UrosString *clonednamep;
  UrosListNode *paramnodep;
  UrosRpcResponse response;
  UrosListNode *nodep;
  uros_err_t err;

  urosAssert(urosStringNotEmpty(namep));

  /* Check if the parameter already exists.*/
  urosMutexLock(&np->status.subParamListLock);
  paramnodep = urosStringListFindByName(&np->status.subParamList, namep);
  urosMutexUnlock(&np->status.subParamListLock);
  urosError(paramnodep != NULL, return UROS_ERR_BADPARAM,
            ("Parameter [%.*s] already subscribed\n", UROS_STRARG(namep)));

  /* Create the storage data in advance.*/
  clonednamep = urosNew(UrosString);
  if (clonednamep == NULL) { return UROS_ERR_NOMEM; }
  *clonednamep = urosStringClone(namep);
  nodep = urosNew(UrosListNode);
  if (clonednamep->datap == NULL || nodep == NULL) {
    urosStringDelete(clonednamep); urosFree(nodep);
    return UROS_ERR_NOMEM;
  }

  /* Subscribe to the topic.*/
  urosRpcResponseObjectInit(&response);
  urosMutexLock(&np->status.subParamListLock);

  /* Master XMLRPC registerSubscriber() */
  err = urosRpcCallSubscribeParam(
    &np->config.masterAddr,
    &np->config.nodeName,
    &np->config.xmlrpcUri,
    namep,
    &response
  );
  urosError(err != UROS_OK, goto _finally,
            ("Error %s while subscribing to parameter [%.*s]\n",
             urosErrorText(err), UROS_STRARG(namep)));

  /* Check for valid codes.*/
  urosError(response.code != UROS_RPCC_SUCCESS,
            { err = UROS_ERR_BADPARAM; goto _finally; },
            ("Response code %d, expected %d\n",
             response.code, UROS_RPCC_SUCCESS));
  urosError(response.httpcode != 200,
            { err = UROS_ERR_BADPARAM; goto _finally; },
            ("Response HTTP code %d, expected 200\n", response.httpcode));

  /* Add to the subscribed topics list.*/
  urosListNodeObjectInit(nodep);
  nodep->datap = (void*)clonednamep;
  urosListAdd(&np->status.subParamList, nodep);

  err = UROS_OK;
_finally:
  /* Cleanup and return.*/
  urosMutexUnlock(&np->status.subParamListLock);
  urosRpcResponseClean(&response);
  return err;
}

/**
 * @brief   Subscribes to a parameter.
 * @details Issues a @p subscribeParam() call to the XMLRPC Master, and
 *          connects to known publishers.
 * @see     urosRpcCallSubscribeParam()
 * @see     urosNodeSubscribeParamByDesc()
 * @warning The access to the parameter registry is thread-safe, but delays of
 *          the XMLRPC communication will delay also any other threads trying
 *          to subscribe/unsubscribe to any parameters.
 *
 * @pre     The parameter has not been registered yet.
 *
 * @param[in] namep
 *          Pointer to the parameter name null-terminated string.
 * @return
 *          Error code.
 */
uros_err_t urosNodeSubscribeParamSZ(const char *namep) {

  UrosString namestr;

  urosAssert(namep != NULL);
  urosAssert(namep[0] != 0);

  namestr = urosStringAssignZ(namep);
  return urosNodeSubscribeParam(&namestr);
}

/**
 * @brief   Subscribes to a parameter.
 * @details Issues an @p unsubscribeParam() call to the XMLRPC Master, and
 *          connects to known publishers.
 * @see     urosRpcCallUnubscribeParam()
 * @warning The access to the parameter registry is thread-safe, but delays of
 *          the XMLRPC communication will delay also any other threads trying
 *          to subscribe/unsubscribe to any parameters.
 *
 * @pre     The parameter has been registered.
 * @post    If successful, the parameter descriptor is unreferenced and deleted
 *          by the parameter registry.
 *
 * @param[in] namep
 *          Pointer to a string which names the parameter to be unregistered.
 * @return
 *          Error code.
 */
uros_err_t urosNodeUnsubscribeParam(const UrosString *namep) {

  static UrosNode *const np = &urosNode;

  UrosRpcResponse response;
  uros_err_t err;
  UrosListNode *nodep;

  urosAssert(urosStringNotEmpty(namep));

  urosRpcResponseObjectInit(&response);
  urosMutexLock(&np->status.subParamListLock);

  /* Check if the parameter was actually subscribed.*/
  nodep = urosStringListFindByName(&np->status.subParamList, namep);
  urosError(nodep == NULL, { err = UROS_ERR_BADPARAM; goto _finally; },
            ("Parameter [%.*s] not found\n", UROS_STRARG(namep)));

  /* Master XMLRPC registerSubscriber() */
  err = urosRpcCallUnsubscribeParam(
    &np->config.masterAddr,
    &np->config.nodeName,
    &np->config.xmlrpcUri,
    namep,
    &response
  );
  urosError(err != UROS_OK, goto _finally,
            ("Cannot unsubscribe from param [%.*s]\n",
             UROS_STRARG(namep)));

  /* Check for valid codes.*/
  urosError(response.code != UROS_RPCC_SUCCESS,
            { err = UROS_ERR_BADPARAM; goto _finally; },
            ("Response code %ld, expected %d\n",
             response.code, UROS_RPCC_SUCCESS));
  urosError(response.httpcode != 200,
            { err = UROS_ERR_BADPARAM; goto _finally; },
            ("Response HTTP code %ld, expected 200\n", response.httpcode));

  /* Remove from the subscribed topics list and delete.*/
  nodep = urosListRemove(&np->status.subParamList, nodep);
  urosAssert(nodep != NULL);
  urosListNodeDelete(nodep, (uros_delete_f)urosStringDelete);

  err = UROS_OK;
_finally:
  /* Cleanup and return.*/
  urosMutexUnlock(&np->status.subParamListLock);
  urosRpcResponseClean(&response);
  return err;
}

/**
 * @brief   Subscribes to a parameter.
 * @details Issues an @p unsubscribeParam() call to the XMLRPC Master, and
 *          connects to known publishers.
 * @see     urosRpcCallUnubscribeParam()
 * @warning The access to the parameter registry is thread-safe, but delays of
 *          the XMLRPC communication will delay also any other threads trying
 *          to subscribe/unsubscribe to any parameters.
 *
 * @pre     The parameter has been registered.
 * @post    If successful, the parameter descriptor is unreferenced and deleted
 *          by the parameter registry.
 *
 * @param[in] namep
 *          Pointer to a null-terminated string which names the parameter to be
 *          unregistered.
 * @return
 *          Error code.
 */
uros_err_t urosNodeUnsubscribeParamSZ(const char *namep) {

  UrosString namestr;

  urosAssert(namep != NULL);
  urosAssert(namep[0] != 0);

  namestr = urosStringAssignZ(namep);
  return urosNodeUnsubscribeParam(&namestr);
}

/**
 * @brief   Find new publishers for a given topic.
 * @details Scans through the provided publishers list to look for any new
 *          publishers.
 *
 * @param[in] topicnamep
 *          Pointer to a non-empty string which names the targeted topic.
 * @param[in] publishersp
 *          Pointer to an @p UrosRpcParam with @p UROS_RPCP_ARRAY class.
 *          It contains the list of current publishers, received for example
 *          through a XMLRPC call to @p subscribeTopic(). Each publisher is
 *          addressed by its URI.
 * @param[out] newpubsp
 *          Pointer to an empty list which will be populated by the newly
 *          discovered publishers, if any.
 * @return
 *          Error code.
 */
uros_err_t urosNodeFindNewPublishers(const UrosString *topicnamep,
                                     const UrosRpcParam *publishersp,
                                     UrosList *newpubsp) {

  static UrosNode *const np = &urosNode;

  uros_err_t err;
  const UrosListNode *tcpnodep;
  const UrosRpcParamNode *paramnodep;
  const UrosTcpRosStatus *tcpstp, *tcpfoundp;
  const UrosString *urip;
  UrosAddr pubaddr;
  (void)err;

  urosAssert(urosStringNotEmpty(topicnamep));
  urosAssert(publishersp != NULL);
  urosAssert(publishersp->class == UROS_RPCP_ARRAY);
  urosAssert(publishersp->value.listp != NULL);
  urosAssert(urosListIsValid(newpubsp));
  urosAssert(newpubsp->length == 0);

  /* Build a list of newly discovered publishers.*/
  urosMutexLock(&np->status.subTcpListLock);
  for (paramnodep = publishersp->value.listp->headp;
       paramnodep != NULL;
       paramnodep = paramnodep->nextp) {

    urip = &paramnodep->param.value.string;
    err = urosUriToAddr(urip, &pubaddr);
    urosAssert(err == UROS_OK);
    tcpfoundp = NULL;
    for (tcpnodep = np->status.subTcpList.headp;
         tcpnodep != NULL;
         tcpnodep = tcpnodep->nextp) {

      tcpstp = (const UrosTcpRosStatus *)tcpnodep->datap;
      if (tcpstp->flags.service == UROS_FALSE) {
        urosAssert(tcpstp->topicp != NULL);
        if (0 == urosStringCmp(&tcpstp->topicp->name, topicnamep)) {
          urosAssert(tcpstp->csp != NULL);
          if (tcpstp->csp->remaddr.ip.dword == pubaddr.ip.dword &&
              tcpstp->csp->remaddr.port == pubaddr.port) {
            tcpfoundp = tcpstp;
          }
        }
      }
    }
    if (tcpfoundp == NULL) {
      UrosAddr *addrp;
      UrosListNode *nodep;

      /* New publisher.*/
      addrp = urosNew(UrosAddr);
      if (addrp == NULL) {
        urosMutexUnlock(&np->status.subTcpListLock);
        return UROS_ERR_NOMEM;
      }
      nodep = urosNew(UrosListNode);
      if (nodep == NULL) {
        urosFree(addrp);
        urosMutexUnlock(&np->status.subTcpListLock);
        return UROS_ERR_NOMEM;
      }
      *addrp = pubaddr;
      nodep->datap = addrp;
      nodep->nextp = NULL;
      urosListAdd(newpubsp, nodep);
    }
  }
  urosMutexUnlock(&np->status.subTcpListLock);

  return UROS_OK;
}

/** @} */
