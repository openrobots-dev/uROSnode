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
 * @file    uros_lld_threading.h
 * @author  Andrea Zoppi <texzk@email.it>
 *
 * @brief   Low-level threading features of the middleware.
 */

#ifndef _UROS_LLD_THREADING_H_
#define _UROS_LLD_THREADING_H_

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include "../urosThreading.h"

/*===========================================================================*/
/* GLOBAL PROTOTYPES                                                         */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

void uros_lld_sem_objectinit(UrosSem *semp, uros_cnt_t n);
void uros_lld_sem_clean(UrosSem *semp);
void uros_lld_sem_wait(UrosSem *semp);
void uros_lld_sem_signal(UrosSem *semp);
uros_cnt_t uros_lld_sem_value(UrosSem *semp);

void uros_lld_mutex_objectinit(UrosMutex *mtxp);
void uros_lld_mutex_clean(UrosMutex *mtxp);
void uros_lld_mutex_lock(UrosMutex *mtxp);
void uros_lld_mutex_unlock(UrosMutex *mtxp);

void uros_lld_condvar_objectinit(UrosCondVar *cvp);
void uros_lld_condvar_clean(UrosCondVar *cvp);
void uros_lld_condvar_wait(UrosCondVar *cvp, UrosMutex *mtxp);
void uros_lld_condvar_signal(UrosCondVar *cvp);
void uros_lld_condvar_broadcast(UrosCondVar *cvp);

UrosThreadId uros_lld_thread_self(void);
const char *uros_lld_thread_getname(UrosThreadId id);
uros_err_t uros_lld_thread_createstatic(UrosThreadId *idp, const char *namep,
                                        uros_prio_t priority,
                                        uros_proc_f routine, void *argp,
                                        void *stackp, size_t stacksize);
uros_err_t uros_lld_thread_createfrommempool(UrosThreadId *idp, const char *namep,
                                             uros_prio_t priority,
                                             uros_proc_f routine, void *argp,
                                             UrosMemPool *mempoolp);
uros_err_t uros_lld_thread_createfromheap(UrosThreadId *idp, const char *namep,
                                          uros_prio_t priority,
                                          uros_proc_f routine, void *argp,
                                          size_t stacksize);
uros_err_t uros_lld_thread_join(UrosThreadId id);
void uros_lld_thread_sleepsec(uint32_t sec);
void uros_lld_thread_sleepmsec(uint32_t msec);
void uros_lld_thread_sleepusec(uint32_t usec);

uint32_t uros_lld_threading_gettimestampmsec(void);

#ifdef __cplusplus
}
#endif
#endif /* _UROS_LLD_THREADING_H_ */
