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
 * @file    urosThreading.h
 * @author  Andrea Zoppi <texzk@email.it>
 *
 * @brief   Threading features of the middleware.
 */

#ifndef _UROSTHREADING_H_
#define _UROSTHREADING_H_

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include "urosBase.h"

/*===========================================================================*/
/* TYPES & MACROS                                                            */
/*===========================================================================*/

/** @addtogroup threading_types */
/** @{ */

/**
 * @brief   Thread pool object.
 */
typedef struct UrosThreadPool {
  UrosMemPool   *stackPoolp;    /**< @brief Memory pool for thread stacks.*/
  uros_cnt_t    size;           /**< @brief Thread pool size.*/
  uros_proc_f   routine;        /**< @brief User routine for children.*/
  const char    *namep;         /**< @brief Default thread name.*/
  uros_prio_t   priority;       /**< @brief Default thread priority.*/
  UrosThreadId  *threadsp;      /**< @brief Thread identifier array.*/
  void          *argp;          /**< @brief Next thread argument pointer.*/
  uros_cnt_t    readyCnt;       /**< @brief Ready threads counter.*/
  UrosMutex     readyMtx;       /**< @brief Ready threads mutex.*/
  UrosCondVar   readyCond;      /**< @brief Ready threads condvar.*/
  uros_cnt_t    busyCnt;        /**< @brief Busy threads counter.*/
  UrosMutex     busyMtx;        /**< @brief Busy threads mutex.*/
  UrosCondVar   busyCond;       /**< @brief Busy threads condvar.*/
  uros_bool_t   exitFlag;       /**< @brief Exit request flag, broadcast.*/
} UrosThreadPool;

/** @} */

/*===========================================================================*/
/* GLOBAL PROTOTYPES                                                         */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

void urosSemObjectInit(UrosSem *semp, uros_cnt_t n);
void urosSemClean(UrosSem *semp);
void urosSemWait(UrosSem *semp);
void urosSemSignal(UrosSem *semp);

void urosMutexObjectInit(UrosMutex *mtxp);
void urosMutexClean(UrosMutex *mtxp);
void urosMutexLock(UrosMutex *mtxp);
void urosMutexUnlock(UrosMutex *mtxp);

void urosCondVarObjectInit(UrosCondVar *cvp);
void urosCondVarClean(UrosCondVar *cvp);
void urosCondVarWait(UrosCondVar *cvp, UrosMutex *mtxp);
void urosCondVarSignal(UrosCondVar *cvp);
void urosCondVarBroadcast(UrosCondVar *cvp);

uros_err_t urosThreadPoolObjectInit(UrosThreadPool *poolp,
                                    UrosMemPool *stackpoolp,
                                    uros_proc_f routine,
                                    const char *namep,
                                    uros_prio_t priority);
void urosThreadPoolClean(UrosThreadPool *poolp);
uros_err_t urosThreadPoolCreateAll(UrosThreadPool *poolp);
uros_err_t urosThreadPoolJoinAll(UrosThreadPool *poolp);
uros_err_t urosThreadPoolStartWorker(UrosThreadPool *poolp, void *argp);
uros_err_t urosThreadPoolWorkerThread(UrosThreadPool *poolp);

UrosThreadId urosThreadSelf(void);
const char *urosThreadGetName(UrosThreadId id);
uros_err_t urosThreadCreateStatic(UrosThreadId *idp, const char *namep,
                                  uros_prio_t priority,
                                  uros_proc_f routine, void *argp,
                                  void *stackp, size_t stacksize);
uros_err_t urosThreadCreateFromMemPool(UrosThreadId *threadp, const char *namep,
                                       uros_prio_t priority,
                                       uros_proc_f routine, void *argp,
                                       UrosMemPool *mempoolp);
uros_err_t urosThreadCreateFromHeap(UrosThreadId *idp, const char *namep,
                                    uros_prio_t priority,
                                    uros_proc_f routine, void *argp,
                                    size_t stacksize);
uros_err_t urosThreadJoin(UrosThreadId id);
void urosThreadSleepSec(uint32_t sec);
void urosThreadSleepMsec(uint32_t msec);
void urosThreadSleepUsec(uint32_t usec);

uint32_t urosGetTimestampMsec(void);

#ifdef __cplusplus
}
#endif
#endif /* _UROSTHREADING_H_ */
