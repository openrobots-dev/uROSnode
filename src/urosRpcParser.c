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
 * @file    urosRpcParser.c
 * @author  Andrea Zoppi <texzk@email.it>
 *
 * @brief   XMLRPC parser methods.
 */

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include "../include/urosBase.h"
#include "../include/urosUser.h"
#include "../include/urosRpcParser.h"

#include <ctype.h>
#include <string.h>

/*===========================================================================*/
/* LOCAL TYPES & MACROS                                                      */
/*===========================================================================*/

#if UROS_RPCPARSER_C_USE_ASSERT == UROS_FALSE && !defined(__DOXYGEN__)
#undef urosAssert
#define urosAssert(expr)
#endif

#if UROS_RPCPARSER_C_USE_ERROR_MSG == UROS_FALSE && !defined(__DOXYGEN__)
#undef urosError
#define urosError(when, action, msgargs) { if (when) { action; } }
#endif

/*===========================================================================*/
/* LOCAL FUNCTIONS                                                           */
/*===========================================================================*/

static int uros_strnicmp(const char *s1, const char *s2, size_t n) {

  if (n != 0) {
    const unsigned char *us1, *us2;
    us1 = (const unsigned char *)s1,
    us2 = (const unsigned char *)s2;
    do {
      int c1 = tolower(*us1);
      int c2 = tolower(*us2++);
      if (c1 != c2) {
        return c1 - c2;
      } else if (*us1++ == 0) { break; }
    } while (--n != 0);
  }
  return 0;
}

uros_err_t uros_rpcparser_parambytag_partial(UrosRpcParser *pp,
                                             UrosRpcParam *paramp) {

  const char *tagp = NULL;
  char *strp = NULL;
  size_t taglen = 0;

  urosAssert(pp != NULL);
  urosAssert(paramp != NULL);
#define _CHKOK   { if (pp->err != UROS_OK) { goto _error; } }
#define _GOT(tokp, toklen) \
    (urosRpcParserExpectQuiet(pp, tokp, toklen) == UROS_OK)

  /* Assume it is a string by default.*/
  urosRpcParserParamValueString(pp, paramp); _CHKOK
  strp = paramp->value.string.datap;

  /* Check if it is another class instead.*/
  urosRpcParserXmlTagBeginNoName(pp); _CHKOK
  if (_GOT("i", 1)) {
    if (_GOT("nt>", 3)) {
      tagp = "int"; taglen = 3;
      urosFree(strp); strp = NULL;
      urosRpcParserParamValueInt(pp, paramp); _CHKOK
    } else if (_GOT("4>", 2)) {
      tagp = "i4"; taglen = 2;
      urosFree(strp); strp = NULL;
      urosRpcParserParamValueInt(pp, paramp); _CHKOK
    }
  } else if (_GOT("b", 1)) {
    if (_GOT("oolean>", 7)) {
      tagp = "boolean"; taglen = 7;
      urosFree(strp); strp = NULL;
      urosRpcParserParamValueBoolean(pp, paramp); _CHKOK
    } else if (_GOT("ase64>", 6)) {
      tagp = "base64"; taglen = 6;
      urosFree(strp); strp = NULL;
      urosRpcParserParamValueBase64(pp, paramp); _CHKOK
    }
  } else if (_GOT("double>", 7)) {
    tagp = "double"; taglen = 6;
    urosFree(strp); strp = NULL;
    urosRpcParserParamValueDouble(pp, paramp); _CHKOK
  } else if (_GOT("str", 3)) {
    if (_GOT("ing>", 4)) {
      tagp = "string"; taglen = 6;
      urosFree(strp); strp = NULL;
      urosRpcParserParamValueString(pp, paramp); _CHKOK
    } else if (_GOT("uct>", 4)) {
      tagp = "struct"; taglen = 6;
      urosFree(strp); strp = NULL;
      urosRpcParserParamValueStruct(pp, paramp); _CHKOK
    }
  } else if (_GOT("array>", 6)) {
    tagp = "array"; taglen = 5;
    urosFree(strp); strp = NULL;
    urosRpcParserParamValueArray(pp, paramp); _CHKOK
  } else if (_GOT("/value>", 7)) {
    /* It was a string.*/
    return pp->err = UROS_OK;
  }
  _CHKOK

  /* Close tags.*/
  urosRpcParserXmlTagClose(pp, tagp, taglen);
  urosRpcParserSkipWs(pp); _CHKOK
  urosRpcParserXmlTagClose(pp, "value", 5); _CHKOK
  return pp->err = UROS_OK;

_error:
  if (strp != NULL) { urosFree(strp); }
  return pp->err;
#undef _GOT
#undef _CHKOK
}

/*===========================================================================*/
/* GLOBAL FUNCTIONS                                                          */
/*===========================================================================*/

/** @addtogroup rpc_parser_funcs */
/** @{ */

/**
 * @brief   Initializes the XMLRPC parser object.
 * @details Assigns the connection and the read buffer.
 *
 * @pre     The parser is not initialized.
 * @post    The parser is assigned a connection and an optional temporary
 *          buffer. The other fields are reset.
 *
 * @param[in,out] pp
 *          Pointer to an allocated @p UrosRpcParser object.
 * @param[in] csp
 *          Pointer to an @p UrosConn connection from which to receive the
 *          stream to be parsed.
 * @param[in] rdbufp
 *          Pointer to the temporary buffer, used to parse strings ahead. Can
 *          be @p NULL only if @p rdbuflen is 0.
 * @param[in] rdbuflen
 *          Length of the temporary read buffer.
 */
void urosRpcParserObjectInit(UrosRpcParser *pp,
                             UrosConn *csp,
                             char *rdbufp, size_t rdbuflen) {

  urosAssert(pp != NULL);
  urosAssert(csp != NULL);
  urosAssert(!(rdbuflen > 0) || (rdbufp != NULL));

  /* Initialize fields.*/
  pp->csp = csp;
  pp->err = UROS_OK;
  pp->rdbufp = rdbufp;
  pp->rdbuflen = rdbuflen;
  pp->curp = NULL;
  pp->pending = 0;
  pp->total = 0;
  pp->mark = 0;
  pp->bufp = NULL;
  pp->buflen = 0;
  pp->contentLength = ~0u;
}

/**
 * @brief   Cleans a @p UrosRpcParser object.
 * @details Resets the state of its internal fields. If needed, the buffer is
 *          deallocated. Does not unlink from the assigned connection.
 *
 * @param[in,out] pp
 *          Pointer to an initialized @p UrosRpcParser object.
 * @param[in] freeBuffer
 *          Deallocates buffers.
 */
void urosRpcParserClean(UrosRpcParser *pp, uros_bool_t freeBuffer) {

  urosAssert(pp != NULL);

  if (freeBuffer) {
    /* Free the read buffer.*/
    urosFree(pp->rdbufp);
    pp->rdbufp = NULL;
    pp->rdbuflen = 0;
  }

  /* Initialize fields.*/
  pp->err = UROS_OK;
  pp->curp = NULL;
  pp->pending = 0;
  pp->total = 0;
  pp->mark = 0;
  pp->bufp = NULL;
  pp->buflen = 0;
  pp->contentLength = ~0u;
}

/**
 * @brief   Refills the parsing buffer.
 * @details Receives enough data to satisfy the <tt>Content-Length</tt>
 *          (set to <tt>~0u</tt> when unknown).
 *
 * @param[in,out] pp
 *          Pointer to an initialized @p UrosRpcParser object.
 * @return
 *          Error code.
 */
uros_err_t urosRpcParserRefill(UrosRpcParser *pp) {

  urosAssert(pp != NULL);

  pp->buflen = pp->contentLength - (pp->total - pp->mark);
  pp->err = urosConnRecv(pp->csp, (void**)&pp->bufp, &pp->buflen);
  urosError(pp->err != UROS_OK, return pp->err,
            ("Error %s while refilling data from "UROS_ADDRFMT"\n",
             urosErrorText(pp->err), UROS_ADDRARG(&pp->csp->remaddr)));
  pp->pending = pp->buflen;
  pp->curp = pp->bufp;
  return pp->err = UROS_OK;
}

/**
 * @brief   Reads a chunk from the incoming stream.
 * @details Copies a chunk of the incoming stream to a pre-allocated buffer.
 *
 * @param[in,out] pp
 *          Pointer to an initialized @p UrosRpcParser object.
 * @param[in,out] chunkp
 *          Pointer to the pre-allocated buffer on which the chunk will be
 *          copied after reception.
 * @param[in] chunklen
 *          Length of the chunk to be read. Can be @p 0.
 * @return
 *          Error code.
 */
uros_err_t urosRpcParserRead(UrosRpcParser *pp,
                             void *chunkp, size_t chunklen) {

  urosAssert(pp != NULL);
  urosAssert(chunkp != NULL);

  if (chunklen == 0) { return pp->err = UROS_OK; }
  while (UROS_TRUE) {
    if (chunklen <= pp->pending) {
      memcpy(chunkp, pp->curp, chunklen);
      chunkp = (void*)((uint8_t*)chunkp + chunklen);
      pp->pending -= chunklen;
      pp->curp += chunklen;
      pp->total += chunklen;
      return pp->err = UROS_OK;
    } else {
      memcpy(chunkp, pp->curp, pp->pending);
      chunkp = (void*)((uint8_t*)chunkp + pp->pending);
      chunklen -= pp->pending;
      pp->total += pp->pending;
      urosRpcParserRefill(pp);
      if (pp->err != UROS_OK) { return pp->err; }
    }
  }
  /* Unreachable code.*/
  urosAssert(0 && "Unreachable code");
  return pp->err = UROS_ERR_PARSE;
}

/**
 * @brief   Expects the parser to be pointing to a token.
 * @details Does not print any parsing error messages.
 *
 * @param[in,out] pp
 *          Pointer to an initialized @p UrosRpcParser object.
 * @param[in] tokp
 *          Pointer to the expected token string.
 * @param[in] toklen
 *          Length of the expected token.
 * @return
 *          Error code.
 * @retval UROS_ERR_PARSE
 *          The received token does not match.
 */
uros_err_t urosRpcParserExpectQuiet(UrosRpcParser *pp,
                                    const char *tokp, size_t toklen) {

  const char *curp;
  size_t pending;

  urosAssert(pp != NULL);
  urosAssert(tokp != NULL);
  urosAssert(toklen > 0);

  curp = tokp;
  pending = toklen;
  while (UROS_TRUE) {
    if (pending <= pp->pending) {
      if (0 == memcmp(pp->curp, curp, pending)) {
        /* The prefix matches the token.*/
        pp->curp += pending;
        pp->pending -= pending;
        pp->total += pending;
        return pp->err = UROS_OK;
      } else {
        return pp->err = UROS_ERR_PARSE;
      }
    } else {
      /* The token may fit only partially.*/
      if (pp->pending == 0 || 0 == memcmp(pp->curp, curp, pp->pending)) {
        /* Skip the matched prefix.*/
        pending -= pp->pending;
        curp += pp->pending;
        pp->total += pp->pending;

        /* Fetch the next chunk and check again.*/
        urosRpcParserRefill(pp);
        if (pp->err != UROS_OK) { return pp->err; }
      } else {
        return pp->err = UROS_ERR_PARSE;
      }
    }
  }
  /* Unreachable code.*/
  urosAssert(0 && "Unreachable code");
  return pp->err = UROS_ERR_PARSE;
}

/**
 * @brief   Expects the parser to be pointing to a token.
 *
 * @param[in,out] pp
 *          Pointer to an initialized @p UrosRpcParser object.
 * @param[in] tokp
 *          Pointer to the expected token string.
 * @param[in] toklen
 *          Length of the expected token.
 * @return
 *          Error code.
 * @retval UROS_ERR_PARSE
 *          The received token does not match.
 */
uros_err_t urosRpcParserExpect(UrosRpcParser *pp,
                               const char *tokp, size_t toklen) {

  const char *curp;
  size_t pending;

  urosAssert(pp != NULL);
  urosAssert(tokp != NULL);
  urosAssert(toklen > 0);

  curp = tokp;
  pending = toklen;
  while (UROS_TRUE) {
    if (pending <= pp->pending) {
      if (0 == memcmp(pp->curp, curp, pending)) {
        /* The prefix matches the token.*/
        pp->curp += pending;
        pp->pending -= pending;
        pp->total += pending;
        return pp->err = UROS_OK;
      } else {
        urosError(UROS_ERR_PARSE, return pp->err = UROS_ERR_PARSE,
                  ("Found [%.*s], expected [%.*s], stream offset %zu, "
                   "remote "UROS_ADDRFMT"\n",
                   (int)pending, pp->curp, (int)pending, curp,
                   pp->total, UROS_ADDRARG(&pp->csp->remaddr)));
      }
    } else {
      /* The token may fit only partially.*/
      if (pp->pending == 0 || 0 == memcmp(pp->curp, curp, pp->pending)) {
        /* Skip the matched prefix.*/
        pending -= pp->pending;
        curp += pp->pending;
        pp->total += pp->pending;

        /* Fetch the next chunk and check again.*/
        urosRpcParserRefill(pp);
        if (pp->err != UROS_OK) { return pp->err; }
      } else {
        urosError(UROS_ERR_PARSE, return pp->err = UROS_ERR_PARSE,
                  ("Found [%.*s], expected [%.*s], stream offset %zu, "
                   "remote "UROS_ADDRFMT"\n",
                   (int)pp->pending, pp->curp, (int)pp->pending, curp,
                   pp->total, UROS_ADDRARG(&pp->csp->remaddr)));
      }
    }
  }
  /* Unreachable code.*/
  urosAssert(0 && "Unreachable code");
  return pp->err = UROS_ERR_PARSE;
}

/**
 * @brief   Expects the parser to be pointing to a token.
 * @details Ignores the case of alphabetical characters.
 *
 * @param[in,out] pp
 *          Pointer to an initialized @p UrosRpcParser object.
 * @param[in] tokp
 *          Pointer to the expected token string.
 * @param[in] toklen
 *          Length of the expected token.
 * @return
 *          Error code.
 * @retval UROS_ERR_PARSE
 *          The received token does not match.
 */
uros_err_t urosRpcParserExpectNoCase(UrosRpcParser *pp,
                                     const char *tokp, size_t toklen) {

  const char *curp;
  size_t pending;

  urosAssert(pp != NULL);
  urosAssert(tokp != NULL);
  urosAssert(toklen > 0);

  curp = tokp;
  pending = toklen;
  while (UROS_TRUE) {
    if (pending <= pp->pending) {
      if (0 == uros_strnicmp(pp->curp, curp, pending)) {
        /* The prefix matches the token.*/
        pp->curp += pending;
        pp->pending -= pending;
        pp->total += pending;
        return pp->err = UROS_OK;
      } else {
        urosError(UROS_ERR_PARSE, return pp->err = UROS_ERR_PARSE,
                  ("Found [%.*s], expected [%.*s], stream offset %zu, "
                   "no case, remote "UROS_ADDRFMT"\n",
                   (int)pending, pp->curp, (int)pending, curp,
                   pp->total, UROS_ADDRARG(&pp->csp->remaddr)));
      }
    } else {
      /* The token may fit only partially.*/
      if (pp->pending == 0 || 0 == uros_strnicmp(pp->curp, curp, pp->pending)) {
        /* Skip the matched prefix.*/
        pending -= pp->pending;
        curp += pp->pending;
        pp->total += pp->pending;

        /* Fetch the next chunk and check again.*/
        urosRpcParserRefill(pp);
        if (pp->err != UROS_OK) { return pp->err; }
      } else {
        urosError(UROS_ERR_PARSE, return pp->err = UROS_ERR_PARSE,
                  ("Found [%.*s], expected [%.*s], stream offset %zu, "
                   "no case, remote "UROS_ADDRFMT"\n",
                   (int)pp->pending, pp->curp, (int)pp->pending, curp,
                   pp->total, UROS_ADDRARG(&pp->csp->remaddr)));
      }
    }
  }
  /* Unreachable code.*/
  urosAssert(0 && "Unreachable code");
  return pp->err = UROS_ERR_PARSE;
}

/**
 * @brief   Expects the parser to be pointing to a token.
 * @details Ignores the case of alphabetical characters.
 *          Does not print any parsing error messages.
 *
 * @param[in,out] pp
 *          Pointer to an initialized @p UrosRpcParser object.
 * @param[in] tokp
 *          Pointer to the expected token string.
 * @param[in] toklen
 *          Length of the expected token.
 * @return
 *          Error code.
 * @retval UROS_ERR_PARSE
 *          The received token does not match.
 */
uros_err_t urosRpcParserExpectNoCaseQuiet(UrosRpcParser *pp,
                                         const char *tokp, size_t toklen) {

  const char *curp;
  size_t pending;

  urosAssert(pp != NULL);
  urosAssert(tokp != NULL);
  urosAssert(toklen > 0);

  curp = tokp;
  pending = toklen;
  while (UROS_TRUE) {
    if (pending <= pp->pending) {
      if (0 == uros_strnicmp(pp->curp, curp, pending)) {
        /* The prefix matches the token.*/
        pp->curp += pending;
        pp->pending -= pending;
        pp->total += pending;
        return pp->err = UROS_OK;
      } else {
        return pp->err = UROS_ERR_PARSE;
      }
    } else {
      /* The token may fit only partially.*/
      if (pp->pending == 0 || 0 == uros_strnicmp(pp->curp, curp, pp->pending)) {
        /* Skip the matched prefix.*/
        pending -= pp->pending;
        curp += pp->pending;
        pp->total += pp->pending;

        /* Fetch the next chunk and check again.*/
        urosRpcParserRefill(pp);
        if (pp->err != UROS_OK) { return pp->err; }
      } else {
        return pp->err = UROS_ERR_PARSE;
      }
    }
  }
  /* Unreachable code.*/
  urosAssert(0 && "Unreachable code");
  return pp->err = UROS_ERR_PARSE;
}

/**
 * @brief   Expects the provided look-ahead character.
 *
 * @param[in] pp
 *          Pointer to an initialized @p UrosRpcParser object.
 * @param[in] c
 *          Expected look-ahead character.
 * @return
 *          Error code.
 * @retval UROS_ERR_PARSE
 *          The look-ahead character does not match.
 */
uros_err_t urosRpcParserLookAhead(UrosRpcParser *pp, char c) {

  urosAssert(pp != NULL);

  urosRpcParserLookAheadQuiet(pp, c);
  urosError(pp->err == UROS_ERR_PARSE, return UROS_ERR_PARSE,
            ("Look-ahead '%c', expected '%c', stream offset %zu, "
             "remote "UROS_ADDRFMT"\n",
             pp->curp[0], c, pp->total, UROS_ADDRARG(&pp->csp->remaddr)));
  return pp->err;
}

/**
 * @brief   Expects the provided look-ahead character.
 * @details Does not print any parsing error messages.
 *
 * @param[in] pp
 *          Pointer to an initialized @p UrosRpcParser object.
 * @param[in] c
 *          Expected look-ahead character.
 * @return
 *          Error code.
 * @retval UROS_ERR_PARSE
 *          The look-ahead character does not match.
 */
uros_err_t urosRpcParserLookAheadQuiet(UrosRpcParser *pp, char c) {

  urosAssert(pp != NULL);

  if (pp->pending == 0) {
    /* Fetch the next chunk.*/
    urosRpcParserRefill(pp);
    if (pp->err != UROS_OK) { return pp->err; }
  }
  return (pp->curp[0] == c) ? UROS_OK : UROS_ERR_PARSE;
}

/**
 * @brief   Fast-forwards to a character.
 * @details Advances the stream until the look-ahead character matches the
 *          reference one.
 *
 * @param[in,out] pp
 *          Pointer to an initialized @p UrosRpcParser object.
 * @param[in] c
 *          Reference character.
 * @return
 *          Error code.
 */
uros_err_t urosRpcParserSkipUntil(UrosRpcParser *pp, char c) {

  urosAssert(pp != NULL);

  /* Fast-forward to the first matching character found.*/
  while (urosRpcParserLookAheadQuiet(pp, c) == UROS_ERR_PARSE) {
    ++pp->total; ++pp->curp; --pp->pending;
  }
  return pp->err;
}

/**
 * @brief   Skips some characters.
 * @details Advances the stream for the provided number of characters.
 *
 * @param[in,out] pp
 *          Pointer to an initialized @p UrosRpcParser object.
 * @param[in] length
 *          Number of characters to skip.
 * @return
 *          Error code.
 */
uros_err_t urosRpcParserSkip(UrosRpcParser *pp, size_t length) {

  urosAssert(pp != NULL);

  while (UROS_TRUE) {
    if (length <= pp->pending) {
      pp->pending -= length;
      pp->curp += length;
      pp->total += length;
      return pp->err = UROS_OK;
    } else {
      length -= pp->pending;
      pp->total += pp->pending;
      urosRpcParserRefill(pp);
      if (pp->err != UROS_OK) { return pp->err; }
    }
  }
  /* Unreachable code.*/
  urosAssert(0 && "Unreachable code");
  return pp->err = UROS_ERR_PARSE;
}

/**
 * @brief   Expects the parser to find a token, and skips it.
 * @details Advances the stream until the expected token is found, and skips
 *          it.
 *
 * @param[in,out] pp
 *          Pointer to an initialized @p UrosRpcParser object.
 * @param[in] tokp
 *          Pointer to the expected token string.
 * @param[in] toklen
 *          Length of the expected token.
 * @return
 *          Error code.
 */
uros_err_t urosRpcParserSkipAfter(UrosRpcParser *pp,
                                  const char *tokp, size_t toklen) {

  const char *curp;
  size_t pending;

  urosAssert(pp != NULL);
  urosAssert(tokp != NULL);
  urosAssert(toklen > 0);

  curp = tokp;
  pending = toklen;
  while (UROS_TRUE) {
    /* Fast-forward to the first matching character of the token part.*/
    while (pp->pending > 0 && pp->curp[0] != curp[0]) {
      ++pp->total; ++pp->curp; --pp->pending;
    }
    if (pending <= pp->pending) {
      if (0 == memcmp(pp->curp, curp, pending)) {
        /* The prefix matches the token.*/
        pp->total += pending;
        pp->curp += pending;
        pp->pending -= pending;
        return pp->err = UROS_OK;
      } else {
        /* Token not matched, try again from the next character.*/
        ++pp->total; ++pp->curp; --pp->pending;
        continue;
      }
    }
    /* The token may fit only partially.*/
    if (pp->pending == 0 || 0 == memcmp(pp->curp, curp, pp->pending)) {
      /* Skip the matched token part.*/
      pp->total += pp->pending;
      curp += pp->pending;
      pending -= pp->pending;

      /* Fetch the next chunk and check again.*/
      urosRpcParserRefill(pp);
      urosError(pp->err != UROS_OK, return pp->err,
                ("Error %s while skipping token [%.*s], "
                 "remote "UROS_ADDRFMT"\n",
                 urosErrorText(pp->err), (int)toklen, tokp,
                 UROS_ADDRARG(&pp->csp->remaddr)));
    }
  }
  /* Unreachable code.*/
  urosAssert(0 && "Unreachable code");
  return pp->err = UROS_ERR_PARSE;
}

/**
 * @brief   Skips any whitespace.
 * @details Skips until a non-whitespace character is found.
 * @note    The whitespace characters are: <tt>' '</tt>, <tt>'\\r'</tt>,
 *          <tt>'\\n'</tt>, <tt>'\\t'</tt>, <tt>'\\v'</tt>.
 *
 * @param[in,out] pp
 *          Pointer to an initialized @p UrosRpcParser object.
 * @return
 *          Error code.
 */
uros_err_t urosRpcParserSkipWs(UrosRpcParser *pp) {

  urosAssert(pp != NULL);

  while (UROS_TRUE) {
    /* Skip all whitespace characters.*/
    while (pp->pending > 0 &&
           (pp->curp[0] == ' ' ||
            pp->curp[0] == '\r' ||
            pp->curp[0] == '\n' ||
            pp->curp[0] == '\t' ||
            pp->curp[0] == '\v')) {

      ++pp->total; ++pp->curp; --pp->pending;
    }
    if (pp->pending > 0) {
      return pp->err = UROS_OK;
    } else {
      /* Fetch the next chunk and check again.*/
      urosRpcParserRefill(pp);
      if (pp->err != UROS_OK) { return pp->err; }
    }
  }
  /* Unreachable code.*/
  urosAssert(0 && "Unreachable code");
  return pp->err = UROS_ERR_PARSE;
}

/**
 * @brief   Expects some whitespace.
 * @details The look-ahead character(s) must be whitespace. Skips until a
 *          non-whitespace character is found.
 * @note    The whitespace characters are: <tt>' '</tt>, <tt>'\\r'</tt>,
 *          <tt>'\\n'</tt>, <tt>'\\t'</tt>, <tt>'\\v'</tt>.
 *
 * @param[in,out] pp
 *          Pointer to an initialized @p UrosRpcParser object.
 * @return
 *          Error code.
 * @retval UROS_ERR_PARSE
 *          No whitespace as look-ahead.
 */
uros_err_t urosRpcParserExpectWs(UrosRpcParser *pp) {

  size_t oldcnt;

  urosAssert(pp != NULL);

  oldcnt = pp->total;
  urosRpcParserSkipWs(pp);
  urosError(pp->err != UROS_OK, return pp->err,
            ("Error %s while skipping whitespace, remote "UROS_ADDRFMT"\n",
             urosErrorText(pp->err), UROS_ADDRARG(&pp->csp->remaddr)));
  urosError(oldcnt == pp->total, return pp->err = UROS_ERR_PARSE,
            ("No whitespace found at stream offset %zu, remote "
             UROS_ADDRFMT"\n", pp->total, UROS_ADDRARG(&pp->csp->remaddr)));
  return pp->err = UROS_OK;
}

/**
 * @brief   Reads a decimal unsigned integer (@p uint32).
 * @warning Accumulates digits as long as they are found, without any overflow
 *          checks.
 *
 * @param[in,out] pp
 *          Pointer to an initialized @p UrosRpcParser object.
 * @param[out] valuep
 *          Pointer to the allocated output result value address.
 * @return
 *          Error code.
 */
uros_err_t urosRpcParserUint32(UrosRpcParser *pp, uint32_t *valuep) {

  uint32_t value = 0;

  urosAssert(pp != NULL);
  urosAssert(valuep != NULL);

  /* Ensure the parser is pointing to a number.*/
  urosError(pp->pending > 0 && (pp->curp[0] < '0' || pp->curp[0] > '9'),
            return pp->err = UROS_ERR_PARSE,
            ("Parser not pointing to a number, pending [%.*s], remote "
             UROS_ADDRFMT"\n", (int)pp->pending, pp->curp,
             UROS_ADDRARG(&pp->csp->remaddr)));

  while(UROS_TRUE) {
    /* Read the decimal unsigned integer.*/
    while (pp->pending > 0 && pp->curp[0] >= '0' && pp->curp[0] <= '9') {
      value = (value * 10) + (uint32_t)(pp->curp[0] - '0');
      ++pp->total; ++pp->curp; --pp->pending;
    }
    if (pp->pending > 0) {
      *valuep = value;
      return pp->err = UROS_OK;
    } else {
      /* Fetch the next chunk and check again.*/
      urosRpcParserRefill(pp);
      if (pp->err == UROS_ERR_EOF) {
        /* Parsing done, just notify that the stream has terminated.*/
        *valuep = value;
        return UROS_ERR_EOF;
      }
      if (pp->err != UROS_OK) { return pp->err; }
    }
  }
  /* Unreachable code.*/
  urosAssert(0 && "Unreachable code");
  return pp->err = UROS_ERR_PARSE;
}

/**
 * @brief   Reads a decimal signed integer (@p int32).
 * @details The sign symbol (<tt>'+'</tt> or <tt>'-'</tt>) can be placed before
 *          the sequence of digits.
 * @warning Accumulates digits as long as they are found, without any overflow
 *          checks.
 *
 * @param[in,out] pp
 *          Pointer to an initialized @p UrosRpcParser object.
 * @param[out] valuep
 *          Pointer to the allocated output result value address.
 * @return
 *          Error code.
 */
uros_err_t urosRpcParserInt32(UrosRpcParser *pp, int32_t *valuep) {

  uros_bool_t negative;

  urosAssert(pp != NULL);
  urosAssert(valuep != NULL);

  /* Read the sign.*/
  urosRpcParserExpectQuiet(pp, "-", 1);
  if (pp->err == UROS_OK) {
    negative = UROS_TRUE;
  } else if (pp->err == UROS_ERR_PARSE) {
    negative = UROS_FALSE;
    urosRpcParserExpectQuiet(pp, "+", 1);
    if (pp->err == UROS_ERR_PARSE) { pp->err = UROS_OK; }
  }
  urosError(pp->err != UROS_OK && pp->err != UROS_ERR_PARSE, return pp->err,
            ("Error %s while parsing the sign of a <int32_t>\n",
             urosErrorText(pp->err)));

  /* Read the value.*/
  urosRpcParserUint32(pp, (uint32_t*)valuep);
  if (negative) { *valuep = -*valuep; }
  return pp->err;
}

/**
 * @brief   Reads a decimal double-precision value (@p double).
 * @details The sign symbol (<tt>'+'</tt> or <tt>'-'</tt>) can be placed before
 *          the sequence of digits.
 * @warning Accumulates digits as long as they are found, without any overflow
 *          checks.
 * @warning It works well only for numbers close to the @p int32 range. This
 *          is not a fully-featured parser of floating-point numbers, at least
 *          not now.
 *
 * @param[in,out] pp
 *          Pointer to an initialized @p UrosRpcParser object.
 * @param[out] valuep
 *          Pointer to the allocated output result value address.
 * @return
 *          Error code.
 */
uros_err_t urosRpcParserDouble(UrosRpcParser *pp, double *valuep) {

  double value = 0.0;
  uros_bool_t negative;

  urosAssert(pp != NULL);
  urosAssert(valuep != NULL);

  /* Read the sign.*/
  urosRpcParserExpectQuiet(pp, "-", 1);
  if (pp->err == UROS_OK) {
    negative = UROS_TRUE;
  } else if (pp->err == UROS_ERR_PARSE) {
    negative = UROS_FALSE;
    urosRpcParserExpectQuiet(pp, "+", 1);
    if (pp->err == UROS_ERR_PARSE) { pp->err = UROS_OK; }
  }
  urosError(pp->err != UROS_OK && pp->err != UROS_ERR_PARSE, return pp->err,
            ("Error %s while parsing the sign of a <double>\n",
             urosErrorText(pp->err)));

  /* Ensure the parser is pointing to a number.*/
  urosError(pp->pending > 0 && (pp->curp[0] < '0' || pp->curp[0] > '9'),
            return pp->err = UROS_ERR_PARSE,
            ("Parser not pointing to a number, pending [%.*s]\n",
             (int)pp->pending, pp->curp));

  /* Read the integral part.*/
  while(UROS_TRUE) {
    while (pp->pending > 0 && pp->curp[0] >= '0' && pp->curp[0] <= '9') {
      value = (value * 10.0) + (double)(pp->curp[0] - '0');
      ++pp->total; ++pp->curp; --pp->pending;
    }
    if (pp->pending > 0) {
      break;
    } else {
      /* Fetch the next chunk and check again.*/
      urosRpcParserRefill(pp);
      if (pp->err != UROS_OK) { return pp->err; }
    }
  }

  /* Read the decimal part, if present.*/
  if (urosRpcParserExpect(pp, ".", 1) == UROS_OK) {
    double mult = 0.1;
    while(UROS_TRUE) {
      while (pp->pending > 0 && pp->curp[0] >= '0' && pp->curp[0] <= '9') {
        value += mult * (double)(pp->curp[0] - '0');
        mult *= 0.1;
        ++pp->total; ++pp->curp; --pp->pending;
      }
      if (pp->pending > 0) {
        break;
      } else {
        /* Fetch the next chunk and check again.*/
        urosRpcParserRefill(pp);
        if (pp->err == UROS_ERR_EOF) {
          if (negative) { value = -value; }
          *valuep = value;
          return UROS_ERR_EOF;
        } else if (pp->err != UROS_OK) {
          break;
        }
      }
    }
  }
  urosError(pp->err != UROS_OK, return pp->err,
            ("Error %s while expecting a decimal point\n",
             urosErrorText(pp->err)));

  /* Return the value.*/
  if (negative) { value = -value; }
  *valuep = value;
  return UROS_OK;
}

/**
 * @brief   Parses an incoming HTTP header section for an XMLRPC request.
 * @details Receives the HTTP header section of a POST request at the root,
 *          which is in the form:
 *          @verbatim POST /RPC2 HTTP/1.(0|1)@endverbatim
 *          The required headers are:
 *          - <tt>Content-Length</tt>, which is used by the parser to receive
 *            the correct number of pending characters,
 *          - <tt>Content-Type</tt>, which must be <tt>text/xml</tt>.
 *          Any other header is simply ignored.
 *
 * @param[in,out] pp
 *          Pointer to an initialized @p UrosRpcParser object.
 * @return
 *          Error code.
 */
uros_err_t urosRpcParserHttpRequest(UrosRpcParser *pp) {

  uros_bool_t isxml = UROS_FALSE;

  urosAssert(pp != NULL);
#define _CHKOK  { if (pp->err != UROS_OK) { return pp->err; } }

  /* Expect it to be a HTTP 1.(0|1) POST header at either / or /RPC2.*/
  urosRpcParserExpect(pp, "POST /", 6); _CHKOK
  urosRpcParserExpectQuiet(pp, "RPC2", 4);
  if (pp->err != UROS_ERR_PARSE && pp->err != UROS_OK) { return pp->err; }
  urosRpcParserExpect(pp, " HTTP/1.", 8); _CHKOK
  if (urosRpcParserExpectQuiet(pp, "1\r\n", 3) == UROS_ERR_PARSE) {
    urosRpcParserExpect(pp, "0\r\n", 3); _CHKOK
  }
  urosError(pp->err != UROS_OK, return pp->err,
            ("Error %s while expecting a valid HTTP POST line, remote "
             UROS_ADDRFMT"\n",
             urosErrorText(pp->err), UROS_ADDRARG(&pp->csp->remaddr)));

  /* Check the required headers.*/
  while (pp->err != UROS_ERR_EOF) {
    /* End of headers section (lone CR-LF).*/
    urosRpcParserExpectQuiet(pp, "\r\n", 2);
    if (pp->err == UROS_OK) {
      if (pp->contentLength < ~0u && isxml) {
        pp->mark = pp->total;
        return pp->err = UROS_OK;
      } else {
        /* Some required headers are missing.*/
        urosError(UROS_ERR_PARSE, UROS_NOP,
                  ("Missing some required HTTP headers, remote "
                   UROS_ADDRFMT"\n", UROS_ADDRARG(&pp->csp->remaddr)));
        return pp->err = UROS_ERR_PARSE;
      }
    }

    /* Content-* header family.*/
    urosRpcParserExpectNoCaseQuiet(pp, "Content-", 8);
    if (pp->err == UROS_OK) {
      /* Content-Length header */
      urosRpcParserExpectNoCaseQuiet(pp, "Length: ", 8);
      if (pp->err == UROS_OK) {
        uint32_t length;
        urosRpcParserUint32(pp, &length); _CHKOK
        pp->contentLength = (size_t)length;
        urosRpcParserExpect(pp, "\r\n", 2); _CHKOK
        continue;
      } else {
        /* Content-Type header.*/
        if (urosRpcParserExpectNoCaseQuiet(pp, "Type: ", 6) == UROS_OK) {
          if (urosRpcParserExpect(pp, "text/xml\r\n", 10) == UROS_OK) {
            isxml = UROS_TRUE;
            continue;
          }
        }
      }
    }

    /* Skip unhandled headers.*/
    urosRpcParserSkipAfter(pp, "\r\n", 2); _CHKOK
  }
  urosError(pp->err != UROS_OK, return pp->err,
            ("Error %s while scanning for HTTP headers at stream offset %zu, "
             "remote "UROS_ADDRFMT"\n", urosErrorText(pp->err), pp->total,
             UROS_ADDRARG(&pp->csp->remaddr)));

  return pp->err = UROS_OK;
#undef _CHKOK
}

/**
 * @brief   Parses an incoming HTTP header section for an XMLRPC response.
 * @details Receives the HTTP header section of an XMLRPC response.
 *          The required headers are:
 *          - <tt>Content-Length</tt>, which is used by the parser to receive
 *            the correct number of pending characters,
 *          - <tt>Content-Type</tt>, which must be <tt>text/xml</tt>.
 *          Any other header is simply ignored.
 *
 * @param[in,out] pp
 *          Pointer to an initialized @p UrosRpcParser object.
 * @param[out] codep
 *          Pointer to the allocated output HTTP status code address.
 * @return
 *          Error code.
 */
uros_err_t urosRpcParserHttpResponse(UrosRpcParser *pp, uint32_t *codep) {

  uros_bool_t isxml = UROS_FALSE;

  urosAssert(pp != NULL);
  urosAssert(codep != NULL);
#define _CHKOK  { if (pp->err != UROS_OK) { return pp->err; } }

  /* Expect it to be a HTTP 1.(0|1) header.*/
  urosRpcParserExpect(pp, "HTTP/1.", 7); _CHKOK
  if (urosRpcParserExpectQuiet(pp, "1 ", 2) == UROS_ERR_PARSE) {
    urosRpcParserExpect(pp, "0 ", 2); _CHKOK
  }
  urosError(pp->err != UROS_OK, return pp->err,
            ("Error %s while expecting a valid HTTP status line, remote "
             UROS_ADDRFMT"\n",
             urosErrorText(pp->err), UROS_ADDRARG(&pp->csp->remaddr)));
  urosRpcParserUint32(pp, codep); _CHKOK
  urosRpcParserSkipAfter(pp, "\r\n", 2); _CHKOK

  /* Check the required headers.*/
  while (pp->err != UROS_ERR_EOF) {
    /* End of headers section (lone CR-LF).*/
    urosRpcParserExpectQuiet(pp, "\r\n", 2);
    if (pp->err == UROS_OK) {
      if (pp->contentLength < ~0u && isxml) {
        pp->mark = pp->total;
        return pp->err = UROS_OK;
      } else {
        /* Some required headers are missing.*/
        urosError(UROS_ERR_PARSE, UROS_NOP,
                  ("Missing some required HTTP headers, remote "
                   UROS_ADDRFMT"\n", UROS_ADDRARG(&pp->csp->remaddr)));
        return pp->err = UROS_ERR_PARSE;
      }
    }

    /* Content-* header family.*/
    urosRpcParserExpectNoCaseQuiet(pp, "Content-", 8);
    if (pp->err == UROS_OK) {
      /* Content-Length header */
      urosRpcParserExpectNoCaseQuiet(pp, "Length: ", 8);
      if (pp->err == UROS_OK) {
        uint32_t length;
        urosRpcParserUint32(pp, &length); _CHKOK
        pp->contentLength = (size_t)length;
        urosRpcParserExpect(pp, "\r\n", 2); _CHKOK
        continue;
      } else {
        /* Content-Type header.*/
        if (urosRpcParserExpectNoCaseQuiet(pp, "Type: ", 6) == UROS_OK) {
          if (urosRpcParserExpect(pp, "text/xml\r\n", 10) == UROS_OK) {
            isxml = UROS_TRUE;
            continue;
          }
        }
      }
    }

    /* Skip unhandled headers.*/
    urosRpcParserSkipAfter(pp, "\r\n", 2); _CHKOK
  }
  urosError(pp->err != UROS_OK, return pp->err,
            ("Error %s while scanning for HTTP headers at stream offset %zu, "
             "remote "UROS_ADDRFMT"\n", urosErrorText(pp->err), pp->total,
             UROS_ADDRARG(&pp->csp->remaddr)));

  return pp->err = UROS_OK;
#undef _CHKOK
}

/**
 * @brief   Expects an XML attribute with a specific value.
 *
 * @param[in,out] pp
 *          Pointer to an initialized @p UrosRpcParser object.
 * @param[in] namep
 *          Pointer to the expected attribute name string.
 * @param[in] namelen
 *          Length of the expected attribute name.
 * @param[in] valp
 *          Pointer to the expected attribute value string.
 * @param[in] vallen
 *          Length of the expected attribute value length.
 * @return
 *          Error code.
 * @retval UROS_ERR_PARSE
 *          Attribute not found, or not matching.
 */
uros_err_t urosRpcParserXmlAttrWVal(UrosRpcParser *pp,
                                    const char *namep, size_t namelen,
                                    const char *valp, size_t vallen) {

  urosAssert(pp != NULL);
  urosAssert(namep != NULL);
  urosAssert(namelen > 0);
  urosAssert(valp != NULL);
  urosAssert(vallen > 0);
#define _CHKOK   { if (pp->err != UROS_OK) { return pp->err; } }

  /* Check the name and value of the attribute.*/
  urosRpcParserExpect(pp, namep, namelen); _CHKOK
  urosRpcParserSkipWs(pp); _CHKOK
  urosRpcParserExpect(pp, "=", 1); _CHKOK
  urosRpcParserSkipWs(pp); _CHKOK
  if (urosRpcParserExpectQuiet(pp, "\"", 1) == UROS_OK) {
    urosRpcParserExpect(pp, valp, vallen); _CHKOK
    urosRpcParserExpect(pp, "\"", 1); _CHKOK
    return pp->err = UROS_OK;
  } else if (pp->err == UROS_ERR_PARSE) {
    urosRpcParserExpect(pp, "'", 1); _CHKOK
    urosRpcParserExpect(pp, valp, vallen); _CHKOK
    urosRpcParserExpect(pp, "'", 1); _CHKOK
    return pp->err = UROS_OK;
  }
  urosError(pp->err != UROS_OK, return pp->err,
            ("Error %s while scanning for XML attribute, stream offset %zu, "
             "pending [%.*s], remote "UROS_ADDRFMT"\n",
             urosErrorText(pp->err), pp->total, (int)pp->pending, pp->curp,
             UROS_ADDRARG(&pp->csp->remaddr)));

  return pp->err = UROS_OK;
#undef _CHKOK
}

/**
 * @brief   Expects the beginning of an XML tag, without name.
 * @details Reads the beginning of a tag, which is simply in the form:
 *          @verbatim <@endverbatim
 *          Automatically skips any XML comments, and the whitespace after
 *          them.
 *
 * @param[in,out] pp
 *          Pointer to an initialized @p UrosRpcParser object.
 * @return
 *          Error code.
 * @retval UROS_ERR_PARSE
 *          No XML tag beginning found.
 */
uros_err_t urosRpcParserXmlTagBeginNoName(UrosRpcParser *pp) {

  urosAssert(pp != NULL);
#define _CHKOK   { if (pp->err != UROS_OK) { return pp->err; } }

  while (UROS_TRUE) {
    urosRpcParserExpect(pp, "<", 1); _CHKOK

    /* Ignore comments.*/
    urosRpcParserExpectQuiet(pp, "!--", 3);
    if (pp->err == UROS_ERR_PARSE) {
      /* Entity tag found.*/
      pp->err = UROS_OK;
      break;
    } else if (pp->err == UROS_OK) {
      urosRpcParserSkipAfter(pp, "-->", 3); _CHKOK
      urosRpcParserSkipWs(pp); _CHKOK
    }
    urosError(pp->err != UROS_OK, return pp->err,
              ("Error %s while scanning for '<', stream offset %zu, "
               "pending [%.*s], remote "UROS_ADDRFMT"\n",
               urosErrorText(pp->err), pp->total, (int)pp->pending, pp->curp,
               UROS_ADDRARG(&pp->csp->remaddr)));
  }
  return pp->err = UROS_OK;

#undef _CHKOK
}

/**
 * @brief   Expects the beginning of an XML tag with its name.
 * @details Reads the beginning of a tag, which is simply in the form:
 *          @verbatim <NAME@endverbatim
 *          Automatically skips any XML comments, and the whitespace after
 *          them.
 *
 * @param[in,out] pp
 *          Pointer to an initialized @p UrosRpcParser object.
 * @param[in] tagp
 *          Pointer to the expected tag name string.
 * @param[in] taglen
 *          Length of the expected tag name.
 * @return
 *          Error code.
 * @retval UROS_ERR_PARSE
 *          No XML tag beginning found.
 */
uros_err_t urosRpcParserXmlTagBegin(UrosRpcParser *pp,
                                    const char *tagp, size_t taglen) {

  urosAssert(pp != NULL);
  urosAssert(tagp != NULL);
  urosAssert(taglen > 0);
#define _CHKOK   { if (pp->err != UROS_OK) { return pp->err; } }

  urosRpcParserXmlTagBeginNoName(pp); _CHKOK
  urosRpcParserExpect(pp, tagp, taglen); _CHKOK
  return pp->err = UROS_OK;

#undef _CHKOK
}

/**
 * @brief   Expects the end of an XML tag.
 * @details Reads the beginning of a tag, which is simply in the form:
 *          @verbatim >@endverbatim
 *
 * @param[in,out] pp
 *          Pointer to an initialized @p UrosRpcParser object.
 * @return
 *          Error code.
 * @retval UROS_ERR_PARSE
 *          No XML tag end found.
 */
uros_err_t urosRpcParserXmlTagEnd(UrosRpcParser *pp) {

  urosAssert(pp != NULL);

  return urosRpcParserExpect(pp, ">", 1);
}

/**
 * @brief   Expects the end of an empty XML tag.
 * @details Reads the end of an empty tag, which is simply in the form:
 *          @verbatim />@endverbatim
 *
 * @param[in,out] pp
 *          Pointer to an initialized @p UrosRpcParser object.
 * @return
 *          Error code.
 * @retval UROS_ERR_PARSE
 *          No XML tag slash/end found.
 */
uros_err_t urosRpcParserXmlTagSlashEnd(UrosRpcParser *pp) {

  urosAssert (pp != NULL);

  return urosRpcParserExpect(pp, "/>", 2);
}

/**
 * @brief   Expects an opening XML tag.
 * @details Reads the opening tag, which is in the form:
 *          @verbatim <NAME >@endverbatim
 *
 * @param[in,out] pp
 *          Pointer to an initialized @p UrosRpcParser object.
 * @param[in] tagp
 *          Pointer to the expected tag name string.
 * @param[in] taglen
 *          Length of the expected tag name.
 * @return
 *          Error code.
 * @retval UROS_ERR_PARSE
 *          No XML tag slash/end found.
 */
uros_err_t urosRpcParserXmlTagOpen(UrosRpcParser *pp,
                                   const char *tagp, size_t taglen) {

  urosAssert(pp != NULL);
  urosAssert(tagp != NULL);
  urosAssert(taglen > 0);
#define _CHKOK   { if (pp->err != UROS_OK) { return pp->err; } }

  urosRpcParserXmlTagBeginNoName(pp); _CHKOK
  urosRpcParserExpect(pp, tagp, taglen); _CHKOK
  urosRpcParserSkipWs(pp); _CHKOK
  urosRpcParserExpect(pp, ">", 1); _CHKOK
  return pp->err = UROS_OK;

  #undef _CHKOK
}

/**
 * @brief   Expects a closing XML tag.
 * @details Reads the closing tag, which is in the form:
 *          @verbatim </NAME >@endverbatim
 *
 * @param[in,out] pp
 *          Pointer to an initialized @p UrosRpcParser object.
 * @param[in] tagp
 *          Pointer to the expected tag name string.
 * @param[in] taglen
 *          Length of the expected tag name.
 * @return
 *          Error code.
 * @retval UROS_ERR_PARSE
 *          No XML tag slash/end found.
 */
uros_err_t urosRpcParserXmlTagClose(UrosRpcParser *pp,
                                    const char *tagp, size_t taglen) {

  urosAssert(pp != NULL);
  urosAssert(tagp != NULL);
  urosAssert(taglen > 0);
#define _CHKOK   { if (pp->err != UROS_OK) { return pp->err; } }

  urosRpcParserExpect(pp, "</", 2); _CHKOK
  urosRpcParserExpect(pp, tagp, taglen); _CHKOK
  urosRpcParserSkipWs(pp); _CHKOK
  urosRpcParserExpect(pp, ">", 1); _CHKOK
  return pp->err = UROS_OK;

#undef _CHKOK
}

/**
 * @brief   Expects an XML header tag.
 * @details The XML header tag is in the (conceptual) form:
@verbatim
<?xml version="1.0"[ encoding="US-ASCII"[ standalone="no"]]?>
@endverbatim

 * @param[in,out] pp
 *          Pointer to an initialized @p UrosRpcParser object.
 * @return
 *          Error code.
 * @retval UROS_ERR_PARSE
 *          No XML header tag found.
 */
uros_err_t urosRpcParserXmlHeader(UrosRpcParser *pp) {

  urosAssert(pp != NULL);
#define _CHKOK   { if (pp->err != UROS_OK) { goto _error; } }

  urosRpcParserExpect(pp, "<?xml", 5); _CHKOK
  urosRpcParserExpectWs(pp); _CHKOK
  urosRpcParserXmlAttrWVal(pp, "version", 7, "1.0", 3); _CHKOK
  urosRpcParserExpectQuiet(pp, "?>", 2);
  if (pp->err == UROS_ERR_PARSE) {
    urosRpcParserExpectWs(pp); _CHKOK
    urosRpcParserExpectQuiet(pp, "?>", 2);
    if (pp->err == UROS_ERR_PARSE) {
      urosRpcParserXmlAttrWVal(pp, "encoding", 8, "US-ASCII", 8);
      urosRpcParserExpectQuiet(pp, "?>", 2);
      if (pp->err == UROS_ERR_PARSE) {
        urosRpcParserExpectWs(pp); _CHKOK
        urosRpcParserExpectQuiet(pp, "?>", 2);
        if (pp->err == UROS_ERR_PARSE) {
          urosRpcParserXmlAttrWVal(pp, "standalone", 10, "no", 2);
          urosRpcParserExpectQuiet(pp, "?>", 2);
          if (pp->err == UROS_ERR_PARSE) {
            urosRpcParserExpectWs(pp); _CHKOK
            urosRpcParserExpect(pp, "?>", 2); _CHKOK
          } else _CHKOK
        } else _CHKOK
      } else _CHKOK
    } else _CHKOK
  } else _CHKOK
  return pp->err = UROS_OK;

_error:
  urosError(pp->err != UROS_OK, UROS_NOP,
            ("Error %s while parsing an XML header, stream offset %zu, "
             "pending [%.*s], remote "UROS_ADDRFMT"\n",
             urosErrorText(pp->err), pp->total, (int)pp->pending, pp->curp,
             UROS_ADDRARG(&pp->csp->remaddr)));
  return pp->err;
#undef _CHKOK
}

/**
 * @brief   Parses an integral XMLRPC parameter value.
 * @see     urosRpcParserInt32()
 *
 * @param[in,out] pp
 *          Pointer to an initialized @p UrosRpcParser object.
 * @param[out] paramp
 *          Pointer to the allocated @p UrosRpcParam result.
 * @return
 *          Error code.
 */
uros_err_t urosRpcParserParamValueInt(UrosRpcParser *pp,
                                      UrosRpcParam *paramp) {

  urosAssert(pp != NULL);
  urosAssert(paramp != NULL);
#define _CHKOK   { if (pp->err != UROS_OK) { return pp->err; } }

  paramp->class = UROS_RPCP_INT;
  urosRpcParserInt32(pp, &paramp->value.int32); _CHKOK
  return pp->err = UROS_OK;

#undef _CHKOK
}

/**
 * @brief   Parses a boolean XMLRPC parameter value.
 * @details The boolean value is either @p 0 or @p 1.
 *
 * @param[in,out] pp
 *          Pointer to an initialized @p UrosRpcParser object.
 * @param[out] paramp
 *          Pointer to the allocated @p UrosRpcParam result.
 * @return
 *          Error code.
 */
uros_err_t urosRpcParserParamValueBoolean(UrosRpcParser *pp,
                                          UrosRpcParam *paramp) {

  urosAssert(pp != NULL);
  urosAssert(paramp != NULL);
#define _CHKOK   { if (pp->err != UROS_OK) { return pp->err; } }

  paramp->class = UROS_RPCP_BOOLEAN;
  urosRpcParserInt32(pp, &paramp->value.int32); _CHKOK
  if (paramp->value.int32 == 0) {
    paramp->value.boolean = UROS_FALSE;
  } else if (paramp->value.int32 == 1) {
    paramp->value.boolean = UROS_TRUE;
  } else {
    urosError(UROS_ERR_PARSE, UROS_NOP,
              ("%ld is not a valid XMLRPC <boolean> value\n",
               paramp->value.int32));
    return pp->err = UROS_ERR_PARSE;
  }
  return pp->err = UROS_OK;

#undef _CHKOK
}

/**
 * @brief   Parses a string XMLRPC parameter value.
 * @details An XMLRPC string ends where a look-ahead <tt>'<'</tt> is found.
 * @warning Do not write comments inside those @p string values without
 *          <tt>\<string\></tt> tags.
 *
 * @param[in,out] pp
 *          Pointer to an initialized @p UrosRpcParser object.
 * @param[out] paramp
 *          Pointer to the allocated @p UrosRpcParam result.
 * @return
 *          Error code.
 */
uros_err_t urosRpcParserParamValueString(UrosRpcParser *pp,
                                         UrosRpcParam *paramp) {

  char *strp = NULL;
  size_t strlen = 0;

  urosAssert(pp != NULL);
  urosAssert(paramp != NULL);

  paramp->class = UROS_RPCP_STRING;

  /* Do not mess with the memory if a tag (a comment too!) is already there.*/
  if (urosRpcParserLookAheadQuiet(pp, '<') == UROS_ERR_PARSE) {
    size_t freelen = pp->rdbuflen;
    char *curp = pp->rdbufp;

    /* Copy until a '<' is found.*/
    while (freelen > 0 && urosRpcParserLookAheadQuiet(pp, '<') == UROS_ERR_PARSE) {
      *curp++ = *pp->curp++;
      --freelen; ++strlen;
      ++pp->total; --pp->pending;
    }
    if (freelen == 0) {
      /* String longer than the parser buffer.*/
      urosError(UROS_ERR_PARSE, UROS_NOP,
                ("String longer than the parser buffer, remote "
                 UROS_ADDRFMT"\n", UROS_ADDRARG(&pp->csp->remaddr)));
      return UROS_ERR_PARSE;
    }

    /* Copy from the reading buffer.*/
    if (strlen > 0) {
      strp = (char*)urosAlloc(strlen);
      urosAssert(strp != NULL);
      memcpy(strp, pp->rdbufp, strlen);
    }
  }
  urosError(pp->err != UROS_OK, return pp->err,
            ("Error %s while scanning for a string value, remote "
             UROS_ADDRFMT"\n",
             urosErrorText(pp->err), UROS_ADDRARG(&pp->csp->remaddr)));

  paramp->value.string.length = strlen;
  paramp->value.string.datap = strp;
  return pp->err = UROS_OK;
}

/**
 * @brief   Parses a double-precision XMLRPC parameter value.
 * @see     urosRpcParserDouble()
 *
 * @param[in,out] pp
 *          Pointer to an initialized @p UrosRpcParser object.
 * @param[out] paramp
 *          Pointer to the allocated @p UrosRpcParam result.
 * @return
 *          Error code.
 */
uros_err_t urosRpcParserParamValueDouble(UrosRpcParser *pp,
                                         UrosRpcParam *paramp) {

  urosAssert(pp != NULL);
  urosAssert(paramp != NULL);
#define _CHKOK   { if (pp->err != UROS_OK) { return pp->err; } }

  paramp->class = UROS_RPCP_DOUBLE;
  urosRpcParserDouble(pp, &paramp->value.real); _CHKOK
  return pp->err = UROS_OK;

#undef _CHKOK
}

/**
 * @brief   Parses a @p base64 XMLRPC parameter value.
 * @warning Not implemented.
 *
 * @param[in,out] pp
 *          Pointer to an initialized @p UrosRpcParser object.
 * @param[out] paramp
 *          Pointer to the allocated @p UrosRpcParam result.
 * @return
 *          Error code.
 */
uros_err_t urosRpcParserParamValueBase64(UrosRpcParser *pp,
                                         UrosRpcParam *paramp) {

  urosAssert(pp != NULL);
  urosAssert(paramp != NULL);
#define _CHKOK   { if (pp->err != UROS_OK) { return pp->err; } }

  paramp->class = UROS_RPCP_BASE64;

  /* TODO: Parse the base64 stream and convert it into a binary form.*/
  urosRpcParserSkipUntil(pp, '<'); _CHKOK
  urosStringObjectInit(&paramp->value.base64);
  urosError(UROS_TRUE, UROS_NOP, ("XMLRPC base64 values not supported\n"));

  return pp->err = UROS_OK;

#undef _CHKOK
}

/**
 * @brief   Parses a struct (key-value map) XMLRPC parameter value.
 * @warning Not implemented.
 *
 * @param[in,out] pp
 *          Pointer to an initialized @p UrosRpcParser object.
 * @param[out] paramp
 *          Pointer to the allocated @p UrosRpcParam result.
 * @return
 *          Error code.
 */
uros_err_t urosRpcParserParamValueStruct(UrosRpcParser *pp,
                                         UrosRpcParam *paramp) {

  urosAssert(pp != NULL);
  urosAssert(paramp != NULL);
#define _CHKOK   { if (pp->err != UROS_OK) { return pp->err; } }

  paramp->class = UROS_RPCP_STRUCT;

  /* FIXME: Parse the XMLRPC struct and map it.*/

  urosRpcParserSkipWs(pp); _CHKOK
  return pp->err = UROS_OK;

#undef _CHKOK
}

/**
 * @brief   Parses an array XMLRPC parameter value.
 * @details The array valued (fields) are stored into a parameter with
 *          @p UrosRpcParamList nature.
 *
 * @param[in,out] pp
 *          Pointer to an initialized @p UrosRpcParser object.
 * @param[out] paramp
 *          Pointer to the allocated @p UrosRpcParam result.
 * @return
 *          Error code.
 */
uros_err_t urosRpcParserParamValueArray(UrosRpcParser *pp,
                                        UrosRpcParam *paramp) {

  urosAssert(pp != NULL);
  urosAssert(paramp != NULL);
#define _CHKOK   { if (pp->err != UROS_OK) { goto _error; } }

  paramp->class = UROS_RPCP_ARRAY;
  paramp->value.listp = urosNew(UrosRpcParamList);
  if (paramp->value.listp == NULL) { return pp->err = UROS_ERR_NOMEM; }
  urosRpcParamListObjectInit(paramp->value.listp);

  /* Parse the data array.*/
  urosRpcParserSkipWs(pp); _CHKOK
  urosRpcParserXmlTagOpen(pp, "data", 4); _CHKOK
  while (UROS_TRUE) {
    UrosRpcParamNode *nodep;
    nodep = urosNew(UrosRpcParamNode);
    if (nodep == NULL) { pp->err = UROS_ERR_NOMEM; goto _error; }
    urosRpcParamNodeObjectInit(nodep, UROS_RPCP__LENGTH);
    urosRpcParamListAppendNode(paramp->value.listp, nodep);
    urosRpcParserSkipWs(pp); _CHKOK
    if (urosRpcParserParamByTagQuiet(pp, &nodep->param) == UROS_OK) {
      /* A parameter was found, reiterate for more (if any).*/
      continue;
    } else if (pp->err == UROS_ERR_PARSE) {
      /* No more nodes found.*/
      urosRpcParamListUnlinkNode(paramp->value.listp, nodep);
      urosRpcParamNodeDelete(nodep, UROS_TRUE);
      if (urosRpcParserExpectQuiet(pp, "/data>", 6) == UROS_ERR_PARSE) {
        urosRpcParserXmlTagClose(pp, "data", 4); _CHKOK
      }
      break;
    }
    urosError(pp->err != UROS_OK, goto _error,
              ("Error %s while scanning for an array value, "
               "stream offset %zu, pending [%.*s], remote "UROS_ADDRFMT"\n",
               urosErrorText(pp->err), pp->total, (int)pp->pending, pp->curp,
               UROS_ADDRARG(&pp->csp->remaddr)));
  }
  return pp->err = UROS_OK;

_error:
  urosRpcParamListDelete(paramp->value.listp, UROS_TRUE);
  paramp->value.listp = NULL;
  return pp->err;
#undef _CHKOK
}

/**
 * @brief   Parses a XMLRPC parameter value by its element tags.
 * @details The XMLRPC parameter class is decoded by reading its opening XML
 *          tag.
 * @note    The parameter value is expected to be enclosed within a
 *          @p value XML element.
 *
 * @param[in,out] pp
 *          Pointer to an initialized @p UrosRpcParser object.
 * @param[out] paramp
 *          Pointer to the allocated @p UrosRpcParam result.
 * @return
 *          Error code.
 */
uros_err_t urosRpcParserParamByTag(UrosRpcParser *pp,
                                   UrosRpcParam *paramp) {

  urosAssert(pp != NULL);
  urosAssert(paramp != NULL);

  urosRpcParserXmlTagOpen(pp, "value", 5);
  if (pp->err != UROS_OK) { return pp->err; };
  return uros_rpcparser_parambytag_partial(pp, paramp);
}

/**
 * @brief   Parses a XMLRPC parameter value by its element tags.
 * @details The XMLRPC parameter class is decoded by reading its opening XML
 *          tag. Does not print any parsing error messages.
 * @note    The parameter value is expected to be enclosed within a
 *          @p value XML element.
 *
 * @param[in,out] pp
 *          Pointer to an initialized @p UrosRpcParser object.
 * @param[out] paramp
 *          Pointer to the allocated @p UrosRpcParam result.
 * @return
 *          Error code.
 */
uros_err_t urosRpcParserParamByTagQuiet(UrosRpcParser *pp,
                                       UrosRpcParam *paramp) {

  urosAssert(pp != NULL);
  urosAssert(paramp != NULL);

  urosRpcParserExpectQuiet(pp, "<value>", 7);
  if (pp->err != UROS_OK) { return pp->err; };
  return uros_rpcparser_parambytag_partial(pp, paramp);
}

/**
 * @brief   Parses a XMLRPC parameter value by its class.
 * @details The XMLRPC parameter class is already written inside the alloacted
 *          result value.
 * @note    The parameter value is expected to be enclosed within a
 *          @p value XML element.
 *
 * @param[in,out] pp
 *          Pointer to an initialized @p UrosRpcParser object.
 * @param[out] paramp
 *          Pointer to the allocated @p UrosRpcParam result.
 * @return
 *          Error code.
 */
uros_err_t urosRpcParserParamByClass(UrosRpcParser *pp,
                                     UrosRpcParam *paramp) {

  urosAssert(pp != NULL);
  urosAssert(paramp != NULL);

  return urosRpcParserParam(pp, paramp, paramp->class);
}

/**
 * @brief   Parses a XMLRPC parameter value.
 * @note    The parameter value is expected to be enclosed within a
 *          @p value XML element.
 *
 * @param[in,out] pp
 *          Pointer to an initialized @p UrosRpcParser object.
 * @param[out] paramp
 *          Pointer to the allocated @p UrosRpcParam result.
 * @param[in] paramclass
 *          Expected parameter class.
 * @return
 *          Error code.
 */
uros_err_t urosRpcParserParam(UrosRpcParser *pp,
                              UrosRpcParam *paramp,
                              uros_rpcparamclass_t paramclass) {

  urosAssert(pp != NULL);
  urosAssert(paramp != NULL);
#define _CHKOK   { if (pp->err != UROS_OK) { return pp->err; } }

  urosRpcParserXmlTagOpen(pp, "value", 5); _CHKOK

  switch (paramclass) {
  case UROS_RPCP_INT: {
    const char *tagp;
    size_t taglen;
    urosRpcParserSkipWs(pp); _CHKOK
    urosRpcParserXmlTagBegin(pp, "i", 1); _CHKOK
    if (urosRpcParserExpectQuiet(pp, "4>", 2) == UROS_OK) {
      tagp = "i4"; taglen = 2;
    } else if (urosRpcParserExpect(pp, "nt>", 3) == UROS_OK) {
      tagp = "int"; taglen = 3;
    } else { return pp->err; }
    urosRpcParserParamValueInt(pp, paramp); _CHKOK
    urosRpcParserXmlTagClose(pp, tagp, taglen); _CHKOK
    urosRpcParserSkipWs(pp); _CHKOK
    break;
  }
  case UROS_RPCP_BOOLEAN: {
    urosRpcParserSkipWs(pp); _CHKOK
    urosRpcParserXmlTagOpen(pp, "boolean", 7); _CHKOK
    urosRpcParserParamValueBoolean(pp, paramp); _CHKOK
    urosRpcParserXmlTagClose(pp, "boolean", 7); _CHKOK
    urosRpcParserSkipWs(pp); _CHKOK
    break;
  }
  case UROS_RPCP_STRING: {
    /* Do not mess with the memory if a tag (a comment too!) is there.*/
    if (urosRpcParserLookAheadQuiet(pp, '<') == UROS_ERR_PARSE) {
      urosRpcParserParamValueString(pp, paramp); _CHKOK
      urosRpcParserXmlTagBeginNoName(pp); _CHKOK
      if (urosRpcParserExpectQuiet(pp, "string>", 7) == UROS_OK) {
        urosFree(paramp->value.string.datap);
        urosRpcParserParamValueString(pp, paramp); _CHKOK
        urosRpcParserXmlTagClose(pp, "string", 6); _CHKOK
        urosRpcParserSkipWs(pp); _CHKOK
      } else {
        urosRpcParserExpect(pp, "/value>", 7); _CHKOK
      }
    } else {
      urosRpcParserXmlTagOpen(pp, "string", 6); _CHKOK
      urosRpcParserParamValueString(pp, paramp); _CHKOK
      urosRpcParserXmlTagClose(pp, "string", 6); _CHKOK
      urosRpcParserSkipWs(pp); _CHKOK
      urosRpcParserXmlTagClose(pp, "value", 5); _CHKOK
    }
    return pp->err = UROS_OK;
  }
  case UROS_RPCP_DOUBLE: {
    urosRpcParserSkipWs(pp); _CHKOK
    urosRpcParserXmlTagOpen(pp, "double", 6); _CHKOK
    urosRpcParserParamValueDouble(pp, paramp); _CHKOK
    urosRpcParserXmlTagClose(pp, "double", 6); _CHKOK
    urosRpcParserSkipWs(pp); _CHKOK
    break;
  }
  case UROS_RPCP_BASE64: {
    urosRpcParserSkipWs(pp); _CHKOK
    urosRpcParserXmlTagOpen(pp, "base64", 6); _CHKOK
    urosRpcParserParamValueBase64(pp, paramp); _CHKOK
    urosRpcParserXmlTagClose(pp, "base64", 6); _CHKOK
    urosRpcParserSkipWs(pp); _CHKOK
    break;
  }
  case UROS_RPCP_STRUCT: {
    urosRpcParserSkipWs(pp); _CHKOK
    urosRpcParserXmlTagOpen(pp, "struct", 6); _CHKOK
    urosRpcParserParamValueStruct(pp, paramp); _CHKOK
    /* TODO: Add support for structs.*/
    urosRpcParserSkipAfter(pp, "</struct>", 9); _CHKOK
    /*urosRpcParserXmlTagClose(pp, "struct", 6); _CHKOK*/
    urosRpcParserSkipWs(pp); _CHKOK
    break;
  }
  case UROS_RPCP_ARRAY: {
    urosRpcParserSkipWs(pp); _CHKOK
    urosRpcParserXmlTagOpen(pp, "array", 5); _CHKOK
    urosRpcParserParamValueArray(pp, paramp); _CHKOK
    urosRpcParserXmlTagClose(pp, "array", 5); _CHKOK
    urosRpcParserSkipWs(pp); _CHKOK
    break;
  }
  default: {
    urosError(UROS_ERR_BADPARAM, UROS_NOP,
              ("Unknown parameter class id %d\n, remote "UROS_ADDRFMT"\n",
               (int)paramclass, UROS_ADDRARG(&pp->csp->remaddr)));
    return pp->err = UROS_ERR_BADPARAM;
  }
  }

  urosRpcParserXmlTagClose(pp, "value", 5); _CHKOK
  return pp->err = UROS_OK;
#undef _CHKOK
}

/**
 * @brief   Parses an XMLRPC method response.
 * @details The expected response starts at the declaration of the XML header.
 *          The method response is parsed, and its decoded values are stored
 *          into a pre-allocated response descriptor.
 *
 * @param[in,out] pp
 *          Pointer to an initialized @p UrosRpcParser object.
 * @param[out] resp
 *          Pointer to an allocated @p UrosRpcResponse response descriptor.
 * @return
 *          Error code.
 */
uros_err_t urosRpcParserMethodResponse(UrosRpcParser *pp,
                                       UrosRpcResponse *resp) {

  urosAssert(pp != NULL);
  urosAssert(resp != NULL);
#define _CHKOK  { if (pp->err != UROS_OK) { return pp->err; } }

  urosRpcResponseObjectInit(resp);
  resp->valuep = urosNew(UrosRpcParam);
  if (resp->valuep == NULL) { return pp->err = UROS_ERR_NOMEM; }

  /* Check the XML header.*/
  urosRpcParserXmlHeader(pp); _CHKOK

  /* Read the response value.*/
  urosRpcParserSkipWs(pp); _CHKOK
  urosRpcParserXmlTagOpen(pp, "methodResponse", 14); _CHKOK
  urosRpcParserSkipWs(pp); _CHKOK
  urosRpcParserXmlTagOpen(pp, "params", 6); _CHKOK
  urosRpcParserSkipWs(pp); _CHKOK
  urosRpcParserXmlTagOpen(pp, "param", 5); _CHKOK
  urosRpcParserSkipWs(pp); _CHKOK
  urosRpcParserXmlTagOpen(pp, "value", 5); _CHKOK
  urosRpcParserSkipWs(pp); _CHKOK
  urosRpcParserXmlTagOpen(pp, "array", 5); _CHKOK
  urosRpcParserSkipWs(pp); _CHKOK
  urosRpcParserXmlTagOpen(pp, "data", 4); _CHKOK
  urosRpcParserSkipWs(pp); _CHKOK

  /* int code */
  urosRpcParamObjectInit(resp->valuep, UROS_RPCP_INT);
  urosRpcParserParam(pp, resp->valuep, UROS_RPCP_INT); _CHKOK
  resp->code = resp->valuep->value.int32;
  urosRpcParserSkipWs(pp); _CHKOK

  /* str statusMessage */
#if UROS_RPCPARSER_USE_STATUS_MSG
  urosRpcParamObjectInit(resp->valuep, UROS_RPCP_STRING);
  urosRpcParserParam(pp, resp->valuep, UROS_RPCP_STRING); _CHKOK
#else
  urosRpcParserXmlTagOpen(pp, "value", 5); _CHKOK
  urosRpcParserSkipUntil(pp, '<'); _CHKOK
  urosRpcParserExpect(pp, "<", 1); _CHKOK
  if (urosRpcParserExpectQuiet(pp, "string>", 7) == UROS_OK) {
    urosRpcParserSkipUntil(pp, '<'); _CHKOK
    urosRpcParserXmlTagClose(pp, "string", 6); _CHKOK
    urosRpcParserSkipWs(pp); _CHKOK
    urosRpcParserXmlTagClose(pp, "value", 5); _CHKOK
  } else if (pp->err == UROS_ERR_PARSE) {
    urosRpcParserExpect(pp, "/value>", 7); _CHKOK
  } else { return pp->err; }
#endif
  urosRpcParserSkipWs(pp); _CHKOK

  /* XMLRPCLegalValue value */
  urosRpcParamObjectInit(resp->valuep, UROS_RPCP__LENGTH);
  urosRpcParserParamByTag(pp, resp->valuep); _CHKOK

  urosRpcParserSkipWs(pp); _CHKOK
  urosRpcParserXmlTagClose(pp, "data", 4); _CHKOK
  urosRpcParserSkipWs(pp); _CHKOK
  urosRpcParserXmlTagClose(pp, "array", 5); _CHKOK
  urosRpcParserSkipWs(pp); _CHKOK
  urosRpcParserXmlTagClose(pp, "value", 5); _CHKOK
  urosRpcParserSkipWs(pp); _CHKOK
  urosRpcParserXmlTagClose(pp, "param", 5); _CHKOK
  urosRpcParserSkipWs(pp); _CHKOK
  urosRpcParserXmlTagClose(pp, "params", 6); _CHKOK
  urosRpcParserSkipWs(pp); _CHKOK
  urosRpcParserXmlTagClose(pp, "methodResponse", 14); _CHKOK
  urosRpcParserSkip(pp, pp->contentLength - (pp->total - pp->mark)); _CHKOK

  return pp->err = UROS_OK;
#undef _CHKOK
}

/** @} */
