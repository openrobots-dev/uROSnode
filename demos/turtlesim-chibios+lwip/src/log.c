/*
Copyright (c) 2012-2013, Andrea Zoppi. All rights reserved.

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

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include "log.h"

#include <ch.h>
#include <hal.h>
#include <stdarg.h>
#if LOG_USE_FLOATS
#include <math.h>
#endif /* LOG_USE_FLOATS */

/*===========================================================================*/
/* TYPES & MACROS                                                            */
/*===========================================================================*/

#define DEF_FLOAT_DIGITS    5           /* Decimal digits of float numbers.*/
#define DEF_FLOAT_PREC      100000      /* Precision of float numbers.*/

/*===========================================================================*/
/* LOCAL VARIABLES                                                           */
/*===========================================================================*/

/**
 * @brief   Lowercase hex characters.
 */
static const char hex_lowercase[16] = {
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};

/**
 * @brief   Uppercase hex characters.
 */
static const char hex_uppercase[16] = {
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

/*===========================================================================*/
/* GLOBAL VARIABLES                                                          */
/*===========================================================================*/

/**
 * @brief   No format, just prints the value itself.
 */
const logfmt_t log_nofmt = {
  { 0, 0, 0x00, 0x00 }
};

/**
 * @brief   Pointer format.
 */
const logfmt_t log_ptrfmt = {
  { 0, 8, LOGFF_HASH | LOGFF_UPPER | LOGFF_DIGITS, 0x00 }
};

/*===========================================================================*/
/* LOCAL PROTOTYPES                                                          */
/*===========================================================================*/

static size_t log_strlen(const char *strp);
#if LOG_USE_WCHAR
static size_t log_wstrlen(const wchar_t *strp);
#endif /* LOG_USE_WCHAR */
#if LOG_USE_UINTS
static char *log_ultoa_div(char *strbuf, uint32_t value, uint32_t divisor);
#endif /* LOG_USE_UINTS */
static const char *log_parse_fmt(const char *fmtstrp, va_list *vap, logfmt_t *fmtp);
static msg_t log_put_by_fmt_i(LogDriver *lp, va_list *vap, logfmt_t fmt);

static msg_t log_fetch_dcstr  (Mailbox *mbp, const char **cstrpp);
static msg_t log_fetch_raw    (Mailbox *mbp, logsym_t *raw);
#if LOG_USE_UINTS
static msg_t log_fetch_uint8  (Mailbox *mbp, uint8_t  *valuep, logfmt_t *fmtp);
static msg_t log_fetch_uint16 (Mailbox *mbp, uint16_t *valuep, logfmt_t *fmtp);
static msg_t log_fetch_uint32 (Mailbox *mbp, uint32_t *valuep, logfmt_t *fmtp);
#if LOG_USE_64BIT
static msg_t log_fetch_uint64 (Mailbox *mbp, uint64_t *valuep, logfmt_t *fmtp);
#endif /* LOG_USE_64BIT */
#endif /* LOG_USE_UINTS */
#if LOG_USE_FLOATS
static msg_t log_fetch_fp     (Mailbox *mbp, float    *valuep, logfmt_t *fmtp);
static msg_t log_fetch_dp     (Mailbox *mbp, double   *valuep, logfmt_t *fmtp);
#endif /* LOG_USE_FLOATS */

static msg_t log_stream_dchn (BaseSequentialStream *sp, Mailbox *mbp);
static msg_t log_stream_dchz (BaseSequentialStream *sp, Mailbox *mbp);
static msg_t log_stream_strn (BaseSequentialStream *sp, Mailbox *mbp);
static msg_t log_stream_cstrn(BaseSequentialStream *sp, Mailbox *mbp);
msg_t logStreamChar   (BaseSequentialStream *sp, char     value, logfmt_t fmt);
#if LOG_USE_WCHAR
msg_t logStreamWchar  (BaseSequentialStream *sp, uint16_t value, logfmt_t fmt);
#endif /* LOG_USE_WCHAR */
#if LOG_USE_UINTS
msg_t logStreamUint32 (BaseSequentialStream *sp, uint32_t value, logfmt_t fmt);
#if LOG_USE_64BIT
msg_t logStreamUint64 (BaseSequentialStream *sp, uint64_t value, logfmt_t fmt);
#endif /* LOG_USE_64BIT */
#endif /* LOG_USE_UINTS */
#if LOG_USE_INTS
msg_t logStreamInt32  (BaseSequentialStream *sp, int32_t  value, logfmt_t fmt);
#if LOG_USE_64BIT
msg_t logStreamInt64  (BaseSequentialStream *sp, int64_t  value, logfmt_t fmt);
#endif /* LOG_USE_64BIT */
#endif /* LOG_USE_INTS */
#if LOG_USE_OCTALS
msg_t logStreamOct32  (BaseSequentialStream *sp, uint32_t value, logfmt_t fmt);
msg_t logStreamOct64  (BaseSequentialStream *sp, uint64_t value, logfmt_t fmt);
#endif /* LOG_USE_OCTALS */
msg_t logStreamHex32  (BaseSequentialStream *sp, uint32_t value, logfmt_t fmt);
#if LOG_USE_64BIT
msg_t logStreamHex64  (BaseSequentialStream *sp, uint64_t value, logfmt_t fmt);
#endif /* LOG_USE_64BIT */
#if LOG_USE_FLOATS
msg_t logStreamDoubleE(BaseSequentialStream *sp, double   value, logfmt_t fmt);
msg_t logStreamDoubleG(BaseSequentialStream *sp, double   value, logfmt_t fmt);
#endif /* LOG_USE_FLOATS */

msg_t logStreamBufValue(BaseSequentialStream *sp, char sign,
                        const char *prefixp, unsigned prelen,
                        const char *valuep, unsigned length,
                        logfmt_t fmt);
static msg_t log_stream_next(BaseSequentialStream *sp, Mailbox *mbp);

/*===========================================================================*/
/* LOCAL FUNCTIONS                                                           */
/*===========================================================================*/

/*~~~ HELPERS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/**
 * @brief   Length of an ASCII string.
 *
 * @param[in] strp
 *          Zero-terminated ASCII string.
 * @return
 *          String length, in characters.
 *
 * @notapi
 */
static size_t log_strlen(const char *strp) {

  size_t length;
  for (length = 0; *strp++; ++length) {}
  return length;
}

/**
 * @brief   Length of an Unicode string.
 *
 * @param[in] strp
 *          Zero-terminated Unicode string.
 * @return
 *          String length, in characters.
 *
 * @notapi
 */
#if LOG_USE_WCHAR || defined(__DOXYGEN__)
static size_t log_wstrlen(const wchar_t *strp) {

  size_t length;
  for (length = 0; *strp++; ++length) {}
  return length;
}
#endif /* LOG_USE_WCHAR */

/**
 * @brief   Writes a decimal representation string of an unsigned integer.
 *
 * @param[in] strbuf
 *          String buffer to hold the written string.
 * @param[in] value
 *          Unsigned integer value to be converted.
 * @param[in] divisor
 *          Decimal divisor; 0 for an integral value, >0 to indicate the
 *          decimal multiplier (e.g. 1000 for 1/1000 values).
 * @return
 *          The pointer past the last written character.
 *
 * @notapi
 */
#if (LOG_USE_UINTS || LOG_USE_INTS || LOG_USE_FLOATS) || defined(__DOXYGEN__)
static char *log_ultoa_div(char *strbuf, uint32_t value, uint32_t divisor) {

  char      *ptr;

  chDbgCheck(strbuf != NULL, "log_ultoa_div");

  if (divisor == 0) { divisor = value; }
  ptr = strbuf + 10;
  do {
    *--ptr = (char)(value % 10) + '0';
    value /= 10;
    divisor /= 10;
  } while (divisor != 0);
  for (divisor = (uint32_t)(strbuf + 10 - ptr); divisor--;) {
    *strbuf++ = *ptr++;
  }
  return strbuf;
}
#endif /* LOG_USE_UINTS || LOG_USE_INTS || LOG_USE_FLOATS */

/*~~~ DRIVER ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/**
 * @brief   Initializes a LogDriver object.
 *
 * @param[out] lp
 *          Pointer to the @p LogDriver structure to be initialized.
 *
 * @init
 */
void logObjectInit(LogDriver *lp) {

  chDbgCheck(lp != NULL, "logObjectInit");

  lp->config = NULL;
  lp->state = LOG_UNINIT;
  chMtxInit(&lp->fetchmtx);
}

/**
 * @brief   Configures and starts a log driver.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] cfgp
 *          Pointer to the log configuration.
 *
 * @api
 */
void logStart(LogDriver *lp, const LogConfig *cfgp) {

  chDbgCheck(lp != NULL, "logStart");
  chDbgCheck(cfgp != NULL, "logStart");
  chDbgCheck(cfgp->stream != NULL, "logStart");
  chDbgCheck(cfgp->bufp != NULL, "logStart");
  chDbgCheck(cfgp->buflen > 0, "logStart");
  chDbgCheck(lp->state == LOG_UNINIT || lp->state == LOG_STOP, "logStart");

  lp->config = cfgp;
  chMBInit(&lp->mb, (msg_t*)lp->config->bufp, lp->config->buflen);
  lp->state = LOG_READY;
}

/**
 * @brief   Stops a log driver.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 *
 * @api
 */
void logStop(LogDriver *lp) {

  chDbgCheck(lp != NULL, "logStop");
  chDbgCheck(lp->state == LOG_READY, "logStop");

  chSysLock();
  lp->state = LOG_STOP;
  logResetI(lp);
  chMtxUnlockAll();
  chSchRescheduleS();
  chSysUnlock();
}

/**
 * @brief   Flushes all enqueued log symbols.
 * @details This function is blocking, it does not exit until all the buffered
 *          symbols are streamed.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 *
 * @api
 */
void logFlush(LogDriver *lp) {

  Mailbox *mbp;
  BaseSequentialStream *sp;
  msg_t err;

  chDbgCheck(lp != NULL, "logFlush");

  mbp = &lp->mb;
  sp = lp->config->stream;
  chSysLock();
  if (chMBGetUsedCountI(mbp) > 0) {
    do {
      chMtxLockS(&lp->fetchmtx);
      chSysUnlock();
      err = log_stream_next(sp, mbp);  /* TODO: Do it only in the flush thread, to optimize stack usage.*/
      chSysLock();
      chMtxUnlockS();
    } while (chMBGetUsedCountI(mbp) > 0 && err == RDY_OK);
  }
  chSysUnlock();
}

/**
 * @brief   Resets the log queue.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 *
 * @iclass
 */
void logResetI(LogDriver *lp) {

  Mailbox *mbp = &lp->mb;

  chDbgCheckClassI();

  mbp->mb_wrptr = mbp->mb_rdptr = mbp->mb_buffer;
  chSemResetI(&mbp->mb_emptysem, mbp->mb_top - mbp->mb_buffer);
  chSemResetI(&mbp->mb_fullsem, 0);
}

/**
 * @brief   Resets the log queue.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 *
 * @api
 */
void logReset(LogDriver *lp) {

  chDbgCheck(lp != NULL, "logReset");

  chSysLock();
  logResetI(lp);
  chSchRescheduleS();
  chSysUnlock();
}

/**
 * @brief   Thread function which keeps flushing a log.
 *
 * @param[in] p
 *          Pointer to an initialized @p LogDriver object.
 * @return
 *          Thread exit status.
 *
 * @notapi
 */
msg_t log_flush_thread(void *p) {

  LogDriver *lp = (LogDriver*)p;
  while (TRUE) {
    logFlush(lp);
  }
  return RDY_OK;
}

/*~~~ INTERRUPT CLASS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/**
 * @brief   Parses the format record out of a format string.
 *
 * @param[in] fmtstrp
 *          Format string.
 * @param[in] vap
 *          Pointer to a @p va_list object, for further parameters.
 * @param[out] fmtp
 *          Pointer to the returned format record.
 * @return
 *          Pointer past the last parsed character.
 *
 * @notapi
 */
static const char *log_parse_fmt(const char *fmtstrp, va_list *vap, logfmt_t *fmtp) {

  logfmt_t fmt = log_nofmt;
  bool_t done;
  char chlen;
  unsigned num, mult;

  chDbgCheck(fmtstrp != NULL, "log_parse_fmt");
  chDbgCheck(fmtp != NULL, "log_parse_fmt");

  /* Decode flags.*/
  for (done = FALSE; !done; ++fmtstrp) {
    switch (*fmtstrp) {
    case '-': { fmt.fmt.flags |= LOGFF_LEFT; break; }
    case '+': { fmt.fmt.flags |= LOGFF_SIGN; break; }
    case ' ': { fmt.fmt.flags |= LOGFF_SPACE; break; }
    case '#': { fmt.fmt.flags |= LOGFF_HASH; break; }
    case '0': { fmt.fmt.flags |= LOGFF_ZERO; break; }
    default:  { done = TRUE; --fmtstrp; break; }
    }
  }

  /* Decode width.*/
  num = 0;
  if (*fmtstrp == '*') {
    fmt.fmt.flags |= LOGFF_WIDTH;
    num = va_arg(*vap, unsigned);
  } else {
    if (*fmtstrp >= '0' && *fmtstrp <= '10') {
      fmt.fmt.flags |= LOGFF_WIDTH;
      for (mult = 1, num = 0; *fmtstrp >= '0' && *fmtstrp <= '10' && mult <= 100; mult *= 10) {
        num += (*fmtstrp++ - '0') * mult;
      }
    }
  }
  if (fmt.fmt.flags & LOGFF_WIDTH) {
    if (num > 0xFF) {
      /* Limit the maximum width to 255 characters.*/
      num = 0xFF;
    }
    fmt.fmt.width = (uint8_t)num;
  }

  /* Decode precision.*/
  num = 0;
  if (*fmtstrp == '.') {
    fmt.fmt.flags |= LOGFF_DIGITS;
    if (*++fmtstrp == '*') {
      num = va_arg(*vap, unsigned);
    } else {
      for (mult = 1, num = 0; *fmtstrp >= '0' && *fmtstrp <= '10' && mult <= 100; mult *= 10) {
        num += (*fmtstrp++ - '0') * mult;
      }
    }
  }
  if (fmt.fmt.flags & LOGFF_DIGITS) {
    if (num > 0xFF) {
      /* Limit the maximum precision to 255 characters.*/
      num = 0xFF;
    }
    fmt.fmt.digits = (uint8_t)num;
  }

  /* Decode length.*/
  switch (fmtstrp[0]) {
  case 'h': {
    if (fmtstrp[1] == 'h') { chlen = 'H'; fmtstrp += 2; break; }
    else                   { chlen = 'h'; ++fmtstrp; break; }
  }
  case 'l': {
    if (fmtstrp[1] == 'l') { chlen = 'L'; fmtstrp += 2; break; }
    else                   { chlen = 'l'; ++fmtstrp; break; }
  }
  case 'j': { chlen = 'j'; ++fmtstrp; break; }
  case 'z': { chlen = 'z'; ++fmtstrp; break; }
  case 't': { chlen = 't'; ++fmtstrp; break; }
  default:  { chlen = 0; break; }
  }

  /* Decode specifier.*/
  switch (*fmtstrp) {
  /* Signed integer.*/
#if LOG_USE_INTS
  case 'd':
  case 'i': {
    switch (chlen) {
    case 0:   { fmt.fmt.type = LOGT_INT32; break; }
    case 'H': { fmt.fmt.type = LOGT_INT8;  break; }
    case 'h': { fmt.fmt.type = LOGT_INT16; break; }
    case 'l': { fmt.fmt.type = LOGT_INT32; break; }
    case 'L': { fmt.fmt.type = LOGT_INT64; break; }
    case 'j': { fmt.fmt.type = LOGT_INTMAX; break; }
    case 'z': { fmt.fmt.type = LOGT_INTSIZE; break; }
    case 't': { fmt.fmt.type = LOGT_INTPTRDIFF; break; }
    default:  { chDbgPanic("Format type parse error"); break; }
    }
    break;
  }
#endif /* LOG_USE_INTS */
#if LOG_USE_UINTS
  /* Unsigned integer.*/
  case 'u': {
    switch (chlen) {
    case 0:   { fmt.fmt.type = LOGT_UINT32; break; }
    case 'H': { fmt.fmt.type = LOGT_UINT8;  break; }
    case 'h': { fmt.fmt.type = LOGT_UINT16; break; }
    case 'l': { fmt.fmt.type = LOGT_UINT32; break; }
    case 'L': { fmt.fmt.type = LOGT_UINT64; break; }
    case 'j': { fmt.fmt.type = LOGT_UINTMAX; break; }
    case 'z': { fmt.fmt.type = LOGT_UINTSIZE; break; }
    case 't': { fmt.fmt.type = LOGT_UINTPTRDIFF; break; }
    default:  { chDbgPanic("Format type parse error"); break; }
    }
    break;
  }
#endif /* LOG_USE_UINTS */
#if LOG_USE_OCTALS
  /* Octal.*/
  case 'o': {
    switch (chlen) {
    case 0:   { fmt.fmt.type = LOGT_OCT32; break; }
    case 'H': { fmt.fmt.type = LOGT_OCT8;  break; }
    case 'h': { fmt.fmt.type = LOGT_OCT16; break; }
    case 'l': { fmt.fmt.type = LOGT_OCT32; break; }
    case 'L': { fmt.fmt.type = LOGT_OCT64; break; }
    case 'j': { fmt.fmt.type = LOGT_OCTMAX; break; }
    case 'z': { fmt.fmt.type = LOGT_OCTSIZE; break; }
    case 't': { fmt.fmt.type = LOGT_OCTPTRDIFF; break; }
    default:  { chDbgPanic("Format type parse error"); break; }
    }
    break;
  }
#endif /* LOG_USE_OCTALS */
  /* Hexadecimal.*/
  case 'X':   { fmt.fmt.flags |= LOGFF_UPPER; }
    /* no break */
  case 'x': {
    switch (chlen) {
    case 0:   { fmt.fmt.type = LOGT_HEX32; break; }
    case 'H': { fmt.fmt.type = LOGT_HEX8;  break; }
    case 'h': { fmt.fmt.type = LOGT_HEX16; break; }
    case 'l': { fmt.fmt.type = LOGT_HEX32; break; }
    case 'L': { fmt.fmt.type = LOGT_HEX64; break; }
    case 'j': { fmt.fmt.type = LOGT_HEXMAX; break; }
    case 'z': { fmt.fmt.type = LOGT_HEXSIZE; break; }
    case 't': { fmt.fmt.type = LOGT_HEXPTRDIFF; break; }
    default:  { chDbgPanic("Format type parse error"); break; }
    }
    break;
  }
#if LOG_USE_FLOATS
  /* Floating point.*/
  case 'F': { fmt.fmt.flags |= LOGFF_UPPER; }
    /* no break */
  case 'f': {
    if (chlen == 0) { fmt.fmt.type = LOGT_DP; }
    else { chDbgPanic("Format type parse error"); }
    break;
  }
  /* Floating point, scientific notation.*/
  case 'E': { fmt.fmt.flags |= LOGFF_UPPER; }
    /* no break */
  case 'e': {
    if (chlen == 0) { fmt.fmt.type = LOGT_DPE; }
    else { chDbgPanic("Format type parse error"); }
    break;
  }
  /* Floating point, shortest form.*/
  case 'G': { fmt.fmt.flags |= LOGFF_UPPER; }
    /* no break */
  case 'g': {
    if (chlen == 0) { fmt.fmt.type = LOGT_DPG; }
    else { chDbgPanic("Format type parse error"); }
    break;
  }
#endif /* LOG_USE_FLOATS */
  /* Character.*/
  case 'c': {
    switch (chlen) {
    case 0:   { fmt.fmt.type = LOGT_CHAR;  break; }
#if LOG_USE_WCHAR
    case 'l': { fmt.fmt.type = LOGT_WCHAR; break; }
#endif /* LOG_USE_WCHAR */
    default:  { chDbgPanic("Format type parse error"); break; }
    }
    break;
  }
  /* Copied string.*/
  case 's': {
    switch (chlen) {
    case 0:   { fmt.fmt.type = LOGT_STRN; break; }
#if LOG_USE_WCHAR
    case 'l': { fmt.fmt.type = LOGT_WSTRN; break; }
#endif /* LOG_USE_WCHAR */
    default:  { chDbgPanic("Format type parse error"); }
    }
    break;
  }
  /* Constant string.*/
  case 'S': {
    switch (chlen) {
    case 0:   { fmt.fmt.type = LOGT_CSTRN; break; }
#if LOG_USE_WCHAR
    case 'l': { fmt.fmt.type = LOGT_WCSTRN; break; }
#endif /* LOG_USE_WCHAR */
    default:  { chDbgPanic("Format type parse error"); }
    }
    break;
  }
  /* Pointer.*/
  case 'p': {
    if (chlen == 0) { fmt.fmt.type = LOGT_PTR; }
    else { chDbgPanic("Format type parse error"); }
    break;
  }
  /* Unknown.*/
  default: { chDbgPanic("Format type parse error"); break; }
  }

  /* Return the decoded format and the next char pointer.*/
  *fmtp = fmt;
  return fmtstrp + 1;
}

/**
 * @brief   Enqueues a log symbol given its format and variable arguments.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] vap
 *          Pointer to the @p va_list from which values are extracted.
 * @param[in] fmt
 *          Format record, with a valid type field.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @iclass
 */
static msg_t log_put_by_fmt_i(LogDriver *lp, va_list *vap, logfmt_t fmt) {

  msg_t err = RDY_OK;

  switch (fmt.fmt.type) {
  case LOGT_CHAR: {
    char c = (char)va_arg(*vap, int);
    err = logPutCharI(lp, c, fmt);
    break;
  }
  case LOGT_STRN: {
    const char *strp = va_arg(*vap, const char *);
    err = logPutStrNI(lp, strp, (uint32_t)log_strlen(strp), fmt);
    break;
  }
  case LOGT_CSTRN: {
    const char *strp = va_arg(*vap, const char *);
    err = logPutCstrNI(lp, strp, (uint32_t)log_strlen(strp), fmt);
    break;
  }
#if LOG_USE_WCHAR
  case LOGT_WCHAR: {
      wchar_t ch = (char)va_arg(*vap, int);
      err = logPutWcharI(lp, ch, fmt);
      break;
    }
    case LOGT_WSTRN: {
    const wchar_t *strp = va_arg(*vap, const wchar_t *);
    err = logPutWstrNI(lp, strp, (uint32_t)log_wstrlen(strp), fmt);
    break;
  }
  case LOGT_WCSTRN: {
    const wchar_t *strp = va_arg(*vap, const wchar_t *);
    err = logPutWcstrNI(lp, strp, (uint32_t)log_wstrlen(strp), fmt);
    break;
  }
#endif /* LOG_USE_WCHAR */
  case LOGT_PTR: {
    const void *ptr = va_arg(*vap, const void *);
    err = logPutPtrI(lp, ptr, fmt);
    break;
  }
#if LOG_USE_INTS
  case LOGT_INT8: {
    int8_t value = (int8_t)va_arg(*vap, int);
    err = logPutInt8I(lp, value, fmt);
    break;
  }
  case LOGT_INT16: {
    int16_t value = (int16_t)va_arg(*vap, int);
    err = logPutInt16I(lp, value, fmt);
    break;
  }
  case LOGT_INT32: {
    int32_t value = va_arg(*vap, int32_t);
    err = logPutInt32I(lp, value, fmt);
    break;
  }
#if LOG_USE_64BIT
  case LOGT_INT64: {
    int64_t value = va_arg(*vap, int64_t);
    err = logPutInt64I(lp, value, fmt);
    break;
  }
#endif /* LOG_USE_64BITS */
#endif /* LOG_USE_INTS */
#if LOG_USE_UINTS
  case LOGT_UINT8: {
    uint8_t value = (uint8_t)va_arg(*vap, unsigned);
    err = logPutUint8I(lp, value, fmt);
    break;
  }
  case LOGT_UINT16: {
    uint16_t value = (uint16_t)va_arg(*vap, unsigned);
    err = logPutUint16I(lp, value, fmt);
    break;
  }
  case LOGT_UINT32: {
    uint32_t value = va_arg(*vap, uint32_t);
    err = logPutUint32I(lp, value, fmt);
    break;
  }
#if LOG_USE_64BIT
  case LOGT_UINT64: {
    uint64_t value = va_arg(*vap, uint64_t);
    err = logPutUint64I(lp, value, fmt);
    break;
  }
#endif /* LOG_USE_64BIT */
#endif /* LOG_USE_UINTS */
#if LOG_USE_OCTALS
  case LOGT_OCT8: {
    uint8_t value = (uint8_t)va_arg(*vap, unsigned);
    err = logPutOct8I(lp, value, fmt);
    break;
  }
  case LOGT_OCT16: {
    uint16_t value = (uint16_t)va_arg(*vap, unsigned);
    err = logPutOct16I(lp, value, fmt);
    break;
  }
  case LOGT_OCT32: {
    uint32_t value = va_arg(*vap, uint32_t);
    err = logPutOct32I(lp, value, fmt);
    break;
  }
#if LOG_USE_64BIT
  case LOGT_OCT64: {
    uint64_t value = va_arg(*vap, uint64_t);
    err = logPutOct64I(lp, value, fmt);
    break;
  }
#endif /* LOG_USE_64BIT */
#endif /* LOG_USE_OCTALS */
  case LOGT_HEX8: {
    uint8_t value = (uint8_t)va_arg(*vap, unsigned);
    err = logPutHex8I(lp, value, fmt);
    break;
  }
  case LOGT_HEX16: {
    uint16_t value = (uint16_t)va_arg(*vap, unsigned);
    err = logPutHex16I(lp, value, fmt);
    break;
  }
  case LOGT_HEX32: {
    uint32_t value = va_arg(*vap, uint32_t);
    err = logPutHex32I(lp, value, fmt);
    break;
  }
#if LOG_USE_64BIT
  case LOGT_HEX64: {
    uint64_t value = va_arg(*vap, uint64_t);
    err = logPutHex64I(lp, value, fmt);
    break;
  }
#endif /* LOG_USE_64BIT */
#if LOG_USE_FLOATS
  case LOGT_FP: {
    float value = (float)va_arg(*vap, double);
    err = logPutFloatI(lp, value, fmt);
    break;
  }
  case LOGT_DP: {
    double value = va_arg(*vap, double);
    err = logPutDoubleI(lp, value, fmt);
    break;
  }
  case LOGT_FPE: {
    float value = (float)va_arg(*vap, double);
    err = logPutFloatEI(lp, value, fmt);
    break;
  }
  case LOGT_DPE: {
    double value = va_arg(*vap, double);
    err = logPutDoubleEI(lp, value, fmt);
    break;
  }
  case LOGT_FPG: {
    float value = (float)va_arg(*vap, double);
    err = logPutFloatGI(lp, value, fmt);
    break;
  }
  case LOGT_DPG: {
    double value = va_arg(*vap, double);
    err = logPutDoubleGI(lp, value, fmt);
    break;
  }
#endif /* LOG_USE_FLOATS */
  default: {
    chDbgPanic("Unhandled format type");
    break;
  }
  }
  return err;
}

/**
 * @brief   Enqueues a direct string to a log.
 * @details The total length is stored in the lowest 24 bits of the first raw
 *          log symbol. The string bytes are stored in the subsequent raw log
 *          symbols, with LSB first in each symbol (bytes group).
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] strp
 *          String to be enqueued.
 * @param[in] length
 *          String length.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @iclass
 */
msg_t logPutDCharNI(LogDriver *lp, const char *strp, uint32_t length) {

  chDbgCheckClassI();
  if (logNumFreeI(lp) >= (((cnt_t)length + 7) >> 2)) {
    int32_t cnt;
    length &= 0x00FFFFFF;
    logPutRawI(lp, (LOGT_DCHN << LOGF_TSH) | length);
    for (cnt = (int32_t)length; cnt > 0; strp += sizeof(logsym_t), cnt -= sizeof(logsym_t)) {
      logPutRawI(lp, *(logsym_t*)strp);
    }
    return RDY_OK;
  } else {
    return RDY_TIMEOUT;
  }
}

/**
 * @brief   Enqueues a direct string to a log.
 * @details Characters are enqueued as a zero-terminated string. The first
 *          three bytes are part of the raw head symbol, LSB first.
 *          If at some point it is not possible to post further characters,
 *          a terminating zero byte is put as MSB of the last enqueued
 *          raw symbol.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] strp
 *          Null-terminated string to be enqueued.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @iclass
 */
msg_t logPutDCharZI(LogDriver *lp, const char *strp) {

  msg_t err;

  err = logPutRawI(lp, (LOGT_DCHZ << LOGF_TSH) | (*(logsym_t*)strp & ~LOGF_TMSK));
  if (err == RDY_OK) {
    if (strp[0] && strp[1] && strp[2]) {
      if (logNumFreeI(lp) > 0) {
        bool_t pending = FALSE;
        strp += sizeof(logsym_t) - 1;
        do {
          pending = strp[0] && strp[1] && strp[2] && strp[3];
          if (pending) {
            if (logNumFreeI(lp) > 1) {
              logPutRawI(lp, *(logsym_t*)strp);
              strp += sizeof(logsym_t);
            } else {
              /* Insert a terminator as MSB.*/
              logPutRawI(lp, *(logsym_t*)strp & ~((logsym_t)0xFFu << ((sizeof(logsym_t) - 1) * 8)));
              return RDY_TIMEOUT;
            }
          } else {
            logPutRawI(lp, *(logsym_t*)strp);
          }
        } while (pending);
      } else {
        return RDY_TIMEOUT;
      }
    }
    return RDY_OK;
  } else {
    return err;
  }
}

/**
 * @brief   Enqueues a direct string to a log.
 * @details Characters are enqueued three per each log symbol, LSB first.
 *          The total length is stored on the lowest 24 bits of the first raw
 *          log symbol.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] strp
 *          String to be enqueued.
 * @param[in] length
 *          String length.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @iclass
 */
msg_t logPutDCharCNI(LogDriver *lp, const char *strp, uint32_t length) {

  msg_t err = RDY_OK;

  for (; length >= 3 && err == RDY_OK; strp += 3, length -= 3) {
    err = logPutRawI(lp, (LOGT_DCH3 << LOGF_TSH) | (*(logsym_t*)strp & ~LOGF_TMSK));
  }
  if (err == RDY_OK) {
    if (length == 2) {
      err = logPutRawI(lp, (LOGT_DCH2 << LOGF_TSH) | (*(logsym_t*)strp & ~LOGF_TMSK));
    } else if (length == 1) {
      err = logPutRawI(lp, (LOGT_DCH1 << LOGF_TSH) | (*(logsym_t*)strp & ~LOGF_TMSK));
    }
  }
  return err;
}

/**
 * @brief   Enqueues a direct string to a log.
 * @details Characters are enqueued three per each raw log symbol, LSB first.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] strp
 *          Null-terminated string to be enqueued.
 * @param[in] length
 *          String length.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @iclass
 */
msg_t logPutDCharCZI(LogDriver *lp, const char *strp) {

  msg_t err = RDY_OK;
  bool_t pending = FALSE;

  do {
    pending = strp[0] && strp[1] && strp[2];
    if (pending) {
      err = logPutRawI(lp, (LOGT_DCH3 << LOGF_TSH) | (*(logsym_t*)strp & ~LOGF_TMSK));
      strp += sizeof(logsym_t) - 1;
    } else {
      if (strp[0]) {
        if (strp[1]) {
          err = logPutRawI(lp, (LOGT_DCH2 << LOGF_TSH) | (*(logsym_t*)strp & ~LOGF_TMSK));
        } else {
          err = logPutRawI(lp, (LOGT_DCH1 << LOGF_TSH) | (*(logsym_t*)strp & ~LOGF_TMSK));
        }
      }
    }
  } while (pending && err == RDY_OK);
  return err;
}

/**
 * @brief   Enqueues a direct constant string to a log.
 * @details Only the address is enqueued, so that constant strings are vapended
 *          with very low overhead.
 * @note    A string is said <i>constant</i> if its contents do not change
 *          between the call of this function and the end of a @ logFlush()
 *          operation.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] strp
 *          Constant string to be enqueued.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @iclass
 */
msg_t logPutDCstrI(LogDriver *lp, const char *strp) {

  chDbgCheckClassI();
  if (logNumFreeI(lp) >= 2) {
    logPutRawI(lp, LOGT_DCSTR << LOGF_TSH);
    logPutRawI(lp, (logsym_t)strp);
    return RDY_OK;
  } else {
    return RDY_TIMEOUT;
  }
}

/**
 * @brief   Enqueues a formatted character to a log.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] c
 *          Character to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @iclass
 */
msg_t logPutCharI(LogDriver *lp, char c, logfmt_t fmt) {

  chDbgCheckClassI();

  if (logNumFreeI(lp) >= 2) {
    logPutRawI(lp, (LOGT_CHAR << LOGF_TSH) | fmt.raw);
    logPutRawI(lp, (logsym_t)c);
    return RDY_OK;
  } else {
    return RDY_TIMEOUT;
  }
}

/**
 * @brief   Enqueues a formatted string to a log.
 * @details The total length is stored in the second raw log symbol.
 *          The string bytes are stored in the subsequent raw log symbols, with
 *          LSB first in each symbol (bytes group).
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] strp
 *          String to be enqueued.
 * @param[in] length
 *          String length.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @iclass
 */
msg_t logPutStrNI(LogDriver *lp, const char *strp, uint32_t length, logfmt_t fmt) {

  if (logNumFreeI(lp) >= (cnt_t)((length + (4 + 4 + 3)) >> 2)) {
    logPutRawI(lp, (LOGT_STRN << LOGF_TSH) | fmt.raw);
    logPutRawI(lp, (logsym_t)length);
    for (; length >= sizeof(logsym_t); strp += sizeof(logsym_t), length -= sizeof(logsym_t)) {
      logPutRawI(lp, *(logsym_t*)strp);
    }
    if (length > 0) {
      logPutRawI(lp, *(logsym_t*)strp);
    }
    return RDY_OK;
  } else {
    return RDY_TIMEOUT;
  }
}

/**
 * @brief   Enqueues a formatted constant string to a log.
 * @details The total length is stored in the second raw log symbol.
 *          The string bytes are stored in the subsequent raw log symbols, with
 *          LSB first in each symbol (bytes group).
 *          Only the address is enqueued, so that constant strings are vapended
 *          with very low overhead.
 * @note    A string is said <i>constant</i> if its contents do not change
 *          between the call of this function and the end of a @ logFlush()
 *          operation.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] strp
 *          Constant string to be enqueued.
 * @param[in] length
 *          String length.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @iclass
 */
msg_t logPutCstrNI(LogDriver *lp, const char *strp, uint32_t length, logfmt_t fmt) {

  chDbgCheckClassI();
  if (logNumFreeI(lp) >= 3) {
    logPutRawI(lp, (LOGT_CSTRN << LOGF_TSH) | fmt.raw);
    logPutRawI(lp, (logsym_t)length);
    logPutRawI(lp, (logsym_t)strp);
    return RDY_OK;
  } else {
    return RDY_TIMEOUT;
  }
}

#if LOG_USE_WCHAR
/**
 * @brief   Enqueues a formatted Unicode character to a log.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] c
 *          Character to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @iclass
 */
msg_t logPutWcharI(LogDriver *lp, wchar_t c, logfmt_t fmt) {

  chDbgCheckClassI();

  if (logNumFreeI(lp) >= 2) {
    logPutRawI(lp, (LOGT_WCHAR << LOGF_TSH) | fmt.raw);
    logPutRawI(lp, (logsym_t)c);
    return RDY_OK;
  } else {
    return RDY_TIMEOUT;
  }
}

/**
 * @brief   Enqueues a formatted Unicode string to a log.
 * @details The total length is stored in the second raw log symbol.
 *          The string bytes are stored in the subsequent raw log symbols, with
 *          LSB first in each symbol (bytes group).
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] strp
 *          Unicode string to be enqueued.
 * @param[in] length
 *          String length.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @iclass
 */
msg_t logPutWstrNI(LogDriver *lp, const wchar_t *strp, uint32_t length, logfmt_t fmt) {

  if (logNumFreeI(lp) >= (cnt_t)((((length + 3) * sizeof(wchar_t)) + (4 + 4)) >> 2)) {
    logPutRawI(lp, (LOGT_WSTRN << LOGF_TSH) | fmt.raw);
    logPutRawI(lp, (logsym_t)(length * sizeof(wchar_t)));
    for (; length >= sizeof(logsym_t);
         strp += sizeof(logsym_t) / sizeof(wchar_t),
         length -= sizeof(logsym_t) / sizeof(wchar_t)) {
      logPutRawI(lp, *(logsym_t*)((uint8_t*)strp));
    }
    if (length > 0) {
      logPutRawI(lp, *(logsym_t*)((uint8_t*)strp));
    }
    return RDY_OK;
  } else {
    return RDY_TIMEOUT;
  }
}

/**
 * @brief   Enqueues a formatted constant Unicode string to a log.
 * @details The total length is stored in the second raw log symbol.
 *          The string bytes are stored in the subsequent raw log symbols, with
 *          LSB first in each symbol (bytes group).
 *          Only the address is enqueued, so that constant strings are vapended
 *          with very low overhead.
 * @note    A string is said <i>constant</i> if its contents do not change
 *          between the call of this function and the end of a @ logFlush()
 *          operation.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] strp
 *          Constant Unicode string to be enqueued.
 * @param[in] length
 *          String length.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @iclass
 */
msg_t logPutWcstrNI(LogDriver *lp, const wchar_t *strp, uint32_t length, logfmt_t fmt) {

  chDbgCheckClassI();
  if (logNumFreeI(lp) >= 3) {
    logPutRawI(lp, (LOGT_WCSTRN << LOGF_TSH) | fmt.raw);
    logPutRawI(lp, (logsym_t)(length * sizeof(wchar_t)));
    logPutRawI(lp, (logsym_t)strp);
    return RDY_OK;
  } else {
    return RDY_TIMEOUT;
  }
}
#endif /* LOG_USE_WCHAR */

/**
 * @brief   Enqueues a formatted memory address to a log.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] ptr
 *          Memory address to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @iclass
 */
msg_t logPutPtrI(LogDriver *lp, const void *ptr, logfmt_t fmt) {

  chDbgCheckClassI();

  if (logNumFreeI(lp) >= 2) {
    logPutRawI(lp, (LOGT_PTR << LOGF_TSH) | fmt.raw);
    logPutRawI(lp, (logsym_t)ptr);
    return RDY_OK;
  } else {
    return RDY_TIMEOUT;
  }
}

#if LOG_USE_UINTS || defined(__DOXYGEN__)
/**
 * @brief   Enqueues a formatted 8-bit unsigned integer to a log.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @iclass
 */
msg_t logPutUint8I(LogDriver *lp, uint8_t value, logfmt_t fmt) {

  chDbgCheckClassI();

  if (logNumFreeI(lp) >= 2) {
    logPutRawI(lp, (LOGT_UINT8 << LOGF_TSH) | fmt.raw);
    logPutRawI(lp, (logsym_t)value);
    return RDY_OK;
  } else {
    return RDY_TIMEOUT;
  }
}

/**
 * @brief   Enqueues a formatted 16-bit unsigned integer to a log.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @iclass
 */
msg_t logPutUint16I(LogDriver *lp, uint16_t value, logfmt_t fmt) {

  chDbgCheckClassI();

  if (logNumFreeI(lp) >= 2) {
    logPutRawI(lp, (LOGT_UINT16 << LOGF_TSH) | fmt.raw);
    logPutRawI(lp, (logsym_t)value);
    return RDY_OK;
  } else {
    return RDY_TIMEOUT;
  }
}

/**
 * @brief   Enqueues a formatted 32-bit unsigned integer to a log.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @iclass
 */
msg_t logPutUint32I(LogDriver *lp, uint32_t value, logfmt_t fmt) {

  chDbgCheckClassI();

  if (logNumFreeI(lp) >= 2) {
    logPutRawI(lp, (LOGT_UINT32 << LOGF_TSH) | fmt.raw);
    logPutRawI(lp, (logsym_t)value);
    return RDY_OK;
  } else {
    return RDY_TIMEOUT;
  }
}

/**
 * @brief   Enqueues a formatted 64-bit unsigned integer to a log.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @iclass
 */
#if LOG_USE_64BIT || defined(__DOXYGEN__)
msg_t logPutUint64I(LogDriver *lp, uint64_t value, logfmt_t fmt) {

  chDbgCheckClassI();

  if (logNumFreeI(lp) >= 3) {
    logPutRawI(lp, (LOGT_UINT64 << LOGF_TSH) | fmt.raw);
    logPutRawI(lp, ((logsym_t*)((uint8_t*)&value))[0]);
    logPutRawI(lp, ((logsym_t*)((uint8_t*)&value))[1]);
    return RDY_OK;
  } else {
    return RDY_TIMEOUT;
  }
}
#endif /* LOG_USE_64BIT */
#endif /* LOG_USE_UINTS */

#if LOG_USE_INTS || defined(__DOXYGEN__)
/**
 * @brief   Enqueues a formatted 8-bit signed integer to a log.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @iclass
 */
msg_t logPutInt8I(LogDriver *lp, int8_t value, logfmt_t fmt) {

  chDbgCheckClassI();

  if (logNumFreeI(lp) >= 2) {
    logPutRawI(lp, (LOGT_INT8 << LOGF_TSH) | fmt.raw);
    logPutRawI(lp, (logsym_t)value);
    return RDY_OK;
  } else {
    return RDY_TIMEOUT;
  }
}

/**
 * @brief   Enqueues a formatted 16-bit signed integer to a log.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @iclass
 */
msg_t logPutInt16I(LogDriver *lp, int16_t value, logfmt_t fmt) {

  chDbgCheckClassI();

  if (logNumFreeI(lp) >= 2) {
    logPutRawI(lp, (LOGT_INT16 << LOGF_TSH) | fmt.raw);
    logPutRawI(lp, (logsym_t)value);
    return RDY_OK;
  } else {
    return RDY_TIMEOUT;
  }
}

/**
 * @brief   Enqueues a formatted 32-bit signed integer to a log.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @iclass
 */
msg_t logPutInt32I(LogDriver *lp, int32_t value, logfmt_t fmt) {

  chDbgCheckClassI();

  if (logNumFreeI(lp) >= 2) {
    logPutRawI(lp, (LOGT_INT32 << LOGF_TSH) | fmt.raw);
    logPutRawI(lp, (logsym_t)value);
    return RDY_OK;
  } else {
    return RDY_TIMEOUT;
  }
}

/**
 * @brief   Enqueues a formatted 64-bit signed integer to a log.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @iclass
 */
#if LOG_USE_64BIT || defined(__DOXYGEN__)
msg_t logPutInt64I(LogDriver *lp, int64_t value, logfmt_t fmt) {

  chDbgCheckClassI();

  if (logNumFreeI(lp) >= 3) {
    logPutRawI(lp, (LOGT_INT64 << LOGF_TSH) | fmt.raw);
    logPutRawI(lp, ((logsym_t*)((uint8_t*)&value))[0]);
    logPutRawI(lp, ((logsym_t*)((uint8_t*)&value))[1]);
    return RDY_OK;
  } else {
    return RDY_TIMEOUT;
  }
}
#endif /* LOG_USE_64BIT */
#endif /* LOG_USE_INTS */

#if LOG_USE_OCTALS || defined(__DOXYGEN__)
/**
 * @brief   Enqueues a formatted 8-bit octal integer to a log.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @iclass
 */
msg_t logPutOct8I(LogDriver *lp, uint8_t value, logfmt_t fmt) {

  chDbgCheckClassI();

  if (logNumFreeI(lp) >= 2) {
    logPutRawI(lp, (LOGT_OCT8 << LOGF_TSH) | fmt.raw);
    logPutRawI(lp, (logsym_t)value);
    return RDY_OK;
  } else {
    return RDY_TIMEOUT;
  }
}

/**
 * @brief   Enqueues a formatted 16-bit octal integer to a log.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @iclass
 */
msg_t logPutOct16I(LogDriver *lp, uint16_t value, logfmt_t fmt) {

  chDbgCheckClassI();

  if (logNumFreeI(lp) >= 2) {
    logPutRawI(lp, (LOGT_OCT16 << LOGF_TSH) | fmt.raw);
    logPutRawI(lp, (logsym_t)value);
    return RDY_OK;
  } else {
    return RDY_TIMEOUT;
  }
}

/**
 * @brief   Enqueues a formatted 32-bit octal integer to a log.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @iclass
 */
msg_t logPutOct32I(LogDriver *lp, uint32_t value, logfmt_t fmt) {

  chDbgCheckClassI();

  if (logNumFreeI(lp) >= 2) {
    logPutRawI(lp, (LOGT_OCT32 << LOGF_TSH) | fmt.raw);
    logPutRawI(lp, (logsym_t)value);
    return RDY_OK;
  } else {
    return RDY_TIMEOUT;
  }
}

/**
 * @brief   Enqueues a formatted 64-bit octal integer to a log.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @iclass
 */
#if LOG_USE_64BIT || defined(__DOXYGEN__)
msg_t logPutOct64I(LogDriver *lp, uint64_t value, logfmt_t fmt) {

  chDbgCheckClassI();

  if (logNumFreeI(lp) >= 3) {
    logPutRawI(lp, (LOGT_OCT64 << LOGF_TSH) | fmt.raw);
    logPutRawI(lp, ((logsym_t*)((uint8_t*)&value))[0]);
    logPutRawI(lp, ((logsym_t*)((uint8_t*)&value))[1]);
    return RDY_OK;
  } else {
    return RDY_TIMEOUT;
  }
}
#endif /* LOG_USE_64BIT */
#endif /* LOG_USE_OCTALS */

/**
 * @brief   Enqueues a formatted 8-bit hexadecimal integer to a log.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @iclass
 */
msg_t logPutHex8I(LogDriver *lp, uint8_t value, logfmt_t fmt) {

  chDbgCheckClassI();

  if (logNumFreeI(lp) >= 2) {
    logPutRawI(lp, (LOGT_HEX8 << LOGF_TSH) | fmt.raw);
    logPutRawI(lp, (logsym_t)value);
    return RDY_OK;
  } else {
    return RDY_TIMEOUT;
  }
}

/**
 * @brief   Enqueues a formatted 16-bit hexadecimal integer to a log.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @iclass
 */
msg_t logPutHex16I(LogDriver *lp, uint16_t value, logfmt_t fmt) {

  chDbgCheckClassI();

  if (logNumFreeI(lp) >= 2) {
    logPutRawI(lp, (LOGT_HEX16 << LOGF_TSH) | fmt.raw);
    logPutRawI(lp, (logsym_t)value);
    return RDY_OK;
  } else {
    return RDY_TIMEOUT;
  }
}

/**
 * @brief   Enqueues a formatted 32-bit hexadecimal integer to a log.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @iclass
 */
msg_t logPutHex32I(LogDriver *lp, uint32_t value, logfmt_t fmt) {

  chDbgCheckClassI();

  if (logNumFreeI(lp) >= 2) {
    logPutRawI(lp, (LOGT_HEX32 << LOGF_TSH) | fmt.raw);
    logPutRawI(lp, (logsym_t)value);
    return RDY_OK;
  } else {
    return RDY_TIMEOUT;
  }
}

/**
 * @brief   Enqueues a formatted 64-bit hexadecimal integer to a log.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @iclass
 */
#if LOG_USE_64BIT || defined(__DOXYGEN__)
msg_t logPutHex64I(LogDriver *lp, uint64_t value, logfmt_t fmt) {

  chDbgCheckClassI();

  if (logNumFreeI(lp) >= 3) {
    logPutRawI(lp, (LOGT_HEX64 << LOGF_TSH) | fmt.raw);
    logPutRawI(lp, ((logsym_t*)((uint8_t*)&value))[0]);
    logPutRawI(lp, ((logsym_t*)((uint8_t*)&value))[1]);
    return RDY_OK;
  } else {
    return RDY_TIMEOUT;
  }
}
#endif /* LOG_USE_64BIT */

#if LOG_USE_FLOATS || defined(__DOXYGEN__)
/**
 * @brief   Enqueues a formatted single precision value to a log.
 * @details The value is printed with the standard decimal notation.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @iclass
 */
msg_t logPutFloatI(LogDriver *lp, float value, logfmt_t fmt) {

  chDbgCheckClassI();

  if (logNumFreeI(lp) >= 2) {
    logPutRawI(lp, (LOGT_FP << LOGF_TSH) | fmt.raw);
    logPutRawI(lp, ((logsym_t*)((uint8_t*)&value))[0]);
    return RDY_OK;
  } else {
    return RDY_TIMEOUT;
  }
}

/**
 * @brief   Enqueues a formatted double precision value to a log.
 * @details The value is printed with the standard decimal notation.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @iclass
 */
msg_t logPutDoubleI(LogDriver *lp, double value, logfmt_t fmt) {

  chDbgCheckClassI();

  if (logNumFreeI(lp) >= 3) {
    logPutRawI(lp, (LOGT_DP << LOGF_TSH) | fmt.raw);
    logPutRawI(lp, ((logsym_t*)((uint8_t*)&value))[0]);
    logPutRawI(lp, ((logsym_t*)((uint8_t*)&value))[1]);
    return RDY_OK;
  } else {
    return RDY_TIMEOUT;
  }
}

/**
 * @brief   Enqueues a formatted single precision value to a log.
 * @details The value is printed with the scientific notation.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @iclass
 */
msg_t logPutFloatEI(LogDriver *lp, float value, logfmt_t fmt) {

  chDbgCheckClassI();

  if (logNumFreeI(lp) >= 2) {
    logPutRawI(lp, (LOGT_FPE << LOGF_TSH) | fmt.raw);
    logPutRawI(lp, ((logsym_t*)((uint8_t*)&value))[0]);
    return RDY_OK;
  } else {
    return RDY_TIMEOUT;
  }
}

/**
 * @brief   Enqueues a formatted double precision value to a log.
 * @details The value is printed with the scientific notation.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @iclass
 */
msg_t logPutDoubleEI(LogDriver *lp, double value, logfmt_t fmt) {

  chDbgCheckClassI();

  if (logNumFreeI(lp) >= 3) {
    logPutRawI(lp, (LOGT_DPE << LOGF_TSH) | fmt.raw);
    logPutRawI(lp, ((logsym_t*)((uint8_t*)&value))[0]);
    logPutRawI(lp, ((logsym_t*)((uint8_t*)&value))[1]);
    return RDY_OK;
  } else {
    return RDY_TIMEOUT;
  }
}

/**
 * @brief   Enqueues a formatted single precision value to a log.
 * @details The value is printed with the shortest notation.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @iclass
 */
msg_t logPutFloatGI(LogDriver *lp, float value, logfmt_t fmt) {

  chDbgCheckClassI();

  if (logNumFreeI(lp) >= 2) {
    logPutRawI(lp, (LOGT_FPG << LOGF_TSH) | fmt.raw);
    logPutRawI(lp, ((logsym_t*)((uint8_t*)&value))[0]);
    return RDY_OK;
  } else {
    return RDY_TIMEOUT;
  }
}

/**
 * @brief   Enqueues a formatted double precision value to a log.
 * @details The value is printed with the shortest notation.
 * @note    This variant is non-blocking, the function returns a timeout
 *          condition if the queue is full.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @iclass
 */
msg_t logPutDoubleGI(LogDriver *lp, double value, logfmt_t fmt) {

  chDbgCheckClassI();

  if (logNumFreeI(lp) >= 3) {
    logPutRawI(lp, (LOGT_DPG << LOGF_TSH) | fmt.raw);
    logPutRawI(lp, ((logsym_t*)((uint8_t*)&value))[0]);
    logPutRawI(lp, ((logsym_t*)((uint8_t*)&value))[1]);
    return RDY_OK;
  } else {
    return RDY_TIMEOUT;
  }
}
#endif /* LOG_USE_FLOATS */

#if LOG_USE_PRINTF || defined(__DOXYGEN__)
msg_t logPutvfI(LogDriver *lp, const char *fmtstrp, va_list *vap) {

  msg_t err = RDY_OK;
  const char *start, *end;
  logfmt_t symfmt;

  chDbgCheckClassI();

  start = end = fmtstrp;
  while (*fmtstrp) {
    if (*fmtstrp++ != '%') {
      ++end;
    } else {
      /* Copy the previous plain string token.*/
      if (end != start) {
        err = logPutDCharNI(lp, start, (uint32_t)(end - start));
      }
      if (err != RDY_OK) {
        return err;
      }

      if (*fmtstrp == '%') {
        /* Print the '%' character.*/
        ++fmtstrp;
        err = logPutDCharI(lp, '%');
      } else {
        /* Decode symbol format and put it.*/
        fmtstrp = log_parse_fmt(fmtstrp, vap, &symfmt);
        err = log_put_by_fmt_i(lp, vap, symfmt);
      }
      if (err == RDY_OK) {
        start = end = fmtstrp;
      } else {
        return err;
      }
    }
  }
  /* Copy the previous plain string token.*/
  if (end != start) {
    err = logPutDCharNI(lp, start, (uint32_t)(end - start));
  }
  return err;
}

msg_t logPutfI(LogDriver *lp, const char *fmtstrp, ...) {

  va_list ap;
  msg_t err = RDY_OK;

  chDbgCheckClassI();

  va_start(ap, fmtstrp);
  err = logPutvfI(lp, fmtstrp, &ap);
  va_end(ap);
  return err;
}
#endif /* LOG_USE_PRINTF */

#if LOG_USE_PRINTFC || defined(__DOXYGEN__)
msg_t logPutvfcI(LogDriver *lp, const char *fmtstrp, va_list *vap) {

  msg_t err = RDY_OK;
  const char *start, *end;
  logfmt_t symfmt;

  chDbgCheckClassI();

  start = end = fmtstrp;
  while (*fmtstrp) {
    if (*fmtstrp++ != '%') {
      ++end;
    } else {
      /* Copy the previous plain string token.*/
      if (end != start) {
        err = logPutCstrNI(lp, start, (uint32_t)(end - start), log_nofmt);
      }
      if (err != RDY_OK) {
        return err;
      }

      if (*fmtstrp == '%') {
        /* Print the '%' character.*/
        ++fmtstrp;
        err = logPutDCharI(lp, '%');
      } else {
        /* Decode symbol format and put it.*/
        fmtstrp = log_parse_fmt(fmtstrp, vap, &symfmt);
        err = log_put_by_fmt_i(lp, vap, symfmt);
      }
      if (err == RDY_OK) {
        start = end = fmtstrp;
      } else {
        return err;
      }
    }
  }
  /* Copy the previous plain string token.*/
  if (end != start) {
    err = logPutCstrNI(lp, start, (uint32_t)(end - start), log_nofmt);
  }
  return err;
}

msg_t logPutfcI(LogDriver *lp, const char *fmtstrp, ...) {

  va_list ap;
  msg_t err = RDY_OK;

  chDbgCheckClassI();

  va_start(ap, fmtstrp);
  logPutvfcI(lp, fmtstrp, &ap);
  va_end(ap);
  return err;
}
#endif /* LOG_USE_PRINTFC */

/*~~~ USER CLASS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/**
 * @brief   Puts a raw symbol into a log queue.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
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
 *          The queue is full and the symbol cannot be appended.
 *
 * @api
 */
msg_t logPutRaw(LogDriver *lp, logsym_t raw) {

  msg_t err;

  chDbgCheck(lp != NULL, "logPutRaw");

  chSysLock();
  err = logPutRawI(lp, raw);
  chSysUnlock();
  return err;
}

/**
 * @brief   Puts a raw symbol into a log queue, with timeouts.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] raw
 *          Raw symbol to be vapended to the queue.
 * @param[in] in
 *          Maximum number of waiting system ticks.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @api
 */
msg_t logPutRawTimeout(LogDriver *lp, logsym_t raw, systime_t time) {

  msg_t err;

  chDbgCheck(lp != NULL, "logPutRawWait");

  chSysLock();
  if (time != TIME_INFINITE) {
    systime_t now = chTimeNow();
    time += now;
    do {
      err = logPutRawI(lp, raw);
      chSchRescheduleS();
      if (err == RDY_TIMEOUT) {
        now = chTimeNow();
      }
    } while (err == RDY_TIMEOUT && now < time);
  } else {
    do {
      err = logPutRawI(lp, raw);
      chSchRescheduleS();
    } while (err == RDY_TIMEOUT);
  }
  chSysUnlock();
  return err;
}

/**
 * @brief   Enqueues a direct string to a log.
 * @details The total length is stored in the lowest 24 bits of the first raw
 *          log symbol. The string bytes are stored in the subsequent raw log
 *          symbols, with LSB first in each symbol (bytes group).
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] strp
 *          String to be enqueued.
 * @param[in] length
 *          String length.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @api
 */
msg_t logPutDCharN(LogDriver *lp, const char *strp, uint32_t length) {

  msg_t err;

  chDbgCheck(lp != NULL, "logPutDCharN");
  chDbgCheck(strp != NULL, "logPutDCharN");
  chDbgCheck(length > 0, "logPutDCharN");

  chSysLock();
  do {
    err = logPutDCharNI(lp, strp, length);
    chSchRescheduleS();
  } while (err == RDY_TIMEOUT);
  chSysUnlock();
  return err;
}

/**
 * @brief   Enqueues a direct string to a log.
 * @details Characters are enqueued as a zero-terminated string. The first
 *          three bytes are part of the raw head symbol, LSB first.
 *          If at some point it is not possible to post further characters,
 *          a terminating zero byte is put as MSB of the last enqueued
 *          raw symbol.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] strp
 *          Null-terminated string to be enqueued.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @api
 */
msg_t logPutDCharZ(LogDriver *lp, const char *strp) {

  msg_t err;

  chDbgCheck(lp != NULL, "logPutDCharZ");
  chDbgCheck(strp != NULL, "logPutDCharZ");

  chSysLock();
  err = logPutDCharZI(lp, strp);
  chSchRescheduleS();
  chSysUnlock();
  return err;
}

/**
 * @brief   Enqueues a direct string to a log.
 * @details Characters are enqueued three per each log symbol, LSB first.
 *          The total length is stored on the lowest 24 bits of the first raw
 *          log symbol.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] strp
 *          String to be enqueued.
 * @param[in] length
 *          String length.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @api
 */
msg_t logPutDCharCN(LogDriver *lp, const char *strp, uint32_t length) {

  msg_t err = RDY_OK;

  chDbgCheck(lp != NULL, "logPutDCharCN");
  chDbgCheck(strp != NULL, "logPutDCharCN");
  chDbgCheck(length < (1ul << 24), "logPutDCharCN");

  for (; length >= 3 && err == RDY_OK; strp += 3, length -= 3) {
    err = logPutRawTimeout(lp, (LOGT_DCH3 << LOGF_TSH) |
                           (*(logsym_t*)strp & ~LOGF_TMSK), TIME_INFINITE);
  }
  if (err == RDY_OK) {
    if (length == 2) {
      err = logPutRawTimeout(lp, (LOGT_DCH2 << LOGF_TSH) |
                             (*(logsym_t*)strp & ~LOGF_TMSK), TIME_INFINITE);
    } else if (length == 1) {
      err = logPutRawTimeout(lp, (LOGT_DCH1 << LOGF_TSH) |
                             (*(logsym_t*)strp & ~LOGF_TMSK), TIME_INFINITE);
    }
  }
  return err;
}

/**
 * @brief   Enqueues a direct string to a log.
 * @details Characters are enqueued three per each raw log symbol, LSB first.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] strp
 *          Null-terminated string to be enqueued.
 * @param[in] length
 *          String length.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @api
 */
msg_t logPutDCharCZ(LogDriver *lp, const char *strp) {

  msg_t err = RDY_OK;
  bool_t pending = FALSE;

  chDbgCheck(lp != NULL, "logPutDCharCZ");
  chDbgCheck(strp != NULL, "logPutDCharCZ");

  do {
    pending = strp[0] && strp[1] && strp[2];
    if (pending) {
      err = logPutRawTimeout(lp, (LOGT_DCH3 << LOGF_TSH) |
                             (*(logsym_t*)strp & ~LOGF_TMSK), TIME_INFINITE);
      strp += 3;
    } else if (strp[0]) {
      if (strp[1]) {
        err = logPutRawTimeout(lp, (LOGT_DCH2 << LOGF_TSH) |
                               (*(logsym_t*)strp & ~LOGF_TMSK), TIME_INFINITE);
      } else {
        err = logPutRawTimeout(lp, (LOGT_DCH1 << LOGF_TSH) |
                               (*(logsym_t*)strp & ~LOGF_TMSK), TIME_INFINITE);
      }
    }
  } while (pending && err == RDY_OK);
  return err;
}

/**
 * @brief   Enqueues a direct constant string to a log.
 * @details Only the address is enqueued, so that constant strings are vapended
 *          with very low overhead.
 * @note    A string is said <i>constant</i> if its contents do not change
 *          between the call of this function and the end of a @ logFlush()
 *          operation.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] strp
 *          Constant string to be enqueued.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @api
 */
msg_t logPutDCstr(LogDriver *lp, const char *strp) {

  msg_t err;

  chDbgCheck(lp != NULL, "logPutDCstr");
  chDbgCheck(strp != NULL, "logPutDCstr");

  chSysLock();
  do {
    err = logPutDCstrI(lp, strp);
    chSchRescheduleS();
  } while (err == RDY_TIMEOUT);
  chSysUnlock();
  return err;
}

/**
 * @brief   Enqueues a formatted character to a log.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] c
 *          Character to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @api
 */
msg_t logPutChar(LogDriver *lp, char c, logfmt_t fmt) {

  msg_t err;

  chDbgCheck(lp != NULL, "logPutChar");

  chSysLock();
  do {
    err = logPutCharI(lp, c, fmt);
    chSchRescheduleS();
  } while (err == RDY_TIMEOUT);
  chSysUnlock();
  return err;
}

/**
 * @brief   Enqueues a formatted string to a log.
 * @details The total length is stored in the second raw log symbol.
 *          The string bytes are stored in the subsequent raw log symbols, with
 *          LSB first in each symbol (bytes group).
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] strp
 *          String to be enqueued.
 * @param[in] length
 *          String length.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @api
 */
msg_t logPutStrN(LogDriver *lp, const char *strp, uint32_t length, logfmt_t fmt) {

  msg_t err;

  chDbgCheck(lp != NULL, "logPutStrN");
  chDbgCheck(strp != NULL, "logPutStrN");
  chDbgCheck(fmt.fmt.type == 0 || fmt.fmt.type == LOGT_STRN, "logPutStrN");

  chSysLock();
  do {
    err = logPutStrNI(lp, strp, length, fmt);
    chSchRescheduleS();
  } while (err == RDY_TIMEOUT);
  chSysUnlock();
  return err;
}

/**
 * @brief   Enqueues a formatted constant string to a log.
 * @details The total length is stored in the second raw log symbol.
 *          The string bytes are stored in the subsequent raw log symbols, with
 *          LSB first in each symbol (bytes group).
 *          Only the address is enqueued, so that constant strings are vapended
 *          with very low overhead.
 * @note    A string is said <i>constant</i> if its contents do not change
 *          between the call of this function and the end of a @ logFlush()
 *          operation.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] strp
 *          Constant string to be enqueued.
 * @param[in] length
 *          String length.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @api
 */
msg_t logPutCstrN(LogDriver *lp, const char *strp, uint32_t length, logfmt_t fmt) {

  msg_t err;

  chDbgCheck(lp != NULL, "logPutCstrN");
  chDbgCheck(strp != NULL, "logPutCstrN");
  chDbgCheck(fmt.fmt.type == 0 || fmt.fmt.type == LOGT_CSTRN, "logPutCstrN");

  chSysLock();
  do {
    err = logPutCstrNI(lp, strp, length, fmt);
    chSchRescheduleS();
  } while (err == RDY_TIMEOUT);
  chSysUnlock();
  return err;
}

#if LOG_USE_WCHAR
/**
 * @brief   Enqueues a formatted Unicode character to a log.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] c
 *          Character to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @api
 */
msg_t logPutWchar(LogDriver *lp, wchar_t c, logfmt_t fmt) {

  msg_t err;

  chDbgCheck(lp != NULL, "logPutWchar");

  chSysLock();
  do {
    err = logPutWcharI(lp, c, fmt);
    chSchRescheduleS();
  } while (err == RDY_TIMEOUT);
  chSysUnlock();
  return err;
}

/**
 * @brief   Enqueues a formatted Unicode string to a log.
 * @details The total length is stored in the second raw log symbol.
 *          The string bytes are stored in the subsequent raw log symbols, with
 *          LSB first in each symbol (bytes group).
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] strp
 *          Unicode string to be enqueued.
 * @param[in] length
 *          String length.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @api
 */
msg_t logPutWstrN(LogDriver *lp, const wchar_t *strp, uint32_t length, logfmt_t fmt) {

  msg_t err;

  chDbgCheck(lp != NULL, "logPutWstrN");
  chDbgCheck(strp != NULL, "logPutWstrN");
  chDbgCheck(fmt.fmt.type == 0 || fmt.fmt.type == LOGT_WSTRN, "logPutWstrN");

  chSysLock();
  do {
    err = logPutWstrNI(lp, strp, length, fmt);
    chSchRescheduleS();
  } while (err == RDY_TIMEOUT);
  chSysUnlock();
  return err;
}

/**
 * @brief   Enqueues a formatted constant Unicode string to a log.
 * @details The total length is stored in the second raw log symbol.
 *          The string bytes are stored in the subsequent raw log symbols, with
 *          LSB first in each symbol (bytes group).
 *          Only the address is enqueued, so that constant strings are vapended
 *          with very low overhead.
 * @note    A string is said <i>constant</i> if its contents do not change
 *          between the call of this function and the end of a @ logFlush()
 *          operation.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] strp
 *          Constant Unicode string to be enqueued.
 * @param[in] length
 *          String length.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @api
 */
msg_t logPutWcstrN(LogDriver *lp, const wchar_t *strp, uint32_t length, logfmt_t fmt) {

  msg_t err;

  chDbgCheck(lp != NULL, "logPutWcstrN");
  chDbgCheck(strp != NULL, "logPutWcstrN");
  chDbgCheck(fmt.fmt.type == 0 || fmt.fmt.type == LOGT_WCSTRN, "logPutWcstrN");

  chSysLock();
  do {
    err = logPutWcstrNI(lp, strp, length, fmt);
    chSchRescheduleS();
  } while (err == RDY_TIMEOUT);
  chSysUnlock();
  return err;
}
#endif /* LOG_USE_WCHAR */

/**
 * @brief   Enqueues a formatted memory address to a log.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] ptr
 *          Memory address to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @api
 */
msg_t logPutPtr(LogDriver *lp, const void *ptr, logfmt_t fmt) {

  msg_t err;

  chDbgCheck(lp != NULL, "logPutInt8");
  chDbgCheck(fmt.fmt.type == 0 || fmt.fmt.type == LOGT_PTR, "logPutPtr");

  chSysLock();
  do {
    err = logPutPtrI(lp, ptr, fmt);
    chSchRescheduleS();
  } while (err == RDY_TIMEOUT);
  chSysUnlock();
  return err;
}

#if LOG_USE_UINTS || defined(__DOXYGEN__)
/**
 * @brief   Enqueues a formatted 8-bit unsigned integer to a log.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @api
 */
msg_t logPutUint8(LogDriver *lp, uint8_t value, logfmt_t fmt) {

  msg_t err;

  chDbgCheck(lp != NULL, "logPutUint8");
  chDbgCheck(fmt.fmt.type == 0 || fmt.fmt.type == LOGT_UINT8, "logPutUint8");

  chSysLock();
  do {
    err = logPutUint8I(lp, value, fmt);
    chSchRescheduleS();
  } while (err == RDY_TIMEOUT);
  chSysUnlock();
  return err;
}

/**
 * @brief   Enqueues a formatted 16-bit unsigned integer to a log.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @api
 */
msg_t logPutUint16(LogDriver *lp, uint16_t value, logfmt_t fmt) {

  msg_t err;

  chDbgCheck(lp != NULL, "logPutUint16");
  chDbgCheck(fmt.fmt.type == 0 || fmt.fmt.type == LOGT_UINT16, "logPutUint16");

  chSysLock();
  do {
    err = logPutUint16I(lp, value, fmt);
    chSchRescheduleS();
  } while (err == RDY_TIMEOUT);
  chSysUnlock();
  return err;
}

/**
 * @brief   Enqueues a formatted 32-bit unsigned integer to a log.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @api
 */
msg_t logPutUint32(LogDriver *lp, uint32_t value, logfmt_t fmt) {

  msg_t err;

  chDbgCheck(lp != NULL, "logPutUint32");
  chDbgCheck(fmt.fmt.type == 0 || fmt.fmt.type == LOGT_UINT32, "logPutUint32");

  chSysLock();
  do {
    err = logPutUint32I(lp, value, fmt);
    chSchRescheduleS();
  } while (err == RDY_TIMEOUT);
  chSysUnlock();
  return err;
}

/**
 * @brief   Enqueues a formatted 64-bit unsigned integer to a log.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @api
 */
#if LOG_USE_64BIT || defined(__DOXYGEN__)
msg_t logPutUint64(LogDriver *lp, uint64_t value, logfmt_t fmt) {

  msg_t err;

  chDbgCheck(lp != NULL, "logPutUint64");
  chDbgCheck(fmt.fmt.type == 0 || fmt.fmt.type == LOGT_UINT64, "logPutUint64");

  chSysLock();
  do {
    err = logPutUint64I(lp, value, fmt);
    chSchRescheduleS();
  } while (err == RDY_TIMEOUT);
  chSysUnlock();
  return err;
}
#endif /* LOG_USE_64BIT */
#endif /* LOG_USE_UINTS */

#if LOG_USE_INTS || defined(__DOXYGEN__)
/**
 * @brief   Enqueues a formatted 8-bit signed integer to a log.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @api
 */
msg_t logPutInt8(LogDriver *lp, int8_t value, logfmt_t fmt) {

  msg_t err;

  chDbgCheck(lp != NULL, "logPutInt8");
  chDbgCheck(fmt.fmt.type == 0 || fmt.fmt.type == LOGT_INT8, "logPutInt8");

  chSysLock();
  do {
    err = logPutInt8I(lp, value, fmt);
    chSchRescheduleS();
  } while (err == RDY_TIMEOUT);
  chSysUnlock();
  return err;
}

/**
 * @brief   Enqueues a formatted 16-bit signed integer to a log.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @api
 */
msg_t logPutInt16(LogDriver *lp, int16_t value, logfmt_t fmt) {

  msg_t err;

  chDbgCheck(lp != NULL, "logPutInt16");
  chDbgCheck(fmt.fmt.type == 0 || fmt.fmt.type == LOGT_INT16, "logPutInt16");

  chSysLock();
  do {
    err = logPutInt16I(lp, value, fmt);
    chSchRescheduleS();
  } while (err == RDY_TIMEOUT);
  chSysUnlock();
  return err;
}

/**
 * @brief   Enqueues a formatted 32-bit signed integer to a log.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @api
 */
msg_t logPutInt32(LogDriver *lp, int32_t value, logfmt_t fmt) {

  msg_t err;

  chDbgCheck(lp != NULL, "logPutInt32");
  chDbgCheck(fmt.fmt.type == 0 || fmt.fmt.type == LOGT_INT32, "logPutInt32");

  chSysLock();
  do {
    err = logPutInt32I(lp, value, fmt);
    chSchRescheduleS();
  } while (err == RDY_TIMEOUT);
  chSysUnlock();
  return err;
}

/**
 * @brief   Enqueues a formatted 64-bit signed integer to a log.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @api
 */
#if LOG_USE_64BIT || defined(__DOXYGEN__)
msg_t logPutInt64(LogDriver *lp, int64_t value, logfmt_t fmt) {

  msg_t err;

  chDbgCheck(lp != NULL, "logPutInt64");
  chDbgCheck(fmt.fmt.type == 0 || fmt.fmt.type == LOGT_INT64, "logPutInt64");

  chSysLock();
  do {
    err = logPutInt64I(lp, value, fmt);
    chSchRescheduleS();
  } while (err == RDY_TIMEOUT);
  chSysUnlock();
  return err;
}
#endif /* LOG_USE_64BITS */
#endif /* LOG_USE_INTS */

#if LOG_USE_OCTALS || defined(__DOXYGEN__)
/**
 * @brief   Enqueues a formatted 8-bit octal integer to a log.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @api
 */
msg_t logPutOct8(LogDriver *lp, uint8_t value, logfmt_t fmt) {

  msg_t err;

  chDbgCheck(lp != NULL, "logPutOct8");
  chDbgCheck(fmt.fmt.type == 0 || fmt.fmt.type == LOGT_OCT8, "logPutOct8");

  chSysLock();
  do {
    err = logPutOct8I(lp, value, fmt);
    chSchRescheduleS();
  } while (err == RDY_TIMEOUT);
  chSysUnlock();
  return err;
}

/**
 * @brief   Enqueues a formatted 16-bit octal integer to a log.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @api
 */
msg_t logPutOct16(LogDriver *lp, uint16_t value, logfmt_t fmt) {

  msg_t err;

  chDbgCheck(lp != NULL, "logPutOct16");
  chDbgCheck(fmt.fmt.type == 0 || fmt.fmt.type == LOGT_OCT16, "logPutOct16");

  chSysLock();
  do {
    err = logPutOct16I(lp, value, fmt);
    chSchRescheduleS();
  } while (err == RDY_TIMEOUT);
  chSysUnlock();
  return err;
}

/**
 * @brief   Enqueues a formatted 32-bit octal integer to a log.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @api
 */
msg_t logPutOct32(LogDriver *lp, uint32_t value, logfmt_t fmt) {

  msg_t err;

  chDbgCheck(lp != NULL, "logPutOct32");
  chDbgCheck(fmt.fmt.type == 0 || fmt.fmt.type == LOGT_OCT32, "logPutOct32");

  chSysLock();
  do {
    err = logPutOct32I(lp, value, fmt);
    chSchRescheduleS();
  } while (err == RDY_TIMEOUT);
  chSysUnlock();
  return err;
}

/**
 * @brief   Enqueues a formatted 64-bit octal integer to a log.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @api
 */
#if LOG_USE_64BIT || defined(__DOXYGEN__)
msg_t logPutOct64(LogDriver *lp, uint64_t value, logfmt_t fmt) {

  msg_t err;

  chDbgCheck(lp != NULL, "logPutOct64");
  chDbgCheck(fmt.fmt.type == 0 || fmt.fmt.type == LOGT_OCT64, "logPutOct64");

  chSysLock();
  do {
    err = logPutOct64I(lp, value, fmt);
    chSchRescheduleS();
  } while (err == RDY_TIMEOUT);
  chSysUnlock();
  return err;
}
#endif /* LOG_USE_64BIT */
#endif /* LOG_USE_OCTALS */

/**
 * @brief   Enqueues a formatted 8-bit hexadecimal integer to a log.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @api
 */
msg_t logPutHex8(LogDriver *lp, uint8_t value, logfmt_t fmt) {

  msg_t err;

  chDbgCheck(lp != NULL, "logPutHex8");
  chDbgCheck(fmt.fmt.type == 0 || fmt.fmt.type == LOGT_HEX8, "logPutHex8");

  chSysLock();
  do {
    err = logPutHex8I(lp, value, fmt);
    chSchRescheduleS();
  } while (err == RDY_TIMEOUT);
  chSysUnlock();
  return err;
}

/**
 * @brief   Enqueues a formatted 16-bit hexadecimal integer to a log.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @api
 */
msg_t logPutHex16(LogDriver *lp, uint16_t value, logfmt_t fmt) {

  msg_t err;

  chDbgCheck(lp != NULL, "logPutHex16");
  chDbgCheck(fmt.fmt.type == 0 || fmt.fmt.type == LOGT_HEX16, "logPutHex16");

  chSysLock();
  do {
    err = logPutHex16I(lp, value, fmt);
    chSchRescheduleS();
  } while (err == RDY_TIMEOUT);
  chSysUnlock();
  return err;
}

/**
 * @brief   Enqueues a formatted 32-bit hexadecimal integer to a log.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @api
 */
msg_t logPutHex32(LogDriver *lp, uint32_t value, logfmt_t fmt) {

  msg_t err;

  chDbgCheck(lp != NULL, "logPutHex32");
  chDbgCheck(fmt.fmt.type == 0 || fmt.fmt.type == LOGT_HEX32, "logPutHex32");

  chSysLock();
  do {
    err = logPutHex32I(lp, value, fmt);
    chSchRescheduleS();
  } while (err == RDY_TIMEOUT);
  chSysUnlock();
  return err;
}

/**
 * @brief   Enqueues a formatted 64-bit hexadecimal integer to a log.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @api
 */
#if LOG_USE_64BIT || defined(__DOXYGEN__)
msg_t logPutHex64(LogDriver *lp, uint64_t value, logfmt_t fmt) {

  msg_t err;

  chDbgCheck(lp != NULL, "logPutHex64");
  chDbgCheck(fmt.fmt.type == 0 || fmt.fmt.type == LOGT_HEX64, "logPutHex64");

  chSysLock();
  do {
    err = logPutHex64I(lp, value, fmt);
    chSchRescheduleS();
  } while (err == RDY_TIMEOUT);
  chSysUnlock();
  return err;
}
#endif /* LOG_USE_64BIT */

#if LOG_USE_FLOATS || defined(__DOXYGEN__)
/**
 * @brief   Enqueues a formatted single precision value to a log.
 * @details The value is printed with the standard decimal notation.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @api
 */
msg_t logPutFloat(LogDriver *lp, float value, logfmt_t fmt) {

  msg_t err;

  chDbgCheck(lp != NULL, "logPutFloat");
  chDbgCheck(fmt.fmt.type == 0 || fmt.fmt.type == LOGT_FP, "logPutFloat");

  chSysLock();
  do {
    err = logPutFloatI(lp, value, fmt);
    chSchRescheduleS();
  } while (err == RDY_TIMEOUT);
  chSysUnlock();
  return err;
}

/**
 * @brief   Enqueues a formatted double precision value to a log.
 * @details The value is printed with the standard decimal notation.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @api
 */
msg_t logPutDouble(LogDriver *lp, double value, logfmt_t fmt) {

  msg_t err;

  chDbgCheck(lp != NULL, "logPutDouble");
  chDbgCheck(fmt.fmt.type == 0 || fmt.fmt.type == LOGT_DP, "logPutDouble");

  chSysLock();
  do {
    err = logPutDoubleI(lp, value, fmt);
    chSchRescheduleS();
  } while (err == RDY_TIMEOUT);
  chSysUnlock();
  return err;
}

/**
 * @brief   Enqueues a formatted single precision value to a log.
 * @details The value is printed with the scientific notation.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @api
 */
msg_t logPutFloatE(LogDriver *lp, float value, logfmt_t fmt) {

  msg_t err;

  chDbgCheck(lp != NULL, "logPutFloatE");
  chDbgCheck(fmt.fmt.type == 0 || fmt.fmt.type == LOGT_FPE, "logPutFloatE");

  chSysLock();
  do {
    err = logPutFloatEI(lp, value, fmt);
    chSchRescheduleS();
  } while (err == RDY_TIMEOUT);
  chSysUnlock();
  return err;
}

/**
 * @brief   Enqueues a formatted double precision value to a log.
 * @details The value is printed with the scientific notation.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @api
 */
msg_t logPutDoubleE(LogDriver *lp, double value, logfmt_t fmt) {

  msg_t err;

  chDbgCheck(lp != NULL, "logPutDoubleE");
  chDbgCheck(fmt.fmt.type == 0 || fmt.fmt.type == LOGT_DPE, "logPutDoubleE");

  chSysLock();
  do {
    err = logPutDoubleEI(lp, value, fmt);
    chSchRescheduleS();
  } while (err == RDY_TIMEOUT);
  chSysUnlock();
  return err;
}

/**
 * @brief   Enqueues a formatted single precision value to a log.
 * @details The value is printed with the shortest decimal notation.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @api
 */
msg_t logPutFloatG(LogDriver *lp, float value, logfmt_t fmt) {

  msg_t err;

  chDbgCheck(lp != NULL, "logPutFloatG");
  chDbgCheck(fmt.fmt.type == 0 || fmt.fmt.type == LOGT_FPG, "logPutFloatG");

  chSysLock();
  do {
    err = logPutFloatGI(lp, value, fmt);
    chSchRescheduleS();
  } while (err == RDY_TIMEOUT);
  chSysUnlock();
  return err;
}

/**
 * @brief   Enqueues a formatted double precision value to a log.
 * @details The value is printed with the shortest decimal notation.
 * @note    This variant is blocking, the function waits until there are enough
 *          free queue slots.
 *
 * @param[in] lp
 *          Pointer to an initialized @p LogDriver object.
 * @param[in] value
 *          Value to be enqueued.
 * @param[in] fmt
 *          Log symbol format.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is full and the symbol cannot be appended.
 *
 * @api
 */
msg_t logPutDoubleG(LogDriver *lp, double value, logfmt_t fmt) {

  msg_t err;

  chDbgCheck(lp != NULL, "logPutDoubleG");
  chDbgCheck(fmt.fmt.type == 0 || fmt.fmt.type == LOGT_DPG, "logPutDoubleG");

  chSysLock();
  do {
    err = logPutDoubleGI(lp, value, fmt);
    chSchRescheduleS();
  } while (err == RDY_TIMEOUT);
  chSysUnlock();
  return err;
}
#endif /* LOG_USE_FLOATS */

#if LOG_USE_PRINTF || defined(__DOXYGEN__)
msg_t logPutvf(LogDriver *lp, const char *fmtstrp, va_list *vap) {

  msg_t err = RDY_OK;
  const char *start, *end;
  logfmt_t symfmt;

  chDbgCheck(lp != NULL, "logPutvf");
  chDbgCheck(fmtstrp != NULL, "logPutvf");

  start = end = fmtstrp;
  while (*fmtstrp) {
    if (*fmtstrp++ != '%') {
      ++end;
    } else {
      /* Copy the previous plain string token.*/
      if (end != start) {
        err = logPutDCharCN(lp, start, (uint32_t)(end - start));
      }
      if (err != RDY_OK) {
        return err;
      }

      if (*fmtstrp == '%') {
        /* Print the '%' character.*/
        ++fmtstrp;
        err = logPutDChar(lp, '%');
      } else {
        /* Decode symbol format and put it.*/
        fmtstrp = log_parse_fmt(fmtstrp, vap, &symfmt);
        chSysLock();
        err = log_put_by_fmt_i(lp, vap, symfmt);
        chSysUnlock();
      }
      if (err == RDY_OK) {
        start = end = fmtstrp;
      } else {
        return err;
      }
    }
  }

  /* Copy the previous plain string token.*/
  if (end != start) {
    err = logPutDCharCN(lp, start, (uint32_t)(end - start));
  }
  return err;
}

msg_t logPutf(LogDriver *lp, const char *fmtstrp, ...) {

  msg_t err;
  va_list ap;

  chDbgCheck(lp != NULL, "logPutf");
  chDbgCheck(fmtstrp != NULL, "logPutf");

  va_start(ap, fmtstrp);
  err = logPutvf(lp, fmtstrp, &ap);
  va_end(ap);
  return err;
}
#endif /* LOG_USE_PRINTF */

#if LOG_USE_PRINTFC || defined(__DOXYGEN__)
msg_t logPutvfc(LogDriver *lp, const char *fmtstrp, va_list *vap) {

  msg_t err = RDY_OK;
  const char *start, *end;
  logfmt_t symfmt;

  chDbgCheck(lp != NULL, "logPutvf");
  chDbgCheck(fmtstrp != NULL, "logPutvf");

  start = end = fmtstrp;
  while (*fmtstrp) {
    if (*fmtstrp++ != '%') {
      ++end;
    } else {
      /* Copy the previous plain string token.*/
      if (end != start) {
        err = logPutCstrN(lp, start, (uint32_t)(end - start), log_nofmt);
      }
      if (err != RDY_OK) {
        return err;
      }

      if (*fmtstrp == '%') {
        /* Print the '%' character.*/
        ++fmtstrp;
        err = logPutDChar(lp, '%');
      } else {
        /* Decode symbol format and put it.*/
        fmtstrp = log_parse_fmt(fmtstrp, vap, &symfmt);
        chSysLock();
        err = log_put_by_fmt_i(lp, vap, symfmt);
        chSysUnlock();
      }
      if (err == RDY_OK) {
        start = end = fmtstrp;
      } else {
        return err;
      }
    }
  }
  /* Copy the previous plain string token.*/
  if (end != start) {
    err = logPutCstrN(lp, start, (uint32_t)(end - start), log_nofmt);
  }
  return err;
}

msg_t logPutfc(LogDriver *lp, const char *fmtstrp, ...) {

  msg_t err;
  va_list ap;

  chDbgCheck(lp != NULL, "logPutfc");
  chDbgCheck(fmtstrp != NULL, "logPutfc");

  va_start(ap, fmtstrp);
  err = logPutvfc(lp, fmtstrp, &ap);
  va_end(ap);
  return err;
}
#endif /* LOG_USE_PRINTFC */

/*===========================================================================*/
/* Streaming functions.                                                      */
/*===========================================================================*/

/**
 * @brief   Fetches a raw log symbol from a log queue.
 * @note    This function is non-blocking, the function returns a timeout
 *          condition if the queue is empty.
 *
 * @param[in] mbp
 *          Pointer to an initialized @p Mailbox object.
 * @param[out] rawp
 *          Pointer to the returned raw symbol.
 * @return
 *          Operation status.
 * @retval RDY_OK
 *          A symbol has been correctly posted.
 * @retval RDY_TIMEOUT
 *          The queue is empty and the symbol cannot be fetched.
 *
 * @notapi
 */
static msg_t log_fetch_raw(Mailbox *mbp, logsym_t *rawp) {

  chDbgCheck(mbp != NULL, "log_fetch_raw");
  chDbgCheck(rawp != NULL, "log_fetch_raw");

  return chMBFetch(mbp, (msg_t*)rawp, TIME_IMMEDIATE);
}

static msg_t log_fetch_dchr(Mailbox *mbp, char *chp, uint16_t *cntp) {

  msg_t err;
  logfmt_t head;

  chDbgCheck(mbp != NULL, "log_fetch_dchr");
  chDbgCheck(chp != NULL, "log_fetch_dchr");
  chDbgCheck(cntp != NULL, "log_fetch_dchr");

  err = log_fetch_raw(mbp, (logsym_t*)&head);
  if (err == RDY_OK) {
    chDbgCheck(head.fmt.type == LOGT_DCHR, "log_fetch_dchr");

    *chp = (char)head.raw;
    *cntp = (uint16_t)(head.raw >> 8);
  }
  return err;
}

static msg_t log_fetch_dcstr(Mailbox *mbp, const char **cstrpp) {

  msg_t err;
  logfmt_t head;

  chDbgCheck(mbp != NULL, "log_fetch_dcstr");
  err = log_fetch_raw(mbp, (logsym_t*)&head);
  if (err == RDY_OK) {
    chDbgCheck(head.fmt.type == LOGT_DCSTR, "log_fetch_dcstr");

    err = log_fetch_raw(mbp, (logsym_t*)cstrpp);
    chDbgCheck(err == RDY_OK, "log_fetch_dcstr");
  }
  return err;
}

#if LOG_USE_UINTS || defined(__DOXYGEN__)
static msg_t log_fetch_uint8(Mailbox *mbp, uint8_t *valuep, logfmt_t *fmtp) {

  msg_t err;

  chDbgCheck(mbp != NULL, "log_fetch_uint8");
  chDbgCheck(valuep != NULL, "log_fetch_uint8");
  chDbgCheck(fmtp != NULL, "log_fetch_uint8");

  err = log_fetch_raw(mbp, (logsym_t*)fmtp);
  if (err == RDY_OK) {
    logsym_t value;
    err = log_fetch_raw(mbp, &value);
    *valuep = (uint8_t)value;
  }
  return err;
}

static msg_t log_fetch_uint16(Mailbox *mbp, uint16_t *valuep, logfmt_t *fmtp) {

  msg_t err;

  chDbgCheck(mbp != NULL, "log_fetch_uint16");
  chDbgCheck(valuep != NULL, "log_fetch_uint16");
  chDbgCheck(fmtp != NULL, "log_fetch_uint16");

  err = log_fetch_raw(mbp, (logsym_t*)fmtp);
  if (err == RDY_OK) {
    logsym_t value;
    err = log_fetch_raw(mbp, &value);
    *valuep = (uint16_t)value;
  }
  return err;
}

static msg_t log_fetch_uint32(Mailbox *mbp, uint32_t *valuep, logfmt_t *fmtp) {

  msg_t err;

  chDbgCheck(mbp != NULL, "log_fetch_uint32");
  chDbgCheck(valuep != NULL, "log_fetch_uint32");
  chDbgCheck(fmtp != NULL, "log_fetch_uint32");

  err = log_fetch_raw(mbp, (logsym_t*)fmtp);
  if (err == RDY_OK) {
    logsym_t value;
    err = log_fetch_raw(mbp, &value);
    *valuep = (uint32_t)value;
  }
  return err;
}

#if LOG_USE_64BIT || defined(__DOXYGEN__)
static msg_t log_fetch_uint64(Mailbox *mbp, uint64_t *valuep, logfmt_t *fmtp) {

  msg_t err;

  chDbgCheck(mbp != NULL, "log_fetch_uint64");
  chDbgCheck(valuep != NULL, "log_fetch_uint64");
  chDbgCheck(fmtp != NULL, "log_fetch_uint64");

  err = log_fetch_raw(mbp, (logsym_t*)fmtp);
  if (err == RDY_OK) {
    logsym_t raw[2];

    err = log_fetch_raw(mbp, &raw[0]);
    chDbgCheck(err == RDY_OK, "log_fetch_uint64");

    err = log_fetch_raw(mbp, &raw[1]);
    chDbgCheck(err == RDY_OK, "log_fetch_uint64");

    *valuep = *(uint64_t*)((uint8_t*)raw);
  }
  return err;
}
#endif /* LOG_USE_64BIT */
#endif /* LOG_USE_UINTS */

#if LOG_USE_FLOATS || defined(__DOXYGEN__)
static msg_t log_fetch_fp(Mailbox *mbp, float *valuep, logfmt_t *fmtp) {

  msg_t err;
  logsym_t head;

  chDbgCheck(mbp != NULL, "log_fetch_fp");
  chDbgCheck(valuep != NULL, "log_fetch_fp");
  chDbgCheck(fmtp != NULL, "log_fetch_fp");

  err = log_fetch_raw(mbp, &head);
  if (err == RDY_OK) {
    err = log_fetch_raw(mbp, (logsym_t*)valuep);
    chDbgCheck(err == RDY_OK, "log_fetch_fp");
  }
  return err;
}

static msg_t log_fetch_dp(Mailbox *mbp, double *valuep, logfmt_t *fmtp) {

  msg_t err;

  chDbgCheck(mbp != NULL, "log_fetch_dp");
  chDbgCheck(valuep != NULL, "log_fetch_dp");
  chDbgCheck(fmtp != NULL, "log_fetch_dp");

  err = log_fetch_raw(mbp, (logsym_t*)fmtp);
  if (err == RDY_OK) {
    logsym_t raw[2];

    err = log_fetch_raw(mbp, &raw[0]);
    chDbgCheck(err == RDY_OK, "log_fetch_dp");

    err = log_fetch_raw(mbp, &raw[1]);
    chDbgCheck(err == RDY_OK, "log_fetch_dp");

    *valuep = *(double*)((uint8_t*)raw);
  }
  return err;
}
#endif /* LOG_USE_FLOATS */

static msg_t log_stream_dchn(BaseSequentialStream *sp, Mailbox *mbp) {

  msg_t err = RDY_OK;
  logfmt_t head;
  char buf[4];
  uint32_t length;

  chDbgCheck(mbp != NULL, "log_stream_dchn");
  chDbgCheck(sp != NULL, "log_stream_dchn");

  err = log_fetch_raw(mbp, (logsym_t*)&head);
  if (err == RDY_OK) {
    length = (int32_t)head.raw & 0x00FFFFFF;
    chDbgCheck(head.fmt.type == LOGT_DCHN, "log_stream_dchn");
    for (; length >= sizeof(logsym_t) && err == RDY_OK; length -= sizeof(logsym_t)) {
      err = chMBFetch(mbp, (msg_t*)buf, TIME_INFINITE);
      chDbgCheck(err == RDY_OK, "log_stream_dchn");

      if (err == RDY_OK) {
        err = chSequentialStreamPut(sp, buf[0]);
        if (err == RDY_OK) {
          err = chSequentialStreamPut(sp, buf[1]);
          if (err == RDY_OK) {
            err = chSequentialStreamPut(sp, buf[2]);
            if (err == RDY_OK) {
              err = chSequentialStreamPut(sp, buf[3]);
            } else break;
          } else break;
        } else break;
      } else break;
    }
    if (length > 0 && length < sizeof(logsym_t) && err == RDY_OK) {
      err = chMBFetch(mbp, (msg_t*)buf, TIME_INFINITE);
      chDbgCheck(err == RDY_OK, "log_stream_dchn");

      if (length >= 1 && err == RDY_OK) {
        err = chSequentialStreamPut(sp, buf[0]);
        if (length >= 2 && err == RDY_OK) {
          err = chSequentialStreamPut(sp, buf[1]);
          if (length >= 3 && err == RDY_OK) {
            err = chSequentialStreamPut(sp, buf[2]);
          }
        }
      }
    }
  }
  return err;
}

static msg_t log_stream_dchz(BaseSequentialStream *sp, Mailbox *mbp) {

  msg_t err = RDY_OK;
  char buf[4];

  chDbgCheck(mbp != NULL, "log_stream_dchz");
  chDbgCheck(sp != NULL, "log_stream_dchz");

  err = log_fetch_raw(mbp, (logsym_t*)buf);
  if (buf[0] && err == RDY_OK) {
    chDbgCheck(buf[3] == LOGT_DCHZ, "log_stream_dchz");
    err = chSequentialStreamPut(sp, buf[0]);
    if (err == RDY_OK) {
      err = chSequentialStreamPut(sp, buf[1]);
      if (err == RDY_OK) {
        err = chSequentialStreamPut(sp, buf[2]);
        while (err == RDY_OK) {
          err = chMBFetch(mbp, (msg_t*)buf, TIME_INFINITE);
          chDbgCheck(err == RDY_OK, "log_stream_dchz");

          if (buf[0] && err == RDY_OK) {
            err = chSequentialStreamPut(sp, buf[0]);
            if (buf[1] && err == RDY_OK) {
              err = chSequentialStreamPut(sp, buf[1]);
              if (buf[2] && err == RDY_OK) {
                err = chSequentialStreamPut(sp, buf[2]);
                if (buf[3] && err == RDY_OK) {
                  err = chSequentialStreamPut(sp, buf[3]);
                } else break;
              } else break;
            } else break;
          } else break;
        }
      }
    }
  }
  return err;
}

msg_t logStreamChar(BaseSequentialStream *sp, char value, logfmt_t fmt) {

  msg_t err = RDY_OK;

  chDbgCheck(sp != NULL, "logStreamChar");

  if ((fmt.fmt.flags & LOGFF_WIDTH) && fmt.fmt.width >= 1) {
    unsigned width = (unsigned)fmt.fmt.width - 1;
    if (fmt.fmt.flags & LOGFF_LEFT) {
      err = chSequentialStreamPut(sp, value);
      while (width-- && err == RDY_OK) {
        err = chSequentialStreamPut(sp, ' ');
      }
    } else {
      while (width-- && err == RDY_OK) {
        err = chSequentialStreamPut(sp, ' ');
      }
      if (err == RDY_OK) {
        err = chSequentialStreamPut(sp, value);
      }
    }
  } else {
    err = chSequentialStreamPut(sp, value);
  }
  return err;
}

static msg_t log_stream_strn(BaseSequentialStream *sp, Mailbox *mbp) {

  msg_t err;
  logfmt_t head;
  uint32_t length, width, limit, margin;
  char buf[4];

  chDbgCheck(sp != NULL, "log_stream_strn");
  chDbgCheck(mbp != NULL, "log_stream_strn");

  err = log_fetch_raw(mbp, (logsym_t*)&head);
  if (err == RDY_OK) {
    chDbgCheck(head.fmt.type == LOGT_STRN || head.fmt.type == LOGT_WSTRN, "log_stream_strn");

    err = log_fetch_raw(mbp, (logsym_t*)&length);
    chDbgCheck(err == RDY_OK, "log_stream_strn");

    /* Handle the padding and width.*/
    limit = (head.fmt.flags & LOGFF_DIGITS) ? (unsigned)head.fmt.digits : length;
    width = (head.fmt.flags & LOGFF_WIDTH) ? (unsigned)head.fmt.width : 0;
    margin = (width > limit) ? (width - limit) : 0;
    length -= limit;

    /* Print the left margin.*/
    if (!(head.fmt.flags & LOGFF_LEFT)) {
      while (margin-- && err == RDY_OK) { err = chSequentialStreamPut(sp, ' '); }
    }

    /* Print the string.*/
    for (; limit >= sizeof(logsym_t) && err == RDY_OK; limit -= sizeof(logsym_t)) {
      err = chMBFetch(mbp, (msg_t*)buf, TIME_INFINITE);
      chDbgCheck(err == RDY_OK, "log_stream_strn");

      err = chSequentialStreamPut(sp, buf[0]);
      if (err == RDY_OK) {
        err = chSequentialStreamPut(sp, buf[1]);
        if (err == RDY_OK) {
          err = chSequentialStreamPut(sp, buf[2]);
          if (err == RDY_OK) {
            err = chSequentialStreamPut(sp, buf[3]);
          }
        }
      }
    }
    if (limit > 0 && limit < sizeof(logsym_t) && err == RDY_OK) {
      err = chMBFetch(mbp, (msg_t*)buf, TIME_INFINITE);
      chDbgCheck(err == RDY_OK, "log_stream_strn");

      if (limit >= 1 && err == RDY_OK) {
        err = chSequentialStreamPut(sp, buf[0]);
        if (limit >= 2 && err == RDY_OK) {
          err = chSequentialStreamPut(sp, buf[1]);
          if (limit >= 3 && err == RDY_OK) {
            err = chSequentialStreamPut(sp, buf[2]);
          }
        }
      }
    }

    /* Print the right margin.*/
    if (head.fmt.flags & LOGFF_LEFT) {
      while (margin-- && err == RDY_OK) { err = chSequentialStreamPut(sp, ' '); }
    }

    /* Fetch the remaining characters, if any.*/
    for (; length >= sizeof(logsym_t); length -= sizeof(logsym_t)) {
      err = chMBFetch(mbp, (msg_t*)buf, TIME_INFINITE);
      chDbgCheck(err == RDY_OK, "log_stream_strn");
    }
    if (length > 0) {
      err = chMBFetch(mbp, (msg_t*)buf, TIME_INFINITE);
      chDbgCheck(err == RDY_OK, "log_stream_strn");
    }
  }
  return err;
}

static msg_t log_stream_cstrn(BaseSequentialStream *sp, Mailbox *mbp) {

  msg_t err;
  logfmt_t head;
  uint32_t length, width, margin;
  const char *strp;

  chDbgCheck(sp != NULL, "log_stream_cstrn");
  chDbgCheck(mbp != NULL, "log_stream_cstrn");

  err = log_fetch_raw(mbp, (logsym_t*)&head);
  if (err == RDY_OK) {
    chDbgCheck(head.fmt.type == LOGT_CSTRN || head.fmt.type == LOGT_WCSTRN, "log_stream_cstrn");

    err = log_fetch_raw(mbp, (logsym_t*)&length);
    chDbgCheck(err == RDY_OK, "log_stream_cstrn");

    err = log_fetch_raw(mbp, (logsym_t*)&strp);
    chDbgCheck(err == RDY_OK, "log_stream_cstrn");

    /* Handle the padding and width.*/
    length = (head.fmt.flags & LOGFF_DIGITS) ? (unsigned)head.fmt.digits : length;
    width = (head.fmt.flags & LOGFF_WIDTH) ? (unsigned)head.fmt.width : 0;
    margin = (width > length) ? (width - length) : 0;

    if (head.fmt.flags & LOGFF_LEFT) {
      /* Print the number and then the margin.*/
      while (length-- && err == RDY_OK) { err = chSequentialStreamPut(sp, *strp++); }
      while (margin-- && err == RDY_OK) { err = chSequentialStreamPut(sp, ' '); }
    } else {
      /* Print the margin and then the number.*/
      while (margin-- && err == RDY_OK) { err = chSequentialStreamPut(sp, ' '); }
      while (length-- && err == RDY_OK) { err = chSequentialStreamPut(sp, *strp++); }
    }
  }
  return err;
}

#if LOG_USE_WCHAR || defined(__DOXYGEN__)
msg_t logStreamWchar(BaseSequentialStream *sp, wchar_t value, logfmt_t fmt) {

  msg_t err = RDY_OK;

  chDbgCheck(sp != NULL, "logStreamWchar");

  if ((fmt.fmt.flags & LOGFF_WIDTH) && fmt.fmt.width >= 1) {
    unsigned width = (unsigned)fmt.fmt.width - 1;
    if (fmt.fmt.flags & LOGFF_LEFT) {
      err = chSequentialStreamPut(sp, (uint8_t)value);
      if (err == RDY_OK) {
        err = chSequentialStreamPut(sp, (uint8_t)(value >> 8));
      }
      while (width-- && err == RDY_OK) {
        err = chSequentialStreamPut(sp, ' ');
      }
    } else {
      while (width-- && err == RDY_OK) {
        err = chSequentialStreamPut(sp, ' ');
      }
      if (err == RDY_OK) {
        err = chSequentialStreamPut(sp, (uint8_t)value);
        if (err == RDY_OK) {
          err = chSequentialStreamPut(sp, (uint8_t)(value >> 8));
        }
      }
    }
  } else {
    err = chSequentialStreamPut(sp, (uint8_t)value);
    if (err == RDY_OK) {
      err = chSequentialStreamPut(sp, (uint8_t)(value >> 8));
    }
  }
  return err;
}
#endif /* LOG_USE_WCHAR */

#if LOG_USE_UINTS || defined(__DOXYGEN__)
msg_t logStreamUint32(BaseSequentialStream *sp, uint32_t value, logfmt_t fmt) {

  msg_t err = RDY_OK;
  char strbuf[2*10 + 1], sign;
  size_t length;

  chDbgCheck(sp != NULL, "logStreamUint32");

  /* Handle the sign.*/
  if (fmt.fmt.flags & LOGFF_SIGN) { sign = '+'; }
  else if (fmt.fmt.flags & LOGFF_SPACE) { sign = ' '; }
  else { sign = 0; }

  /* Conversion into string.*/
  length = log_ultoa_div(strbuf, value, 0) - strbuf;

  /* Print the formatted string value.*/
  err = logStreamBufValue(sp, sign, NULL, 0, strbuf, length, fmt);

  return err;
}

#if LOG_USE_64BIT || defined(__DOXYGEN__)
msg_t logStreamUint64(BaseSequentialStream *sp, uint64_t value, logfmt_t fmt) {

  chDbgCheck(sp != NULL, "logStreamUint64");

  if (value < (1ull << 32u)) {
    /* Print short numbers as decimal integers.*/
    return logStreamUint32(sp, (uint32_t)value, fmt);
  } else {
    /* Print long numbers as hex (does not involve 64bit mul/div).*/
    if (fmt.fmt.flags & LOGFF_SIGN) {
      fmt.fmt.flags &= ~LOGFF_SIGN;
      fmt.fmt.flags |= LOGFF_SPACE;
    }
    return logStreamHex64(sp, value, fmt);
  }
}
#endif /* LOG_USE_64BIT */
#endif /* LOG_USE_UINTS */

#if LOG_USE_INTS || defined(__DOXYGEN__)
msg_t logStreamInt32(BaseSequentialStream *sp, int32_t value, logfmt_t fmt) {

  msg_t err = RDY_OK;
  char strbuf[2*10 + 1], sign;
  size_t length;

  chDbgCheck(sp != NULL, "logStreamInt32");

  /* Handle the sign.*/
  if (value > 0) {
    if (fmt.fmt.flags & LOGFF_SIGN) { sign = '+'; }
    else if (fmt.fmt.flags & LOGFF_SPACE) { sign = ' '; }
    else { sign = 0; }
  }
  else if (value < 0) { sign = '-'; value = -value; }
  else if (fmt.fmt.flags & LOGFF_SPACE) { sign = ' '; }
  else { sign = 0; }

  /* Conversion into string.*/
  length = log_ultoa_div(strbuf, (uint32_t)value, 0) - strbuf;

  /* Print the formatted string value.*/
  err = logStreamBufValue(sp, sign, NULL, 0, strbuf, length, fmt);

  return err;
}

#if LOG_USE_64BIT || defined(__DOXYGEN__)
msg_t logStreamInt64(BaseSequentialStream *sp, int64_t value, logfmt_t fmt) {

  chDbgCheck(sp != NULL, "logStreamInt64");

  if (value < (1ll << 32u) && value > -(1ll << 32u)) {
    /* Print short numbers as decimal integers.*/
    return logStreamInt32(sp, (int32_t)value, fmt);
  } else {
    /* Print long numbers as hex (does not involve 64bit mul/div).*/
    if (fmt.fmt.flags & LOGFF_SIGN) {
      fmt.fmt.flags &= ~LOGFF_SIGN;
      fmt.fmt.flags |= LOGFF_SPACE;
    }
    return logStreamHex64(sp, (uint64_t)value, fmt);
  }
}
#endif /* LOG_USE_64BIT */
#endif /* LOG_USE_INTS */

#if LOG_USE_OCTALS || defined(__DOXYGEN__)
msg_t logStreamOct32(BaseSequentialStream *sp, uint32_t value, logfmt_t fmt) {

  msg_t err = RDY_OK;
  char strbuf[11], *ptr, sign;
  const char *prefix;
  size_t length, i;

  chDbgCheck(sp != NULL, "logStreamOct32");

  /* Handle the sign and prefix.*/
  if (fmt.fmt.flags & LOGFF_SIGN) { sign = '+'; }
  else if (fmt.fmt.flags & LOGFF_SPACE) { sign = ' '; }
  else { sign = 0; }
  prefix = (fmt.fmt.flags & LOGFF_HASH) ? "0" : NULL;

  /* Conversion into string.*/
  for (i = 11; i > 1 && ((value >> ((i - 1) * 3u)) & 0x07) == 0; --i) {}
  length = i;
  for (ptr = strbuf; i--; ++ptr) {
    *ptr = (char)((value >> (i * 3u)) & 0x07) + '0';
  }

  /* Print the formatted string value.*/
  err = logStreamBufValue(sp, sign, prefix, prefix ? 1 : 0, strbuf, length, fmt);

  return err;
}

#if LOG_USE_64BIT || defined(__DOXYGEN__)
msg_t logStreamOct64(BaseSequentialStream *sp, uint64_t value, logfmt_t fmt) {

  msg_t err = RDY_OK;
  char strbuf[22], *ptr, sign;
  const char *prefix;
  size_t length, i;

  chDbgCheck(sp != NULL, "logStreamOct64");

  /* Handle the sign and prefix.*/
  if (fmt.fmt.flags & LOGFF_SIGN) { sign = '+'; }
  else if (fmt.fmt.flags & LOGFF_SPACE) { sign = ' '; }
  else { sign = 0; }
  prefix = (fmt.fmt.flags & LOGFF_HASH) ? "0" : NULL;

  /* Conversion into string.*/
  for (i = 22; i > 1 && ((value >> ((i - 1) * 3u)) & 0x07) == 0; --i) {}
  length = i;
  for (ptr = strbuf; i--; ++ptr) {
    *ptr = ((char)(value >> (i * 3u)) & 0x07) + '0';
  }

  /* Print the formatted string value.*/
  err = logStreamBufValue(sp, sign, prefix, prefix ? 1 : 0, strbuf, length, fmt);

  return err;
}
#endif /* LOG_USE_64BIT */
#endif /* LOG_USE_OCTALS */

msg_t logStreamHex32(BaseSequentialStream *sp, uint32_t value, logfmt_t fmt) {

  msg_t err = RDY_OK;
  char strbuf[8], *ptr, sign;
  const char *prefix;
  size_t length, i;

  chDbgCheck(sp != NULL, "logStreamHex32");

  /* Handle the sign and prefix.*/
  if (fmt.fmt.flags & LOGFF_SIGN) { sign = '+'; }
  else if (fmt.fmt.flags & LOGFF_SPACE) { sign = ' '; }
  else { sign = 0; }
  if (fmt.fmt.flags & LOGFF_HASH) {
    prefix = (fmt.fmt.flags & LOGFF_UPPER) ? "0x" : "0x"; /* I hate capital 'X'...*/
  } else { prefix = NULL; }

  /* Conversion into string.*/
  for (i = 8; i > 1 && ((value >> ((i - 1) << 2)) & 0x0F) == 0; --i) {}
  length = i;
  if (fmt.fmt.flags & LOGFF_UPPER) {
    for (ptr = strbuf; i--; ++ptr) {
      *ptr = hex_uppercase[ (value >> (i << 2)) & 0x0F ];
    }
  } else {
    for (ptr = strbuf; i--; ++ptr) {
      *ptr = hex_lowercase[ (value >> (i << 2)) & 0x0F ];
    }
  }

  /* Print the formatted string value.*/
  err = logStreamBufValue(sp, sign, prefix, prefix ? 2 : 0, strbuf, length, fmt);

  return err;
}

#if LOG_USE_64BIT || defined(__DOXYGEN__)
msg_t logStreamHex64(BaseSequentialStream *sp, uint64_t value, logfmt_t fmt) {

  msg_t err = RDY_OK;

  chDbgCheck(sp != NULL, "logStreamHex64");

  /* Print the first 4 hex bytes.*/
  if (value >= (1ull << 32u)) {
    fmt.fmt.width = (fmt.fmt.width > 8) ? (fmt.fmt.width - 8) : 0;
  }
  err = logStreamHex32(sp, ((uint32_t*)((uint8_t*)&value))[1], fmt);
  if (err == RDY_OK) {
    /* Disable prefixes and print the second 4 hex bytes.*/
    fmt.fmt.digits = 8;
    fmt.fmt.flags &= ~(LOGFF_SIGN | LOGFF_SPACE | LOGFF_HASH | LOGFF_WIDTH);
    fmt.fmt.flags |= LOGFF_ZERO | LOGFF_DIGITS;
    err = logStreamHex32(sp, ((uint32_t*)((uint8_t*)&value))[0], fmt);
  }
  return err;
}
#endif /* LOG_USE_64BIT */

#if LOG_USE_FLOATS || defined(__DOXYGEN__)
#if !LOG_USE_STRICT_FLOATS || defined(__DOXYGEN__)
msg_t logStreamDoubleStrict(BaseSequentialStream *sp, double value, logfmt_t fmt) {

  msg_t err = RDY_OK;
  char strbuf[3*10 + 1], *ptr, sign;
  uint32_t integral, prec;
  size_t length;

  chDbgCheck(sp != NULL, "logStreamDoubleStrict");

  /* Handle the sign.*/
  if (value > 0) {
    if (fmt.fmt.flags & LOGFF_SIGN) { sign = '+'; }
    else if (fmt.fmt.flags & LOGFF_SPACE) { sign = ' '; }
    else { sign = 0; }
  }
  else if (value < 0) { sign = '-'; value = -value; }
  else if (fmt.fmt.flags & LOGFF_SPACE) { sign = ' '; }
  else { sign = 0; }

  /* Conversion into string.*/
  if (fmt.fmt.flags & LOGFF_DIGITS) {
    integral = (uint32_t)fmt.fmt.digits;
    if (integral > 10) { integral = 10; }
    for (prec = 1; integral--; prec *= 10) {}
  } else {
    prec = 10000;
  }
  integral = (uint32_t)value;
  ptr = log_ultoa_div(strbuf, integral, 0);
  if (value != (double)integral) {
    *ptr++ = '.';
    integral = (uint32_t)((value - integral) * prec);
    ptr = log_ultoa_div(ptr, integral, prec / 10);
  } else if (fmt.fmt.flags & LOGFF_HASH) {
    *ptr++ = '.';
  }
  length = ptr - strbuf;

  /* Print the formatted string value.*/
  fmt.fmt.flags &= ~LOGFF_DIGITS;
  err = logStreamBufValue(sp, sign, NULL, 0, strbuf, length, fmt);

  return err;
}
#endif /* !LOG_USE_STRICT_FLOATS */

msg_t logStreamDoubleE(BaseSequentialStream *sp, double value, logfmt_t fmt) {

  msg_t err = RDY_OK;
  char strbuf[3*10 + 1], *ptr, sign;
  int32_t exp;
  uint32_t prec, digits, i, integral;
  uint64_t man64, bitfield;
  size_t length;

  chDbgCheck(sp != NULL, "logStreamDoubleE");

  /* Get fields.*/
  if (fmt.fmt.flags & LOGFF_DIGITS) {
    digits = (fmt.fmt.digits <= 16) ? (uint32_t)fmt.fmt.digits : 16;
    for (i = digits, prec = 1; i--; prec *= 10) {}
  } else {
    digits = DEF_FLOAT_DIGITS;
    prec = DEF_FLOAT_PREC;
  }
  bitfield = *(uint64_t*)((uint8_t*)&value);
  exp = (int32_t)(bitfield >> 52) & 0x7FF;
  man64 = (bitfield & ((1ull << 53) - 1ull)) | (1ull << 53); /* 64-bits mantissa.*/

  /* Handle the sign.*/
  if (value > 0) {
    if (fmt.fmt.flags & LOGFF_SIGN) { sign = '+'; }
    else if (fmt.fmt.flags & LOGFF_SPACE) { sign = ' '; }
    else { sign = 0; }
  }
  else if (value < 0) { sign = '-'; value = -value; }
  else if (fmt.fmt.flags & LOGFF_SPACE) { sign = ' '; }
  else { sign = 0; }

  /* Conversion into string.*/
  if (exp == 0x7FF) {
    if (man64 << 1) {
      strbuf[0] = 'N';
      strbuf[1] = 'a';
      strbuf[2] = 'N';
      length = 3;
      if (fmt.fmt.flags & LOGFF_SIGN) {
        fmt.fmt.flags &= ~LOGFF_SIGN;
        fmt.fmt.flags |= LOGFF_SPACE;
      }
    } else {
      strbuf[0] = 'I';
      strbuf[1] = 'n';
      strbuf[2] = 'f';
      length = 3;
    }
  } else {
    if (exp == 0x000) {
      if (man64 << 1) {
        /* Denormalized number.*/ /* FIXME: untested */
        strbuf[0] = '0';
        strbuf[1] = '.';
        integral = (uint32_t)(value * prec);
        ptr = log_ultoa_div(strbuf + 2, integral, prec / 10);
        length = ptr - strbuf;
      } else {
        /* Zero.*/
        const char *src = "0.0000000000000000";
        char *dst = strbuf;
        length = (digits > 0) ? (digits + 2) : ((fmt.fmt.flags & LOGFF_HASH) ? 2 : 1);
        while (length--) { *dst++ = *src++; }
      }
    } else {
      /* Generic number.*/
      exp = (int32_t)floor(log10(value));
      value *= pow(10.0, (double)(-exp));

      /* Write the normalized number.*/
      integral = (uint32_t)value;
      ptr = log_ultoa_div(strbuf, integral, 0);
      if (value != (double)integral) {
        *ptr++ = '.';
        integral = (int32_t)((value - integral) * prec);
        ptr = log_ultoa_div(ptr, integral, prec / 10);
      } else if (fmt.fmt.flags & LOGFF_HASH) {
        *ptr++ = '.';
      }

      /* Write the exponent.*/
      *ptr++ = (fmt.fmt.flags & LOGFF_UPPER) ? 'E' : 'e';
      if (exp >= 0) { *ptr++ = '+'; }
      else          { *ptr++ = '-'; exp = -exp; }
      ptr = log_ultoa_div(ptr, (uint32_t)exp, 0);
      length = ptr - strbuf;
    }
  }

  /* Print the formatted string value.*/
  fmt.fmt.flags &= ~LOGFF_DIGITS;
  err = logStreamBufValue(sp, sign, NULL, 0, strbuf, length, fmt);

  return err;
}

msg_t logStreamDoubleG(BaseSequentialStream *sp,
                              double value, logfmt_t fmt) {

  chDbgCheck(sp != NULL, "logStreamDoubleG");

#if LOG_USE_STRICT_FLOATS
  /* TODO */
  return RDY_OK;
#else
  /* Choice based on the value instead of its representation.*/
  if (value >= -0x80000000 && value <= 0x7FFFFFFF) {
    return logStreamDoubleStrict(sp, value, fmt);
  } else {
    return logStreamDoubleG(sp, value, fmt);
  }
#endif
}
#endif /* LOG_USE_FLOATS */

/**
 * @brief   Formats and streams a buffered string.
 *
 * @param[in] sp
 *          Pointer to the stream on which symbols are printed.
 * @param[in] sign
 *          Sign character. Can be <tt>'+'</tt>, <tt>'-'</tt>, <tt>' '</tt> or
 *          <tt>0</tt> (none).
 * @param[in] prefix
 *          Pointer to the prefix string, or @p NULL.
 * @param[in] prelen
 *          Prefix length, mandatory.
 * @param[in] value
 *          Pointer to the string value.
 * @param[in] length
 *          Length of the string value.
 * @param[in] fmt
 *          Format record.
 * @return
 *          Operation status (see <tt>BaseSequentialStream::put()</tt>).
 *
 * @notapi
 */
msg_t logStreamBufValue(BaseSequentialStream *sp, char sign,
                        const char *prefixp, size_t prelen,
                        const char *valuep, size_t length,
                        logfmt_t fmt) {

  msg_t err = RDY_OK;
  size_t digits, width, zeroes, margin;

  chDbgCheck(sp != NULL, "logStreamBufValue");
  chDbgCheck(!(length > 0) || valuep != NULL, "logStreamBufValue");
  chDbgCheck(!(prelen > 0) || prefixp != NULL, "logStreamBufValue");

  /* Handle the padding and width.*/
  digits = (fmt.fmt.flags & LOGFF_DIGITS) ? (size_t)fmt.fmt.digits : length;
  width = (fmt.fmt.flags & LOGFF_WIDTH) ? (size_t)fmt.fmt.width : 0;
  digits = (length > digits) ? length : digits;
  zeroes = (digits > length) ? (digits - length) : 0;
  margin = digits + (sign != 0) + prelen;
  margin = (width > margin) ? (width - margin) : 0;

  if (fmt.fmt.flags & LOGFF_LEFT) {
    /* Sign, prefix, leading zeroes, value, margin.*/
    if    (sign     && err == RDY_OK) { err = chSequentialStreamPut(sp, sign); }
    while (prelen-- && err == RDY_OK) { err = chSequentialStreamPut(sp, *prefixp++); }
    while (zeroes-- && err == RDY_OK) { err = chSequentialStreamPut(sp, '0'); }
    while (length-- && err == RDY_OK) { err = chSequentialStreamPut(sp, *valuep++); }
    while (margin-- && err == RDY_OK) { err = chSequentialStreamPut(sp, ' '); }
  } else {
    /* Margin, sign, prefix, leading zeroes, value.*/
    if (fmt.fmt.flags & LOGFF_ZERO) {
      if    (sign     && err == RDY_OK) { err = chSequentialStreamPut(sp, sign); }
      while (prelen-- && err == RDY_OK) { err = chSequentialStreamPut(sp, *prefixp++); }
      while (margin-- && err == RDY_OK) { err = chSequentialStreamPut(sp, '0'); }
    } else {
      while (margin-- && err == RDY_OK) { err = chSequentialStreamPut(sp, ' '); }
      if    (sign     && err == RDY_OK) { err = chSequentialStreamPut(sp, sign); }
      while (prelen-- && err == RDY_OK) { err = chSequentialStreamPut(sp, *prefixp++); }
    }
    while (zeroes-- && err == RDY_OK) { err = chSequentialStreamPut(sp, '0'); }
    while (length-- && err == RDY_OK) { err = chSequentialStreamPut(sp, *valuep++); }
  }
  return err;
}

/**
 * @brief   Streams the next enqueued symbol.
 *
 * @param[in] sp
 *          Pointer to the stream on which symbols are printed.
 * @param[in] mbp
 *          Pointer to the log @param Mailbox queue.
 * @return
 *          Operation status (see <tt>BaseSequentialStream::put()</tt>}).
 *
 * @notapi
 */
static msg_t log_stream_next(BaseSequentialStream *sp, Mailbox *mbp) {

  msg_t err = RDY_OK;
  logfmt_t fmt;

  chDbgCheck(mbp != NULL, "log_stream_next");
  chDbgCheck(sp != NULL, "log_stream_next");

  chSysLock();
  if (chMBGetUsedCountI(mbp) > 0) {
    fmt.raw = (logsym_t)chMBPeekI(mbp);
  } else {
    err = RDY_TIMEOUT;
  }
  chSysUnlock();
  if (err == RDY_OK) {
    switch (fmt.fmt.type) {
    case LOGT_DCH1: {
      logsym_t raw;
      err = log_fetch_raw(mbp, &raw);
      if (err == RDY_OK) {
        err = chSequentialStreamPut(sp, (uint8_t)raw);
      }
      break;
    }
    case LOGT_DCH2: {
      logsym_t raw;
      err = log_fetch_raw(mbp, &raw);
      if (err == RDY_OK) {
        err = chSequentialStreamPut(sp, (uint8_t)raw);
        if (err == RDY_OK) {
          err = chSequentialStreamPut(sp, (uint8_t)(raw >> 8));
        }
      }
      break;
    }
    case LOGT_DCH3: {
      logsym_t raw;
      err = log_fetch_raw(mbp, &raw);
      if (err == RDY_OK) {
        err = chSequentialStreamPut(sp, (uint8_t)raw);
        if (err == RDY_OK) {
          err = chSequentialStreamPut(sp, (uint8_t)(raw >> 8));
          if (err == RDY_OK) {
            err = chSequentialStreamPut(sp, (uint8_t)(raw >> 16));
          }
        }
      }
      break;
    }
    case LOGT_DCHN: {
      err = log_stream_dchn(sp, mbp);
      break;
    }
    case LOGT_DCHZ: {
      err = log_stream_dchz(sp, mbp);
      break;
    }
    case LOGT_DCHR: {
      char c;
      uint16_t cnt;
      err = log_fetch_dchr(mbp, &c, &cnt);
      while (cnt-- && err == RDY_OK) {
        err = chSequentialStreamPut(sp, (uint8_t)c);
      }
      break;
    }
    case LOGT_DCSTR: {
      const char *strp;
      err = log_fetch_dcstr(mbp, &strp);
      while (*strp && err == RDY_OK) {
        err = chSequentialStreamPut(sp, (uint8_t)*strp++);
      }
      break;
    }
    case LOGT_STRN: {
      err = log_stream_strn(sp, mbp);
      break;
    }
    case LOGT_CSTRN: {
      err = log_stream_cstrn(sp, mbp);
      break;
    }
    case LOGT_CHAR: {
      char value;
      err = log_fetch_uint8(mbp, (uint8_t*)&value, &fmt);
      if (err == RDY_OK) {
        err = logStreamChar(sp, value, fmt);
      }
      break;
    }
#if LOG_USE_WCHAR
    case LOGT_WCHAR: {
      wchar_t value;
      err = log_fetch_uint16(mbp, (uint16_t*)&value, &fmt);
      if (err == RDY_OK) {
        err = logStreamWchar(sp, value, fmt);
      }
      break;
    }
    case LOGT_WSTRN: {
      err = log_stream_strn(sp, mbp);
      break;
    }
    case LOGT_WCSTRN: {
      err = log_stream_cstrn(sp, mbp);
      break;
    }
#endif /* LOG_USE_WCHAR */
    case LOGT_PTR: {
      uint32_t addr;
      err = log_fetch_uint32(mbp, &addr, &fmt);
      if (err == RDY_OK) {
        err = logStreamHex32(sp, addr, fmt);
      }
      break;
    }
#if LOG_USE_INTS
    case LOGT_INT8: {
      int8_t value;
      err = log_fetch_uint8(mbp, (uint8_t*)&value, &fmt);
      if (err == RDY_OK) {
        err = logStreamInt32(sp, (int32_t)value, fmt);
      }
      break;
    }
    case LOGT_INT16: {
      int16_t value;
      err = log_fetch_uint16(mbp, (uint16_t*)&value, &fmt);
      if (err == RDY_OK) {
        err = logStreamInt32(sp, (int32_t)value, fmt);
      }
      break;
    }
    case LOGT_INT32: {
      uint32_t value;
      err = log_fetch_uint32(mbp, (uint32_t*)&value, &fmt);
      if (err == RDY_OK) {
        err = logStreamInt32(sp, value, fmt);
      }
      break;
    }
#if LOG_USE_64BIT
    case LOGT_INT64: {
      uint64_t value;
      err = log_fetch_uint64(mbp, (uint64_t*)&value, &fmt);
      if (err == RDY_OK) {
        err = logStreamInt64(sp, value, fmt);
      }
      break;
    }
#endif /* LOG_USE_64BIT */
#endif /* LOG_USE_INTS */
#if LOG_USE_UINTS
    case LOGT_UINT8: {
      uint8_t value;
      err = log_fetch_uint8(mbp, &value, &fmt);
      if (err == RDY_OK) {
        err = logStreamUint32(sp, (uint32_t)value, fmt);
      }
      break;
    }
    case LOGT_UINT16: {
      uint16_t value;
      err = log_fetch_uint16(mbp, &value, &fmt);
      if (err == RDY_OK) {
        err = logStreamUint32(sp, (uint32_t)value, fmt);
      }
      break;
    }
    case LOGT_UINT32: {
      uint32_t value;
      err = log_fetch_uint32(mbp, &value, &fmt);
      if (err == RDY_OK) {
        err = logStreamUint32(sp, value, fmt);
      }
      break;
    }
#if LOG_USE_64BIT
    case LOGT_UINT64: {
      uint64_t value;
      err = log_fetch_uint64(mbp, &value, &fmt);
      if (err == RDY_OK) {
        err = logStreamUint64(sp, value, fmt);
      }
      break;
    }
#endif /* LOG_USE_64BIT */
#endif /* LOG_USE_UINTS */
#if LOG_USE_OCTALS
    case LOGT_OCT8: {
      uint8_t value;
      err = log_fetch_uint8(mbp, &value, &fmt);
      if (err == RDY_OK) {
        err = logStreamOct32(sp, (uint32_t)value, fmt);
      }
      break;
    }
    case LOGT_OCT16: {
      uint16_t value;
      err = log_fetch_uint16(mbp, &value, &fmt);
      if (err == RDY_OK) {
        err = logStreamOct32(sp, (uint32_t)value, fmt);
      }
      break;
    }
    case LOGT_OCT32: {
      uint32_t value;
      err = log_fetch_uint32(mbp, &value, &fmt);
      if (err == RDY_OK) {
        err = logStreamOct32(sp, value, fmt);
      }
      break;
    }
#if LOG_USE_64BIT
    case LOGT_OCT64: {
      uint64_t value;
      err = log_fetch_uint64(mbp, &value, &fmt);
      if (err == RDY_OK) {
        err = logStreamOct64(sp, value, fmt);
      }
      break;
    }
#endif /* LOG_USE_64BIT */
#endif /* LOG_USE_OCTALS */
    case LOGT_HEX8: {
      uint8_t value;
      err = log_fetch_uint8(mbp, &value, &fmt);
      if (err == RDY_OK) {
        err = logStreamHex32(sp, (uint32_t)value, fmt);
      }
      break;
    }
    case LOGT_HEX16: {
      uint16_t value;
      err = log_fetch_uint16(mbp, &value, &fmt);
      if (err == RDY_OK) {
        err = logStreamHex32(sp, (uint32_t)value, fmt);
      }
      break;
    }
    case LOGT_HEX32: {
      uint32_t value;
      err = log_fetch_uint32(mbp, &value, &fmt);
      if (err == RDY_OK) {
        err = logStreamHex32(sp, value, fmt);
      }
      break;
    }
#if LOG_USE_64BIT
    case LOGT_HEX64: {
      uint64_t value;
      err = log_fetch_uint64(mbp, &value, &fmt);
      if (err == RDY_OK) {
        err = logStreamHex64(sp, value, fmt);
      }
      break;
    }
#endif /* LOG_USE_64BIT */
#if LOG_USE_FLOATS
    case LOGT_FP: {
      float value;
      err = log_fetch_fp(mbp, &value, &fmt);
      if (err == RDY_OK) {
        err = logStreamDoubleStrict(sp, (double)value, fmt);
      }
      break;
    }
    case LOGT_DP: {
      double value;
      err = log_fetch_dp(mbp, &value, &fmt);
      if (err == RDY_OK) {
        err = logStreamDoubleStrict(sp, value, fmt);
      }
      break;
    }
    case LOGT_FPE: {
      float value;
      err = log_fetch_fp(mbp, &value, &fmt);
      if (err == RDY_OK) {
        err = logStreamDoubleE(sp, (double)value, fmt);
      }
      break;
    }
    case LOGT_DPE: {
      double value;
      err = log_fetch_dp(mbp, &value, &fmt);
      if (err == RDY_OK) {
        err = logStreamDoubleE(sp, value, fmt);
      }
      break;
    }
    case LOGT_FPG: {
      float value;
      err = log_fetch_fp(mbp, &value, &fmt);
      if (err == RDY_OK) {
        err = logStreamDoubleG(sp, (double)value, fmt);
      }
      break;
    }
    case LOGT_DPG: {
      double value;
      err = log_fetch_dp(mbp, &value, &fmt);
      if (err == RDY_OK) {
        err = logStreamDoubleG(sp, value, fmt);
      }
      break;
    }
#endif /* LOG_USE_FLOATS */
    case LOGT_NULL: {
      logsym_t dummy;
      err = log_fetch_raw(mbp, &dummy);
      break;
    }
    default: {
      chDbgPanic("Unknown message type");
      break;
    }
    }
  }
  return err;
}

/** @} */
