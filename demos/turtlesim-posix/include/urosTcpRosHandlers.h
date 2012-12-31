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
 * @file    urosTcpRosHandlers.h
 * @author  Andrea Zoppi <texzk@email.it>
 *
 * @brief   TCPROS topic and service handlers.
 */

#ifndef _UROSTCPROSHANDLERS_H_
#define _UROSTCPROSHANDLERS_H_

/*============================================================================*/
/* HEADER FILES                                                               */
/*============================================================================*/

#include "urosTcpRosTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*/
/* PUBLISHED TOPIC PROTOTYPES                                                 */
/*============================================================================*/

/*~~~ PUBLISHED TOPIC: /rosout ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

uros_err_t pub_tpc__rosout(UrosTcpRosStatus *tcpstp);

/*~~~ PUBLISHED TOPIC: /turtleX/pose ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

uros_err_t pub_tpc__turtleX__pose(UrosTcpRosStatus *tcpstp);

/*============================================================================*/
/* SUBSCRIBED TOPIC PROTOTYPES                                                */
/*============================================================================*/

/*~~~ SUBSCRIBED TOPIC: /turtleX/command_velocity ~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

uros_err_t sub_tpc__turtleX__command_velocity(UrosTcpRosStatus *tcpstp);

/*============================================================================*/
/* PUBLISHED SERVICE PROTOTYPES                                               */
/*============================================================================*/

/*~~~ PUBLISHED SERVICE: /clear ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

uros_err_t pub_srv__clear(UrosTcpRosStatus *tcpstp);

/*~~~ PUBLISHED SERVICE: /kill ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

uros_err_t pub_srv__kill(UrosTcpRosStatus *tcpstp);

/*~~~ PUBLISHED SERVICE: /spawn ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

uros_err_t pub_srv__spawn(UrosTcpRosStatus *tcpstp);

/*~~~ PUBLISHED SERVICE: /turtleX/set_pen ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

uros_err_t pub_srv__turtleX__set_pen(UrosTcpRosStatus *tcpstp);

/*~~~ PUBLISHED SERVICE: /turtleX/teleport_absolute ~~~~~~~~~~~~~~~~~~~~~~~~~~*/

uros_err_t pub_srv__turtleX__teleport_absolute(UrosTcpRosStatus *tcpstp);

/*~~~ PUBLISHED SERVICE: /turtleX/teleport_relative ~~~~~~~~~~~~~~~~~~~~~~~~~~*/

uros_err_t pub_srv__turtleX__teleport_relative(UrosTcpRosStatus *tcpstp);

/*============================================================================*/
/* GLOBAL PROTOTYPES                                                          */
/*============================================================================*/

void urosTcpRosPublishTopics(void);
void urosTcpRosUnpublishTopics(void);

void urosTcpRosSubscribeTopics(void);
void urosTcpRosUnsubscribeTopics(void);

void urosTcpRosPublishServices(void);
void urosTcpRosUnpublishServices(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _UROSTCPROSHANDLERS_H_ */

