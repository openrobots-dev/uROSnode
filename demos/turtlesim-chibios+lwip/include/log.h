/*
Copyright (c) 2012, Andrea Zoppi. All rights reserved.

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

#ifndef _LOG_H_
#define _LOG_H_

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include "halconf.h"

#if HAL_USE_LOG || defined(__DOXYGEN__)

#include "logcfg.h"
#include <stdarg.h>

/*===========================================================================*/
/* TYPES & MACROS                                                            */
/*===========================================================================*/

/** @addtogroup log_macros */
/** @{ */

/** @name Configuration switches */
/** @{ */

/**
 * @brief   Enables support for Unicode characters (strings only).
 */
#if !defined(LOG_USE_WCHAR) || defined(__DOXYGEN__)
#define LOG_USE_WCHAR               FALSE
#endif

/**
 * @brief   Enables support for 64 bit numbers.
 */
#if !defined(LOG_USE_64BIT) || defined(__DOXYGEN__)
#define LOG_USE_64BIT               TRUE
#endif

/**
 * @brief   Enables support for unsigned integers.
 */
#if !defined(LOG_USE_UINTS) || defined(__DOXYGEN__)
#define LOG_USE_UINTS               TRUE
#endif

/**
 * @brief   Enables support for signed integers.
 */
#if !defined(LOG_USE_INTS) || defined(__DOXYGEN__)
#define LOG_USE_INTS                TRUE
#endif

/**
 * @brief   Enables support for octal numbers.
 */
#if !defined(LOG_USE_OCTALS) || defined(__DOXYGEN__)
#define LOG_USE_OCTALS              TRUE
#endif

/**
 * @brief   Enables support for floating point numbers.
 */
#if !defined(LOG_USE_FLOATS) || defined(__DOXYGEN__)
#define LOG_USE_FLOATS              TRUE
#endif

/**
 * @brief   Strict C @p printf() format for floating point numbers.
 *
 * @details If enabled, floats are printed following the standard C @p printf()
 *          format instead of the simplified one.
 */
#if !defined(LOG_USE_STRICT_FLOATS) || defined(__DOXYGEN__)
#define LOG_USE_STRICT_FLOATS       FALSE   // TODO
#endif

/**
 * @brief   Enables support for @p logprintf().
 */
#if !defined(LOG_USE_PRINTF) || defined(__DOXYGEN__)
#define LOG_USE_PRINTF              TRUE
#endif

/**
 * @brief   Enables support for @p logprintfc().
 */
#if !defined(LOG_USE_PRINTFC) || defined(__DOXYGEN__)
#define LOG_USE_PRINTFC             TRUE
#endif

/** @} */

#define LOG_UNKNOWN_SYMBOL  -100    /**< Unknown symbol error message.*/

/** @name Format record masks */
/** @{ */
#define LOGF_WSH    0                       /** @brief Bit shift of the format width.*/
#define LOGF_WMSK   (0xFFul << LOGF_WSH)    /** @brief Bit mask of the format width.*/
#define LOGF_PSH    8                       /** @brief Bit shift of the format precision.*/
#define LOGF_PMSK   (0xFFul << LOGF_PSH)    /** @brief Bit mask of the format precision.*/
#define LOGF_FSH    16                      /** @brief Bit shift of the format flags.*/
#define LOGF_FMSK   (0xFFul << LOGF_FSH)    /** @brief Bit mask of the format flags.*/
#define LOGF_TSH    24                      /** @brief Bit shift of the format type.*/
#define LOGF_TMSK   (0xFFul << LOGF_TSH)    /** @brief Bit mask of the format type.*/
/** @} */

/** @} */

/** @addtogroup log_types */
/** @{ */

/**
 * @brief   Unicode character type.
 */
#if (LOG_USE_WCHAR && defined(LOG_DEFINE_WCHAR)) || defined(__DOXYGEN__)
typedef uint16_t wchar_t;
#endif /* LOG_USE_WCHAR && defined(LOG_DEFINE_WCHAR) */

/**
 * @brief   Log state machine possible states.
 */
typedef enum logstate_t {
  LOG_UNINIT    = 0,        /**< @brief Log uninitialized.*/
  LOG_STOP      = 1,        /**< @brief Log stopped.*/
  LOG_READY     = 2,        /**< @brief Log ready.*/
  LOG_ERROR     = 3         /**< @brief Unrecoverable log error.*/
} logstate_t;

/**
 * @brief   Log format codes.
 */
enum logfmtcode_t {
  LOGT_DCH1     = 0x01u,    /**< @brief Direct character (LSB).*/
  LOGT_DCH2     = 0x02u,    /**< @brief Direct two characters (LSB first).*/
  LOGT_DCH3     = 0x03u,    /**< @brief Direct three characters (LSB first).*/
  LOGT_DCHN     = 0x04u,    /**< @brief Direct long string (direct 24 bits length, next datum).*/
  LOGT_DCHZ     = 0x05u,    /**< @brief Direct zero-terminated string (direct first 3 chars, LSB first).*/
  LOGT_DCHR     = 0x06u,    /**< @brief Direct repeated character (direct 16 bits counter, character << 16).*/
  LOGT_DCSTR    = 0x07u,    /**< @brief Direct constant string (next pointer).*/

  LOGT_CHAR     = 0x08u,    /**< @brief ASCII character (next).*/
  LOGT_STRN     = 0x0Au,    /**< @brief Copied ASCII string (next: length, data).*/
  LOGT_CSTRN    = 0x0Cu,    /**< @brief Constant ASCII string (next: length, pointer).*/
  LOGT_WCHAR    = 0x09u,    /**< @brief Unicode character (next).*/
  LOGT_WSTRN    = 0x0Bu,    /**< @brief Copied Unicode string (next: length, data).*/
  LOGT_WCSTRN   = 0x0Du,    /**< @brief Constant Unicode string (next: length, pointer).*/

  LOGT_PTR      = 0x60u,    /**< @brief Pointer (next).*/

  LOGT_UINT8    = 0x20u,    /**< @brief Unsigned 8 bits integer (next).*/
  LOGT_UINT16   = 0x21u,    /**< @brief Unsigned 16 bits integer (next).*/
  LOGT_UINT32   = 0x22u,    /**< @brief Unsigned 32 bits integer (next).*/
  LOGT_UINT64   = 0x23u,    /**< @brief Unsigned 32 bits integer (next, little endian).*/

  LOGT_INT8     = 0x10u,    /**< @brief Signed 8 bits integer (next).*/
  LOGT_INT16    = 0x11u,    /**< @brief Signed 16 bits integer (next).*/
  LOGT_INT32    = 0x12u,    /**< @brief Signed 32 bits integer (next).*/
  LOGT_INT64    = 0x13u,    /**< @brief Signed 64 bits integer (next, little lendian).*/

  LOGT_OCT8     = 0x40u,    /**< @brief Octal 8 bits (next).*/
  LOGT_OCT16    = 0x41u,    /**< @brief Octal 16 bits (next).*/
  LOGT_OCT32    = 0x42u,    /**< @brief Octal 32 bits (next).*/
  LOGT_OCT64    = 0x43u,    /**< @brief Octal 64 bits (next, little endian).*/

  LOGT_HEX8     = 0x30u,    /**< @brief Hex 8 bits (next).*/
  LOGT_HEX16    = 0x31u,    /**< @brief Hex 16 bits (next).*/
  LOGT_HEX32    = 0x32u,    /**< @brief Hex 32 bits (next).*/
  LOGT_HEX64    = 0x33u,    /**< @brief Hex 64 bits (next, little endian).*/

  LOGT_FP       = 0x50u,    /**< @brief Float (next).*/
  LOGT_DP       = 0x51u,    /**< @brief Double (next, little endian).*/
  LOGT_FPE      = 0x52u,    /**< @brief Float, scientific notation (next).*/
  LOGT_DPE      = 0x53u,    /**< @brief Double, scientific notation (next, little endian).*/
  LOGT_FPG      = 0x54u,    /**< @brief Float, shortest form (next).*/
  LOGT_DPG      = 0x55u,    /**< @brief Double, shortest form (next, little endian).*/

  LOGT_NULL     = 0x00u     /**< @brief Null object, used for resynchronization.*/
};

/**
 * @brief   Log symbol type, compatible with  @p logfmt_t, @p msg_t, and pointers.
 */
typedef uint32_t    logsym_t;

/**
 * @brief   Log symbol format record.
 *
 * @details This record describes the format of the symbol being streamed.
 */
typedef union logfmt_t {
  struct {
    uint8_t     width;          /**< @brief Minimum width.*/
    uint8_t     digits;         /**< @brief Minimum digits.*/
    uint8_t     flags;          /**< @brief Format flags.*/
    uint8_t     type;           /**< @brief Symbol type (see <tt>LOGT_*</tt>).*/
  }             fmt;            /**< @brief Format fields.*/
  logsym_t      raw;            /**< @brief Raw representation.*/
} logfmt_t;

/**
 * @brief   Format record flags.
 */
enum logfmtflags_t {
  LOGFF_LEFT    = (1u << 0),    /**< @brief Align to the left.*/
  LOGFF_SIGN    = (1u << 1),    /**< @brief Print the sign.*/
  LOGFF_SPACE   = (1u << 2),    /**< @brief Print a sign space for values >= 0.*/
  LOGFF_HASH    = (1u << 3),    /**< @brief Print "0x" / force decimal point.*/
  LOGFF_ZERO    = (1u << 4),    /**< @brief Left pad with zeroes.*/
  LOGFF_UPPER   = (1u << 5),    /**< @brief Upper case.*/
  LOGFF_WIDTH   = (1u << 6),    /**< @brief Use width parameter.*/
  LOGFF_DIGITS  = (1u << 7)     /**< @brief Use precision parameter.*/
};

/**
 * @brief   Log driver configuration.
 */
typedef struct LogConfig {
  BaseSequentialStream  *stream;    /**< @brief Output stream.*/
  logsym_t              *bufp;      /**< @brief Buffered symbols.*/
  cnt_t                 buflen;     /**< @brief Buffer length (in symbols).*/
} LogConfig;

/**
 * @brief   Log driver.
 */
typedef struct LogDriver {
  logstate_t            state;      /**< @brief Log driver state.*/
  Mailbox               mb;         /**< @brief Symbols mailbox.*/
  Mutex                 fetchmtx;   /**< @brief Fetch semaphore.*/
  const LogConfig       *config;    /**< @brief Log driver configuration.*/
} LogDriver;

/** @} */

/*===========================================================================*/
/* Interrupt class macros.                                                   */
/*===========================================================================*/

/**
 * @brief   Returns the number of free symbol slots into a queue.
 * @note    Can be invoked in any system state but if invoked out of a locked
 *          state then the returned value may change after reading.
 * @note    The returned value can be less than zero when there are waiting
 *          threads on the internal semaphore.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object-
 * @return
 *          Number of empty symbol slots.
 *
 * @iclass
 */
#define logNumFreeI(lp) \
  chMBGetFreeCountI(&(lp)->mb)

/**
 * @brief   Returns the number of used symbol slots into a queue.
 * @note    Can be invoked in any system state but if invoked out of a locked
 *          state then the returned value may change after reading.
 * @note    The returned value can be less than zero when there are waiting
 *          threads on the internal semaphore.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object
 * @return
 *          Number of queued symbols.
 *
 * @iclass
 */
#define logNumUsedI(lp) \
  chMBGetUsedCountI(&(lp)->mb)

/**
 * @brief   Returns the next raw symbol in the queue without removing it.
 * @pre     A symbol must be waiting in the queue for this function to work
 *          or it would return garbage. The correct way to use this macro is
 *          to use @p logNumUsedI() and then use this macro, all within
 *          a lock state.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @return
 *          Raw head symbol of the queue.
 *
 * @iclass
 */
#define logPeekRawI(lp) \
  chMBPeekI(&(lp)->mb)

/**
 * @brief   Retrieves a raw log symbol from a log queue.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is empty.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[out] rawp
 *          Pointer to a log symbol variable for the received symbol.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly fetched.
 * @retval RDY_TIMEOUT
 *          The queue is empty and a symbol cannot be fetched.
 *
 * @iclass
 */
#define logFetchRawI(lp, rawp) \
  chMBFetchI(&(lp)->mb, (rawp))

/**
 * @brief   Puts a raw symbol into a log queue.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] raw
 *          Raw symbol to be vapended to the queue.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be vapended.
 *
 * @iclass
 */
#define logPutRawI(lp, raw) \
  chMBPostI(&(lp)->mb, (logsym_t)(raw))

/**
 * @brief   Puts a null symbol into a log queue.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be vapended.
 *
 * @iclass
 */
#define logPutNullI(lp) \
  logPutRawI((lp), (LOGT_NULL << LOGF_TSH))

/**
 * @brief   Puts a direct character symbol into a log queue.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] c
 *          Character to be vapended to the queue.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be vapended.
 *
 * @iclass
 */
#define logPutDCharI(lp, c) \
  logPutRawI((lp), (LOGT_DCH1 << LOGF_TSH) | \
             ((logsym_t)(c) & 0xFF))

/**
 * @brief   Puts a two direct characters symbol into a log queue.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] c1
 *          First character to be vapended to the queue.
 * @param[in] c2
 *          Second character to be vapended to the queue.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be vapended.
 *
 * @iclass
 */
#define logPutDChar2I(lp, c1, c2) \
  logPutRawI((lp), (LOGT_DCH2 << LOGF_TSH) | \
             ((logsym_t)(c1) & 0xFF) | \
             (((logsym_t)(c2) & 0xFF) << 8))

/**
 * @brief   Puts a three direct characters symbol into a log queue.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
            Pointer to an initialized @p LogDriver object.
 * @param[in] c1
 *          First character to be vapended to the queue.
 * @param[in] c2
 *          Second character to be vapended to the queue.
 * @param[in] c3
 *          Third character to be vapended to the queue.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be vapended.
 *
 * @iclass
 */
#define logPutDChar3I(lp, c1, c2, c3) \
  logPutRawI((lp), (LOGT_DCH3 << LOGF_TSH) | \
             ((logsym_t)(c1) & 0xFF) | \
             (((logsym_t)(c2) & 0xFF) << 8) | \
             (((logsym_t)(c3) & 0xFF) << 16))

/**
 * @brief   Puts a direct repeated character symbol into a log queue.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
            Pointer to an initialized @p LogDriver object.
 * @param[in] c
 *          Character to be repeated.
 * @param[in] r
 *          Number of repetitions.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be vapended.
 *
 * @iclass
 */
#define logPutDCharRI(lp, c, r) \
  logPutRawI((lp), (LOGT_DCHR << LOGF_TSH) | \
             ((logsym_t)(c) & 0xFF) | \
             (((logsym_t)(r) & 0xFFFF) << 8))

/**
 * @brief   Puts a newline symbol into a log queue.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
            Pointer to an initialized @p LogDriver object.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be vapended.
 *
 * @iclass
 */
#define logPutNLI(lp) \
  logPutDChar2I((lp), '\r', '\n')

/**
 * @brief   Puts an unsigned integer symbol into a log queue.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
            Pointer to an initialized @p LogDriver object.
 * @param[in] v
 *          Value to be vapended to the queue.
 * @param[in] fmt
 *          Format record.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be vapended.
 *
 * @iclass
 */
#define logPutUintI(lp, v, fmt) \
  logPutUint32I((lp), (v), (fmt))

/**
 * @brief   Puts a signed integer symbol into a log queue.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] v
 *          Value to be vapended to the queue.
 * @param[in] fmt
 *          Format record.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be vapended.
 *
 * @iclass
 */
#define logPutIntI(lp, v, fmt) \
  logPutInt32I((lp), (v), (fmt))

/**
 * @brief   Puts a string symbol into a log queue.
 * @details vapends a zero-terminated string to the log queue. It is
 *          designed to introduce the minimum locking overhead.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full, and correctly truncates the
 *          copied string.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] v
 *          Value to be vapended to the queue.
 * @param[in] fmt
 *          Format record.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be vapended.
 *
 * @iclass
 */
#define logPutStrI(lp, strp, fmt) \
  logPutCharZI((lp), (strp), (fmt))

#if LOG_USE_PRINTF || LOG_USE_PRINTFC || defined(__DOXYGEN__)

/**
 * @brief   Shorthand for @p logPutvfI().
 */
#define logprintvfI \
    logPutvfI

/**
 * @brief   Shorthand for @p logPutfI().
 */
#define logprintfI \
  logPutfI

/**
 * @brief   Shorthand for @p logPutvfcI().
 */
#define logprintvfcI \
  logPutvfcI

/**
 * @brief   Shorthand for @p logPutfcI().
 */
#define logprintfcI \
  logPutfcI

#endif

/*===========================================================================*/
/* User class macros.                                                        */
/*===========================================================================*/

/**
 * @brief   Puts a null symbol into a log queue.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be vapended.
 *
 * @api
 */
#define logPutNull(lp) \
  logPutRawTimeout((lp), (LOGT_NULL << LOGF_TSH), TIME_INFINITE)

/**
 * @brief   Puts a direct character symbol into a log queue.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] c
 *          Character to be vapended to the queue.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be vapended.
 *
 * @api
 */
#define logPutDChar(lp, c) \
  logPutRawTimeout((lp), (LOGT_DCH1 << LOGF_TSH) | \
                   ((logsym_t)(c) & 0xFF), TIME_INFINITE)

/**
 * @brief   Puts a two direct characters symbol into a log queue.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] c1
 *          First character to be vapended to the queue.
 * @param[in] c2
 *          Second character to be vapended to the queue.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be vapended.
 *
 * @api
 */
#define logPutDChar2(lp, c1, c2) \
  logPutRawTimeout((lp), (LOGT_DCH2 << LOGF_TSH) | \
                   ((logsym_t)(c1) & 0xFF) | \
                   (((logsym_t)(c2) & 0xFF) << 8), \
                   TIME_INFINITE)

/**
 * @brief   Puts a three direct characters symbol into a log queue.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] c1
 *          First character to be vapended to the queue.
 * @param[in] c2
 *          Second character to be vapended to the queue.
 * @param[in] c3
 *          Third character to be vapended to the queue.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be vapended.
 *
 * @api
 */
#define logPutDChar3(lp, c1, c2, c3) \
  logPutRawTimeout((lp), (LOGT_DCH3 << LOGF_TSH) | \
                   ((logsym_t)(c1) & 0xFF) | \
                   (((logsym_t)(c2) & 0xFF) << 8) | \
                   (((logsym_t)(c3) & 0xFF) << 16), \
                   TIME_INFINITE)

/**
 * @brief   Puts a direct repeated character symbol into a log queue.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] c
 *          Character to be repeated.
 * @param[in] r
 *          Number of repetitions.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be vapended.
 *
 * @api
 */
#define logPutDCharR(lp, c, r) \
  logPutRawTimeout((lp), (LOGT_DCHR << LOGF_TSH) | \
                   ((logsym_t)(c) & 0xFF) | \
                   (((logsym_t)(r) & 0xFFFF) << 8), \
                   TIME_INFINITE)

/**
 * @brief   Puts a newline symbol into a log queue.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be vapended.
 *
 * @api
 */
#define logPutNL(lp) \
  logPutDChar2((lp), '\r', '\n')

/**
 * @brief   Puts an unsigned integer symbol into a log queue.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] v
 *          Value to be vapended to the queue.
 * @param[in] fmt
 *          Format record.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be vapended.
 *
 * @api
 */
#define logPutUint(lp, v, fmt) \
  logPutUint32((lp), (v), (fmt))

/**
 * @brief   Puts a signed integer symbol into a log queue.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] v
 *          Value to be vapended to the queue.
 * @param[in] fmt
 *          Format record.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be vapended.
 *
 * @api
 */
#define logPutInt(lp, v, fmt) \
  logPutInt32((lp), (v), (fmt))

/**
 * @brief   Puts a string symbol into a log queue.
 * @details vapends a zero-terminated string to the log queue. It is
 *          designed to introduce the minimum locking overhead.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] v
 *          Value to be vapended to the queue.
 * @param[in] fmt
 *          Format record.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be vapended.
 *
 * @iclass
 */
#define logPutStr(lp, strp, fmt) \
  logPutCharCZ((lp), (strp), (fmt))

#if LOG_USE_PRINTF || LOG_USE_PRINTFC || defined(__DOXYGEN__)

/**
 * @brief Shorthand for @p logPutvf().
 */
#define logprintvf \
    logPutvfI

/**
 * @brief Shorthand for @p logPutf().
 */
#define logprintf \
  logPutfI

/**
 * @brief Shorthand for @p logPutvfc().
 */
#define logprintvfc \
  logPutvfcI

/**
 * @brief Shorthand for @p logPutfc().
 */
#define logprintfc \
  logPutfcI

#endif

/*===========================================================================*/
/* GLOBAL VARIABLES                                                          */
/*===========================================================================*/

extern const logfmt_t log_nofmt;
extern const logfmt_t log_ptrfmt;

/*===========================================================================*/
/* GLOBAL PROTOTYPES                                                         */
/*===========================================================================*/

#if defined(__cplusplus)
extern "C" {
#endif

/*~~~ DRIVER ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

void logObjectInit(LogDriver *lp);
void logStart(LogDriver *lp, const LogConfig *cfgp);
void logStop(LogDriver *lp);
void logFlush(LogDriver *lp);
void logResetI(LogDriver *lp);
void logReset(LogDriver *lp);
msg_t log_flush_thread(void *p);

/*~~~ INTERRUPT CLASS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

msg_t logPutDCharNI  (LogDriver *lp, const char *strp, uint32_t length);
msg_t logPutDCharZI  (LogDriver *lp, const char *strp);
msg_t logPutDCharCNI (LogDriver *lp, const char *strp, uint32_t length);
msg_t logPutDCharCZI (LogDriver *lp, const char *strp);
msg_t logPutDCstrI   (LogDriver *lp, const char *strp);

msg_t logPutCharI    (LogDriver *lp, char          c, logfmt_t fmt);
msg_t logPutStrNI    (LogDriver *lp, const char *strp, uint32_t length, logfmt_t fmt);
msg_t logPutCstrNI   (LogDriver *lp, const char *strp, uint32_t length, logfmt_t fmt);
#if LOG_USE_WCHAR
msg_t logPutWcharI   (LogDriver *lp, wchar_t          c, logfmt_t fmt);
msg_t logPutWstrNI   (LogDriver *lp, const wchar_t *strp, uint32_t length, logfmt_t fmt);
msg_t logPutWcstrNI  (LogDriver *lp, const wchar_t *strp, uint32_t length, logfmt_t fmt);
#endif /* LOG_USE_WCHAR */

msg_t logPutPtrI     (LogDriver *lp, const void *ptr, logfmt_t fmt);

#if LOG_USE_UINTS
msg_t logPutUint8I   (LogDriver *lp, uint8_t   value, logfmt_t fmt);
msg_t logPutUint16I  (LogDriver *lp, uint16_t  value, logfmt_t fmt);
msg_t logPutUint32I  (LogDriver *lp, uint32_t  value, logfmt_t fmt);
msg_t logPutUint64I  (LogDriver *lp, uint64_t  value, logfmt_t fmt);
#endif /* LOG_USE_UINTS */

#if LOG_USE_INTS
msg_t logPutInt8I    (LogDriver *lp, int8_t    value, logfmt_t fmt);
msg_t logPutInt16I   (LogDriver *lp, int16_t   value, logfmt_t fmt);
msg_t logPutInt32I   (LogDriver *lp, int32_t   value, logfmt_t fmt);
msg_t logPutInt64I   (LogDriver *lp, int64_t   value, logfmt_t fmt);
#endif /* LOG_USE_INTS */

#if LOG_USE_OCTALS
msg_t logPutOct8I    (LogDriver *lp, uint8_t   value, logfmt_t fmt);
msg_t logPutOct16I   (LogDriver *lp, uint16_t  value, logfmt_t fmt);
msg_t logPutOct32I   (LogDriver *lp, uint32_t  value, logfmt_t fmt);
msg_t logPutOct64I   (LogDriver *lp, uint64_t  value, logfmt_t fmt);
#endif /* LOG_USE_OCTALS */

msg_t logPutHex8I    (LogDriver *lp, uint8_t   value, logfmt_t fmt);
msg_t logPutHex16I   (LogDriver *lp, uint16_t  value, logfmt_t fmt);
msg_t logPutHex32I   (LogDriver *lp, uint32_t  value, logfmt_t fmt);
msg_t logPutHex64I   (LogDriver *lp, uint64_t  value, logfmt_t fmt);

#if LOG_USE_FLOATS
msg_t logPutFloatI   (LogDriver *lp, float     value, logfmt_t fmt);
msg_t logPutDoubleI  (LogDriver *lp, double    value, logfmt_t fmt);
msg_t logPutFloatEI  (LogDriver *lp, float     value, logfmt_t fmt);
msg_t logPutDoubleEI (LogDriver *lp, double    value, logfmt_t fmt);
msg_t logPutFloatGI  (LogDriver *lp, float     value, logfmt_t fmt);
msg_t logPutDoubleGI (LogDriver *lp, double    value, logfmt_t fmt);
#endif /* LOG_USE_FLOATS */

#if LOG_USE_PRINTF
msg_t logPutvfI  (LogDriver *lp, const char *fmt, va_list *vap);
msg_t logPutfI   (LogDriver *lp, const char *fmt, ...);
#endif /* LOG_USE_PRINTF */
#if LOG_USE_PRINTFC
msg_t logPutvfcI (LogDriver *lp, const char *fmt, va_list *vap);
msg_t logPutfcI  (LogDriver *lp, const char *fmt, ...);
#endif /* LOG_USE_PRINTFC */

/*~~~ USER CLASS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

msg_t logPutRawTimeout(LogDriver *lp, logsym_t raw, systime_t time);
msg_t logPutRaw(LogDriver *lp, logsym_t raw);

msg_t logPutDCharN  (LogDriver *lp, const char *strp, uint32_t length);
msg_t logPutDCharZ  (LogDriver *lp, const char *strp);
msg_t logPutDCharCN (LogDriver *lp, const char *strp, uint32_t length);
msg_t logPutDCharCZ (LogDriver *lp, const char *strp);
msg_t logPutDCstr   (LogDriver *lp, const char *strp);

msg_t logPutChar    (LogDriver *lp, char           c, logfmt_t fmt);
msg_t logPutStrN    (LogDriver *lp, const char *strp, uint32_t length, logfmt_t fmt);
msg_t logPutCstrN   (LogDriver *lp, const char *strp, uint32_t length, logfmt_t fmt);
#if LOG_USE_WCHAR
msg_t logPutWchar   (LogDriver *lp, wchar_t           c, logfmt_t fmt);
msg_t logPutWstrN   (LogDriver *lp, const wchar_t *strp, uint32_t length, logfmt_t fmt);
msg_t logPutWcstrN  (LogDriver *lp, const wchar_t *strp, uint32_t length, logfmt_t fmt);
#endif /* LOG_USE_WCHAR */

msg_t logPutPtr     (LogDriver *lp, const void *ptr, logfmt_t fmt);

#if LOG_USE_UINTS
msg_t logPutUint8   (LogDriver *lp, uint8_t   value, logfmt_t fmt);
msg_t logPutUint16  (LogDriver *lp, uint16_t  value, logfmt_t fmt);
msg_t logPutUint32  (LogDriver *lp, uint32_t  value, logfmt_t fmt);
msg_t logPutUint64  (LogDriver *lp, uint64_t  value, logfmt_t fmt);
#endif /* LOG_USE_UINTS */

#if LOG_USE_INTS
msg_t logPutInt8    (LogDriver *lp, int8_t    value, logfmt_t fmt);
msg_t logPutInt16   (LogDriver *lp, int16_t   value, logfmt_t fmt);
msg_t logPutInt32   (LogDriver *lp, int32_t   value, logfmt_t fmt);
msg_t logPutInt64   (LogDriver *lp, int64_t   value, logfmt_t fmt);
#endif /* LOG_USE_ITNS */

#if LOG_USE_OCTALS
msg_t logPutOct8    (LogDriver *lp, uint8_t   value, logfmt_t fmt);
msg_t logPutOct16   (LogDriver *lp, uint16_t  value, logfmt_t fmt);
msg_t logPutOct32   (LogDriver *lp, uint32_t  value, logfmt_t fmt);
msg_t logPutOct64   (LogDriver *lp, uint64_t  value, logfmt_t fmt);
#endif /* LOG_USE_OCTALS */

msg_t logPutHex8    (LogDriver *lp, uint8_t   value, logfmt_t fmt);
msg_t logPutHex16   (LogDriver *lp, uint16_t  value, logfmt_t fmt);
msg_t logPutHex32   (LogDriver *lp, uint32_t  value, logfmt_t fmt);
msg_t logPutHex64   (LogDriver *lp, uint64_t  value, logfmt_t fmt);

#if LOG_USE_FLOATS
msg_t logPutFloat   (LogDriver *lp, float     value, logfmt_t fmt);
msg_t logPutDouble  (LogDriver *lp, double    value, logfmt_t fmt);
msg_t logPutFloatE  (LogDriver *lp, float     value, logfmt_t fmt);
msg_t logPutDoubleE (LogDriver *lp, double    value, logfmt_t fmt);
msg_t logPutFloatG  (LogDriver *lp, float     value, logfmt_t fmt);
msg_t logPutDoubleG (LogDriver *lp, double    value, logfmt_t fmt);
#endif /* LOG_USE_FLOATS */

#if LOG_USE_PRINTF
msg_t logPutvf  (LogDriver *lp, const char *fmtstrp, va_list *vap);
msg_t logPutf   (LogDriver *lp, const char *fmtstrp, ...);
#endif /* LOG_USE_PRINTF */
#if LOG_USE_PRINTFC
msg_t logPutvfc (LogDriver *lp, const char *fmtstrp, va_list *vap);
msg_t logPutfc  (LogDriver *lp, const char *fmtstrp, ...);
#endif /* LOG_USE_PRINTFC */

#if defined(__cplusplus)
}
#endif

#endif /* HAL_USE_LOG */
#endif /* _LOG_H_ */
