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
 * @file    urosBase.h
 * @author  Andrea Zoppi <texzk@email.it>
 *
 * @brief   Basic features of the middleware.
 */

#ifndef _UROSBASE_H_
#define _UROSBASE_H_

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include <urosconf.h>

#include <stddef.h>
#include <stdint.h>
#include <string.h>

/*===========================================================================*/
/* TYPES & MACROS                                                            */
/*===========================================================================*/

/** @addtogroup base_macros */
/** @{ */

/** @brief @p false boolean value. */
#define UROS_FALSE  (0)

/** @brief @p true boolean value. */
#define UROS_TRUE   (!UROS_FALSE)

/** @brief <em>No operation</em> placeholder.*/
#define UROS_NOP    (void)(0)

/**
 * @brief   Endianness of the architecture.
 * @details By default, it is set to <i>little-endian</i> through the constant
 *          @p 123. To enable <i>big-endian</i>ness, define it as @p 321.
 */
#if !defined(UROS_ENDIANNESS) || defined(__DOXYGEN__)
#define UROS_ENDIANNESS 123
#endif
#if UROS_ENDIANNESS != 123 && UROS_ENDIANNESS != 321
#error "UROS_ENDIANNESS must be either 123 or 321"
#endif

/**
 * @brief   Pre-processes @p expr as a ROM string.
 *
 * @param[in] expr
 *          Expression to be pre-processed as a ROM string.
 * @return
 *          ROM string representing @p expr.
 *
 * @par     Example
 *          @code{.c}
 *          UROS_STRINGIFY(a == b)
 *          @endcode
 *          results in
 *          @code{.c}
 *          "a == b"
 *          @endcode
 */
#define UROS_STRINGIFY(expr)    #expr

/**
 * @brief   Pre-processes the value of @p define as a ROM string.
 *
 * @param[in] define
 *          Pre-processor definition holding the value to be represented.
 * @return
 *          ROM string representing the value of the @p define definition.
 *
 * @par     Example
 *          @code{.c}
 *          #define VALUE 123+456
 *
            UROS_STRINGIFY2(VALUE)
 *          @endcode
 *          results in
 *          @code{.c}
 *          "123+456"
 *          @endcode
 */
#define UROS_STRINGIFY2(define) UROS_STRINGIFY(define)

/**
 * @brief   Makes the formatted parameters for an @p UrosString object.
 * @details Generates a list of values to be passed to a variable arguments
 *          function.
 * @note    To be used with the <tt>"%.*s"</tt> format string, which must be
 *          supported by a @p printf() compatible user function.
 *
 * @param[in] stringp
 *          Pointer to an @p UrosString object.
 * @return
 *          List of values for a variable arguments function.
 *
 * @par     Example
 * @anchor  base_ex_strarg
 *          @code{.c}
 *          UrosString str = urosStringCloneZ("Hello World");
 *          printf("I'll print: %.*s!", UROS_STRARG(&str));
 *          urosStringClean(&str);
 *          @endcode
 *          prints
 *          @verbatim I'll print: Hello World!@endverbatim
 */
#define UROS_STRARG(stringp) \
  ((unsigned)((stringp)->length)), ((stringp)->datap)

#if !defined(UROS_STACK_BLKSIZE) || defined(__DOXYGEN__)
/**
 * @brief   Size of a stack memory block.
 * @note    This macro should be overridden in <tt>urosconf.h</tt>, so that it
 *          can satisfy memory alignment rules, if any.
 *
 * @param[in] stksize
 *          Nominal stack size, in bytes. The total allocated space may be
 *          greater because of alignment rules, or space reserved for the
 *          <i>thread control block</i>.
 * @return
 *          Size of a stack memory block.
 */
#  define UROS_STACK_BLKSIZE(stksize) \
  ((size_t)(stksize))
#endif

#if !defined(UROS_STACK) || defined(__DOXYGEN__)
/**
 * @brief   Defines a static stack memory chunk.
 * @note    The stack is meant to be allocated statically, as a global
 *          variable.
 * @note    This macro should be overridden in <tt>urosconf.h</tt>, so that it
 *          can satisfy memory alignment rules, if any.
 *
 * @param[in] varname
 *          Name of the array variable to be defined.
 * @param[in] stksize
 *          Nominal stack size, in bytes. The total allocated space may be
 *          greater because of alignment rules, or space reserved for the
 *          <i>thread control block</i>.
 */
#  define UROS_STACK(varname, stksize) \
  uint8_t varname[UROS_STACK_BLKSIZE(stksize)]
#endif /* !defined(UROS_STACK) || defined(__DOXYGEN__) */

#if !defined(UROS_STACKPOOL_BLKSIZE) || defined(__DOXYGEN__)
/**
 * @brief   Size of a stack pool memory block.
 * @details Automatically adds the space for the reserved @p UrosMemPool
 *          pointer at the very beginning of the stack memory chunk.
 * @note    This macro should be overridden in <tt>urosconf.h</tt>, so that it
 *          can satisfy memory alignment rules, if any.
 *
 * @param[in] stksize
 *          Nominal stack size, in bytes. The total allocated space may be
 *          greater because of alignment rules, or space reserved for the
 *          <i>thread control block</i>.
 * @return
 *          Size of a stack pool memory block.
 */
#  define UROS_STACKPOOL_BLKSIZE(stksize) \
  (sizeof(void*) + (size_t)(stksize))
#endif /* !defined(UROS_STACKPOOL_BLKSIZE) || defined(__DOXYGEN__) */

#if !defined(UROS_STACKPOOL) || defined(__DOXYGEN__)
/**
 * @brief   Defines a stack pool memory chunk.
 * @details The defined memory pool automatically adds the space for the
 *          reserved @p UrosMemPool pointer at the very beginning of each
 *          stack memory chunk.
 * @note    The stacks are meant to be allocated statically, as a global
 *          variable.
 * @note    This macro should be overridden in <tt>urosconf.h</tt>, so that it
 *          can satisfy memory alignment rules, if any.
 *
 * @param[in] varname
 *          Name of the array variable to be defined.
 * @param[in] stksize
 *          Nominal stack size, in bytes. The total allocated space may be
 *          greater because of alignment rules, or space reserved for the
 *          <i>thread control block</i>.
 * @param[in] numstacks
 *          Number of stacks in the pool.
 */
#  define UROS_STACKPOOL(varname, stksize, numstacks) \
  uint8_t varname[(numstacks)][UROS_STACKPOOL_BLKSIZE(stksize)]
#endif /* !defined(UROS_STACKPOOL) || defined(__DOXYGEN__) */

#if UROS_USE_ASSERT == UROS_FALSE || !defined(urosAssert) || defined(__DOXYGEN__)
#  if defined(urosAssert)
#    undef urosAssert
#  endif
/**
 * @brief   Evaluates an assertion.
 * @details If the constraint expression does not hold, the system is halted.
 * @warning An assertion is a @b very strong assumption. If an assertion does
 *          not hold, the @b whole system is halted. This is why it is commonly
 *          used only for very strict constraints, which must always hold.
 * @note    By default it does nothing. The user must provide an @p urosAssert()
 *          macro inside the <tt>urosconf.h</tt> configuration file to achieve
 *          the required behavior with platform-dependent code.
 * @note    Assertions can be disabled inside the <tt>urosconf.h</tt>
 *          configuration file. A global switch is @p UROS_USE_ASSERT.
 *
 * @param[in] expr
 *          Assertion constraint expression, which must hold.
 *
 * @par     Example
 *          @code{.c}
 *          #include <assert.h>
 *
 *          #define urosAssert(expr) \
 *            assert(expr)
 *
 *          void load_value(int *resultp) {
 *
 *              urosAssert(resultp != NULL);
 *
 *              *resultp = 123;
 *          }
 *          @endcode
 */
#  define urosAssert(expr)  UROS_NOP
#endif /* !UROS_USE_ASSERT || !defined(urosAssert) || defined(__DOXYGEN__) */

#if UROS_USE_ERROR_MSG == UROS_FALSE || !defined(urosError) || defined(__DOXYGEN__)
#  if defined(urosError)
#    undef urosError
#  endif
/**
 * @brief   Checks if an error occurred, and generates a message.
 * @details If the error condition holds, a formatted error message is
 *          generated, and a statement (even a code block) is executed.
 * @note    By default it simply ignores the formatted message. The user must
 *          provide an @p urosError() macro inside the <tt>urosconf.h</tt>
 *          configuration file to achieve the required behavior with
 *          platform-dependent code.
 * @note    Error messages can be disabled inside the <tt>urosconf.h</tt>
 *          configuration file. A global switch is @p UROS_USE_ERROR_MSG.
 *
 * @param[in] when
 *          Error condition, which should not hold for a correct code.
 * @param[in] action
 *          A statement, or code block, which is executed if the error
 *          condition holds.
 * @param[in] msgargs
 *          Error message arguments. Must be enclosed between round parentheses
 *          in a @p printf() compatible format.
 *
 * @par     Example
 *          @code{.c}
 *          #include <urosUser.h>
 *
 *          #define urosError(when, action, msgargs) \
 *            { if (when) { \
 *                urosUserErrPrintf("Error at %s:%d\n" \
 *                                  "  function: %s\n" \
 *                                  "  reason:   %s\n" \
 *                                  "  message:  ", \
 *                                  __FILE__, __LINE__, \
 *                                  __PRETTY_FUNCTION__, \
 *                                  #when); \
 *                urosUserErrPrintf msgargs ; \
 *                { action; } } }
 *
 *          UrosString globalPassword = { 0, NULL };
 *
 *          uros_bool_t set_password(const UrosString *passwp) {
 *
 *              urosAssert(urosStringIsValid(passwp));
 *
 *              urosError(passwp->length < 8, return UROS_FALSE,
 *                        ("Password [%.*s] too short: length %zu < 8",
 *                         UROS_STRARG(passwp), passwp->length));
 *
 *              urosStringClean(&globalPassword);
 *              globalPassword = urosStringClone(passwp);
 *              return UROS_TRUE;
 *          }
 *          @endcode
 *          when called with <tt>"F0o_B4r"</tt> as password, prints
 @verbatim
 Error at security.c:123
   function: set_password
   reason:   passwp->length < 8
   message:  Password [F0o_B4r] too short: length 7 < 8
 @endverbatim
 */
#  define urosError(when, action, msgargs) { if (when) { action; } }
#endif /* !UROS_USE_ERROR_MSG || !defined(urosError) || defined(__DOXYGEN__) */

/**
 * @brief   Allocates a typed object.
 * @details Allocates a memory chunk on the heap with the size of the provided
 *          type.
 * @see     urosAlloc()
 *
 * @param[in,out] heapp
 *          Pointer to an initialized @p UrosHeap object, default if @p NULL.
 * @param[in] type
 *          Type of the object to be allocated. To be valid, a valid pointer
 *          type must be obtained by appending a @p * to @p type.
 * @return
 *          The address of the allocated memory chunk, casted to a pointer to
 *          the provided type.
 * @retval NULL
 *          There is not enough contiguous free memory to allocate a memory
 *          block of the requested size.
 *
 * @par     Example
 *          @code{.c}
 *          int *valuep;
 *          valuep = urosNew(NULL, int);
 *          if (valuep != NULL) {
 *            *valuep = 123;
 *            printf("%d", *valuep);
 *            urosFree(valuep);
 *          }
 *          @endcode
 */
#define urosNew(heapp, type) \
  ((type *)urosAlloc(heapp, sizeof(type)))

/**
 * @brief   Allocates an array.
 * @details Allocates a memory chunk which can hold an array of contiguous
 *          objects of the provided type.
 *
 * @param[in,out] heapp
 *          Pointer to an initialized @p UrosHeap object, default if @p NULL.
 * @param[in] n
 *          Number of objects to be allocated.
 * @param[in] type
 *          Type of the object to be allocated. To be valid, a valid pointer
 *          type must be obtained by appending a @p * to @p type.
 * @return
 *          The address of the allocated memory chunk, casted to a pointer to
 *          the provided type.
 * @retval NULL
 *          There is not enough contiguous free memory to allocate a memory
 *          block of the requested size.
 */
#define urosArrayNew(heapp, n, type) \
  ((type *)urosAlloc(heapp, (size_t)(n) * sizeof(type)))

/**
 * @brief   Allocates an array.
 * @details Allocates a memory chunk which can hold an array of contiguous
 *          chunks with the provided size.
 *
 * @param[in,out] heapp
 *          Pointer to an initialized @p UrosHeap object, default if @p NULL.
 * @param[in] n
 *          Number of objects to be allocated.
 * @param[in] size
 *          Size of each single chunk to be allocated.
 * @return
 *          The address of the allocated memory chunk.
 * @retval NULL
 *          There is not enough contiguous free memory to allocate a memory
 *          block of the requested size.
 */
#define urosArrayAlloc(heapp, n, size) \
  urosAlloc(heapp, (size_t)(n) * (size_t)(size))

/** @} */

/** @addtogroup base_types */
/** @{ */

/**
 * @brief   Boolean data type.
 */
typedef uint8_t uros_bool_t;

/**
 * @brief   Unsigned counter data type.
 */
typedef uint32_t uros_cnt_t;

/**
 * @brief   ROS time.
 */
typedef struct uros_time_t {
  uint32_t  sec;    /**< @brief Seconds component.*/
  uint32_t  nsec;   /**< @brief Nanoseconds component.*/
} uros_time_t;

/**
 * @brief   ROS duration.
 */
typedef struct uros_duration_t {
  int32_t   sec;    /**< @brief Seconds component.*/
  int32_t   nsec;   /**< @brief Nanoseconds component.*/
} uros_duration_t;

/**
 * @brief   Error codes enumerator.
 */
enum {
  UROS_OK               =    0, /**< @brief No errors.*/
  UROS_ERR_TIMEOUT      = -100, /**< @brief Timeout lost.*/
  UROS_ERR_NOMEM        = -101, /**< @brief Not enough free memory.*/
  UROS_ERR_PARSE        = -102, /**< @brief Parsing error.*/
  UROS_ERR_EOF          = -103, /**< @brief End of file/stream reached.*/
  UROS_ERR_BADPARAM     = -104, /**< @brief Bad parameter.*/
  UROS_ERR_NOCONN       = -105, /**< @brief Inactive connection.*/
  UROS_ERR_BADCONN      = -106, /**< @brief Bad connection, check the low-level error code.*/
  UROS_ERR_NOTIMPL      = -107  /**< @brief Feature not implemented.*/
};

/** @name Function pointers */
/** @{ */

/**
 * @brief   One-parameter procedure function pointer.
 *
 * @param[in] data
 *          Pointer to a generic data structure, or embedded value.
 * @return
 *          Error code.
 */
typedef uros_err_t (*uros_proc_f)(void *data);

/**
 * @brief   Predicate function.
 * @details Used to evaluate a predicate on an object.
 *
 * @param[in] obj
 *          Predicate object to be evaluated.
 * @return
 *          Value of the predicate evaluation.
 */
typedef uros_bool_t (*uros_pred_f)(const void *obj);

/**
 * @brief   Comparison function.
 * @details Used to evaluate the comparison of two objects; e.g. equality,
 *          inclusion, ordered comparison, etc.
 *
 * @param[in] obj1p
 *          First operand object.
 * @param[in] obj2p
 *          Second operand object.
 * @return
 *          Comparison predicate evaluation.
 */
typedef uros_bool_t (*uros_cmp_f)(const void *obj1p, const void *obj2p);

/**
 * @brief   Allocation function.
 * @details Generic allocation function pointer, compatible with a @p malloc()
 *          signature.
 *
 * @param[in] size
 *          Size of the memory block to be allocated, in bytes.
 * @return
 *          The address of the allocated memory chunk.
 * @retval NULL
 *          There is not enough contiguous free memory to allocate a memory
 *          block of the requested size.
 */
typedef void *(*uros_alloc_f)(size_t size);

/**
 * @brief   Deletion procedure.
 * @details Generic function pointer called to free the memory block allocated
 *          for an object.
 *
 * @post    @p objp points to an invalid address.
 *
 * @param[in] obj
 *          Pointer to the memory block to be deallocated.
 *          A @p NULL value will simply be ignored.
 */
typedef void (*uros_delete_f)(void *objp);

/** @} */

#if UROS_USE_BUILTIN_MEMPOOL || defined(__DOXYGEN__)
/**
 * @brief   Built-in memory pool object.
 */
typedef struct UrosMemPool {
  void          *headp;         /**< @brief Pointer to the first free block.*/
  size_t        blockSize;      /**< @brief Block size.*/
  uros_alloc_f  allocator;      /**< @brief Allocation provider.*/
  uros_cnt_t    free;           /**< @brief Number of free blocks.*/
  UrosMutex     lock;           /**< @brief Memory pool lock.*/
} UrosMemPool;
#endif

/**
 * @brief   String object.
 * @details A string is stored as an object containing the string length and
 *          the pointer to the character data.
 */
typedef struct UrosString {
  size_t    length;             /**< @brief String length.*/
  char      *datap;             /**< @brief String data.*/
} UrosString;

/** @name List */
/** @{ */

/**
 * @brief   List node, forward only.
 */
typedef struct UrosListNode {
  void                  *datap; /**< @brief Generic data pointer.*/
  struct UrosListNode   *nextp; /**< @brief Pointer to the next list entry node.*/
} UrosListNode;

/**
 * @brief   Linked list, forward only.
 */
typedef struct UrosList {
  UrosListNode  *headp;         /**< @brief Pointer to the list head node.*/
  uros_cnt_t    length;         /**< @brief Number of list entries.*/
} UrosList;

/** @} */

/** @name Messaging related */
/** @{ */

/**
 * @brief   Message type descriptor.
 */
typedef struct UrosMsgType {
  UrosString name;       /**< @brief Type name.*/
  UrosString desc;       /**< @brief Long description.*/
  UrosString md5str;     /**< @brief Textual MD5 sum.*/
} UrosMsgType;

/**
 * @brief   Topic and service flags.
 */
typedef struct uros_topicflags_t {
  unsigned service    : 1;      /**< @brief Service/!TcpStatus connection.*/
  unsigned probe      : 1;      /**< @brief Just probing, do not call the handler.*/
  unsigned persistent : 1;      /**< @brief Peristent service connection (by client).*/
  unsigned latching   : 1;      /**< @brief Latching mode (i.e. send the
                                            last value to new subscribers).*/
  unsigned noDelay    : 1;      /**< @brief Nagle algorithm disabled.*/
  unsigned deleted    : 1;      /**< @brief Deleted topic descriptor, free asap.*/
} uros_topicflags_t;

/**
 * @brief   Topic descriptor.
 */
typedef struct UrosTopic {
  UrosString        name;       /**< @brief Topic/Service name.*/
  const UrosMsgType *typep;     /**< @brief Topic/Service message type.*/
  uros_proc_f       procf;      /**< @brief Procedure handler.*/
  uros_topicflags_t flags;      /**< @brief Topic/Service flags.*/

  /* Allocation stuff.*/
  uros_cnt_t        refcnt;     /**< @brief Reference counter.*/
} UrosTopic;

/** @} */
/** @} */

/*===========================================================================*/
/* GLOBAL VARIABLES                                                          */
/*===========================================================================*/

extern UrosList urosMsgTypeList;
extern UrosList urosSrvTypeList;

extern const uros_topicflags_t uros_nulltopicflags;
extern const uros_topicflags_t uros_nullserviceflags;

/*===========================================================================*/
/* GLOBAL PROTOTYPES                                                         */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

void urosInit(void);
const char *urosErrorText(uros_err_t err);

void *urosAlloc(UrosMemHeap *heapp, size_t size);
void urosFree(void *chunkp);

void urosMemPoolObjectInit(UrosMemPool *poolp, size_t blocksize,
                           uros_alloc_f allocator);
void *urosMemPoolAlloc(UrosMemPool *poolp);
void urosMemPoolFree(UrosMemPool *poolp, void *objp);
uros_cnt_t urosMemPoolNumFree(UrosMemPool *poolp);
void urosMemPoolLoadArray(UrosMemPool *poolp, void *arrayp, uros_cnt_t n);
size_t urosMemPoolBlockSize(UrosMemPool *poolp);

void urosStringObjectInit(UrosString *strp);
UrosString urosStringAssignN(const char *datap, size_t datalen);
UrosString urosStringAssignZ(const char *szp);
UrosString urosStringClone(const UrosString *strp);
UrosString urosStringCloneN(const char *datap, size_t datalen);
UrosString urosStringCloneZ(const char *szp);
void urosStringClean(UrosString *strp);
void urosStringDelete(UrosString *strp);
uros_bool_t urosStringIsValid(const UrosString *strp);
uros_bool_t urosStringNotEmpty(const UrosString *strp);
int urosStringCmp(const UrosString *str1, const UrosString *str2);

void urosMsgTypeObjectInit(UrosMsgType *typep);
void urosMsgTypeClean(UrosMsgType *typep);
void urosMsgTypeDelete(UrosMsgType *typep);

uros_bool_t urosMsgTypeNodeHasName(const UrosListNode *nodep,
                                   const UrosString *namep);

void urosRegisterStaticMsgType(const UrosString *namep,
                               const UrosString *descp,
                               const UrosString *md5sump);
void urosRegisterStaticMsgTypeSZ(const char *namep,
                                 const char *descp,
                                 const char *md5sump);
const UrosMsgType *urosFindStaticMsgType(const UrosString *namep);
const UrosMsgType *urosFindStaticMsgTypeSZ(const char *namep);

void urosRegisterStaticSrvType(const UrosString *namep,
                               const UrosString *descp,
                               const UrosString *md5sump);
void urosRegisterStaticSrvTypeSZ(const char *namep,
                                 const char *descp,
                                 const char *md5sump);
const UrosMsgType *urosFindStaticSrvType(const UrosString *namep);
const UrosMsgType *urosFindStaticSrvTypeSZ(const char *namep);

void urosTopicObjectInit(UrosTopic *tp);
void urosTopicClean(UrosTopic *tp);
void urosTopicDelete(UrosTopic *tp);
uros_cnt_t urosTopicRefInc(UrosTopic *tp);
uros_cnt_t urosTopicRefDec(UrosTopic *tp);

void urosListNodeObjectInit(UrosListNode *np);
void urosListNodeDelete(UrosListNode *np, uros_delete_f datadelf);

void urosListObjectInit(UrosList *lstp);
void urosListClean(UrosList *lstp, uros_delete_f datadelf);
void urosListDelete(UrosList *lstp, uros_delete_f datadelf);
uros_cnt_t urosListLength(const UrosList *lstp);
uros_bool_t urosListIsValid(const UrosList *lstp);
uros_bool_t urosListNotEmpty(const UrosList *lstp);
uros_bool_t urosListContains(const UrosList *lstp, const UrosListNode *np);
int urosListIndexOf(const UrosList *lstp, const UrosListNode *np);
UrosListNode *urosListFind(const UrosList *lstp,
                           uros_cmp_f filter, const void *featurep);
void urosListAdd(UrosList *lstp, UrosListNode *np);
UrosListNode *urosListRemove(UrosList *lstp, const UrosListNode *np);

uros_bool_t urosStringListNodeHasString(const UrosListNode *np,
                                        const UrosString *strp);
UrosListNode *urosStringListFindByName(const UrosList *lstp,
                                       const UrosString *strp);

uros_bool_t urosTopicListNodeHasTopic(const UrosListNode *np,
                                      const UrosTopic *topicp);
uros_bool_t urosTopicListNodeHasName(const UrosListNode *np,
                                     const UrosString *namep);
UrosListNode *urosTopicListFindByTopic(const UrosList *lstp,
                                       const UrosTopic *topicp);
UrosListNode *urosTopicListFindByName(const UrosList *lstp,
                                      const UrosString *namep);

#ifdef __cplusplus
}
#endif
#endif /* _UROSBASE_H_ */
