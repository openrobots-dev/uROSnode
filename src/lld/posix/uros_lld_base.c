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

#include <malloc.h>

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

  (void)heapp;

  urosAssert(heapp == NULL);

  /* Default memory block allocation.*/
  return malloc(size);
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
    free(chunkp);
  }
}

/** @} */
