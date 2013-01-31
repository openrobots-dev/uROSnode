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
 * @file    urosBase.c
 * @author  Andrea Zoppi <texzk@email.it>
 *
 * @brief   Basic features of the middleware.
 */

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include "../include/urosBase.h"
#include "../include/urosUser.h"
#include "../include/urosNode.h"
#include "../include/lld/uros_lld_base.h"

#include <string.h>

/*===========================================================================*/
/* LOCAL TYPES & MACROS                                                      */
/*===========================================================================*/

#if UROS_BASE_C_USE_ASSERT == UROS_FALSE && !defined(__DOXYGEN__)
#undef urosAssert
#define urosAssert(expr)
#endif

#if UROS_BASE_C_USE_ERROR_MSG == UROS_FALSE && !defined(__DOXYGEN__)
#undef urosError
#define urosError(when, action, msgargs) { if (when) { action; } }
#endif

/*===========================================================================*/
/* GLOBAL VARIABLES                                                          */
/*===========================================================================*/

/**
 * @brief   List of supported message types.
 * @note    This list has to be initialized at system startup, and not
 *          changed anymore.
 */
UrosList urosMsgTypeList;

/**
 * @brief   List of supported service types.
 * @note    This list has to be initialized at system startup, and not
 *          changed anymore.
 */
UrosList urosSrvTypeList;

/** @brief Null topic flags.*/
const uros_topicflags_t uros_nulltopicflags =
  { UROS_FALSE, UROS_FALSE, UROS_FALSE, UROS_FALSE, UROS_FALSE, UROS_FALSE };

/** @brief Null service flags.*/
const uros_topicflags_t uros_nullserviceflags =
  { UROS_TRUE, UROS_FALSE, UROS_FALSE, UROS_FALSE, UROS_FALSE, UROS_FALSE };

/*===========================================================================*/
/* GLOBAL FUNCTIONS                                                          */
/*===========================================================================*/

/** @addtogroup base_funcs */
/** @{ */

/*~~~ SYSTEM ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name System */
/** @{ */

/**
 * @brief   Middleware initialization.
 * @details This procedure initializes the middleware at system start-up.
 *
 * @pre     The operating system is working.
 * @pre     The middleware was not initialized.
 * @post    The @p urosNode singleton is initialized and configured.
 * @post    Static types are registered.
 * @post    The thread pools are filled with ready threads.
 */
void urosInit(void) {

  /* Initialize low-level services and data.*/
  uros_lld_init();

  /* Initialize the Node module.*/
  urosNodeObjectInit(&urosNode);
  urosUserNodeConfigLoad((UrosNodeConfig*)&urosNode.config);

  /* Register static types.*/
  urosListObjectInit(&urosMsgTypeList);
  urosListObjectInit(&urosSrvTypeList);
  urosUserRegisterStaticTypes();
}

/**
 * @brief   Text of an error code.
 *
 * @param[in] err
 *          Error code.
 * @return
 *          Text of the error code.
 */
const char *urosErrorText(uros_err_t err) {

#define _CASE(id)   case id: return UROS_STRINGIFY(id);
  switch (err) {
  _CASE(UROS_OK)
  _CASE(UROS_ERR_TIMEOUT)
  _CASE(UROS_ERR_NOMEM)
  _CASE(UROS_ERR_PARSE)
  _CASE(UROS_ERR_EOF)
  _CASE(UROS_ERR_BADPARAM)
  _CASE(UROS_ERR_NOCONN)
  _CASE(UROS_ERR_BADCONN)
  _CASE(UROS_ERR_NOTIMPL)
  default:
    return "UROS_ERR__UNKNOWN";
  }
#undef _CASE
}

/**
 * @brief   Allocates a memory block.
 * @details This function tries to allocate a memory block of the required size
 *          inside the default heap of the operating system.
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
void *urosAlloc(UrosMemHeap *heapp, size_t size) {

#if UROS_USE_ERROR_MSG && UROS_BASE_C_USE_ERROR_MSG
  void *chunk = uros_lld_alloc(heapp, size);
  urosError(chunk == NULL, return NULL,
            ("Not enough free heap memory to allocate %u bytes\n",
             (unsigned)size));
  return chunk;
#else
  return uros_lld_alloc(heapp, size);
#endif
}

/**
 * @brief   Deallocates a memory block.
 *
 * @pre     The block addressed by @p chunkp must have been allocated with
 *          @p urosAlloc().
 * @post    @p chunkp points to an invalid address.
 *
 * @param[in] chunkp
 *          Pointer to the memory block to be deallocated.
 *          A @p NULL value will simply be ignored.
 */
void urosFree(void *chunkp) {

  uros_lld_free(chunkp);
}

/** @} */

/*~~~ MEMORY POOL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

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
void urosMemPoolObjectInit(UrosMemPool *poolp, size_t blocksize,
                           uros_alloc_f allocator) {

#if UROS_USE_BUILTIN_MEMPOOL != UROS_FALSE
  urosAssert(poolp != NULL);
  urosAssert(blocksize > sizeof(void*));

  poolp->blockSize = blocksize;
  poolp->allocator = allocator;
  poolp->headp = NULL;
  poolp->free = 0;
  urosMutexObjectInit(&poolp->lock);
#else
  uros_lld_mempool_objectinit(poolp, blocksize, allocator);
#endif
}

/**
 * @brief   Requests a free block from a memory pool.
 * @details This function gets a free memory block from the memory pool, and
 *          returns its pointer. If there are no free blocks but an
 *          @p allocator was provided, then it tries to allocate a further
 *          block and return its pointer.
 * @see     uros_alloc_f
 *
 * @param[in,out] poolp
 *          Pointer to an initialized @p UrosMemPool object.
 * @return
 *          Pointer to the allocated memory block, excluding the reserved
 *          pointer.
 * @retval NULL
 *          There are no free blocks inside the memory pool. Additionally,
 *          if the @p allocator was defined, it cannot allocate a further
 *          block and make the pool grow by one element.
 */
void *urosMemPoolAlloc(UrosMemPool *poolp) {

#if UROS_USE_BUILTIN_MEMPOOL != UROS_FALSE
  void *datap = NULL;

  urosAssert(poolp != NULL);

  urosMutexLock(&poolp->lock);
  if (poolp->free > 0) {
    /* There is a free block, get it.*/
    --poolp->free;
    datap = poolp->headp;
    poolp->headp = *(void**)datap;

    /* Skip the reserved pointer.*/
    datap = (void*)((uint8_t*)datap + sizeof(void*));
  } else if (poolp->allocator != NULL) {
    /* Allocate a new block.*/
    datap = poolp->allocator(poolp->blockSize);
    if (datap != NULL) {
      /* Skip the reserved pointer.*/
      datap = (void*)((uint8_t*)datap + sizeof(void*));
    }
  }
  urosMutexUnlock(&poolp->lock);

  return datap;
#else
  return uros_lld_mempool_alloc(poolp);
#endif
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
void urosMemPoolFree(UrosMemPool *poolp, void *objp) {

#if UROS_USE_BUILTIN_MEMPOOL != UROS_FALSE
  urosAssert(poolp != NULL);
  urosAssert(objp != NULL);

  /* Include the reserved pointer.*/
  objp = (void*)((uint8_t*)objp - sizeof(void*));

  /* Set as the first free block available.*/
  urosMutexLock(&poolp->lock);
  *(void**)objp = poolp->headp;
  poolp->headp = objp;
  ++poolp->free;
  urosMutexUnlock(&poolp->lock);
#else
  uros_lld_mempool_free(poolp, objp);
#endif
}

/**
 * @brief   Gets the number of free memory pool blocks.
 *
 * @param[in] poolp
 *          Pointer to an initialized @p UrosMemPool object.
 * @return
 *          Number of free memory pool blocks.
 */
uros_cnt_t urosMemPoolNumFree(UrosMemPool *poolp) {

#if UROS_USE_BUILTIN_MEMPOOL != UROS_FALSE
  uros_cnt_t free;

  urosAssert(poolp != NULL);

  urosMutexLock(&poolp->lock);
  free = poolp->free;
  urosMutexUnlock(&poolp->lock);
  return free;
#else
  return uros_lld_mempool_numfree(poolp);
#endif
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
void urosMemPoolLoadArray(UrosMemPool *poolp, void *arrayp, uros_cnt_t n) {

#if UROS_USE_BUILTIN_MEMPOOL != UROS_FALSE
  uros_cnt_t i;

  urosAssert(poolp != NULL);
  urosAssert(arrayp != NULL);
  urosAssert(n > 0);

  /* Add each block to the pool.*/
  for (i = 0; i < n; ++i) {
    void *blockp = (void*)((uint8_t*)arrayp + i * poolp->blockSize +
                           sizeof(void*));
    urosMemPoolFree(poolp, blockp);
  }
#else
  uros_lld_mempool_loadarray(poolp, arrayp, n);
#endif
}

/**
 * @brief   Gets the memory pool block size.
 *
 * @param[in] poolp
 *          Pointer to an initialized @p UrosMemPool object.
 * @return
 *          Size of a memory pool block, including the reserved pointer.
 */
size_t urosMemPoolBlockSize(UrosMemPool *poolp) {

#if UROS_USE_BUILTIN_MEMPOOL != UROS_FALSE
  urosAssert(poolp != NULL);

  return poolp->blockSize;
#else
  return uros_lld_mempool_blocksize(poolp);
#endif
}

/** @} */

/*~~~ STRING ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name String */
/** @{ */

/**
 * @brief   Initializes a string object.
 * @details The object is set as an empty string.
 *
 * @pre     The object addressed by @p strp is allocated but not initialized.
 * @post    The object has @p 0 (zero) length and @p NULL data.
 *
 * @param[in,out] strp
 *          Pointer to an allocated @p UrosString object.
 */
void urosStringObjectInit(UrosString *strp) {

  urosAssert(strp != NULL);

  strp->length = 0;
  strp->datap = NULL;
}

/**
 * @brief   String pointing to a memory chunk.
 * @details Addresses the generated string descriptor to a memory chunk.
 *
 * @param[in] datap
 *          Pointer to a valid memory address.
 * @param[in] datalen
 *          Length of the string to be copied.
 * @return
 *          The string object (descriptor).
 */
UrosString urosStringAssignN(const char *datap, size_t datalen) {

  UrosString str;

  urosAssert(!(datalen > 0) || (datap != NULL));

  str.length = datalen;
  str.datap = (char*)datap;
  return str;
}

/**
 * @brief   The string object points to a memory chunk.
 * @details Addresses the generated string descriptor to a null-terminated
 *          string.
 *
 * @param[in] szp
 *          Pointer to a null-terminated string. Can be @p NULL.
 * @return
 *          The string object (descriptor).
 */
UrosString urosStringAssignZ(const char *szp){

  UrosString str;

  if (szp != NULL) {
    str.length = strlen(szp);
    str.datap = (char*)szp;
  } else {
    str.length = 0;
    str.datap = NULL;
  }
  return str;
}

/**
 * @brief   Clones a string object.
 * @details Duplicates the contents from an existing @p UrosString object and
 *          returns the cloned object.
 *
 * @param[in] strp
 *          Pointer to an initialized @p UrosString object. Can be @p NULL.
 * @return
 *          The cloned string object (descriptor).
 * @retval NULL
 *          Not enough free memory.
 */
UrosString urosStringClone(const UrosString *strp) {

  UrosString clone;

  if (strp != NULL) {
    clone.length = strp->length;
    if (clone.length > 0) {
      clone.datap = (char*)urosAlloc(NULL, clone.length);
      urosAssert(clone.datap != NULL);
      memcpy(clone.datap, strp->datap, clone.length);
    } else {
      clone.datap = NULL;
    }
  } else {
    clone.length = 0;
    clone.datap = NULL;
  }
  return clone;
}

/**
 * @brief   Clones a string object from a memory chunk.
 * @details Duplicates the contents from an addressed memory chunk.
 *
 * @param[in] datap
 *          Pointer to a valid memory address.
 * @param[in] datalen
 *          Length of the string to be copied.
 * @return
 *          The cloned string object (descriptor).
 * @retval NULL
 *          Not enough free memory.
 */
UrosString urosStringCloneN(const char *datap, size_t datalen) {

  UrosString clone;

  urosAssert(!(datalen > 0) || datap != NULL);

  if (datalen > 0) {
    clone.length = datalen;
    clone.datap = (char*)urosAlloc(NULL, datalen);
    urosAssert(clone.datap != NULL);
    memcpy(clone.datap, datap, datalen);
  } else {
    clone.length = 0;
    clone.datap = NULL;
  }
  return clone;
}

/**
 * @brief   Clones a string object from a null-terminated string.
 * @details Duplicates the contents from a null-terminated string.
 *
 * @param[in] szp
 *          Pointer to a null-terminated string. Can be @p NULL.
 * @return
 *          The cloned string object (descriptor).
 * @retval NULL
 *          Not enough free memory.
 */
UrosString urosStringCloneZ(const char *szp) {

  UrosString clone;

  if (szp != NULL) {
    clone.length = strlen(szp);
    if (clone.length > 0) {
      clone.datap = (char*)urosAlloc(NULL, clone.length);
      urosAssert(clone.datap != NULL);
      memcpy(clone.datap, szp, clone.length);
    } else {
      clone.datap = NULL;
    }
  } else {
    clone.length = 0;
    clone.datap = NULL;
  }
  return clone;
}

/**
 * @brief   Cleans a string object.
 * @details Deallocates the memory chunk of the string data, and invalidates the
 *          string.
 *
 * @pre     The string data must have been allocated with @p urosAlloc().
 * @post    @p strp points to an empty @p UrosString string object.
 *
 * @param[in,out] strp
 *          Pointer to an initialized @p UrosString object.
 */
void urosStringClean(UrosString *strp) {

  urosAssert(strp != NULL);

  strp->length = 0;
  if (strp->datap != NULL) {
    urosFree(strp->datap);
    strp->datap = NULL;
  }
}

/**
 * @brief   Deallocates a string object.
 * @details Deallocates the memory chunk of the string data, and the object
 *          itself.
 *
 * @pre     The string object must have been allocated with @p urosAlloc().
 * @pre     The string data must have been allocated with @p urosAlloc().
 * @post    @p strp points to an invalid address.
 *
 * @param[in] strp
 *          Pointer to an initialized @p UrosString object, or @p NULL.
 */
void urosStringDelete(UrosString *strp) {

  if (strp != NULL) {
    urosFree(strp->datap);
    urosFree(strp);
  }
}

/**
 * @brief   Checks if pointing to a valid string object.
 * @details Checks is the addressed object points to a valid memory address,
 *          and its descriptor is consistent.
 *
 * @note    A string object addressed by @p strp is consistent when:
 *          <pre>(strp->length == 0) == (strp->datap == NULL)</pre>
 *
 * @param[in] strp
 *          Pointer to a presumed consistent @p UrosString object.
 * @return
 *          @p true when @p strp is not @p NULL and the string descriptor is
 *          consistent.
 */
uros_bool_t urosStringIsValid(const UrosString *strp) {

  return strp != NULL && ((strp->length == 0) == (strp->datap == NULL));
}

/**
 * @brief   Checks if a string is not empty.
 * @details Checks if the addressed string is a consistent, non-empty string.
 * @see     urosStringIsValid
 *
 * @param[in] strp
 *          Pointer to a presumed @p UrosString object.
 * @return
 *          @p true when @p strp is not @p NULL and the string is not
 *          empty.
 */
uros_bool_t urosStringNotEmpty(const UrosString *strp) {

  return strp != NULL && strp->length > 0 && strp->datap != NULL;
}

/**
 * @brief   Compares two string objects.
 * @details This function starts comparing the first character of each string.
 *          If they are equal to each other, it continues with the following
 *          pairs until the characters differ or until there are no more
 *          characters in one string.
 * @note    This is functionally equivalent to @p strcmp(), but involves
 *          @p UrosString objects instead of null-terminated strings.
 * @note    Two empty strings are equal.
 * @note    If a string is empty but the other is not, then the first character
 *          is the returned value (with the appropriate sign).
 *
 * @param[in] str1
 *          Pointer to the first @p UrosString operand, valid string.
 * @param[in] str2
 *          Pointer to the second @p UrosString operand, valid string.
 * @return  Integral value indicating the relationship between the strings.
 * @retval 0
 *          Both strings are equal.
 * @retval positive
 *          The first character that does not match has a greater value in
 *          @p str1 than in @p str2.
 * @retval negative
 *          The first character that does not match has a lesser value in
 *          @p str1 than in @p str2.
 */
int urosStringCmp(const UrosString *str1, const UrosString *str2) {

  size_t len1, len2;
  const char *ptr1, *ptr2;

  urosAssert(urosStringIsValid(str1));
  urosAssert(urosStringIsValid(str2));

  len1 = str1->length;
  len2 = str2->length;
  ptr1 = str1->datap;
  ptr2 = str2->datap;
  if      (len1 == 0) { return (len2 > 0) ? *ptr2 : 0; }
  else if (len2 == 0) { return (len1 > 0) ? *ptr1 : 0; }
  if (len1 == len2) {
    while (*ptr1 == *ptr2 && --len1 > 0) {
      ++ptr1; ++ptr2;
    }
    return (int)*ptr1 - (int)*ptr2;
  } else if (len1 > len2) {
    return (int)ptr1[len2];
  } else {
    return (int)ptr2[len1];
  }
}

/** @} */

/*~~~ MESSAGE TYPE ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Message type */
/** @{ */

/**
 * @brief   Initializes a message type descriptor.
 * @details The object is set as an empty message type descriptor.
 *
 * @pre     The object addressed by @p typep is allocated but not initialized.
 * @post    The fields of the object are all empty strings.
 *
 * @param[in,out] typep
 *          Pointer to an allocated @p UrosMsgType object.
 */
void urosMsgTypeObjectInit(UrosMsgType *typep) {

  urosAssert(typep != NULL);

  urosStringObjectInit(&typep->name);
  urosStringObjectInit(&typep->desc);
  urosStringObjectInit(&typep->md5str);
}

/**
 * @brief   Cleans a message type descriptor.
 * @details Deallocates all the memory chunks allocated by the fields of the
 *          object.
 *
 * @pre     The string data must have been allocated with @p urosAlloc().
 * @post    The fields of the object are deallocated, strings are empty.
 * @post    @p typep points to an empty @p UrosMsgType descriptor.
 *
 * @param[in,out] typep
 *          Pointer to an initialized @p UrosMsgType object.
 */
void urosMsgTypeClean(UrosMsgType *typep) {

  urosAssert(typep != NULL);

  urosStringClean(&typep->name);
  urosStringClean(&typep->desc);
  urosStringClean(&typep->md5str);
}

/**
 * @brief   Deallocates a message type descriptor.
 * @details Deallocates the memory chunk of the object and all its fields.
 *
 * @pre     The object and its fields must have been allocated with
 *          @p urosAlloc().
 * @post    @p typep points to an invalid address.
 *
 * @param[in] typep
 *          Pointer to an initialized @p UrosMsgType object, or @p NULL.
 */
void urosMsgTypeDelete(UrosMsgType *typep) {

  if (typep != NULL) {
    urosMsgTypeClean(typep);
    urosFree(typep);
  }
}

/**
 * @brief   Checks if the list node links to a message type with the requested
 *          name.
 * @see     urosStringCmp()
 *
 * @param[in] nodep
 *          Pointer to an initialized @p UrosListNode which points to an
 *          initialized @p UrosMsgType descriptor.
 * @param[in] namep
 *          Pointer to a non-empty @p UrosString.
 * @return
 *          @p true <i>iif</i> the message type of the node has the requested
 *          name.
 */
uros_bool_t urosMsgTypeNodeHasName(const UrosListNode *nodep,
                                   const UrosString *namep) {

  return 0 == urosStringCmp(&((const UrosMsgType *)nodep->datap)->name,
                            namep);
}

/**
 * @brief   Registers a message type to the static global list.
 * @note    Best practice to statically register all the message types at boot
 *          time.
 * @see     urosMsgTypeList
 *
 * @pre     No message type was registered with the same name before.
 * @pre     No publishers nor subscribers have been queried before, so that
 *          the message types list is still consistent.
 *
 * @param[in] namep
 *          Type name, not empty. Its contents must be valid for the whole
 *          life of the program.
 * @param[in] descp
 *          Long description, can be @p NULL. Its contents must be valid for
 *          the whole life of the program.
 * @param[in] md5sump
 *          MD5 sum of the type description, not empty. Its contents must be
 *          valid for the whole life of the program.
 */
void urosRegisterStaticMsgType(const UrosString *namep,
                               const UrosString *descp,
                               const UrosString *md5sump) {

  UrosMsgType *typep;
  UrosListNode *nodep;

  urosAssert(urosStringNotEmpty(namep));
  urosAssert(urosStringNotEmpty(md5sump));

  typep = urosNew(NULL, UrosMsgType);
  urosAssert(typep != NULL);
  typep->name = *namep;
  typep->desc = *descp;
  typep->md5str = *md5sump;

  nodep = urosNew(NULL, UrosListNode);
  urosAssert(nodep != NULL);
  urosListNodeObjectInit(nodep);
  nodep->datap = typep;
  urosListAdd(&urosMsgTypeList, nodep);
}

/**
 * @brief   Registers a message type to the static global list.
 * @note    Best practice to statically register all the message types at boot
 *          time.
 * @see     urosMsgTypeList
 *
 * @pre     No message type was registered with the same name before.
 * @pre     No publishers nor subscribers have been queried before, so that
 *          the message types list is still consistent.
 *
 * @param[in] namep
 *          Type name, null-terminated string, not empty. Its contents must be
 *          valid for the whole life of the program.
 * @param[in] descp
 *          Long description, null-terminated string, can be @p NULL. Its
 *          contents must be valid for the whole life of the program.
 * @param[in] md5sump
 *          MD5 sum of the type description, null-terminated string, not empty.
 *          Its contents must be valid for the whole life of the program.
 */
void urosRegisterStaticMsgTypeSZ(const char *namep,
                                 const char *descp,
                                 const char *md5sump) {

  UrosMsgType *typep;
  UrosListNode *nodep;

  urosAssert(namep != NULL);
  urosAssert(namep[0] != 0);
  urosAssert(md5sump != NULL);
  urosAssert(md5sump[0] != 0);

  typep = urosNew(NULL, UrosMsgType);
  urosAssert(typep != NULL);
  typep->name = urosStringAssignZ(namep);
  typep->desc = urosStringAssignZ(descp);
  typep->md5str = urosStringAssignZ(md5sump);

  nodep = urosNew(NULL, UrosListNode);
  urosAssert(nodep != NULL);
  urosListNodeObjectInit(nodep);
  nodep->datap = typep;
  urosListAdd(&urosMsgTypeList, nodep);
}

/**
 * @brief   Gets the message type descriptor with the requested name.
 * @details Scans the message type register to get the address of the one with
 *          the requested name, if existing.
 * @note    This function is not thread safe, because all the message types are
 *          supposed to be registered only at boot time.
 * @see     urosMsgTypeList
 * @see     urosListFind
 * @see     urosMsgTypeHasName
 *
 * @param[in] namep
 *          Requested type name, not empty.
 * @return
 *          Pointer to the registered message type descriptor with the
 *          requested name.
 * @retval NULL
 *          No such type name found.
 */
const UrosMsgType *urosFindStaticMsgType(const UrosString *namep) {

  const UrosListNode *foundp;

  urosAssert(urosStringNotEmpty(namep));

  foundp = urosListFind(&urosMsgTypeList,
                        (uros_cmp_f)urosMsgTypeNodeHasName, namep);
  return (foundp != NULL) ? (const UrosMsgType *)foundp->datap : NULL;
}

/**
 * @brief   Gets the message type descriptor with the requested name.
 * @details Scans the message type register to get the address of the one with
 *          the requested name, if existing.
 * @note    This function is not thread safe, because all the message types are
 *          supposed to be registered only at boot time.
 * @see     urosMsgTypeList
 * @see     urosListFind
 * @see     urosMsgTypeHasName
 *
 * @param[in] namep
 *          Requested type name, null-terminated string, @p NULL forbidden.
 * @return
 *          Pointer to the registered message type descriptor with the
 *          requested name.
 * @retval NULL
 *          No such type name found.
 */
const UrosMsgType *urosFindStaticMsgTypeSZ(const char *namep) {

  UrosString namestr;
  const UrosListNode *foundp;

  urosAssert(namep != NULL);
  urosAssert(namep[0] != 0);

  namestr = urosStringAssignZ(namep);
  foundp = urosListFind(&urosMsgTypeList,
                        (uros_cmp_f)urosMsgTypeNodeHasName, &namestr);
  return (foundp != NULL) ? (const UrosMsgType *)foundp->datap : NULL;
}

/** @} */

/*~~~ SERVICE TYPE ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Service type */
/** @{ */

/**
 * @brief   Registers a service type to the static global list.
 * @note    Best practice to statically register all the service types at boot
 *          time.
 * @see     urosMsgTypeList
 *
 * @pre     No service type was registered with the same name before.
 * @pre     No publishers nor subscribers have been queried before, so that
 *          the service types list is still consistent.
 *
 * @param[in] namep
 *          Type name, not empty. Its contents must be valid for the whole
 *          life of the program.
 * @param[in] descp
 *          Long description, can be @p NULL. Its contents must be valid for
 *          the whole life of the program.
 * @param[in] md5sump
 *          MD5 sum of the type description, not empty. Its contents must be
 *          valid for the whole life of the program.
 */
void urosRegisterStaticSrvType(const UrosString *namep,
                               const UrosString *descp,
                               const UrosString *md5sump) {

  UrosMsgType *typep;
  UrosListNode *nodep;

  urosAssert(urosStringNotEmpty(namep));
  urosAssert(urosStringNotEmpty(md5sump));

  typep = urosNew(NULL, UrosMsgType);
  urosAssert(typep != NULL);
  typep->name = *namep;
  typep->desc = *descp;
  typep->md5str = *md5sump;

  nodep = urosNew(NULL, UrosListNode);
  urosAssert(nodep != NULL);
  urosListNodeObjectInit(nodep);
  nodep->datap = typep;
  urosListAdd(&urosSrvTypeList, nodep);
}

/**
 * @brief   Registers a service type to the static global list.
 * @note    Best practice to statically register all the service types at boot
 *          time.
 * @see     urosMsgTypeList
 *
 * @pre     No service type was registered with the same name before.
 * @pre     No publishers nor subscribers have been queried before, so that
 *          the service types list is still consistent.
 *
 * @param[in] namep
 *          Type name, null-terminated string, not empty. Its contents must be
 *          valid for the whole life of the program.
 * @param[in] descp
 *          Long description, null-terminated string, can be @p NULL. Its
 *          contents must be valid for the whole life of the program.
 * @param[in] md5sump
 *          MD5 sum of the type description, null-terminated string, not empty.
 *          Its contents must be valid for the whole life of the program.
 */
void urosRegisterStaticSrvTypeSZ(const char *namep,
                                 const char *descp,
                                 const char *md5sump) {

  UrosMsgType *typep;
  UrosListNode *nodep;

  urosAssert(namep != NULL);
  urosAssert(namep[0] != 0);
  urosAssert(md5sump != NULL);
  urosAssert(md5sump[0] != 0);

  typep = urosNew(NULL, UrosMsgType);
  urosAssert(typep != NULL);
  typep->name = urosStringAssignZ(namep);
  typep->desc = urosStringAssignZ(descp);
  typep->md5str = urosStringAssignZ(md5sump);

  nodep = urosNew(NULL, UrosListNode);
  urosAssert(nodep != NULL);
  urosListNodeObjectInit(nodep);
  nodep->datap = typep;
  urosListAdd(&urosSrvTypeList, nodep);
}

/**
 * @brief   Gets the service type descriptor with the requested name.
 * @details Scans the service type register to get the address of the one with
 *          the requested name, if existing.
 * @note    This function is not thread safe, because all the service types are
 *          supposed to be registered only at boot time.
 * @see     urosSrvTypeList
 * @see     urosListFind
 * @see     urosMsgTypeHasName
 *
 * @param[in] namep
 *          Requested type name, not empty.
 * @return
 *          Pointer to the registered service type descriptor with the
 *          requested name.
 * @retval NULL
 *          No such type name found.
 */
const UrosMsgType *urosFindStaticSrvType(const UrosString *namep) {

  const UrosListNode *foundp;

  urosAssert(urosStringNotEmpty(namep));

  foundp = urosListFind(&urosSrvTypeList,
                        (uros_cmp_f)urosMsgTypeNodeHasName, namep);
  return (foundp != NULL) ? (const UrosMsgType *)foundp->datap : NULL;
}

/**
 * @brief   Gets the service type descriptor with the requested name.
 * @details Scans the service type register to get the address of the one with
 *          the requested name, if existing.
 * @note    This function is not thread safe, because all the service types are
 *          supposed to be registered only at boot time.
 * @see     urosMsgTypeList
 * @see     urosListFind
 * @see     urosMsgTypeHasName
 *
 * @param[in] namep
 *          Requested type name, null-terminated string, @p NULL forbidden.
 * @return
 *          Pointer to the registered service type descriptor with the
 *          requested name.
 * @retval NULL
 *          No such type name found.
 */
const UrosMsgType *urosFindStaticSrvTypeSZ(const char *namep) {

  UrosString namestr;
  const UrosListNode *foundp;

  urosAssert(namep != NULL);
  urosAssert(namep[0] != 0);

  namestr = urosStringAssignZ(namep);
  foundp = urosListFind(&urosSrvTypeList,
                        (uros_cmp_f)urosMsgTypeNodeHasName, &namestr);
  return (foundp != NULL) ? (const UrosMsgType *)foundp->datap : NULL;
}

/** @} */

/*~~~ TOPIC ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Topic */
/** @{ */

/**
 * @brief   Initializes a topic descriptor.
 * @details The object is set as an empty topic descriptor.
 *
 * @pre     The object addressed by @p tp is allocated but not initialized.
 * @post    The fields of the object are all empty strings.
 * @post    All the numeric values are nullified.
 *
 * @param[in,out] tp
 *          Pointer to an allocated @p UrosTopic object.
 */
void urosTopicObjectInit(UrosTopic *tp) {

  urosAssert(tp != NULL);

  urosStringObjectInit(&tp->name);
  tp->typep = NULL;
  tp->procf = NULL;
  tp->refcnt = 0;
  memset(&tp->flags, 0, sizeof(tp->flags));
}

/**
 * @brief   Cleans a topic descriptor.
 * @details Deallocates all the memory chunks allocated by the fields of the
 *          object.
 *
 * @pre     The string data must have been allocated with @p urosAlloc().
 * @post    The fields of the object are deallocated, strings are empty.
 * @post    @p tp points to a nullified @p UrosTopic descriptor.
 *
 * @param[in,out] tp
 *          Pointer to an initialized @p UrosTopic object.
 */
void urosTopicClean(UrosTopic *tp) {

  urosAssert(tp != NULL);
  urosAssert(tp->refcnt == 0);

  urosStringClean(&tp->name);
  tp->typep = NULL;
  tp->procf = NULL;
  tp->refcnt = 0;
}

/**
 * @brief   Deallocates a topic descriptor.
 * @details Deallocates the memory chunk of the object and all its fields.
 *
 * @pre     The object and its fields must have been allocated with
 *          @p urosAlloc().
 * @post    @p tp points to an invalid address.
 *
 * @param[in] tp
 *          Pointer to an initialized @p UrosTopic object, or @p NULL.
 */
void urosTopicDelete(UrosTopic *tp) {

  if (tp != NULL) {
    urosTopicClean(tp);
    urosFree(tp);
  }
}

/**
 * @brief   Incrememts the topic reference count.
 *
 * @pre     The topic is within a locked context.
 * @post    The topic reference count is incremented by one.
 *
 * @param[in,out] tp
 *          Pointer to the referenced topic descriptor.
 * @return
 *          Updated reference count.
 */
uros_cnt_t urosTopicRefInc(UrosTopic *tp) {

  urosAssert(tp != NULL);

  return ++tp->refcnt;
}

/**
 * @brief   Decrememts the topic reference count.
 *
 * @pre     The topic is within a locked context.
 * @pre     There is at least one referenced counted.
 * @post    The topic reference count is decremented by one.
 *
 * @param[in,out] tp
 *          Pointer to the unreferenced topic descriptor.
 * @return
 *          Updated reference count.
 */
uros_cnt_t urosTopicRefDec(UrosTopic *tp) {

  urosAssert(tp != NULL);
  urosAssert(tp->refcnt > 0);

  return --tp->refcnt;
}

/** @} */

/*~~~ LIST ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name List */
/** @{ */

/**
 * @brief   Initializes a list node.
 *
 * @pre     The object addressed by @p np is allocated but not initialized.
 * @post    The node has no referenced data.
 * @post    The node has no successor.
 *
 * @param[in,out] np
 *          Pointer to an allocated @p UrosListNode object.
 */
void urosListNodeObjectInit(UrosListNode *np) {

  urosAssert(np != NULL);

  np->datap = NULL;
  np->nextp = NULL;
}

/**
 * @brief   Deallocates a list node.
 * @details Deallocates the list node object. If a data deletion function is
 *          provided, the node data is deleted through a call to it.
 * @see     urosFree()
 *
 * @pre     The object and its fields must have been allocated with
 *          @p urosAlloc().
 * @post    @p np points to an invalid address.
 * @post    If the data deletion function is provided, the node data is
 *          deleted.
 *
 * @param[in] np
 *          Pointer to an initialized @p UrosListNode object. Can be @p NULL.
 * @param[in] datadelf
 *          Data deletion function. Can be @p NULL if only the node object
 *          @p np has to be deleted.
 */
void urosListNodeDelete(UrosListNode *np, uros_delete_f datadelf) {

  if (np != NULL) {
    if (datadelf != NULL) {
      datadelf(np->datap);
    }
    urosFree(np);
  }
}

/**
 * @brief   Initializes a list object.
 *
 * @pre     The object addressed by @p lstp is allocated but not initialized.
 * @post    The list is empty.
 *
 * @param[in,out] lstp
 *          Pointer to an allocated @p UrosList object.
 */
void urosListObjectInit(UrosList *lstp) {

  urosAssert(lstp != NULL);

  lstp->headp = NULL;
  lstp->length = 0;
}

/**
 * @brief   Cleans a list.
 * @details Removes all the nodes from a list. If a data deletion function is
 *          provided, the node data is deleted through a call to it.
 *
 * @pre     The object and its fields must have been allocated with
 *          @p urosAlloc().
 * @post    @p lstp points to an empty list.
 * @post    If the data deletion function is provided, each node data is
 *          deleted.
 *
 * @param[in] lstp
 *          Pointer to an initialized @p UrosList object.
 * @param[in] datadelf
 *          Data deletion function. Can be @p NULL if only the node objects
 *          have to be deleted, but not their referenced data.
 */
void urosListClean(UrosList *lstp, uros_delete_f datadelf) {

  UrosListNode *curp, *nextp;

  urosAssert(urosListIsValid(lstp));

  for (curp = lstp->headp; curp != NULL; curp = nextp) {
    nextp = curp->nextp;
    urosListNodeDelete(curp, datadelf);
  }
  lstp->headp = NULL;
  lstp->length = 0;
}

/**
 * @brief   Deallocates a list object.
 * @details Removes all the nodes from a list. If a data deletion function is
 *          provided, the node data is deleted through a call to it.
 *
 * @pre     The object and its fields must have been allocated with
 *          @p urosAlloc().
 * @post    @p lstp points to an invalid address.
 * @post    If the data deletion function is provided, each node data is
 *          deleted.
 *
 * @param[in] lstp
 *          Pointer to an initialized @p UrosList object, or @p NULL.
 * @param[in] datadelf
 *          Data deletion function. Can be @p NULL if only the list object
 *          @p lstp and its @p UrosListNode objects have to be deleted, but
 *          not their referenced data.
 */
void urosListDelete(UrosList *lstp, uros_delete_f datadelf) {

  if (lstp != NULL) {
    urosListClean(lstp, datadelf);
    urosFree(lstp);
  }
}

/**
 * @brief   List length.
 *
 * @param[in] lstp
 *      Pointer to an initialized @p UrosList object.
 * @return
 *      List length.
 */
uros_cnt_t urosListLength(const UrosList *lstp) {

  urosAssert(lstp != NULL);

  return lstp->length;
}

/**
 * @brief   Checks if pointing to a valid list object.
 * @details Checks is the addressed object points to a valid memory address,
 *          and its descriptor is consistent.
 *
 * @note    A list object addressed by @p lstp is consistent when:
 *          <pre>(strp->length == 0) == (strp->headp == NULL)</pre>
 *
 * @param[in] lstp
 *          Pointer to a presumed consistent @p UrosList object.
 * @return
 *          @p true when @p lstp is not @p NULL and the list descriptor is
 *          consistent.
 */
uros_bool_t urosListIsValid(const UrosList *lstp) {

  return lstp != NULL && ((lstp->headp == NULL) == (lstp->length == 0));
}

/**
 * @brief   Checks if a list is not empty.
 * @details Checks if the addressed list is a consistent, non-empty list.
 * @see     urosListIsValid
 *
 * @param[in] lstp
 *          Pointer to a presumed @p UrosList object.
 * @return
 *          @p true when @p lstp is not @p NULL and the list is not
 *          empty.
 */
uros_bool_t urosListNotEmpty(const UrosList *lstp) {

  return lstp != NULL && lstp->headp != NULL && lstp->length > 0;
}

/**
 * @brief   Checks if the list contains the referenced node.
 * @details Scans the list until a node with the requested address is found, or
 *          there are no more entries.
 *
 * @param[in] lstp
 *          Pointer to an initialized @p UrosList object.
 * @param[in] np
 *          Pointer to an @p UrosListNode supposed to belong to the list.
 * @return
 *          @p true if a node with the requested address is found, @p false
 *          otherwise.
 */
uros_bool_t urosListContains(const UrosList *lstp, const UrosListNode *np) {

  UrosListNode *curp;

  urosAssert(urosListIsValid(lstp));

  for (curp = lstp->headp; curp != NULL; curp = curp->nextp) {
    if (curp == np) {
      return UROS_TRUE;
    }
  }
  return UROS_FALSE;
}

/**
 * @brief   Gets the index of the referenced node inside a list.
 *
 * @param[in] lstp
 *          Pointer to an initialized @p UrosList object.
 * @param[in] np
 *          Pointer to an @p UrosListNode supposed to belong to the list.
 * @return
 *          The index of the node inside the list.
 * @retval -1
 *          The list does not contain the addressed node.
 */
int urosListIndexOf(const UrosList *lstp, const UrosListNode *np) {

  UrosListNode *curp;
  int i;

  urosAssert(urosListIsValid(lstp));

  for (curp = lstp->headp, i = 0; curp != NULL; ++i, curp = curp->nextp) {
    if (curp == np) {
      return i;
    }
  }
  return -1;
}

/**
 * @brief   First matching node occurrence of the list.
 * @details Scans the list and returns the first matching node entry.
 *
 * @param[in] lstp
 *          Pointer to an initialized @p UrosList object.
 * @param[in] filter
 *          Comparison function applied to each list node, with the following
 *          signature:
 *          @code{.c}
 *          uros_bool_t (*filter)(const UrosListNode *np, const void *featurep);
 *          @endcode
 * @param[in] featurep
 *          Pointer to a feature that a matching node must satisfy by passing
 *          it to @p filter.
 * @return
 *          Pointer to the first node which satisfies the requested feature.
 * @retval NULL
 *          No such node found.
 */
UrosListNode *urosListFind(const UrosList *lstp,
                           uros_cmp_f filter, const void *featurep) {

  UrosListNode *curp;

  urosAssert(urosListIsValid(lstp));
  urosAssert(filter != NULL);

  for (curp = lstp->headp; curp != NULL; curp = curp->nextp) {
    if (filter(curp, featurep)) {
      return curp;
    }
  }
  return NULL;
}

/**
 * @brief   Adds a node to the list.
 * @details The node is added to the head of the list.
 *
 * @pre     No node of the list has the same address of @p np (it is unique).
 * @post    The head of the list is @p np.
 * @post    The list length is incremented by one.
 *
 * @param[in,out] lstp
 *          Pointer to an initialized @p UrosList object.
 * @param[in] np
 *          Pointer to the node to be added, @p NULL forbidden.
 */
void urosListAdd(UrosList *lstp, UrosListNode *np) {

  urosAssert(urosListIsValid(lstp));
  urosAssert(np != NULL);
  urosAssert(np->nextp == NULL);

  np->nextp = lstp->headp;
  lstp->headp = np;
  ++lstp->length;
}

/**
 * @brief   Removes a node from the list.
 * @details The node, if found, is unlinked from the list and returned.
 *
 * @post    The node does not belong to the list.
 * @post    The node has no successors.
 * @post    The list is consistent.
 *
 * @param[in,out] lstp
 *          Pointer to an initialized @p UrosList object.
 * @param[in] np
 *          Pointer to the node to be removed from the list.
 * @return
 *          Pointer to the removed (unlinked) node.
 * @retval np
 *          The addressed node, if found inside the list.
 * @retval NULL
 *          No such node found inside the list.
 */
UrosListNode *urosListRemove(UrosList *lstp, const UrosListNode *np) {

  UrosListNode *curp, *prevp = NULL;

  urosAssert(urosListIsValid(lstp));
  urosAssert(np != NULL);

  for (curp = lstp->headp; curp != NULL; prevp = curp, curp = curp->nextp) {
    if (curp == np) {
      if (prevp != NULL) {
        prevp->nextp = curp->nextp;
      } else {
        lstp->headp = curp->nextp;
      }
      curp->nextp = NULL;
      --lstp->length;
      return curp;
    }
  }
  return NULL;
}

/**
 * @brief   Checks if the list node data equals the referenced string.
 * @see     urosStringCmp()
 *
 * @param[in] np
 *          Pointer to an initialized @p UrosListNode which points to an
 *          initialized @p UrosString object.
 * @param[in] strp
 *          Pointer to the reference initialized @p UrosString object.
 * @return
 *          @p true <i>iif</i> the string of the node is equal to the
 *          referenced one.
 */
uros_bool_t urosStringListNodeHasString(const UrosListNode *np,
                                        const UrosString *strp) {

  urosAssert(np != NULL);
  urosAssert(urosStringIsValid(strp));

  return 0 == urosStringCmp((const UrosString *)np->datap, strp);
}

/**
 * @brief   Gets the list node referencing the referenced string.
 * @details Scans the list to find the node referencing the string equal to the
 *          referenced one.
 * @note    This function is not thread safe, please lock it with a dedicated
 *          locking primitive (e.g. @p UrosMutex).
 * @see     urosListFind
 * @see     urosStringListNodeHasName
 *
 * @param[in] lstp
 *          Pointer to an initialized @p UrosList with @p UrosString entries.
 * @param[in] strp
 *          Requested string value.
 * @return
 *          Pointer to the list node with data equal to the reference string.
 * @retval NULL
 *          No such topic node found.
 */
UrosListNode *urosStringListFindByName(const UrosList *lstp,
                                       const UrosString *strp) {

  return urosListFind(lstp, (uros_cmp_f)urosStringListNodeHasString, strp);
}

/**
 * @brief   Checks if the list node is linked to the referenced topic.
 * @details The comparison is done at reference (pointer) level.
 *
 * @param[in] np
 *          Pointer to an initialized @p UrosListNode which points to an
 *          initialized @p UrosTopic descriptor.
 * @param[in] topicp
 *          Pointer to the reference initialized @p UrosTopic object.
 * @return
 *          @p true <i>iif</i> the topic of the node has the requested topic.
 */
uros_bool_t urosTopicListNodeHasTopic(const UrosListNode *np,
                                      const UrosTopic *topicp) {

  urosAssert(np != NULL);

  return ((const UrosTopic *)np->datap) == topicp;
}

/**
 * @brief   Checks if the list node links to a topic with the requested name.
 * @see     urosStringCmp()
 *
 * @param[in] np
 *          Pointer to an initialized @p UrosListNode which points to an
 *          initialized @p UrosTopic descriptor.
 * @param[in] namep
 *          Pointer to the reference non-empty @p UrosString object.
 * @return
 *          @p true <i>iif</i> the topic of the node has the requested name.
 */
uros_bool_t urosTopicListNodeHasName(const UrosListNode *np,
                                     const UrosString *namep) {

  urosAssert(np != NULL);
  urosAssert(urosStringNotEmpty(namep));

  return 0 == urosStringCmp(&((const UrosTopic *)np->datap)->name, namep);
}

/**
 * @brief   Gets the list node referencing a specific topic.
 * @details Scans the list to find the node referencing a specific topic.
 * @note    This function is not thread safe, please lock it with a dedicated
 *          locking primitive (e.g. @p UrosMutex).
 * @see     urosListFind
 * @see     urosTopicListNodeHasTopic
 *
 * @param[in] lstp
 *          Pointer to an initialized @p UrosList with @p UrosTopic entries.
 * @param[in] topicp
 *          Requested topic reference, @p NULL forbidden.
 * @return
 *          Pointer to the registered type descriptor with the requested name.
 * @retval NULL
 *          No such topic node found.
 */
UrosListNode *urosTopicListFindByTopic(const UrosList *lstp,
                                       const UrosTopic *topicp) {

  return urosListFind(lstp, (uros_cmp_f)urosTopicListNodeHasTopic, topicp);
}

/**
 * @brief   Gets the list node referencing a topic with the requested name.
 * @details Scans the list to find the node referencing the topic with the
 *          requested name.
 * @note    This function is not thread safe, please lock it with a dedicated
 *          locking primitive (e.g. @p UrosMutex).
 * @see     urosListFind
 * @see     urosTopicListNodeHasName
 *
 * @param[in] lstp
 *          Pointer to an initialized @p UrosList with @p UrosTopic entries.
 * @param[in] namep
 *          Requested type name string.
 * @return
 *          Pointer to the registered type descriptor with the requested name.
 * @retval NULL
 *          No such topic node found.
 */
UrosListNode *urosTopicListFindByName(const UrosList *lstp,
                                      const UrosString *namep) {

  return urosListFind(lstp, (uros_cmp_f)urosTopicListNodeHasName, namep);
}

/** @} */

/** @} */
