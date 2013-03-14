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
 * @file    urosRpcStreamer.c
 * @author  Andrea Zoppi <texzk@email.it>
 *
 * @brief   XMLRPC streamer functions.
 */

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include "../include/urosBase.h"
#include "../include/urosUser.h"
#include "../include/urosRpcStreamer.h"

#include <string.h>

/*===========================================================================*/
/* LOCAL TYPES & MACROS                                                      */
/*===========================================================================*/

#if UROS_RPCSTREAMER_C_USE_ASSERT == UROS_FALSE && !defined(__DOXYGEN__)
#undef urosAssert
#define urosAssert(expr)
#endif

#if UROS_RPCSTREAMER_C_USE_ERROR_MSG == UROS_FALSE && !defined(__DOXYGEN__)
#undef urosError
#define urosError(when, action, msgargs) { if (when) { action; } }
#endif

/*===========================================================================*/
/* GLOBAL FUNCTIONS                                                          */
/*===========================================================================*/

/** @addtogroup rpc_streamer_funcs */
/** @{ */

/**
 * @brief   Initializes a XMLRPC streamer.
 * @details All the fields are initialized. A connection and a buffer are
 *          assigned to this object.
 *
 * @pre     The streamer is uninitialized.
 *
 * @param[in,out] sp
 *          Pointer to an uninitialized @p UrosRpcStreamer object.
 * @param[in] csp
 *          Pointer to an allocated @p UrosConn connection object.
 * @param[in] bufp
 *          Pointer to the write buffer.
 * @param[in] buflen
 *          Length of the write buffer, in bytes.
 */
void urosRpcStreamerObjectInit(UrosRpcStreamer *sp,
                               UrosConn *csp,
                               char *bufp, size_t buflen) {

  urosAssert(sp != NULL);
  urosAssert(csp != NULL);
  urosAssert(bufp != NULL);
  urosAssert(buflen > 0);

  /* Initialize fields.*/
  sp->csp = csp;
  sp->err = UROS_OK;
  sp->bufp = bufp,
  sp->buflen = buflen;
  sp->curp = bufp;
  sp->free = buflen;
  sp->total = 0;
  sp->mark = 0;
}

/**
 * @brief   Cleans a @p UrosRpcStreamer object.
 * @details Resets the state of its internal fields. If needed, the buffer is
 *          deallocated. Does not unlink from the assigned connection.
 *
 * @param[in,out] sp
 *          Pointer to an initialized @p UrosRpcStreamer object.
 * @param[in] freeBuffer
 *          Deallocates buffers.
 */
void urosRpcStreamerClean(UrosRpcStreamer *sp, uros_bool_t freeBuffer) {

  urosAssert(sp != NULL);

  if (freeBuffer) {
    /* Free the buffer.*/
    urosFree(sp->bufp);
    sp->bufp = NULL;
    sp->buflen = 0;
  }

  /* Initialize fields.*/
  sp->err = UROS_OK;
  sp->curp = sp->bufp;
  sp->free = sp->buflen;
  sp->total = 0;
  sp->mark = 0;
}

/**
 * @brief   Flushes the write buffer.
 * @details Waits untill all the data of the write buffer is streamed.
 *
 * @param[in,out] sp
 *          Pointer to an initialzied @p UrosRpcStreamer object.
 * @return
 *          Error code.
 */
uros_err_t urosRpcStreamerFlush(UrosRpcStreamer *sp) {

  urosAssert(sp != NULL);

  if (sp->free == sp->buflen) {
    urosAssert(sp->curp == sp->bufp);
    return sp->err = UROS_OK;
  }

  /* Send buffered data.*/
  sp->err = urosConnSend(sp->csp, sp->bufp, sp->buflen - sp->free);
  urosError(sp->err != UROS_OK, return sp->err,
            ("Error %s while streaming data chunk [%.*s], remote "
             UROS_ADDRFMT"\n",
             urosErrorText(sp->err), (int)sp->buflen, sp->bufp,
             UROS_ADDRARG(&sp->csp->remaddr)));
  sp->free = sp->buflen;
  sp->curp = sp->bufp;
  return sp->err = UROS_OK;
}

/**
 * @brief   Writes some data.
 * @details The data chunk is written to the write buffer. Every time it
 *          becomes full, buffered data is flushed. The write counter is
 *          incremented by the chunk length.
 *
 * @param[in,out] sp
 *          Pointer to an initialized @p UrosRpcStreamer object.
 * @param[in] chunkp
 *          Pointer to the data chunk to be sent. Can be @p NULL only if
 *          @p chunklen is @p 0.
 * @param[in] chunklen
 *          Length of the data chunk, in bytes. Can be @p 0.
 * @return
 *          Error code.
 */
uros_err_t urosRpcStreamerWrite(UrosRpcStreamer *sp,
                                const void *chunkp, size_t chunklen) {

  urosAssert(sp != NULL);
  urosAssert(!(chunklen > 0) || (chunkp != NULL));

  if (chunklen == 0) {
    return sp->err = UROS_OK;
  }
  while (UROS_TRUE) {
    if (chunklen <= sp->free) {
      /* Chunk completely fits inside the buffer.*/
      memcpy(sp->curp, chunkp, chunklen);
      sp->total += chunklen;
      sp->curp += chunklen;
      sp->free -= chunklen;
      return sp->err = UROS_OK;
    } else {
      /* Chunk only partially fits inside the buffer.*/
      memcpy(sp->curp, chunkp, sp->free);
      sp->total += sp->free;
      chunkp = (void*)((uint8_t*)chunkp + sp->free);
      chunklen -= sp->free;
      sp->free = 0;

      /* Flush the buffered data and continue.*/
      urosRpcStreamerFlush(sp);
      urosError(sp->err != UROS_OK, return sp->err,
                ("Error %s while flushing chunk [%.*s], remote "
                 UROS_ADDRFMT"\n",
                 urosErrorText(sp->err),
                 (int)(sp->buflen - sp->free), sp->bufp,
                 UROS_ADDRARG(&sp->csp->remaddr)));
    }
  }
  /* Unreachable code.*/
  urosAssert(0);
  return sp->err = UROS_ERR_BADPARAM;
}

/**
 * @brief   Writes a decimal unsigned integer (@p uint32).
 * @details Decimal format without leading zeroes.
 *
 * @param[in,out] sp
 *          Pointer to an initialized @p UrosRpcStreamer object.
 * @param[in] value
 *          Value to be written.
 * @return
 *          Error code.
 */
uros_err_t urosRpcStreamerUint32(UrosRpcStreamer *sp, uint32_t value) {

  char *ptr, strbuf[10];

  urosAssert(sp != NULL);

  /* Convert to string.*/
  ptr = &strbuf[10];
  do {
    *--ptr = (char)(value % 10) + '0';
    value /= 10;
  } while (value != 0);

  /* Stream the value.*/
  return urosRpcStreamerWrite(sp, ptr,
                              (ptrdiff_t)strbuf - (ptrdiff_t)ptr + 10);
}

/**
 * @brief   Writes a decimal signed integer (@p int32).
 * @details Decimal format without leading zeroes. Sign symbol only for
 *          negative values (<tt>'-'</tt>).
 *
 * @param[in,out] sp
 *          Pointer to an initialized @p UrosRpcStreamer object.
 * @param[in] value
 *          Value to be written.
 * @return
 *          Error code.
 */
uros_err_t urosRpcStreamerInt32(UrosRpcStreamer *sp, int32_t value) {

  urosAssert(sp != NULL);

  if (value < 0) {
    value = -value;
    urosRpcStreamerWrite(sp, "-", 1);
    if (sp->err != UROS_OK) { return sp->err; }
  }
  return urosRpcStreamerUint32(sp, (uint32_t)value);
}

/**
 * @brief   Writes an IPv4 address.
 * @details Using the standard format, for example:
 *          @verbatim 192.168.1.1@endverbatim
 *
 * @param[in,out] sp
 *          Pointer to an initialized @p UrosRpcStreamer object.
 * @param[in] ip
 *          IPv4 address to write.
 * @return
 *          Error code.
 */
uros_err_t urosRpcStreamerIp(UrosRpcStreamer *sp, UrosIp ip) {

  urosAssert(sp != NULL);
#define _CHKOK  { if (sp->err != UROS_OK) { return sp->err; } }

  urosRpcStreamerUint32(sp, (uint32_t)ip.fields.field1); _CHKOK
  urosRpcStreamerWrite(sp, ".", 1); _CHKOK
  urosRpcStreamerUint32(sp, (uint32_t)ip.fields.field2); _CHKOK
  urosRpcStreamerWrite(sp, ".", 1); _CHKOK
  urosRpcStreamerUint32(sp, (uint32_t)ip.fields.field3); _CHKOK
  urosRpcStreamerWrite(sp, ".", 1); _CHKOK
  urosRpcStreamerUint32(sp, (uint32_t)ip.fields.field4); _CHKOK

  return sp->err = UROS_OK;
#undef _CHKOK
}

/**
 * @brief   Writes an HTTP POST line.
 * @details Using HTTP 1.0. Automatically appends CR-LF.
 *
 * @param[in,out] sp
 *          Pointer to an initialized @p UrosRpcStreamer object.
 * @return
 *          Error code.
 */
uros_err_t urosRpcStreamerHttpPost(UrosRpcStreamer *sp) {

  return urosRpcStreamerWrite(sp, "POST /RPC2 HTTP/1.0\r\n", 21);
}

/**
 * @brief   Writes an HTTP status line.
 * @details Using HTTP 1.0. Writes also the human-readable description.
 *          Automatically appends CR-LF.
 *
 * @param[in,out] sp
 *          Pointer to an initialized @p UrosRpcStreamer object.
 * @param[in] code
 *          Code of the HTTP status. Currently supporting: 200, 201, 202, 204,
 *          400, 401, 403, 404, 500, 501, 502, 503.
 * @return
 *          Error code.
 */
uros_err_t urosRpcStreamerHttpStatus(UrosRpcStreamer *sp, uint32_t code) {

  const char *txtp;

  urosAssert(sp != NULL);
#define _CHKOK   { if (sp->err != UROS_OK) { return sp->err; } }

  /* Stream the HTTP response line.*/
  urosRpcStreamerWrite(sp, "HTTP/1.0 ", 9); _CHKOK
  urosRpcStreamerUint32(sp, code); _CHKOK
  urosRpcStreamerWrite(sp, " ", 1); _CHKOK
  switch (code) {
  case 200: { txtp = "OK"; break; }
  case 201: { txtp = "Created"; break; }
  case 202: { txtp = "Accepted"; break; }
  case 204: { txtp = "No Content"; break; }
  case 400: { txtp = "Bad Request"; break; }
  case 401: { txtp = "Unauthorized"; break; }
  case 403: { txtp = "Forbidden"; break; }
  case 404: { txtp = "Not Found"; break; }
  case 500: { txtp = "Internal Server Error"; break; }
  case 501: { txtp = "Not Implemented"; break; }
  case 502: { txtp = "Bad Gateway"; break; }
  case 503: { txtp = "Service Unavailable"; break; }
  default: {
    urosError(UROS_ERR_BADPARAM, UROS_NOP,
              ("Unhandled HTTP code %lu", (long unsigned int)code));
    return sp->err = UROS_ERR_BADPARAM;
  }
  }
  urosRpcStreamerWrite(sp, txtp, strlen(txtp)); _CHKOK;
  urosRpcStreamerWrite(sp, "\r\n", 2); _CHKOK

  return sp->err = UROS_OK;
#undef _CHKOK
}

/**
 * @brief   Writes an HTTP header.
 * @details Automatically appends CR-LF.
 *
 * @param[in,out] sp
 *          Pointer to an initialized @p UrosRpcStreamer object.
 * @param[in] keyp
 *          Pointer to the key string.
 * @param[in] keylen
 *          Length of the key string, in bytes.
 * @param[in] valp
 *          Pointer to the value string.
 * @param[in] vallen
 *          Length of the value string, in bytes.
 * @return
 *          Error code.
 */
uros_err_t urosRpcStreamerHttpHeader(UrosRpcStreamer *sp,
                                     const char *keyp, size_t keylen,
                                     const char *valp, size_t vallen) {

  urosAssert(sp != NULL);
  urosAssert(keyp != NULL);
  urosAssert(keylen > 0);
  urosAssert(valp != NULL);
#define _CHKOK   { if (sp->err != UROS_OK) { return sp->err; } }

  urosRpcStreamerWrite(sp, keyp, keylen); _CHKOK
  urosRpcStreamerWrite(sp, ": ", 2); _CHKOK
  urosRpcStreamerWrite(sp, valp, vallen); _CHKOK;
  urosRpcStreamerWrite(sp, "\r\n", 2); _CHKOK

  return sp->err = UROS_OK;
#undef _CHKOK
}

/**
 * @brief   Writes the end of HTTP headers section.
 * @details Simply writes a CR-LF.
 *
 * @param[in,out] sp
 *          Pointer to an initialized @p UrosRpcStreamer object.
 * @return
 *          Error code.
 */
uros_err_t urosRpcStreamerHttpEnd(UrosRpcStreamer *sp) {

  return urosRpcStreamerWrite(sp, "\r\n", 2);
}

/**
 * @brief   Writes a hacked <tt>Content-Length</tt> HTTP header.
 * @details Writes the <tt>Content-Length</tt> header, with the value of
 *          @p UROS_RPCSTREAMER_FIXLEN.
 * @see     UROS_RPCSTREAMER_FIXLEN
 * @see     urosRpcStreamerXmlEndHack()
 *
 * @pre     The <tt>Content-Length</tt> HTTP header is the last header.
 *
 * @param[in,out] sp
 *          Pointer to an initialized @p UrosRpcStreamer object.
 * @return
 *          Error code.
 */
uros_err_t urosRpcStreamerHttpContentLength(UrosRpcStreamer *sp) {

  urosAssert(sp != NULL);
#define _CHKOK   { if (sp->err != UROS_OK) { return sp->err; } }

  /* Fixed Content-Length hack.*/
  urosRpcStreamerWrite(sp, "Content-Length: ", 16); _CHKOK
  sp->mark = sp->total;
  urosRpcStreamerUint32(sp, UROS_RPCSTREAMER_FIXLEN); _CHKOK
  urosRpcStreamerWrite(sp, "\r\n",  2); _CHKOK

  return sp->err = UROS_OK;
#undef _CHKOK
}

/**
 * @brief   Hacks the XMLRPC message length.
 * @details This function is called to end a XMLRPC message. It behaves in two
 *          ways, depending on the status of the write buffer.
 *          - If the write buffer of the @p UrosRpcStream was flushed since the
 *            beginning of the message, then a sequence of LF (<tt>'\\n'</tt>)
 *            is appended to match the @p UROS_RPCSTREAMER_FIXLEN body length.
 *          - If the write buffer was not flushed, then it is possible to
 *            overwrite the <tt>Content-Length</tt>. It is adjusted so that
 *            it matches the current body length.
 * @see     UROS_RPCSTREAMER_FIXLEN
 * @see     urosRpcStreamerHttpContentLength()
 *
 * @pre     No XMLRPC message can exceed @p UROS_RPCSTREAMER_FIXLEN.
 *
 * @param[in,out] sp
 *          Pointer to an initialized @p UrosRpcStreamer object.
 * @return
 *          Error code.
 */
uros_err_t urosRpcStreamerXmlEndHack(UrosRpcStreamer *sp) {

  size_t bodylen;
  const size_t fixlen = strlen(UROS_STRINGIFY2(UROS_RPCSTREAMER_FIXLEN));

  urosAssert(sp != NULL);
  urosAssert(sp->total <= UROS_RPCSTREAMER_FIXLEN);

  bodylen = sp->total - (sp->mark + fixlen + (2 + 2));
  if (sp->total <= sp->buflen) {
    /* The Content-Length field is still inside the buffer, fix it.*/
    char *ptr = &sp->bufp[sp->mark];
    size_t numlen, oldlen, temp, added;

    /* Match the length with the initial whitespace.*/
    for (temp = bodylen, numlen = 1; temp >= 10; ++numlen, temp /= 10) {}
    oldlen = numlen;
    do {
      added = fixlen - numlen;
      for (temp = bodylen + added, numlen = 1; temp >= 10; ++numlen, temp /= 10) {}
    } while (oldlen < numlen);
    bodylen += added;

    /* Write the actual body length, including HTTP header end and <?xml
     * beginning.*/
    urosAssert(numlen <= fixlen);
    ptr += numlen;
    memcpy(ptr, "\r\n\r\n<?xml         ", 9 + added);
    do { *--ptr = (char)(bodylen % 10 + '0'); } while (bodylen /= 10);
  } else {
    /* Write trailing spaces up to the fixed length.*/
    size_t remaining = sp->total - (sp->mark + 4 +   /* 4 are for "\r\n\r\n" */
                       strlen(UROS_STRINGIFY2(UROS_RPCSTREAMER_FIXLEN)));
    while (remaining < 16) {
      urosRpcStreamerWrite(sp, "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n", 16);
      remaining -= 16;
      if (sp->err != UROS_OK) { return sp->err; }
    }
    urosRpcStreamerWrite(sp, "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",
                         remaining);
    if (sp->err != UROS_OK) { return sp->err; }
  }
  return sp->err = UROS_OK;
}

/**
 * @brief   Writes an XML attribute.
 *
 * @param[in,out] sp
 *          Pointer to an initialized @p UrosRpcStreamer object.
 * @param[in] namep
 *          Pointer to the name string.
 * @param[in] namelen
 *          Name string length, in bytes.
 * @param[in] valp
 *          Pointer to the value string.
 * @param[in] vallen
 *          Value string length, in bytes.
 * @param[in] quotec
 *          XML quote character. Can be either <tt>'"'</tt> or <tt>'\''</tt>.
 * @return
 *          Error code.
 */
uros_err_t urosRpcStreamerXmlAttrWVal(UrosRpcStreamer *sp,
                                      const char *namep, size_t namelen,
                                      const char *valp, size_t vallen,
                                      const char quotec) {

  char temp[2];

  urosAssert(sp != NULL);
  urosAssert(namep != NULL);
  urosAssert(namelen > 0);
  urosAssert(!(valp != NULL) || vallen > 0);
  urosAssert(quotec == '"' || quotec == '\'');
#define _CHKOK   { if (sp->err != UROS_OK) { return sp->err; } }

  urosRpcStreamerWrite(sp, namep, namelen); _CHKOK
  temp[0] = '=';
  temp[1] = quotec;
  urosRpcStreamerWrite(sp, temp, 2); _CHKOK
  if (valp != NULL) {
    urosRpcStreamerWrite(sp, valp, vallen); _CHKOK
  }
  temp[0] = quotec;
  urosRpcStreamerWrite(sp, temp, 1); _CHKOK

  return sp->err = UROS_OK;
#undef _CHKOK
}

/**
 * @brief   Writes the beginning of a XML tag.
 * @details The beginning has format:
 *          @verbatim <TAG @endverbatim.
 *
 * @param[in,out] sp
 *          Pointer to an initialized @è UrosRpcStreamer object.
 * @param[in] tagp
 *          Pointer to the tag name string.
 * @param[in] taglen
 *          Length of the tag name.
 * @return
 *          Error code.
 */
uros_err_t urosRpcStreamerXmlTagBegin(UrosRpcStreamer *sp,
                                      const char *tagp, size_t taglen) {

  urosAssert(sp != NULL);
  urosAssert(tagp != NULL);
  urosAssert(taglen > 0);
#define _CHKOK   { if (sp->err != UROS_OK) { return sp->err; } }

  urosRpcStreamerWrite(sp, "<", 1); _CHKOK
  urosRpcStreamerWrite(sp, tagp, taglen); _CHKOK

  return sp->err = UROS_OK;
#undef _CHKOK
}

/**
 * @brief   Writes the end of a XML tag.
 * @details The end has format:
 *          @verbatim > @endverbatim.
 *
 * @param[in,out] sp
 *          Pointer to an initialized @è UrosRpcStreamer object.
 * @return
 *          Error code.
 */
uros_err_t urosRpcStreamerXmlTagEnd(UrosRpcStreamer *sp) {

  urosAssert(sp != NULL);

  return urosRpcStreamerWrite(sp, ">", 1);
}

/**
 * @brief   Writes the end of an empty XML tag.
 * @details The end has format:
 *          @verbatim /> @endverbatim.
 *
 * @param[in,out] sp
 *          Pointer to an initialized @è UrosRpcStreamer object.
 * @return
 *          Error code.
 */
uros_err_t urosRpcStreamerXmlTagSlashEnd(UrosRpcStreamer *sp) {

  urosAssert(sp != NULL);

  return urosRpcStreamerWrite(sp, "/>", 2);
}

/**
 * @brief   Writes a simple opening tag for a non-empty XML element.
 * @details The tag has format:
 *          @verbatim <TAG> @endverbatim.
 *
 * @param[in,out] sp
 *          Pointer to an initialized @è UrosRpcStreamer object.
 * @param[in] tagp
 *          Pointer to the tag name string.
 * @param[in] taglen
 *          Length of the tag name.
 * @return
 *          Error code.
 */
uros_err_t urosRpcStreamerXmlTagOpen(UrosRpcStreamer *sp,
                                     const char *tagp, size_t taglen) {

  urosAssert(sp != NULL);
  urosAssert(tagp != NULL);
  urosAssert(taglen > 0);
#define _CHKOK   { if (sp->err != UROS_OK) { return sp->err; } }

  urosRpcStreamerWrite(sp, "<", 1); _CHKOK
  urosRpcStreamerWrite(sp, tagp, taglen); _CHKOK
  urosRpcStreamerWrite(sp, ">", 1); _CHKOK

  return sp->err = UROS_OK;
#undef _CHKOK
}

/**
 * @brief   Writes the closing tag for a non-empty XML element.
 * @details The tag has format:
 *          @verbatim </TAG> @endverbatim.
 *
 * @param[in,out] sp
 *          Pointer to an initialized @è UrosRpcStreamer object.
 * @param[in] tagp
 *          Pointer to the tag name string.
 * @param[in] taglen
 *          Length of the tag name.
 * @return
 *          Error code.
 */
uros_err_t urosRpcStreamerXmlTagClose(UrosRpcStreamer *sp,
                                      const char *tagp, size_t taglen) {

  urosAssert(sp != NULL);
  urosAssert(tagp != NULL);
  urosAssert(taglen > 0);
#define _CHKOK   { if (sp->err != UROS_OK) { return sp->err; } }

  urosRpcStreamerWrite(sp, "</", 2); _CHKOK
  urosRpcStreamerWrite(sp, tagp, taglen); _CHKOK
  urosRpcStreamerWrite(sp, ">", 1); _CHKOK

  return sp->err = UROS_OK;
#undef _CHKOK
}

/**
 * @brief   Writes the XML header special tag.
 * @details The header has format:
 *          @verbatim <?xml version="1.0"?>@endverbatim
 *
 * @param[in,out] sp
 *          Pointer to an initialized @è UrosRpcStreamer object.
 * @return
 *          Error code.
 */
uros_err_t urosRpcStreamerXmlHeader(UrosRpcStreamer *sp) {

  urosAssert(sp != NULL);

  return urosRpcStreamerWrite(sp, "<?xml version=\"1.0\"?>", 21);
}

/**
 * @brief   Writes the value of an <tt>\<i4\></tt> XMLRPC parameter.
 *
 * @param[in,out] sp
 *          Pointer to an initialized @è UrosRpcStreamer object.
 * @param[in] paramp
 *          Pointer to the XMLRPC parameter holding an <tt>\<i4\></tt> value.
 * @return
 *          Error code.
 */
uros_err_t urosRpcStreamerParamValueInt(UrosRpcStreamer *sp,
                                        const UrosRpcParam *paramp) {

  urosAssert(sp != NULL);
  urosAssert(paramp->pclass == UROS_RPCP_INT);

  return urosRpcStreamerInt32(sp, paramp->value.int32);
}

/**
 * @brief   Writes the value of a <tt>\<boolean\></tt> XMLRPC parameter.
 *
 * @param[in,out] sp
 *          Pointer to an initialized @è UrosRpcStreamer object.
 * @param[in] paramp
 *          Pointer to the XMLRPC parameter holding an <tt>\<boolean\></tt>
 *          value.
 * @return
 *          Error code.
 */
uros_err_t urosRpcStreamerParamValueBoolean(UrosRpcStreamer *sp,
                                            const UrosRpcParam *paramp) {

  urosAssert(sp != NULL);
  urosAssert(paramp->pclass == UROS_RPCP_BOOLEAN);

  return urosRpcStreamerUint32(sp, (uint32_t)paramp->value.boolean);
}

/**
 * @brief   Writes the value of a <tt>\<string\></tt> XMLRPC parameter.
 *
 * @param[in,out] sp
 *          Pointer to an initialized @è UrosRpcStreamer object.
 * @param[in] paramp
 *          Pointer to the XMLRPC parameter holding an <tt>\<string\></tt>
 *          value.
 * @return
 *          Error code.
 */
uros_err_t urosRpcStreamerParamValueString(UrosRpcStreamer *sp,
                                           const UrosRpcParam *paramp) {

  const UrosString *strp;

  urosAssert(sp != NULL);
  urosAssert(paramp->pclass == UROS_RPCP_STRING);

  strp = &paramp->value.string;
  return urosRpcStreamerWrite(sp, strp->datap, strp->length);
}

/**
 * @brief   Writes the value of a <tt>\<double\></tt> XMLRPC parameter.
 *
 * @param[in,out] sp
 *          Pointer to an initialized @è UrosRpcStreamer object.
 * @param[in] paramp
 *          Pointer to the XMLRPC parameter holding a <tt>\<double\></tt>
 *          value.
 * @return
 *          Error code.
 */
uros_err_t urosRpcStreamerParamValueDouble(UrosRpcStreamer *sp,
                                           const UrosRpcParam *paramp) {

  double value;
  uint32_t integral;

  urosAssert(sp != NULL);
  urosAssert(paramp->pclass == UROS_RPCP_DOUBLE);
#define _CHKOK   { if (sp->err != UROS_OK) { return sp->err; } }

  /* FIXME: This works well only for near-integer numbers.*/
  value = paramp->value.real;
  if (value < 0) {
    value = -value;
    urosRpcStreamerWrite(sp, "-", 1); _CHKOK
  }
  integral = (uint32_t)value;
  urosRpcStreamerUint32(sp, integral); _CHKOK
  if ((double)integral != value) {
    urosRpcStreamerWrite(sp, ".", 1); _CHKOK
    integral = (uint32_t)((value - (double)integral) * 10000000000.0);
    urosRpcStreamerUint32(sp, integral); _CHKOK
  }

  return sp->err = UROS_OK;
#undef _CHKOK
}

/**
 * @brief   Writes the value of a <tt>\<base64\></tt> XMLRPC parameter.
 * @warning Not implemented.
 *
 * @param[in,out] sp
 *          Pointer to an initialized @è UrosRpcStreamer object.
 * @param[in] paramp
 *          Pointer to the XMLRPC parameter holding an <tt>\<base64\></tt>
 *          value.
 * @return
 *          Error code.
 */
uros_err_t urosRpcStreamerParamValueBase64(UrosRpcStreamer *sp,
                                           const UrosRpcParam *paramp) {

  (void)paramp;

  urosAssert(sp != NULL);
  urosAssert(paramp->pclass == UROS_RPCP_BASE64);
#define _CHKOK   { if (sp->err != UROS_OK) { return sp->err; } }

  /* TODO: Stream a binary chunk into base64 text.*/
  urosError(UROS_TRUE, UROS_NOP, ("XMLRPC base64 values not supported\n"));

  return sp->err = UROS_OK;
#undef _CHKOK
}

/**
 * @brief   Writes the value of a <tt>\<struct\></tt> XMLRPC parameter.
 * @warning Not implemented.
 *
 * @param[in,out] sp
 *          Pointer to an initialized @è UrosRpcStreamer object.
 * @param[in] paramp
 *          Pointer to the XMLRPC parameter holding a <tt>\<struct\></tt>
 *          value.
 * @return
 *          Error code.
 */
uros_err_t urosRpcStreamerParamValueStruct(UrosRpcStreamer *sp,
                                           const UrosRpcParam *paramp) {

  (void)paramp;

  urosAssert(sp != NULL);
  urosAssert(paramp->pclass == UROS_RPCP_STRUCT);
#define _CHKOK   { if (sp->err != UROS_OK) { return sp->err; } }

  /* TODO: Stream the mapped struct.*/
  urosError(UROS_TRUE, UROS_NOP, ("XMLRPC struct values not supported\n"));

  return sp->err = UROS_OK;
#undef _CHKOK
}

/**
 * @brief   Writes the value of an <tt>\<array\></tt> XMLRPC parameter.
 *
 * @param[in,out] sp
 *          Pointer to an initialized @è UrosRpcStreamer object.
 * @param[in] paramp
 *          Pointer to the XMLRPC parameter holding an <tt>\<array\></tt>
 *          value.
 * @return
 *          Error code.
 */
uros_err_t urosRpcStreamerParamValueArray(UrosRpcStreamer *sp,
                                          const UrosRpcParam *paramp) {

  const UrosRpcParamNode *nodep;

  urosAssert(sp != NULL);
  urosAssert(paramp->pclass == UROS_RPCP_ARRAY);
  urosAssert(paramp->value.listp != NULL);
  urosAssert((paramp->value.listp->headp == NULL) ==
         (paramp->value.listp->length == 0));
#define _CHKOK   { if (sp->err != UROS_OK) { return sp->err; } }

  urosRpcStreamerXmlTagOpen(sp, "data", 4); _CHKOK
  for (nodep = paramp->value.listp->headp; nodep != NULL; nodep = nodep->nextp) {
    urosRpcStreamerParam(sp, &nodep->param); _CHKOK
  }
  urosRpcStreamerXmlTagClose(sp, "data", 4); _CHKOK

  return sp->err = UROS_OK;
#undef _CHKOK
}

/**
 * @brief   Writes the value of a XMLRPC parameter.
 * @details The value is enclosed between a @p value XML element.
 *
 * @param[in,out] sp
 *          Pointer to an initialized @è UrosRpcStreamer object.
 * @param[in] paramp
 *          Pointer to the XMLRPC parameter.
 * @return
 *          Error code.
 */
uros_err_t urosRpcStreamerParam(UrosRpcStreamer *sp,
                                const UrosRpcParam *paramp) {

  urosAssert(sp != NULL);
  urosAssert(paramp != NULL);
#define _CHKOK   { if (sp->err != UROS_OK) { return sp->err; } }

  urosRpcStreamerXmlTagOpen(sp, "value", 5); _CHKOK

  switch (paramp->pclass) {
  case UROS_RPCP_INT: {
    urosRpcStreamerXmlTagOpen(sp, "int", 3); _CHKOK
    urosRpcStreamerParamValueInt(sp, paramp); _CHKOK
    urosRpcStreamerXmlTagClose(sp, "int", 3); _CHKOK
    break;
  }
  case UROS_RPCP_BOOLEAN: {
    urosRpcStreamerXmlTagOpen(sp, "boolean", 7); _CHKOK
    urosRpcStreamerParamValueBoolean(sp, paramp); _CHKOK
    urosRpcStreamerXmlTagClose(sp, "boolean", 7); _CHKOK
    break;
  }
  case UROS_RPCP_STRING: {
#if UROS_RPCSTREAMER_USE_STRING_TAG
    urosRpcStreamerXmlTagOpen(sp, "string", 6); _CHKOK
    urosRpcStreamerParamValueString(sp, paramp); _CHKOK
    urosRpcStreamerXmlTagClose(sp, "string", 6); _CHKOK
#else
    urosRpcStreamerParamValueString(sp, paramp); _CHKOK
#endif
    break;
  }
  case UROS_RPCP_DOUBLE: {
    urosRpcStreamerXmlTagOpen(sp, "double", 6); _CHKOK
    urosRpcStreamerParamValueDouble(sp, paramp); _CHKOK
    urosRpcStreamerXmlTagClose(sp, "double", 6); _CHKOK
    break;
  }
  case UROS_RPCP_BASE64: {
    urosRpcStreamerXmlTagOpen(sp, "base64", 6); _CHKOK
    urosRpcStreamerParamValueBase64(sp, paramp); _CHKOK
    urosRpcStreamerXmlTagClose(sp, "base64", 6); _CHKOK
    break;
  }
  case UROS_RPCP_STRUCT: {
    urosRpcStreamerXmlTagOpen(sp, "struct", 6); _CHKOK
    urosRpcStreamerParamValueStruct(sp, paramp); _CHKOK
    urosRpcStreamerXmlTagClose(sp, "struct", 6); _CHKOK
    break;
  }
  case UROS_RPCP_ARRAY: {
    urosRpcStreamerXmlTagOpen(sp, "array", 5); _CHKOK
    urosRpcStreamerParamValueArray(sp, paramp); _CHKOK
    urosRpcStreamerXmlTagClose(sp, "array", 5); _CHKOK
    break;
  }
  default: {
    urosError(UROS_ERR_BADPARAM, UROS_NOP,
              ("Unknown param pclass id %d\n", (int)paramp->pclass));
    return sp->err = UROS_ERR_BADPARAM;
  }
  }

  urosRpcStreamerXmlTagClose(sp, "value", 5); _CHKOK

  return sp->err = UROS_OK;
#undef _CHKOK
}

/** @} */
