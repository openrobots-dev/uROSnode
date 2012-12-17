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
 * @file    tcpros_handlers.c
 * @author  Andrea Zoppi <texzk@email.it>
 *
 * @brief   Hanlder functions for TCPROS topic and service streams.
 */

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include "tcpros_handlers.h"
#include "msg_types.h"
#include "app.h"

#include <urosBase.h>
#include <urosUser.h>
#include <unistd.h>
#include <stdio.h>

/*===========================================================================*/
/* LOCAL FUNCTIONS                                                           */
/*===========================================================================*/

static uros_err_t send_logmsg(UrosTcpRosStatus *tcpstp,
                              const struct rosgraph_msgs__Log *msgp) {

  uint32_t size;
  const UrosListNode *nodep;

  urosAssert(tcpstp != NULL);
  urosAssert(msgp != NULL);
#define _CHKOK  { if (tcpstp->err != UROS_OK) { return tcpstp->err; } }

  /* Compute the body length.*/
  size = (5 + 3) * sizeof(uint32_t) + sizeof(struct time_t) + sizeof(uint8_t) +
         msgp->header.frame_id.length + msgp->name.length + msgp->msg.length +
         msgp->file.length + msgp->function.length;
  for (nodep = msgp->topics.headp; nodep != NULL; nodep = nodep->nextp) {
    size += (uint32_t)((const UrosString *)nodep->datap)->length;
  }

  /* Write the body length.*/
  urosTcpRosSend(tcpstp, &size, sizeof(uint32_t)); _CHKOK

  /* Write the message body.*/
  urosTcpRosSendRaw(tcpstp, msgp->header.seq); _CHKOK
  urosTcpRosSendRaw(tcpstp, msgp->header.time); _CHKOK
  urosTcpRosSendString(tcpstp, &msgp->header.frame_id); _CHKOK
  urosTcpRosSendRaw(tcpstp, msgp->level); _CHKOK
  urosTcpRosSendString(tcpstp, &msgp->name); _CHKOK
  urosTcpRosSendString(tcpstp, &msgp->msg); _CHKOK
  urosTcpRosSendString(tcpstp, &msgp->file); _CHKOK
  urosTcpRosSendString(tcpstp, &msgp->function); _CHKOK
  urosTcpRosSendRaw(tcpstp, msgp->line); _CHKOK
  size = msgp->topics.length;
  urosTcpRosSendRaw(tcpstp, size); _CHKOK
  for (nodep = msgp->topics.headp; nodep != NULL; nodep = nodep->nextp) {
    urosTcpRosSendString(tcpstp, (const UrosString *)nodep->datap); _CHKOK
  }

  return tcpstp->err = UROS_OK;
}

/*===========================================================================*/
/* GLOBAL FUNCTIONS                                                          */
/*===========================================================================*/

uros_err_t tpc__turtleX__command_velocity(UrosTcpRosStatus *tcpstp) {

  struct msg__turtlesim__Velocity velocity;
  uint32_t hdrlen;
  turtle_t *turtlep = &turtles[0]; /* FIXME: Find a way to provide arguments.*/

  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
#define _CHKOK  { if (tcpstp->err != UROS_OK) { return tcpstp->err; } }

  while (!urosTcpRosStatusCheckExit(tcpstp)) {
    /* Get the next command.*/
    urosTcpRosRecv(tcpstp, &hdrlen, sizeof(hdrlen)); _CHKOK
    urosError((size_t)hdrlen != sizeof(struct msg__turtlesim__Velocity),
              return tcpstp->err = UROS_ERR_BADCONN,
              ("Message length %lu (0x%0.8lX), expected %zu (0x%0.8zX)\n",
               hdrlen, hdrlen, sizeof(struct msg__turtlesim__Velocity),
               sizeof(struct msg__turtlesim__Velocity)));
    urosTcpRosRecv(tcpstp, &velocity, sizeof(velocity)); _CHKOK
#if 1
    /* Dump the received packet contents.*/
    printf("PID: %d\n", (int)urosThreadSelf());
    printf("turtle1/command_velocity\n");
    printf("\tlinear  %f\n", velocity.linear);
    printf("\tangular %f\n", velocity.angular);
#endif

    /* Execute the command.*/
    urosMutexLock(&turtlep->lock);
    turtlep->pose.linear_velocity = velocity.linear;
    turtlep->pose.angular_velocity = velocity.angular;
    turtlep->countdown = 1000;
    urosMutexUnlock(&turtlep->lock);
  }
  return tcpstp->err;
#undef _CHKOK
}

uros_err_t tpc__turtleX__pose(UrosTcpRosStatus *tcpstp) {

  static const uint32_t hdrlen = (uint32_t)sizeof(struct msg__turtlesim__Pose);

  struct msg__turtlesim__Pose pose_sample;
  turtle_t *turtlep = &turtles[0]; /* FIXME: Find a way to provide arguments.*/

  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));

  /* Send a pose sample every 10ms.*/
  tcpstp->err = UROS_OK;
  while (!urosTcpRosStatusCheckExit(tcpstp) && tcpstp->err == UROS_OK) {
    urosMutexLock(&turtlep->lock);
    if (turtlep->status != TURTLE_ALIVE) {
      /* The turtle is dead, unreference it and end the stream.*/
      turtle_unref(turtlep);
      urosMutexUnlock(&turtlep->lock);
      break;
    }
    pose_sample = turtlep->pose;
    urosMutexUnlock(&turtlep->lock);
    urosTcpRosSendRaw(tcpstp, hdrlen);
    urosTcpRosSendRaw(tcpstp, pose_sample);
    usleep(10000);
  }
  return tcpstp->err;
#undef _CHKOK
}

uros_err_t tpc__rosout(UrosTcpRosStatus *tcpstp) {

  static const UrosString zerostr = { 1, "0" };
  static const UrosString msgstr = { 20, "<handle__rosout()>\r\n" };

  struct rosgraph_msgs__Log msg;

  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));

  msg.header.frame_id = zerostr;
  msg.header.seq = 0;
  msg.header.time.sec = 0;
  msg.header.time.nsec = 0;
  msg.level = rosgraph_msgs__Log_level_DEBUG;
  msg.name = urosNode.config.nodeName;
  msg.msg = msgstr;
  msg.file.length = strlen(__FILE__);
  msg.file.datap = (char*)__FILE__;
  msg.function.length = strlen(__PRETTY_FUNCTION__);
  msg.function.datap = (char*)__PRETTY_FUNCTION__;
  msg.line = __LINE__;
  urosListObjectInit(&msg.topics);

  tcpstp->err = UROS_OK;
  while (!urosTcpRosStatusCheckExit(tcpstp) && tcpstp->err == UROS_OK) {
    /* Send an heartbeat message every 1s.*/
    ++msg.header.seq;
    send_logmsg(tcpstp, &msg);
    sleep(1);
  }
  return tcpstp->err;
#undef _CHKOK
}

