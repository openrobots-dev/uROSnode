/*
Copyright (c) 2012, Politecnico di Milano. All rights reserved.

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
 * @file    tcpros_handlers.h
 * @author  Andrea Zoppi <texzk@email.it>
 *
 * @brief   Hanlder functions for TCPROS topic and service streams.
 */

#ifndef _TCPROS_HANDLERS_H_
#define _TCPROS_HANDLERS_H_

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include <urosBase.h>
#include <urosTcpRos.h>

/*===========================================================================*/
/* GLOBAL PROTOTYPES                                                         */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

uros_err_t tpc__turtleX__command_velocity(UrosTcpRosStatus *tcpstp);
uros_err_t tpc__turtleX__pose(UrosTcpRosStatus *tcpstp);

uros_err_t tpc__rosout(UrosTcpRosStatus *tcpstp);

uros_err_t svc__clear(UrosTcpRosStatus *tcpstp);
uros_err_t svc__reset(UrosTcpRosStatus *tcpstp);
uros_err_t svc__kill(UrosTcpRosStatus *tcpstp);
uros_err_t svc__spawn(UrosTcpRosStatus *tcpstp);
uros_err_t svc__turtleX__set_pen(UrosTcpRosStatus *tcpstp);
uros_err_t svc__turtleX__teleport_absolute(UrosTcpRosStatus *tcpstp);
uros_err_t svc__turtleX__teleport_relative(UrosTcpRosStatus *tcpstp);

#ifdef __cplusplus
}
#endif
#endif /* _TCPROS_HANDLERS_H_ */
