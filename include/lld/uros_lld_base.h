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
 * @file    uros_lld_base.h
 * @author  Andrea Zoppi <texzk@email.it>
 *
 * @brief   Low-level basic features of the middleware.
 */

#ifndef _UROS_LLD_BASE_H_
#define _UROS_LLD_BASE_H_

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include "../urosBase.h"

/*===========================================================================*/
/* GLOBAL PROTOTYPES                                                         */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

void uros_lld_init(void);
void *uros_lld_alloc(UrosMemHeap *heapp, size_t size);
void uros_lld_free(void *chunkp);

#if UROS_USE_BUILTIN_MEMPOOL == UROS_FALSE
void uros_lld_mempool_objectinit(UrosMemPool *poolp, size_t blocksize,
                                 uros_alloc_f provider);
void *uros_lld_mempool_alloc(UrosMemPool *poolp);
void uros_lld_mempool_free(UrosMemPool *poolp, void *objp);
uros_cnt_t uros_lld_mempool_numfree(UrosMemPool *poolp);
void uros_lld_mempool_loadarray(UrosMemPool *poolp,
                                void *objp, uros_cnt_t n);
size_t uros_lld_mempool_blocksize(UrosMemPool *poolp);
#endif /* UROS_USE_BUILTIN_MEMPOOL == UROS_FALSE */

#ifdef __cplusplus
}
#endif
#endif /* _UROS_LLD_BASE_H_ */
