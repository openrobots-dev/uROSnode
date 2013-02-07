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
 * @file    urosNode.h
 * @author  Andrea Zoppi <texzk@email.it>
 *
 * @brief   Node features of the middleware.
 */

#ifndef _UROSNODE_H_
#define _UROSNODE_H_

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include "urosBase.h"
#include "urosThreading.h"
#include "urosTcpRos.h"
#include "urosRpcSlave.h"

/*===========================================================================*/
/* TYPES & MACROS                                                            */
/*===========================================================================*/

/** @addtogroup node_macros */
/** @{ */

/** @brief Enables a periodic check of the Master node reachability.*/
#if !defined(UROS_NODE_POLL_MASTER) || defined(__DOXYGEN__)
#define UROS_NODE_POLL_MASTER       UROS_FALSE
#endif

/** @brief Exit condition polling period, in milliseconds.*/
#if !defined(UROS_NODE_POLL_PERIOD) || defined(__DOXYGEN__)
#define UROS_NODE_POLL_PERIOD       2000
#endif

/** @} */

/** @addtogroup node_types */
/** @{ */

/**
 * @brief   Node states.
 */
typedef enum uros_nodestate_t {
  UROS_NODE_UNINIT = 0,                 /**< @brief Node is uninitialized.*/
  UROS_NODE_IDLE,                       /**< @brief Node is stopped.*/
  UROS_NODE_STARTUP,                    /**< @brief Startup sequence.*/
  UROS_NODE_RUNNING,                    /**< @brief Node is running.*/
  UROS_NODE_SHUTDOWN                    /**< @brief Shutdown sequence.*/
} uros_nodestate_t;

/**
 * @brief   Node configuration descriptor.
 */
typedef struct UrosNodeConfig {
  /* Local node settings.*/
  UrosString        nodeName;           /**< @brief Node name.*/
  UrosAddr          xmlrpcAddr;         /**< @brief XMLRPC Listener address.*/
  UrosString        xmlrpcUri;          /**< @brief XMLRPC Listener URI.*/
  UrosAddr          tcprosAddr;         /**< @brief TCPROS Listener address.*/
  UrosString        tcprosUri;          /**< @brief TCPROS Listener URI.*/

  /* Master (remote) settings.*/
  UrosAddr          masterAddr;         /**< @brief ROS Master XMLRPC server address.*/
  UrosString        masterUri;          /**< @brief ROS Master XMLRPC server URI.*/
} UrosNodeConfig;

/**
 * @brief   Node status record.
 */
typedef struct UrosNodeStatus {
  /* Status variables.*/
  uros_nodestate_t  state;              /**< @brief Current node state.*/
  int32_t           xmlrpcPid;          /**< @brief PID of the XMLRPC Listener process.*/
  UrosList          subTopicList;       /**< @brief List of subscribed topics.*/
  UrosList          pubTopicList;       /**< @brief List of published topics.*/
  UrosList          pubServiceList;     /**< @brief List of published services.*/
  UrosList          subParamList;       /**< @brief List of parameter subscriptions.*/
  UrosList          subTcpList;         /**< @brief Subscribed TCPROS connections.*/
  UrosList          pubTcpList;         /**< @brief Published TCPROS connections.*/

  UrosMutex         stateLock;          /**< @brief State and exit lock.*/
  UrosMutex         xmlrpcPidLock;      /**< @brief PID lock.*/
  UrosMutex         subTopicListLock;   /**< @brief Topic subscriptions lock.*/
  UrosMutex         pubTopicListLock;   /**< @brief Topic publications lock.*/
  UrosMutex         pubServiceListLock; /**< @brief Publisher connections lock.*/
  UrosMutex         subParamListLock;   /**< @brief Parameter subscriptions lock.*/
  UrosMutex         subTcpListLock;     /**< @brief Subscribed connections lock.*/
  UrosMutex         pubTcpListLock;     /**< @brief Published connections lock.*/

  /* Threads stuff.*/
  UrosMemPool       tcpcliMemPool;      /**< @brief TCPROS Client worker stack pool.*/
  UrosMemPool       tcpsvrMemPool;      /**< @brief TCPROS Server worker stack pool.*/
  UrosMemPool       slaveMemPool;       /**< @brief XMLRPC Slave worker stack pool.*/
  UrosThreadPool    tcpcliThdPool;      /**< @brief TCPROS Client worker thread pool.*/
  UrosThreadPool    tcpsvrThdPool;      /**< @brief TCPROS Server worker thread pool.*/
  UrosThreadPool    slaveThdPool;       /**< @brief XMLRPC Slave worker thread pool.*/
  UrosThreadId      xmlrpcListenerId;   /**< @brief XMLRPC Listener thread id.*/
  UrosThreadId      tcprosListenerId;   /**< @brief TCPROS Listener thread id.*/
  UrosThreadId      nodeThreadId;       /**< @brief Node thread id.*/
  uros_bool_t       exitFlag;           /**< @brief Thread exit flag.*/
  UrosString        exitMsg;            /**< @brief Exit message string.*/
} UrosNodeStatus;

/**
 * @brief   Node object.
 */
typedef struct UrosNode {
  const UrosNodeConfig  config;         /**< @brief Node configuration (loaded at boot time).*/
  UrosNodeStatus        status;         /**< @brief Node status.*/
} UrosNode;

/** @} */

/*===========================================================================*/
/* GLOBAL VARIABLES                                                          */
/*===========================================================================*/

extern UrosNode urosNode;

/*===========================================================================*/
/* GLOBAL PROTOTYPES                                                         */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

void urosNodeObjectInit(UrosNode *np);
uros_err_t urosNodeCreateThread(void);
uros_err_t urosNodeThread(void *argp);

void urosNodeConfigLoadDefaults(UrosNodeConfig *cfgp);

uros_err_t urosNodePublishTopic(const UrosString *namep,
                                const UrosString *typep,
                                uros_proc_f procf,
                                uros_topicflags_t flags);
uros_err_t urosNodePublishTopicSZ(const char *namep,
                                  const char *typep,
                                  uros_proc_f procf,
                                  uros_topicflags_t flags);
uros_err_t urosNodePublishTopicByDesc(UrosTopic *topicp);
uros_err_t urosNodeUnpublishTopic(const UrosString *namep);
uros_err_t urosNodeUnpublishTopicSZ(const char *namep);

uros_err_t urosNodeSubscribeTopic(const UrosString *namep,
                                  const UrosString *typep,
                                  uros_proc_f procf,
                                  uros_topicflags_t flags);
uros_err_t urosNodeSubscribeTopicSZ(const char *namep,
                                    const char *typep,
                                    uros_proc_f procf,
                                    uros_topicflags_t flags);
uros_err_t urosNodeSubscribeTopicByDesc(UrosTopic *topicp);
uros_err_t urosNodeUnsubscribeTopic(const UrosString *namep);
uros_err_t urosNodeUnsubscribeTopicSZ(const char *namep);

uros_err_t urosNodeCallService(const UrosString *namep,
                               const UrosString *typep,
                               uros_tcpsrvcall_t callf,
                               uros_topicflags_t flags,
                               void *resobjp);
uros_err_t urosNodeCallServiceSZ(const char *namep,
                                 const char *typep,
                                 uros_tcpsrvcall_t callf,
                                 uros_topicflags_t flags,
                                 void *resobjp);
uros_err_t urosNodeCallServiceByDesc(const UrosTopic *servicep,
                                     void *resobjp);

uros_err_t urosNodePublishService(const UrosString *namep,
                                  const UrosString *typep,
                                  uros_proc_f procf,
                                  uros_topicflags_t flags);
uros_err_t urosNodePublishServiceSZ(const char *namep,
                                    const char *typep,
                                    uros_proc_f procf,
                                    uros_topicflags_t flags);
uros_err_t urosNodePublishServiceByDesc(const UrosTopic *servicep);
uros_err_t urosNodeUnpublishService(const UrosString *namep);
uros_err_t urosNodeUnpublishServiceSZ(const char *namep);

uros_err_t urosNodeSubscribeParam(const UrosString *namep);
uros_err_t urosNodeSubscribeParamSZ(const char *namep);
uros_err_t urosNodeUnsubscribeParam(const UrosString *namep);
uros_err_t urosNodeUnsubscribeParamSZ(const char *namep);

uros_err_t urosNodeFindNewTopicPublishers(const UrosString *topicnamep,
                                          const UrosRpcParam *publishersp,
                                          UrosList *newpubsp);
uros_err_t urosNodeResolveTopicPublisher(const UrosAddr *apiaddrp,
                                         const UrosString *namep,
                                         UrosAddr *tcprosaddrp);
uros_err_t urosNodeResolveServicePublisher(const UrosString *namep,
                                           UrosAddr *pubaddrp);

#ifdef __cplusplus
}
#endif
#endif /* _UROSNODE_H_ */
