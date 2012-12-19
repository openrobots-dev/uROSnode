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
/* GLOBAL VARIABLES                                                          */
/*===========================================================================*/

/**
 * @brief   Node singleton.
 */
UrosNode urosNode;

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

  urosMutexObjectInit(&stp->xmlrpcPidLock);
  urosMutexObjectInit(&stp->subTopicListLock);
  urosMutexObjectInit(&stp->pubTopicListLock);
  urosMutexObjectInit(&stp->pubServiceListLock);
  urosMutexObjectInit(&stp->subParamListLock);
  urosMutexObjectInit(&stp->subTcpListLock);
  urosMutexObjectInit(&stp->pubTcpListLock);

  /* Initialize stack pools.*/
  urosUserAllocMemPools(np);

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
  urosMutexLock(&urosNode.status.pubTopicListLock);
  topicnodep = urosTopicListFindByName(&urosNode.status.pubTopicList, namep);
  urosMutexUnlock(&urosNode.status.pubTopicListLock);
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

  static const UrosNodeConfig *cfgp = &urosNode.config;
  UrosRpcResponse response;
  uros_err_t err;
  UrosListNode *nodep;

  urosAssert(topicp != NULL);
  urosAssert(urosStringNotEmpty(&topicp->name));
  urosAssert(topicp->typep != NULL);
  urosAssert(urosStringNotEmpty(&topicp->typep->name));
  urosAssert(topicp->refcnt == 0);

  urosRpcResponseObjectInit(&response);
  urosMutexLock(&urosNode.status.pubTopicListLock);

  /* Master XMLRPC registerPublisher() */
  err = urosRpcCallRegisterPublisher(
    &cfgp->masterAddr,
    &cfgp->nodeName,
    &topicp->name,
    &topicp->typep->name,
    &cfgp->xmlrpcUri,
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
  urosListAdd(&urosNode.status.pubTopicList, nodep);

  err = UROS_OK;
_finally:
  /* Cleanup and return.*/
  urosMutexUnlock(&urosNode.status.pubTopicListLock);
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

  UrosListNode *tcprosnodep, *topicnodep;
  UrosTopic *topicp;
  uros_err_t err;
  UrosRpcResponse res;

  urosAssert(urosStringNotEmpty(namep));

  /* Find the topic descriptor.*/
  urosMutexLock(&urosNode.status.pubTopicListLock);
  topicnodep = urosTopicListFindByName(&urosNode.status.pubTopicList, namep);
  if (topicnodep == NULL) {
    urosError(topicnodep == NULL,
              { err = UROS_ERR_BADPARAM; goto _finally; },
              ("Topic [%.*s] not published\n", UROS_STRARG(namep)));
  }
  topicp = (UrosTopic*)topicnodep->datap;

  /* Unregister the topic on the Master node.*/
  err = urosRpcCallUnregisterPublisher(
    &urosNode.config.masterAddr,
    &urosNode.config.nodeName,
    namep,
    &urosNode.config.xmlrpcUri,
    &res
  );
  urosError(err != UROS_OK, UROS_NOP,
            ("Error %s while unregistering as publisher of topic [%.*s]\n",
             urosErrorText(err), UROS_STRARG(namep)));

  /* Unregister the topic locally.*/
  topicp->flags.deleted = UROS_TRUE;
  tcprosnodep = urosListRemove(&urosNode.status.pubTopicList, topicnodep);
  urosAssert(tcprosnodep == topicnodep);

  urosMutexLock(&urosNode.status.pubTcpListLock);
  if (urosNode.status.pubTcpList.length > 0) {
    /* Tell each publishing TCPROS thread to exit.*/
    for (tcprosnodep = urosNode.status.pubTcpList.headp;
         tcprosnodep != NULL;
         tcprosnodep = tcprosnodep->nextp) {

      UrosTcpRosStatus *tcpstp = (UrosTcpRosStatus*)tcprosnodep->datap;
      if (tcpstp->topicp == topicp && !tcpstp->flags.service) {
        urosMutexLock(&tcpstp->threadExitMtx);
        tcpstp->threadExit = UROS_TRUE;
        urosMutexUnlock(&tcpstp->threadExitMtx);
      }
    }
    /* NOTE: The last exiting thread freeds the topic descriptor.*/
  } else {
    /* No TCPROS connections, just free the descriptor immediately.*/
    urosListNodeDelete(topicnodep, (uros_delete_f)urosTopicDelete);
  }
  urosMutexUnlock(&urosNode.status.pubTcpListLock);

_finally:
  urosMutexUnlock(&urosNode.status.pubTopicListLock);
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
  urosMutexLock(&urosNode.status.subTopicListLock);
  topicnodep = urosTopicListFindByName(&urosNode.status.subTopicList, namep);
  urosMutexUnlock(&urosNode.status.subTopicListLock);
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

  static const UrosNodeConfig *cfgp = &urosNode.config;
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
  urosMutexLock(&urosNode.status.subTopicListLock);

  /* Master XMLRPC registerSubscriber() */
  err = urosRpcCallRegisterSubscriber(
    &cfgp->masterAddr,
    &cfgp->nodeName,
    &topicp->name,
    &topicp->typep->name,
    &cfgp->xmlrpcUri,
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
  urosListAdd(&urosNode.status.subTopicList, nodep);

  err = UROS_OK;
_finally:
  /* Cleanup and return.*/
  urosMutexUnlock(&urosNode.status.subTopicListLock);
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

  UrosListNode *tcprosnodep, *topicnodep;
  UrosTopic *topicp;
  uros_err_t err;
  UrosRpcResponse res;

  urosAssert(urosStringNotEmpty(namep));

  /* Find the topic descriptor.*/
  urosMutexLock(&urosNode.status.subTopicListLock);
  topicnodep = urosTopicListFindByName(&urosNode.status.subTopicList, namep);
  if (topicnodep == NULL) {
    urosError(topicnodep == NULL,
              { err = UROS_ERR_BADPARAM; goto _finally; },
              ("Topic [%.*s] not subscribed\n", UROS_STRARG(namep)));
  }
  topicp = (UrosTopic*)topicnodep->datap;

  /* Unregister the topic on the Master node.*/
  err = urosRpcCallUnregisterSubscriber(
    &urosNode.config.masterAddr,
    &urosNode.config.nodeName,
    namep,
    &urosNode.config.xmlrpcUri,
    &res
  );
  urosError(err != UROS_OK, UROS_NOP,
            ("Error %s while unregistering as subscriber of topic [%.*s]\n",
             urosErrorText(err), UROS_STRARG(namep)));

  /* Unregister the topic locally.*/
  topicp->flags.deleted = UROS_TRUE;
  tcprosnodep = urosListRemove(&urosNode.status.subTopicList, topicnodep);
  urosAssert(tcprosnodep == topicnodep);

  urosMutexLock(&urosNode.status.subTcpListLock);
  if (urosNode.status.subTcpList.length > 0) {
    /* Tell each subscribing TCPROS thread to exit.*/
    for (tcprosnodep = urosNode.status.subTcpList.headp;
         tcprosnodep != NULL;
         tcprosnodep = tcprosnodep->nextp) {

      UrosTcpRosStatus *tcpstp = (UrosTcpRosStatus*)tcprosnodep->datap;
      if (tcpstp->topicp == topicp && !tcpstp->flags.service) {
        urosMutexLock(&tcpstp->threadExitMtx);
        tcpstp->threadExit = UROS_TRUE;
        urosMutexUnlock(&tcpstp->threadExitMtx);
      }
    }
    /* NOTE: The last exiting thread freeds the topic descriptor.*/
  } else {
    /* No TCPROS connections, just free the descriptor immediately.*/
    urosListNodeDelete(topicnodep, (uros_delete_f)urosTopicDelete);
  }
  urosMutexUnlock(&urosNode.status.subTcpListLock);

_finally:
  urosMutexUnlock(&urosNode.status.subTopicListLock);
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
  urosMutexLock(&urosNode.status.pubServiceListLock);
  servicenodep = urosTopicListFindByName(&urosNode.status.pubServiceList,
                                         namep);
  urosMutexUnlock(&urosNode.status.pubServiceListLock);
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

  static const UrosNodeConfig *cfgp = &urosNode.config;
  UrosRpcResponse response;
  uros_err_t err;
  UrosListNode *nodep;

  urosAssert(servicep != NULL);
  urosAssert(urosStringNotEmpty(&servicep->name));
  urosAssert(servicep->typep != NULL);
  urosAssert(urosStringNotEmpty(&servicep->typep->name));

  urosRpcResponseObjectInit(&response);
  urosMutexLock(&urosNode.status.pubServiceListLock);

  /* Master XMLRPC registerPublisher() */
  err = urosRpcCallRegisterService(
    &cfgp->masterAddr,
    &cfgp->nodeName,
    &servicep->name,
    &cfgp->tcprosUri,
    &cfgp->xmlrpcUri,
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
  urosListAdd(&urosNode.status.pubServiceList, nodep);

  err = UROS_OK;
_finally:
  /* Cleanup and return.*/
  urosMutexUnlock(&urosNode.status.pubServiceListLock);
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

  UrosListNode *tcprosnodep, *servicenodep;
  UrosTopic *servicep;
  uros_err_t err;
  UrosRpcResponse res;

  urosAssert(urosStringNotEmpty(namep));

  /* Find the service descriptor.*/
  urosMutexLock(&urosNode.status.pubServiceListLock);
  servicenodep = urosTopicListFindByName(&urosNode.status.pubServiceList,
                                         namep);
  urosError(servicenodep == NULL,
            { err = UROS_ERR_BADPARAM; goto _finally; },
            ("Service [%.*s] not published\n", UROS_STRARG(namep)));
  servicep = (UrosTopic*)servicenodep->datap;

  /* Unregister the service on the Master node.*/
  err = urosRpcCallUnregisterService(
    &urosNode.config.masterAddr,
    &urosNode.config.nodeName,
    namep,
    &urosNode.config.tcprosUri,
    &res
  );
  urosError(err != UROS_OK, goto _finally,
            ("Error %s while unregistering as publisher of service [%.*s]\n",
             urosErrorText(err), UROS_STRARG(namep)));

  /* Unregister the service locally.*/
  servicep->flags.deleted = UROS_TRUE;
  tcprosnodep = urosListRemove(&urosNode.status.pubServiceList, servicenodep);
  urosAssert(tcprosnodep == servicenodep);

  urosMutexLock(&urosNode.status.pubTcpListLock);
  if (urosNode.status.pubTcpList.length > 0) {
    /* Tell each publishing TCPROS thread to exit.*/
    for (tcprosnodep = urosNode.status.pubTcpList.headp;
         tcprosnodep != NULL;
         tcprosnodep = tcprosnodep->nextp) {

      UrosTcpRosStatus *tcpstp = (UrosTcpRosStatus*)tcprosnodep->datap;
      if (tcpstp->topicp == servicep && tcpstp->flags.service) {
        urosMutexLock(&tcpstp->threadExitMtx);
        tcpstp->threadExit = UROS_TRUE;
        urosMutexUnlock(&tcpstp->threadExitMtx);
      }
    }
    /* NOTE: The last exiting thread freeds the service descriptor.*/
  } else {
    /* No TCPROS connections, just free the descriptor immediately.*/
    urosListNodeDelete(servicenodep, (uros_delete_f)urosTopicDelete);
  }
  urosMutexUnlock(&urosNode.status.pubTcpListLock);

_finally:
  urosMutexUnlock(&urosNode.status.pubServiceListLock);
  return err;
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

  static const UrosNodeConfig *cfgp = &urosNode.config;

  UrosString *clonednamep;
  UrosListNode *paramnodep;
  UrosRpcResponse response;
  UrosListNode *nodep;
  uros_err_t err;

  urosAssert(urosStringNotEmpty(namep));

  /* Check if the parameter already exists.*/
  urosMutexLock(&urosNode.status.subParamListLock);
  paramnodep = urosStringListFindByName(&urosNode.status.subParamList, namep);
  urosMutexUnlock(&urosNode.status.subParamListLock);
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
  urosMutexLock(&urosNode.status.subParamListLock);

  /* Master XMLRPC registerSubscriber() */
  err = urosRpcCallSubscribeParam(
    &cfgp->masterAddr,
    &cfgp->nodeName,
    &cfgp->xmlrpcUri,
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
  urosListAdd(&urosNode.status.subParamList, nodep);

  err = UROS_OK;
_finally:
  /* Cleanup and return.*/
  urosMutexUnlock(&urosNode.status.subParamListLock);
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

  static const UrosNodeConfig *cfgp = &urosNode.config;
  UrosRpcResponse response;
  uros_err_t err;
  UrosListNode *nodep;

  urosAssert(urosStringNotEmpty(namep));

  urosRpcResponseObjectInit(&response);
  urosMutexLock(&urosNode.status.subParamListLock);

  /* Check if the parameter was actually subscribed.*/
  nodep = urosStringListFindByName(&urosNode.status.subParamList, namep);
  urosError(nodep == NULL, { err = UROS_ERR_BADPARAM; goto _finally; },
            ("Topic [%.*s] not found\n", UROS_STRARG(namep)));

  /* Master XMLRPC registerSubscriber() */
  err = urosRpcCallUnsubscribeParam(
    &cfgp->masterAddr,
    &cfgp->nodeName,
    &cfgp->xmlrpcUri,
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
  nodep = urosListRemove(&urosNode.status.subParamList, nodep);
  urosAssert(nodep != NULL);
  urosListNodeDelete(nodep, (uros_delete_f)urosStringDelete);

  err = UROS_OK;
_finally:
  /* Cleanup and return.*/
  urosMutexUnlock(&urosNode.status.subParamListLock);
  urosRpcResponseClean(&response);
  return err;
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
  urosMutexLock(&urosNode.status.subTcpListLock);
  for (paramnodep = publishersp->value.listp->headp;
       paramnodep != NULL;
       paramnodep = paramnodep->nextp) {

    urip = &paramnodep->param.value.string;
    err = urosUriToAddr(urip, &pubaddr);
    urosAssert(err == UROS_OK);
    tcpfoundp = NULL;
    for (tcpnodep = urosNode.status.subTcpList.headp;
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
        urosMutexUnlock(&urosNode.status.subTcpListLock);
        return UROS_ERR_NOMEM;
      }
      nodep = urosNew(UrosListNode);
      if (nodep == NULL) {
        urosFree(addrp);
        urosMutexUnlock(&urosNode.status.subTcpListLock);
        return UROS_ERR_NOMEM;
      }
      *addrp = pubaddr;
      nodep->datap = addrp;
      nodep->nextp = NULL;
      urosListAdd(newpubsp, nodep);
    }
  }
  urosMutexUnlock(&urosNode.status.subTcpListLock);

  return UROS_OK;
}

/** @} */
