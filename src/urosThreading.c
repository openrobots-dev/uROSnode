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
 * @file    urosThreading.c
 * @author  Andrea Zoppi <texzk@email.it>
 *
 * @brief   Threading features of the middleware.
 */

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include "../include/urosBase.h"
#include "../include/urosUser.h"
#include "../include/urosThreading.h"
#include "../include/lld/uros_lld_threading.h"

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

/** @addtogroup threading_funcs */
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
void urosSemObjectInit(UrosSem *semp, uros_cnt_t n) {

  uros_lld_sem_objectinit(semp, n);
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
void urosSemClean(UrosSem *semp) {

  uros_lld_sem_clean(semp);
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
void urosSemWait(UrosSem *semp) {

  uros_lld_sem_wait(semp);
}

/**
 * @brief   Semaphore signal.
 * @details Increments a semaphore counter.
 *
 * @param[in,out] semp
 *          Pointer to an initialized @p UrosSem object.
 */
void urosSemSignal(UrosSem *semp) {

  uros_lld_sem_signal(semp);
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
uros_cnt_t urosSemValue(UrosSem *semp) {

  return uros_lld_sem_value(semp);
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
void urosMutexObjectInit(UrosMutex *mtxp) {

  uros_lld_mutex_objectinit(mtxp);
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
void urosMutexClean(UrosMutex *mtxp) {

  uros_lld_mutex_clean(mtxp);
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
void urosMutexLock(UrosMutex *mtxp) {

  uros_lld_mutex_lock(mtxp);
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
void urosMutexUnlock(UrosMutex *mtxp) {

  uros_lld_mutex_unlock(mtxp);
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
void urosCondVarObjectInit(UrosCondVar *cvp) {

  uros_lld_condvar_objectinit(cvp);
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
void urosCondVarClean(UrosCondVar *cvp) {

  uros_lld_condvar_clean(cvp);
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
void urosCondVarWait(UrosCondVar *cvp, UrosMutex *mtxp) {

  uros_lld_condvar_wait(cvp, mtxp);
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
void urosCondVarSignal(UrosCondVar *cvp) {

  uros_lld_condvar_signal(cvp);
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
void urosCondVarBroadcast(UrosCondVar *cvp) {

  uros_lld_condvar_broadcast(cvp);
}

/** @} */

/*~~~ THREAD POOL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Thread pool */
/** @{ */

/**
 * @brief   Initializes a thread pool
 * @details The thread pool is initialized with the related attributes, and any
 *          private members.
 *
 * @pre     The thread pool is not initialized.
 *
 * @param[in,out] poolp
 *          Pointer to an allocated @p UrosThreadPool object.
 * @param[in] stackpoolp
 *          Pointer to an initialized memory pool.
 * @param[in] routine
 *          Routine of the worker threads.
 * @param[in] namep
 *          Pointer to the default thread name, valid for the whole the thread
 *          life. Null-terminated string.
 * @param[in] priority
 *          Default worker thread priority.
 * @return
 *          Error code.
 */
uros_err_t urosThreadPoolObjectInit(UrosThreadPool *poolp,
                                    UrosMemPool *stackpoolp,
                                    uros_proc_f routine,
                                    const char *namep,
                                    uros_prio_t priority) {

  urosAssert(poolp != NULL);
  urosAssert(stackpoolp != NULL);
  urosAssert(routine != NULL);

  poolp->stackPoolp = stackpoolp;
  poolp->size = urosMemPoolNumFree(stackpoolp);
  urosAssert(poolp->size > 0);
  poolp->argp = NULL;
  poolp->routine = routine;
  poolp->namep = namep;
  poolp->priority = priority;
  poolp->threadsp =
    (UrosThreadId*)urosArrayAlloc(poolp->size,
                                  urosMemPoolBlockSize(stackpoolp));
  if (poolp->threadsp != NULL) {
    unsigned i;
    for (i = 0; i < poolp->size; ++i) {
      poolp->threadsp[i] = UROS_NULL_THREADID;
    }
  }
  poolp->readyCnt = 0;
  poolp->busyCnt = 0;
  urosMutexObjectInit(&poolp->readyMtx);
  urosMutexObjectInit(&poolp->busyMtx);
  urosCondVarObjectInit(&poolp->readyCond);
  urosCondVarObjectInit(&poolp->busyCond);
  poolp->exitFlag = UROS_FALSE;

  return (poolp->threadsp != NULL) ? UROS_OK : UROS_ERR_NOMEM;
}

/**
 * @brief   Cleans a thread pool.
 * @details Deinitializes private members, and deallocates any allocated
 *          objects.
 * @note    The related memory pool is not deallocated nor uninitialized.
 *
 * @pre     The thread pool is initialized.
 * @pre     No worker thread is running.
 * @post    The thread pool is not initialized, call @p urosThreadPoolObjectInit()
 *          to use it again.
 *
 * @param[in,out] poolp
 *          Pointer to an initialized memory pool.
 */
void urosThreadPoolClean(UrosThreadPool *poolp) {

  urosAssert(poolp != NULL);

  poolp->stackPoolp = NULL;
  poolp->size = 0;
  poolp->argp = NULL;
  poolp->routine = NULL;
  urosFree(poolp->threadsp);
  poolp->readyCnt = 0;
  poolp->busyCnt = 0;
  urosMutexClean(&poolp->readyMtx);
  urosMutexClean(&poolp->busyMtx);
  urosCondVarClean(&poolp->readyCond);
  urosCondVarClean(&poolp->busyCond);
  poolp->exitFlag = UROS_FALSE;
}

/**
 * @brief   Creates all the thread pool threads.
 * @details The thread pool is filled with new worker threads. The fucntion
 *          waits until all the worker threads are ready.
 * @see     urosThreadPoolJoinAll()
 * @see     urosThreadPoolClean()
 *
 * @pre     The thread pools has no allocated worker threads.
 * @post    All the worker threads are ready.
 *
 * @param[in,out] poolp
 *          Pointer to an initialized @p UrosThreadPool object.
 * @return
 *          Error code.
 */
uros_err_t urosThreadPoolCreateAll(UrosThreadPool *poolp) {

  uros_cnt_t i;
  uros_err_t err;

  urosAssert(poolp != NULL);
  urosAssert(poolp->stackPoolp != NULL);
  urosAssert(poolp->threadsp != NULL);
  urosAssert(poolp->size > 0);

  /* Create the threads from the memory pool.*/
  poolp->exitFlag = UROS_FALSE;
  for (i = 0; i < poolp->size; ++i) {
    err = urosThreadCreateFromMemPool(&poolp->threadsp[i], poolp->namep,
                                      poolp->priority,
                                      (uros_proc_f)urosThreadPoolWorkerThread,
                                      (void*)poolp, poolp->stackPoolp);
    urosAssert(err != UROS_ERR_NOMEM);
    urosError(err != UROS_OK, return err,
              ("Error [%s] while creating thread %d (of %d) in pool\n",
               strerror(err), i, poolp->size));
  }

  /* Wait until all the threads are ready.*/
  urosMutexLock(&poolp->readyMtx);
  while (poolp->readyCnt < poolp->size) {
    urosCondVarWait(&poolp->readyCond, &poolp->readyMtx);
  }
  urosMutexUnlock(&poolp->readyMtx);
  return UROS_OK;
}

/**
 * brief    Joins all the worker threads.
 * @details Waits for all the worker threads to terminate.
 * @see     urosThreadPoolClean()
 *
 * @pre     The worker threads are created.
 * @post    The worker threads have terminated and cannot be started again.
 *
 * @param[in,out] poolp
 *          Pointer to an initialized @p UrosThreadPool object.
 * @return
 *          Error code.
 */
uros_err_t urosThreadPoolJoinAll(UrosThreadPool *poolp) {

  uros_cnt_t i;

  urosAssert(poolp != NULL);
  urosAssert(poolp->stackPoolp != NULL);
  urosAssert(!(poolp->size > 0) || (poolp->threadsp != NULL));

  /* Wait for all the running threads to terminate.*/
  urosMutexLock(&poolp->readyMtx);
  while (poolp->readyCnt < poolp->size) {
    urosCondVarWait(&poolp->readyCond, &poolp->readyMtx);
  }
  urosMutexUnlock(&poolp->readyMtx);

  /* Restart all the threads with the exit flag set.*/
  urosMutexLock(&poolp->busyMtx);
  poolp->exitFlag = UROS_TRUE;
  poolp->busyCnt = poolp->size;
  urosCondVarBroadcast(&poolp->busyCond);
  urosMutexUnlock(&poolp->busyMtx);

  /* Wait until all the threads have exited.*/
  urosMutexLock(&poolp->readyMtx);
  while (poolp->readyCnt < poolp->size) {
    urosCondVarWait(&poolp->readyCond, &poolp->readyMtx);
  }
  urosMutexUnlock(&poolp->readyMtx);
  for (i = 0; i < poolp->size; ++i) {
    urosThreadJoin(poolp->threadsp[i]);
    poolp->threadsp[i] = UROS_NULL_THREADID;
  }
  return UROS_OK;
}

/**
 * @brief   Starts a worker thread.
 * @details Waits until there is a ready worker thread in the pool, selects it,
 *          passes the argument provided by the user to it, and lets it run.
 *
 * @pre     The worker threads are created.
 * @post    The worker thread is running the routine assigne to the pool, with
 *          the argument provided by the user.
 *
 * @param[in,out] poolp
 *          Pointer to an initialized @p UrosThreadPool object.
 * @param[in] argp
 *          Argument passed to the thread routine.
 * @return
 *          Error code.
 */
uros_err_t urosThreadPoolStartWorker(UrosThreadPool *poolp, void *argp) {

  urosAssert(poolp != NULL);
  urosAssert(poolp->stackPoolp != NULL);

  /* Wait for a free thread.*/
  urosMutexLock(&poolp->readyMtx);
  while (poolp->readyCnt == 0) {
    urosCondVarWait(&poolp->readyCond, &poolp->readyMtx);
  }
  --poolp->readyCnt;
  urosMutexUnlock(&poolp->readyMtx);

  /* Start a thread with the given argument pointer.*/
  urosMutexLock(&poolp->busyMtx);
  poolp->argp = argp;
  ++poolp->busyCnt;
  urosCondVarSignal(&poolp->busyCond);
  urosMutexUnlock(&poolp->busyMtx);
  return UROS_OK;
}

/**
 * @brief   Worker thread.
 * @details This is the actual thread function created by @p urosThreadPoolCreateAll().
 *          After creation, it waits until the user starts this thread by
 *          calling @p urosThreadPoolStartWorker(), when this is the first
 *          ready thread selected.
 *
 *          After executing the routine defined with urosThreadPoolObjectInit(),
 *          this thread goes back into the waiting state.
 *
 *          To exit from this thread, it first has to be put back into the
 *          waiting state. Then, the @p exitFlag of the pool must be set to
 *          @p true, so that by calling @p urosThreadPoolStartWorker() again
 *          on this thread, the latter knows it must exit and not execute the
 *          user routine.
 *
 * @param[in] poolp
 *          Pointer to the @p UrosThreadPool which created this thread.
 * @return
 *          Error code returned by the user routine.
 */
uros_err_t urosThreadPoolWorkerThread(UrosThreadPool *poolp) {

  uros_bool_t exit;
  uros_err_t err;
  void *argp;

  urosAssert(poolp != NULL);
  urosAssert(poolp->routine != NULL);

  /* Notify the creation of a thread.*/
  urosMutexLock(&poolp->readyMtx);
  ++poolp->readyCnt;
  urosCondVarSignal(&poolp->readyCond);
  urosMutexUnlock(&poolp->readyMtx);

  do {
    /* Wait for the user routine to start with the provided argument pointer.*/
    urosMutexLock(&poolp->busyMtx);
    while (poolp->busyCnt == 0) {
      urosCondVarWait(&poolp->busyCond, &poolp->busyMtx);
    }
    --poolp->busyCnt;
    argp = poolp->argp;
    exit = poolp->exitFlag;
    urosMutexUnlock(&poolp->busyMtx);
    if (!exit) {
      /* Launch the wrapped user thread routine.*/
      urosAssert(poolp->routine != NULL);
      err = poolp->routine(argp);
    } else {
      err = UROS_OK;
    }

    /* Notify the release of this thread.*/
    urosMutexLock(&poolp->readyMtx);
    ++poolp->readyCnt;
    urosCondVarSignal(&poolp->readyCond);
    urosMutexUnlock(&poolp->readyMtx);
  } while (!exit);
  return err;
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
UrosThreadId urosThreadSelf(void) {

  return uros_lld_thread_self();
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
const char *urosThreadGetName(UrosThreadId id) {

  return uros_lld_thread_getname(id);
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
uros_err_t urosThreadCreateStatic(UrosThreadId *idp, const char *namep,
                                  uros_prio_t priority,
                                  uros_proc_f routine, void *argp,
                                  void *stackp, size_t stacksize) {

  uros_err_t err;
  err = uros_lld_thread_createstatic(idp, namep, priority,
                                     routine, argp,
                                     stackp, stacksize);
  urosError(err != UROS_OK, UROS_NOP,
            ("Error %s while creating a new static thread\n",
             urosErrorText(err)));
  return UROS_OK;
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
uros_err_t urosThreadCreateFromMemPool(UrosThreadId *idp, const char *namep,
                                       uros_prio_t priority,
                                       uros_proc_f routine, void *argp,
                                       UrosMemPool *mempoolp) {

  uros_err_t err;
  err = uros_lld_thread_createfrommempool(idp, namep, priority,
                                           routine, argp,
                                           mempoolp);
  urosError(err != UROS_OK, UROS_NOP,
            ("Error %s while creating a new static thread\n",
             urosErrorText(err)));
  return UROS_OK;
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
uros_err_t urosThreadCreateFromHeap(UrosThreadId *idp, const char *namep,
                                    uros_prio_t priority,
                                    uros_proc_f routine, void *argp,
                                    size_t stacksize) {

  uros_err_t err;
  err = uros_lld_thread_createfromheap(idp, namep, priority,
                                        routine, argp,
                                        stacksize);
  urosError(err != UROS_OK, UROS_NOP,
            ("Error %s while creating a new thread from the heap\n",
             urosErrorText(err)));
  return UROS_OK;
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
uros_err_t urosThreadJoin(UrosThreadId id) {

  uros_err_t err;
  err = uros_lld_thread_join(id);
  urosError(err != UROS_OK, UROS_NOP,
            ("Error %s while joining the thread %p\n",
             urosErrorText(err), (uintptr_t)id));
  return UROS_OK;
}

/**
 * @brief   Sleeps for some seconds.
 * @details Puts the thread in sleep state for the provided amount of time.
 *
 * @param[in] sec
 *          Number of seconds to sleep.
 */
void urosThreadSleepSec(uint32_t sec) {

  uros_lld_thread_sleepsec(sec);
}

/**
 * @brief   Sleeps for some milliseconds.
 * @details Puts the thread in sleep state for the provided amount of time.
 *
 * @param[in] msec
 *          Number of milliseconds to sleep.
 */
void urosThreadSleepMsec(uint32_t msec) {

  uros_lld_thread_sleepmsec(msec);
}

/**
 * @brief   Sleeps for some microseconds.
 * @details Puts the thread in sleep state for the provided amount of time.
 *
 * @param[in] usec
 *          Number of microseconds to sleep.
 */
void urosThreadSleepUsec(uint32_t usec) {

  uros_lld_thread_sleepusec(usec);
}

/**
 * @brief   Current timestamp in milliseconds.
 * @note    The resolution is in milliseconds, but the precision may not be.
 *
 * @return
 *          The current timestamp, with a resolution of one millisecond.
 */
uint32_t urosGetTimestampMsec(void) {

  return uros_lld_threading_gettimestampmsec();
}

/** @} */
/** @} */
