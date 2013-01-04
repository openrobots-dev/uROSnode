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
 * @file    uros_lld_node.c
 * @author  Andrea Zoppi <texzk@email.it>
 *
 * @brief   Low-level Node features implementation.
 */

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include "../../../include/lld/uros_lld_node.h"
#include "../../../include/urosUser.h"

#include <stdio.h>

/*===========================================================================*/
/* LOCAL TYPES & MACROS                                                      */
/*===========================================================================*/

#if UROS_NODE_C_USE_ASSERT == UROS_FALSE && !defined(__DOXYGEN__)
#undef urosAssert
#define urosAssert(expr)
#endif

/** @addtogroup node_lld_macros */
/** @{ */

/**
 * @brief   File where the Node configuration is stored.
 */
#if !defined(UROS_NODECONFIG_FILENAME) || defined(__DOXYGEN__)
#define UROS_NODECONFIG_FILENAME    "urosNode.config"
#endif

/** @} */

/*===========================================================================*/
/* LOCAL FUNCTIONS                                                           */
/*===========================================================================*/

static void uros_nodeconfig_readstring(UrosString *strp, FILE *fp) {

  size_t length, n;
  char *datap;

  urosAssert(strp != NULL);
  urosAssert(fp != NULL);

  /* Read the string length.*/
  n = fread(&length, sizeof(size_t), 1, fp);
  urosError(n < 1, return,
            ("Cannot read string length at offset 0x%.8lX\n", ftell(fp)));

  /* Read the string data.*/
  datap = (char*)urosAlloc(length);
  urosAssert(datap != NULL);
  n = fread(datap, length, 1, fp);
  urosError(n < 1, return,
            ("Cannot read string data (%zu bytes) at offset 0x%.8lX\n",
             length, ftell(fp)));

  /* Assign the data back.*/
  strp->length = length;
  strp->datap = datap;
}

static void uros_nodeconfig_writestring(const UrosString *strp, FILE *fp) {

  size_t n;

  urosAssert(strp != NULL);
  urosAssert(fp != NULL);

  /* Write the string length.*/
  n = fwrite(&strp->length, sizeof(size_t), 1, fp);
  urosError(n < 1, return,
            ("Cannot write string length (%zu bytes) at offset 0x%.8lX\n",
             strp->length, ftell(fp)));

  /* Write the string data.*/
  n = fwrite(strp->datap, strp->length, 1, fp);
  urosError(n < 1, return,
            ("Cannot write string data [%.*s] at offset 0x%.8lX\n",
             UROS_STRARG(strp), ftell(fp)));
}

static void uros_nodeconfig_readaddr(UrosAddr *addrp, FILE *fp) {

  size_t n;

  urosAssert(addrp != NULL);
  urosAssert(fp != NULL);

  n = fread(addrp, sizeof(UrosAddr), 1, fp);
  urosError(n < 1, return,
            ("Cannot read connection address at offset 0x%.8lX\n", ftell(fp)));
}

static void uros_nodeconfig_writeaddr(const UrosAddr *addrp, FILE *fp) {

  size_t n;

  urosAssert(addrp != NULL);
  urosAssert(fp != NULL);

  n = fwrite(addrp, sizeof(UrosAddr), 1, fp);
  urosError(n < 1, return,
            ("Cannot write connection address ("UROS_ADDRFMT") at offset 0x%.8lX\n",
             UROS_ADDRARG(addrp), ftell(fp)));
}

/*===========================================================================*/
/* GLOBAL FUNCTIONS                                                          */
/*===========================================================================*/

/** @addtogroup node_lld_funcs */
/** @{ */

/**
 * @brief   Loads node configuration.
 * @details Any previously allocated data is freed, then the configuration is
 *          loaded from a static non-volatile memory chunk.
 * @see     uros_lld_nodeconfig_load()
 *
 * @pre     The related @p UrosNode is initialized.
 *
 * @param[in,out] cfgp
 *          Pointer to the target configuration descriptor.
 */
void uros_lld_nodeconfig_load(UrosNodeConfig *cfgp) {

  FILE *fp;

  urosAssert(cfgp != NULL);

  /* Clean any allocated variables.*/
  urosStringClean(&cfgp->nodeName);
  urosStringClean(&cfgp->xmlrpcUri);
  urosStringClean(&cfgp->tcprosUri);
  urosStringClean(&cfgp->masterUri);

  /* Read from file.*/
  fp = fopen(UROS_NODECONFIG_FILENAME, "rb");
  if (fp == NULL) {
    /* File not found, load default values and save them.*/
    urosError(fp == NULL, UROS_NOP,
              ("Cannot open file ["UROS_NODECONFIG_FILENAME"] for reading\n"
               "  (The default configuration will be written there if possible)\n"));
    urosNodeConfigLoadDefaults(cfgp);
    urosNodeConfigSave(cfgp);
    return;
  }

  uros_nodeconfig_readstring(&cfgp->nodeName, fp);
  uros_nodeconfig_readaddr  (&cfgp->xmlrpcAddr, fp);
  uros_nodeconfig_readstring(&cfgp->xmlrpcUri, fp);
  uros_nodeconfig_readaddr  (&cfgp->tcprosAddr, fp);
  uros_nodeconfig_readstring(&cfgp->tcprosUri, fp);
  uros_nodeconfig_readaddr  (&cfgp->masterAddr, fp);
  uros_nodeconfig_readstring(&cfgp->masterUri, fp);

  fclose(fp);
}

/**
 * @brief   Saves the node configuration.
 * @details The node configuration is saved to a static non-volatile memory
 *          chunk.
 * @see     uros_lld_nodeconfig_save()
 *
 * @pre     The related @p UrosNode is initialized.
 *
 * @param[in] cfgp
 *          Pointer to the configuration descriptor to be saved.
 */
void uros_lld_nodeconfig_save(const UrosNodeConfig *cfgp) {

  FILE *fp;

  urosAssert(cfgp != NULL);

  /* Write to file.*/
  fp = fopen(UROS_NODECONFIG_FILENAME, "wb");
  urosError(fp == NULL, return,
            ("Cannot open file ["UROS_NODECONFIG_FILENAME"] for writing\n"));

  uros_nodeconfig_writestring(&cfgp->nodeName, fp);
  uros_nodeconfig_writeaddr  (&cfgp->xmlrpcAddr, fp);
  uros_nodeconfig_writestring(&cfgp->xmlrpcUri, fp);
  uros_nodeconfig_writeaddr  (&cfgp->tcprosAddr, fp);
  uros_nodeconfig_writestring(&cfgp->tcprosUri, fp);
  uros_nodeconfig_writeaddr  (&cfgp->masterAddr, fp);
  uros_nodeconfig_writestring(&cfgp->masterUri, fp);

  fflush(fp);
  fclose(fp);
}

/** @} */
