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
 * @file    uros_lld_base.c
 * @author  Andrea Zoppi <texzk@email.it>
 *
 * @brief   Low-level basic features implementation.
 */

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include "../../../include/lld/uros_lld_base.h"

/*===========================================================================*/
/* LOCAL TYPES & MACROS                                                      */
/*===========================================================================*/

#if UROS_BASE_C_USE_ASSERT == UROS_FALSE && !defined(__DOXYGEN__)
#undef urosAssert
#define urosAssert(expr)
#endif

/*===========================================================================*/
/* GLOBAL FUNCTIONS                                                          */
/*===========================================================================*/

/** @addtogroup base_lld_funcs */
/** @{ */

/**
 * @brief   Initializes the low-level system.
 *
 * @pre     The low-level system has not been initialized yet.
 */
void uros_lld_init(void) {

  /* Nothing to initialize.*/
}

/**
 * @brief   Allocates a memory block.
 * @details This function tries to allocate a memory block of the required size
 *          inside the default heap of the operating system.
 * @see     urosAlloc()
 *
 * @pre     There is enough contiguous free space inside the default heap.
 *
 * @param[in,out] heapp
 *          Pointer to an initialized @p UrosMemHeap object, default @p NULL.
 * @param[in] size
 *          Size of the memory block to be allocated, in bytes.
 * @return
 *          The address of the allocated memory chunk.
 * @retval NULL
 *          There is not enough contiguous free memory to allocate a memory
 *          block of the requested size.
 */
void *uros_lld_alloc(UrosMemHeap *heapp, size_t size) {

  /* Allocate into the provided heap.*/
  return chHeapAlloc(heapp, size);
}

/**
 * @brief   Deallocates a memory block.
 * @see     urosFree()
 *
 * @pre     The block pointed by @p chunkp must have been allocated with
 *          @p urosAlloc().
 * @post    @p chunkp points to an invalid address.
 *
 * @param[in] chunkp
 *          Pointer to the memory block to be deallocated.
 *          A @p NULL value will simply be ignored.
 */
void uros_lld_free(void *chunkp) {

  /* Default memory block deallocation.*/
  if (chunkp != NULL) {
    chHeapFree(chunkp);
  }
}

/*~~~ MEMORY POOL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#if UROS_USE_BUILTIN_MEMPOOL == UROS_FALSE || defined(__DOXYGEN__)
/** @name Memory pool */
/** @{ */

/**
 * @brief   Initializes a memory pool ojbect.
 * @see     uros_alloc_f
 *
 * @pre     The object addressed by @p poolp is allocated but not initialized.
 *
 * @param[in,out] poolp
 *          Pointer to an allocated @p UrosMemPool object.
 * @param[in] blocksize
 *          Size of a memory pool block.
 * @param[in] allocator
 *          Pointer to an allocator function. If not @p NULL, when no blocks
 *          are free but a further one is requested, the @p allocator will make
 *          the pool grow by one element.
 */
void uros_lld_mempool_objectinit(UrosMemPool *poolp, size_t blocksize,
                                 uros_alloc_f allocator) {

  urosAssert(poolp != NULL);
  urosAssert(blocksize > 0);

  chPoolInit(poolp, blocksize, (memgetfunc_t)allocator);
}

/**
 * @brief   Allocates a memory block.
 * @details This function tries to allocate a memory block of the required size
 *          inside the default heap of the operating system.
 * @see     urosAlloc()
 *
 * @pre     There is enough contiguous free space inside the default heap.
 *
 * @param[in] size
 *          Size of the memory block to be allocated, in bytes.
 * @return
 *          The address of the allocated memory chunk.
 * @retval NULL
 *          There is not enough contiguous free memory to allocate a memory
 *          block of the requested size.
 */
void *uros_lld_mempool_alloc(UrosMemPool *poolp) {

  void *objp;

  urosAssert(poolp != NULL);

  objp = chPoolAlloc(poolp);
  if (objp != NULL) {
    /* Skip the reserved pointer.*/
    objp = (void*)((uint8_t*)objp + sizeof(void*));
  }
  return objp;
}

/**
 * @brief   Releases a memory pool block.
 * @details The caller releases a memory block, and puts it back into the
 *          memory pool.
 *
 * @pre     The block addressed by @p objp was allocated with a memory pool
 *          with a compatible block size.
 * @post    @p objp points to an invalid address.
 *
 * @param[in,out] poolp
 *          Pointer to an initialized @p UrosMemPool object.
 * @param[in] objp
 *          Pointer to a memory block previously allocated from a compatible
 *          memory pool (reserved pointer excluded).
 */
void uros_lld_mempool_free(UrosMemPool *poolp, void *objp) {

  urosAssert(poolp != NULL);
  urosAssert(objp != NULL);

  /* Include the reserved pointer.*/
  objp = (void*)((uint8_t*)objp - sizeof(void*));
  chPoolFree(poolp, objp);
}

/**
 * @brief   Gets the number of free memory pool blocks.
 *
 * @param[in] poolp
 *          Pointer to an initialized @p UrosMemPool object.
 * @return
 *          Number of free memory pool blocks.
 */
uros_cnt_t uros_lld_mempool_numfree(UrosMemPool *poolp) {

  uros_cnt_t free = 0;
  struct pool_header *curp;

  urosAssert(poolp != NULL);

  /* Return the number of free blocks.*/
  chSysLock();
  for (curp = poolp->mp_next; curp != NULL; ++free, curp = curp->ph_next) {}
  chSysUnlock();

  return free;
}

/**
 * @brief   Adds free memory pool blocks from an array.
 * @details An array of memory blocks is added to the free list of the provided
 *          memory pool.
 *
 * @pre     The number of elements @p n is positive.
 * @pre     A reserved pointer is stored at the very beginning of each array
 *          element. The remainder (payload) of each element is size-compatible
 *          with the memory pool block size.
 *
 * @param[in,out] poolp
 *          Pointer to an initialize @p UrosMemPool object.
 * @param[in] arrayp
 *          Pointer to the source array.
 * @param[in] n
 *          Number of array elements.
 */
void uros_lld_mempool_loadarray(UrosMemPool *poolp,
                                void *objp, uros_cnt_t n) {

  urosAssert(poolp != NULL);
  urosAssert(objp != NULL);

  /* NOTE: Each block needs a reserved pointer at the very beginning.*/
  chPoolLoadArray(poolp, objp, n);
}

/**
 * @brief   Gets the memory pool block size.
 *
 * @param[in] poolp
 *          Pointer to an initialized @p UrosMemPool object.
 * @return
 *          Size of a memory pool block, including the reserved pointer.
 */
size_t uros_lld_mempool_blocksize(UrosMemPool *poolp) {

  urosAssert(poolp != NULL);

  return poolp->mp_object_size;
}
/** @} */
#endif /* UROS_USE_BUILTIN_MEMPOOL == UROS_FALSE || defined(__DOXYGEN__) */

/** @} */
