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
 * @file    urosUser.h
 * @author  Andrea Zoppi <texzk@email.it>
 *
 * @brief   User-defined callback functions.
 */

#ifndef _UROSUSER_H_
#define _UROSUSER_H_

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include "urosBase.h"
#include "urosNode.h"
#include "urosRpcCall.h"

/*===========================================================================*/
/* GLOBAL PROTOTYPES                                                         */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

void urosUserNodeConfigLoad(UrosNodeConfig *cfgp);
void urosUserNodeConfigSave(const UrosNodeConfig *cfgp);

uros_err_t urosUserShutdown(const UrosString *msgp);

void urosUserRegisterStaticTypes(void);

uros_err_t urosUserPublishTopics(void);
uros_err_t urosUserUnpublishTopics(void);

uros_err_t urosUserSubscribeTopics(void);
uros_err_t urosUserUnsubscribeTopics(void);

uros_err_t urosUserPublishServices(void);
uros_err_t urosUserUnpublishServices(void);

uros_err_t urosUserSubscribeParams(void);
uros_err_t urosUserUnsubscribeParams(void);
uros_err_t urosUserParamUpdate(const UrosString *keyp,
                               const UrosRpcParam *paramp);

#ifdef __cplusplus
}
#endif
#endif /* _UROSUSER_H_ */
