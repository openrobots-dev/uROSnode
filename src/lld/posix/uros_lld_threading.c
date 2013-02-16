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
 * @file    uros_lld_threading.c
 * @author  Andrea Zoppi <texzk@email.it>
 *
 * @brief   Low-level threading features implementation.
 */

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include "../../../include/lld/uros_lld_threading.h"
#include "../../../include/urosUser.h"

#include <pthread.h>
#include <sys/time.h>
#include <errno.h>

/*===========================================================================*/
/* LOCAL TYPES & MACROS                                                      */
/*===========================================================================*/

#if UROS_THREADING_C_USE_ASSERT == UROS_FALSE && !defined(__DOXYGEN__)
#undef urosAssert
#define urosAssert(expr)
#endif

/*===========================================================================*/
/* GLOBAL FUNCTIONS                                                          */
/*===========================================================================*/

/** @addtogroup threading_lld_funcs */
/** @{ */

/*~~~ SEMAPHORE ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Semaphore */
/** @{ */

/**
 * @brief   Initializes a semaphore.
 * @details The semaphore object is initialized, and an initial count is
 *          assigned.
 *
 * @pre     The semaphore is not initialized.
 *
 * @param[in,out] semp
 *          Pointer to an allocated @p UrosSem object.
 * @param n
 *          Initial semaphore count.
 */
void uros_lld_sem_objectinit(UrosSem *semp, uros_cnt_t n) {

  urosAssert(semp != NULL);

  semp->counter = n;
  pthread_mutex_init(&semp->mutex, NULL);
  pthread_cond_init(&semp->cond, NULL);
}

/**
 * @brief   Cleans a semaphore.
 * @details Deallocates any memory chunks allocated by the object itself.
 *
 * @pre     The semaphore is initialized.
 * @post    The semaphore is not initialized, call @p urosSemObjectInit() to
 *          use it again.
 *
 * @param[in,out] semp
 *          Pointer to an initialized @p UrosSem object.
 */
void uros_lld_sem_clean(UrosSem *semp) {

  urosAssert(semp != NULL);

  pthread_mutex_destroy(&semp->mutex);
  pthread_cond_destroy(&semp->cond);
}

/**
 * @brief   Waits for a semaphore signal.
 * @details Waits until the semaphore counter is positive. When positive, it
 *          is decremented and the procedure returns the control to the caller.
 * @note    Depending on the low-level implementation, it may support a
 *          <i>priority inversion</i> mechanism.
 *
 * @param[in,out] semp
 *          Pointer to an initialized @p UrosSem object.
 */
void uros_lld_sem_wait(UrosSem *semp) {

  urosAssert(semp != NULL);

  pthread_mutex_lock(&semp->mutex);
  while (semp->counter <= 0) {
    pthread_cond_wait(&semp->cond, &semp->mutex);
  }
  --semp->counter;
  pthread_mutex_unlock(&semp->mutex);
}

/**
 * @brief   Semaphore signal.
 * @details Increments a semaphore counter.
 *
 * @param[in,out] semp
 *          Pointer to an initialized @p UrosSem object.
 */
void uros_lld_sem_signal(UrosSem *semp) {

  urosAssert(semp != NULL);

  pthread_mutex_lock(&semp->mutex);
  ++semp->counter;
  pthread_cond_signal(&semp->cond);
  pthread_mutex_unlock(&semp->mutex);
}

/**
 * @brief   Gets the semaphore value.
 * @details Reads the current semaphore value.
 * @warning Thread-safe, but the value may not be consistent after a call to
 *          this function.
 *
 * @param[in] semp
 *          Pointer to an initialized @p UrosSem object.
 * @return
 *          Current semaphore count.
 */
uros_cnt_t uros_lld_sem_value(UrosSem *semp) {

  uros_cnt_t value;

  urosAssert(semp != NULL);

  pthread_mutex_lock(&semp->mutex);
  value = semp->counter;
  pthread_mutex_unlock(&semp->mutex);
  return value;
}

/** @} */

/*~~~ MUTEX ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Mutex */
/** @{ */

/**
 * @brief   Initializes a mutex.
 * @details The mutex object is initialized, and set as unlocked.
 *
 * @pre     The mutex is not initialized.
 * @post    The mutex is unlocked.
 *
 * @param[in,out] mtxp
 *          Pointer to an allocated @p UrosMutex object.
 */
void uros_lld_mutex_objectinit(UrosMutex *mtxp) {

  pthread_mutexattr_t attr;
  int err;
  (void)err;

  urosAssert(mtxp != NULL);

  pthread_mutexattr_init(&attr);
  pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
  err = pthread_mutex_init(mtxp, &attr);
  urosAssert(err == 0);
}

/**
 * @brief   Cleans a mutex.
 * @details Deallocates any memory chunks allocated by the object itself.
 *
 * @pre     The mutex is initialized.
 * @post    The mutex is not initialized, call @p urosMutexObjectInit() to
 *          use it again.
 *
 * @param[in,out] mtxp
 *          Pointer to an initialized @p UrosMutex object.
 */
void uros_lld_mutex_clean(UrosMutex *mtxp) {

  urosAssert(mtxp != NULL);

  pthread_mutex_destroy(mtxp);
}

/**
 * @brief   Locks a mutex.
 * @details Tries to lock a mutex. If the mutex is already locked, it waits
 *          until it is unlocked.
 * @note    Depending on the low-level implementation, it may support a
 *          <i>priority inversion</i> mechanism.
 *
 * @post    The mutex is locked.
 *
 * @param[in,out] mtxp
 *          Pointer to an initialized @p UrosMutex object.
 */
void uros_lld_mutex_lock(UrosMutex *mtxp) {

  urosAssert(mtxp != NULL);

  pthread_mutex_lock(mtxp);
}

/**
 * @brief   Unlocks a mutex.
 *
 * @pre     The mutex is locked.
 * @post    The mutex is unlocked.
 *
 * @param[in,out] mtxp
 *          Pointer to a locked @p UrosMutex object.
 */
void uros_lld_mutex_unlock(UrosMutex *mtxp) {

  urosAssert(mtxp != NULL);

  pthread_mutex_unlock(mtxp);
}

/** @} */

/*~~~ CONDITION VARIABLE ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Condition variable */
/** @{ */

/**
 * @brief   Initializes a condvar.
 *
 * @pre     The condvar is not initialized.
 * @post    The condvar is initialized.
 *
 * @param[in,out] cvp
 *          Pointer to an allocated @p UrosCondVar object.
 */
void uros_lld_condvar_objectinit(UrosCondVar *cvp) {

  urosAssert(cvp != NULL);

  pthread_cond_init(cvp, NULL);
}

/**
 * @brief   Cleans a condvar.
 * @details Deallocates any memory chunks allocated by the object itself.
 *
 * @pre     The condvar is initialized.
 * @post    The condvar is not initialized, call @p urosConvVarObjectInit() to
 *          use it again.
 *
 * @param[in,out] cvp
 *          Pointer to an initialized @p UrosCondVar object.
 */
void uros_lld_condvar_clean(UrosCondVar *cvp) {

  urosAssert(cvp != NULL);

  pthread_cond_destroy(cvp);
}

/**
 * @brief   Waits for a condvar signal.
 * @details Waits until the condvar is signalled.
 * @note    This procedure must be called within a lock zone guarded by a
 *          mutex, shared by the waiting and the signalling thread.
 *
 * @param[in,out] cvp
 *          Pointer to an initialized @p UrosCondVar object.
 * @param[in,out] mtxp
 *          Pointer to the mutex guarding this condvar.
 */
void uros_lld_condvar_wait(UrosCondVar *cvp, UrosMutex *mtxp) {

  urosAssert(cvp != NULL);

  pthread_cond_wait(cvp, mtxp);
}

/**
 * @brief   Single condvar signal.
 * @details Signals a condvar to a single waiting thread.
 * @note    This procedure may be called within a lock zone guarded by a mutex,
 *          shared by the waiting and the signalling thread.
 *
 * @param[in,out] cvp
 *          Pointer to an initialized @p UrosCondVar object.
 */
void uros_lld_condvar_signal(UrosCondVar *cvp) {

  urosAssert(cvp != NULL);

  pthread_cond_signal(cvp);
}

/**
 * @brief   Broadcast condvar signal.
 * @details Signals a condvar to all of the waiting threads.
 * @note    This procedure may be called within a lock zone guarded by a mutex,
 *          shared by the waiting and the signalling thread.
 *
 * @param[in,out] cvp
 *          Pointer to an initialized @p UrosCondVar object.
 */
void uros_lld_condvar_broadcast(UrosCondVar *cvp) {

  urosAssert(cvp != NULL);

  pthread_cond_broadcast(cvp);
}

/** @} */

/*~~~ THREAD ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Thread */
/** @{ */

/**
 * @brief   Gets the current thread identifier
 *
 * @return
 *          The current thread identifier.
 */
UrosThreadId uros_lld_thread_self(void) {

  return pthread_self();
}

/**
 * @brief   Gets the name of the thread.
 *
 * @param[in] id
 *          Thread identifier.
 * @return
 *          Pointer to the name of the thread string, null-terminated.
 * @retval NULL
 *          No name assigned to the thread.
 */
const char *uros_lld_thread_getname(UrosThreadId id) {

  /* TODO: Get the thread name.*/
  (void)id;
  return NULL;
}

/**
 * @brief   Creates a thread with a static stack.
 * @details Creates a new thread. The memory chunk for the stack is externally
 *          declared, and is simply referenced. It is usually allocated as a
 *          static buffer.
 *
 * @pre     The stack is big enough to avoid stack overflow.
 *
 * @param[out] idp
 *          Pointer to the @p UrosThreadId where the created thread id is
 *          stored.
 * @param[in] namep
 *          Pointer to the default thread name, valid for the whole the thread
 *          life. Null-terminated string.
 * @param[in] priority
 *          Thread priority.
 * @param[in] routine
 *          Thread routine, entry point of the thread program.
 * @param[in] argp
 *          Argument passed to the thread routine. Can be @p NULL.
 * @param[in] stackp
 *          Pointer to the externally allocated stack buffer.
 * @param[in] stacksize
 *          Stack size, in bytes.
 * @return
 *          Error code.
 */
uros_err_t uros_lld_thread_createstatic(UrosThreadId *idp, const char *namep,
                                        uros_prio_t priority,
                                        uros_proc_f routine, void *argp,
                                        void *stackp, size_t stacksize) {

  int err;
  pthread_attr_t attr;
  struct sched_param param;
  (void)namep;

  urosAssert(idp != NULL);
  urosAssert(routine != NULL);
  urosAssert(stackp != NULL);
  urosAssert(stacksize > 0);

  /* Set thread attributes.*/
  err = pthread_attr_init(&attr);
  urosError(err != 0, goto _error,
            ("Error [%s] while initializing thread attributes\n",
             strerror(err)));

  err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  urosError(err != 0, goto _error,
            ("Error [%s] while setting as joinable\n", strerror(err)));

  err = pthread_attr_setschedpolicy(&attr, SCHED_RR);
  urosError(err != 0, goto _error,
            ("Error [%s] while setting the Round-Robin scheduling policy\n",
             strerror(err)));

  param.__sched_priority = priority;
  err = pthread_attr_setschedparam(&attr, &param);
  urosError(err != 0, goto _error,
            ("Error [%s] while setting the thread priority (%d)\n",
             strerror(err), priority));

  err = pthread_attr_setstack(&attr, stackp, stacksize);
  urosError(err != 0, goto _error,
            ("Error [%s] while setting thread stack (%u bytes at 0x%.*X)\n",
             strerror(err), (unsigned)stacksize,
             (unsigned)(2*sizeof(uintptr_t)),
             (unsigned)(uintptr_t)stackp));

  /* Create the thread.*/
  err = pthread_create(idp, &attr, (void*(*)(void*))routine, (void*)argp);
  urosError(err != 0, goto _error,
            ("Error [%s] while creating thread (routine at 0x%.*X)\n",
             strerror(err), (unsigned)(2*sizeof(uintptr_t)),
             (unsigned)(uintptr_t)routine));

#ifdef __USE_GNU
  err = pthread_setname_np(*idp, namep);
  urosError(err != 0, goto _error,
            ("Error [%s] while setting the thread name [%s]\n",
             strerror(err), namep));
#endif

  pthread_attr_destroy(&attr);
  return UROS_OK;

_error:
  pthread_attr_destroy(&attr);
  return UROS_ERR_BADPARAM;
}

/**
 * @brief   Creates a thread allocating the stack from a memory pool.
 * @details Creates a new thread. The memory chunk for the stack is allocated
 *          from a memory pool.
 * @warning The thread creation is suspended until there is a free slot in the
 *          memory pool.
 *
 * @pre     The stack is big enough to avoid stack overflow.
 *
 * @param[out] idp
 *          Pointer to the @p UrosThreadId where the created thread id is
 *          stored.
 * @param[in] namep
 *          Pointer to the default thread name, valid for the whole the thread
 *          life. Null-terminated string.
 * @param[in] priority
 *          Thread priority.
 * @param[in] routine
 *          Thread routine, entry point of the thread program.
 * @param[in] argp
 *          Argument passed to the thread routine. Can be @p NULL.
 * @param[in] mempoolp
            Pointer to the memory pool where to allocate a stack.
 * @return
 *          Error code.
 */
uros_err_t uros_lld_thread_createfrommempool(UrosThreadId *idp, const char *namep,
                                             uros_prio_t priority,
                                             uros_proc_f routine, void *argp,
                                             UrosMemPool *mempoolp) {

  int err;
  void *stackp;

  urosAssert(idp != NULL);
  urosAssert(routine != NULL);
  urosAssert(mempoolp != NULL);

  stackp = urosMemPoolAlloc(mempoolp);
  if (stackp == NULL) { return UROS_ERR_NOMEM; }
  err = uros_lld_thread_createstatic(idp, namep, priority,
                                     routine, argp,
                                     stackp, mempoolp->blockSize);
  if (err == 0) { return UROS_OK; }
  else          { urosFree(stackp); return UROS_ERR_BADPARAM; }
}

/**
 * @brief   Creates a thread allocating the stack on the deafult heap.
 *
 * @pre     The stack is big enough to avoid stack overflow.
 *
 * @param[out] idp
 *          Pointer to the @p UrosThreadId where the created thread id is
 *          stored.
 * @param[in] namep
 *          Pointer to the default thread name, valid for the whole the thread
 *          life. Null-terminated string.
 * @param[in] priority
 *          Thread priority.
 * @param[in] routine
 *          Thread routine, entry point of the thread program.
 * @param[in] argp
 *          Argument passed to the thread routine. Can be @p NULL.
 * @param[in] stacksize
 *          Stack size, in bytes.
 * @return
 *          Error code.
 */
uros_err_t uros_lld_thread_createfromheap(UrosThreadId *idp, const char *namep,
                                          uros_prio_t priority,
                                          uros_proc_f routine, void *argp,
                                          size_t stacksize) {

  int err;
  void *stackp;

  urosAssert(idp != NULL);
  urosAssert(routine != NULL);
  urosAssert(stacksize > 0);

  stackp = urosAlloc(NULL, stacksize);
  if (stackp == NULL) { return UROS_ERR_NOMEM; }
  err = uros_lld_thread_createstatic(idp, namep, priority, routine, argp,
                                     stackp, stacksize);
  if (err == 0) { return UROS_OK; }
  else          { urosFree(stackp); return UROS_ERR_BADPARAM; }
}

/**
 * @brief   Joins a thread.
 * @details Waits until the required thread has exited, and releases its
 *          allocated memory.
 *
 * @post    The required thread does not exist anymore.
 * @post    Any resources allocated by the thread creation are released (e.g.
 *          stack).
 *
 * @param[in] id
 *          Identifier of the thread being joined.
 * @return
 *          Error code.
 */
uros_err_t uros_lld_thread_join(UrosThreadId id) {

  uros_err_t msg;
  int err;
  (void)err;

  err = pthread_join(id, (void**)&msg);
  urosError(err != 0, return UROS_ERR_BADPARAM,
            ("Error [%s] while joining thread id %u\n",
             strerror(err), (unsigned)id));
  return msg;
}

/**
 * @brief   Sleeps for some seconds.
 * @details Puts the thread in sleep state for the provided amount of time.
 *
 * @param[in] sec
 *          Number of seconds to sleep.
 */
void uros_lld_thread_sleepsec(uint32_t sec) {

  struct timespec to1, to2 = { 0, 0 };
  struct timespec *to1p = &to1, *to2p = &to2, *tmp;

  to1.tv_sec = sec;
  to1.tv_nsec = 0;

  while (nanosleep(to1p, to2p) == -1) {
    if (errno != EINTR) { return; }
    tmp = to1p;
    to1p = to2p;
    to2p = tmp;
  }
}

/**
 * @brief   Sleeps for some milliseconds.
 * @details Puts the thread in sleep state for the provided amount of time.
 *
 * @param[in] msec
 *          Number of milliseconds to sleep.
 */
void uros_lld_thread_sleepmsec(uint32_t msec) {

  struct timespec to1, to2 = { 0, 0 };
  struct timespec *to1p = &to1, *to2p = &to2, *tmp;

  to1.tv_sec = msec / 1000;
  to1.tv_nsec = (msec % 1000) * 1000000;

  while (nanosleep(to1p, to2p) == -1) {
    if (errno != EINTR) { return; }
    tmp = to1p;
    to1p = to2p;
    to2p = tmp;
  }
}

/**
 * @brief   Sleeps for some microseconds.
 * @details Puts the thread in sleep state for the provided amount of time.
 *
 * @param[in] usec
 *          Number of microseconds to sleep.
 */
void uros_lld_thread_sleepusec(uint32_t usec) {

  struct timespec to1, to2 = { 0, 0 };
  struct timespec *to1p = &to1, *to2p = &to2, *tmp;

  to1.tv_sec = usec / 1000000;
  to1.tv_nsec = (usec % 1000000) * 1000;

  while (nanosleep(to1p, to2p) == -1) {
    if (errno != EINTR) { return; }
    tmp = to1p;
    to1p = to2p;
    to2p = tmp;
  }
}

/**
 * @brief   Current timestamp in milliseconds.
 * @note    The resolution is in milliseconds, but the precision may not be.
 *
 * @return
 *          The current timestamp, with a resolution of one millisecond.
 */
uint32_t uros_lld_threading_gettimestampmsec(void) {

  struct timeval tv;
  int err;
  (void)err;

  err = gettimeofday(&tv, NULL);
  urosAssert(err == 0);

  return (uint32_t)tv.tv_sec * 1000 + (uint32_t)tv.tv_usec / 1000;
}

/** @} */
/** @} */
