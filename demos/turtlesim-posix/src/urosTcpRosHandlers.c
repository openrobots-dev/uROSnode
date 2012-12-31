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
 * @file    urosTcpRosHandlers.c
 * @author  Andrea Zoppi <texzk@email.it>
 *
 * @brief   TCPROS topic and service handlers.
 */

/*============================================================================*/
/* HEADER FILES                                                               */
/*============================================================================*/

#include "urosTcpRosHandlers.h"
#include "app.h"

#include <urosNode.h>
#include <math.h>

/*============================================================================*/
/* TYPES & MACROS                                                             */
/*============================================================================*/

#define min(a,b)    (((a) <= (b)) ? (a) : (b))
#define max(a,b)    (((a) >= (b)) ? (a) : (b))

/*============================================================================*/
/* PUBLISHED TOPIC FUNCTIONS                                                  */
/*============================================================================*/

/** @addtogroup tcpros_pubtopic_funcs */
/** @{ */

/*~~~ PUBLISHED TOPIC: /rosout ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Topic <tt>/rosout</tt> publisher */
/** @{ */

/**
 * @brief   TCPROS <tt>/rosout</tt> published topic handler.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @return
 *          Error code.
 */
uros_err_t pub_tpc__rosout(UrosTcpRosStatus *tcpstp) {

  struct msg__rosgraph_msgs__Log *msgp = NULL;
  uint32_t length;
  uros_bool_t constant = UROS_TRUE;

  urosAssert(tcpstp != NULL);
  urosAssert(tcpstp->topicp != NULL);
  urosAssert(!tcpstp->topicp->flags.service);
  urosAssert(urosConnIsValid(tcpstp->csp));

  /* Published messages loop.*/
  while (!urosTcpRosStatusCheckExit(tcpstp)) {
    /* Get the next message from the queue.*/
    rosout_fetch(&msgp);
    constant = msgp->header.frame_id.datap[0] != '0';
    msgp->header.frame_id.datap = "0";

    /* Send the message.*/
    length = (uint32_t)length_msg__rosgraph_msgs__Log(msgp);
    if (urosTcpRosSendRaw(tcpstp, length) != UROS_OK) { goto _finally; }
    send_msg__rosgraph_msgs__Log(tcpstp, msgp);
    if (tcpstp->err != UROS_OK) { goto _finally; }

    /* Deallocate the message if not constant.*/
    if (!constant) {
      urosStringClean(&msgp->msg);
    }
    urosFree(msgp);
    msgp = NULL;
  }
  tcpstp->err = UROS_OK;

_finally:
  /* Message deinitialization and deallocation.*/
  if (!constant) { urosStringClean(&msgp->msg); }
  urosFree(msgp);
  return tcpstp->err;
}

/** @} */

/*~~~ PUBLISHED TOPIC: /turtleX/pose ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Topic <tt>/turtleX/pose</tt> publisher */
/** @{ */

/**
 * @brief   TCPROS <tt>/turtleX/pose</tt> published topic handler.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @return
 *          Error code.
 */
uros_err_t pub_tpc__turtleX__pose(UrosTcpRosStatus *tcpstp) {

  struct msg__turtlesim__Pose *msgp = NULL;
  uint32_t length;
  turtle_t *turtlep;

  urosAssert(tcpstp != NULL);
  urosAssert(tcpstp->topicp != NULL);
  urosAssert(!tcpstp->topicp->flags.service);
  urosAssert(urosConnIsValid(tcpstp->csp));

  /* Get the turtle slot.*/
  turtlep = turtle_refbypath(&tcpstp->topicp->name);
  if (turtlep == NULL) { return UROS_ERR_BADPARAM; }

  /* Message allocation and initialization.*/
  msgp = urosNew(struct msg__turtlesim__Pose);
  if (msgp == NULL) { return UROS_ERR_NOMEM; }
  init_msg__turtlesim__Pose(msgp);

  /* Published messages loop.*/
  while (!urosTcpRosStatusCheckExit(tcpstp)) {
    /* Get the turtle pose, if still alive.*/
    urosMutexLock(&turtlep->lock);
    if (turtlep->status != TURTLE_ALIVE) {
      turtle_unref(turtlep);
      urosMutexUnlock(&turtlep->lock);
      break;
    }
    msgp->x = turtlep->pose.x;
    msgp->y = turtlep->pose.y;
    msgp->theta = turtlep->pose.theta;
    msgp->linear_velocity = turtlep->pose.linear_velocity;
    msgp->angular_velocity = turtlep->pose.angular_velocity;
    urosMutexUnlock(&turtlep->lock);

    /* Send the message.*/
    length = (uint32_t)length_msg__turtlesim__Pose(msgp);
    if (urosTcpRosSendRaw(tcpstp, length) != UROS_OK) { goto _finally; }
    send_msg__turtlesim__Pose(tcpstp, msgp);
    if (tcpstp->err != UROS_OK) { goto _finally; }

    /* Dispose the contents of the message.*/
    clean_msg__turtlesim__Pose(msgp);

    /* Send the next pose every 10ms.*/
    urosThreadSleepMsec(10);
  }
  tcpstp->err = UROS_OK;

_finally:
  /* Message deinitialization and deallocation.*/
  clean_msg__turtlesim__Pose(msgp);
  urosFree(msgp);
  return tcpstp->err;
}

/** @} */

/** @} */

/*============================================================================*/
/* SUBSCRIBED TOPIC FUNCTIONS                                                 */
/*============================================================================*/

/** @addtogroup tcpros_subtopic_funcs */
/** @{ */

/*~~~ SUBSCRIBED TOPIC: /turtleX/command_velocity ~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Topic <tt>/turtleX/command_velocity</tt> subscriber */
/** @{ */

/**
 * @brief   TCPROS <tt>/turtleX/command_velocity</tt> subscribed topic handler.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @return
 *          Error code.
 */
uros_err_t sub_tpc__turtleX__command_velocity(UrosTcpRosStatus *tcpstp) {

  struct msg__turtlesim__Velocity *msgp = NULL;
  uint32_t length;
  turtle_t *turtlep;

  urosAssert(tcpstp != NULL);
  urosAssert(tcpstp->topicp != NULL);
  urosAssert(!tcpstp->topicp->flags.service);
  urosAssert(urosConnIsValid(tcpstp->csp));

  /* Get the turtle slot.*/
  turtlep = turtle_refbypath(&tcpstp->topicp->name);
  if (turtlep == NULL) { return UROS_ERR_BADPARAM; }

  urosMutexLock(&turtlep->lock);
  if (turtlep->status != TURTLE_ALIVE) {
    return tcpstp->err = UROS_OK;
  }
  ++turtlep->refCnt;
  urosMutexUnlock(&turtlep->lock);

  /* Message allocation and initialization.*/
  msgp = urosNew(struct msg__turtlesim__Velocity);
  if (msgp == NULL) { return UROS_ERR_NOMEM; }
  init_msg__turtlesim__Velocity(msgp);

  /* Subscribed messages loop.*/
  while (!urosTcpRosStatusCheckExit(tcpstp)) {
    /* Receive the next message.*/
    if (urosTcpRosRecvRaw(tcpstp, length) != UROS_OK) { goto _finally; }
    recv_msg__turtlesim__Velocity(tcpstp, msgp);
    if (tcpstp->err != UROS_OK) { goto _finally; }
    if ((size_t)length != length_msg__turtlesim__Velocity(msgp)) {
      tcpstp->err = UROS_ERR_BADPARAM; goto _finally;
    }

    /* Start a new turtle movement for 1s.*/
    urosMutexLock(&turtlep->lock);
    turtlep->pose.linear_velocity = msgp->linear;
    turtlep->pose.angular_velocity = msgp->angular;
    turtlep->countdown = 1000;
    urosMutexUnlock(&turtlep->lock);

    /* Dispose the contents of the message.*/
    clean_msg__turtlesim__Velocity(msgp);
  }
  tcpstp->err = UROS_OK;

_finally:
  /* Message deinitialization and deallocation.*/
  clean_msg__turtlesim__Velocity(msgp);
  urosFree(msgp);
  urosMutexLock(&turtlep->lock);
  turtle_unref(turtlep);
  urosMutexUnlock(&turtlep->lock);
  return tcpstp->err;
}

/** @} */

/** @} */

/*============================================================================*/
/* PUBLISHED SERVICE FUNCTIONS                                                */
/*============================================================================*/

/** @addtogroup tcpros_pubservice_funcs */
/** @{ */

/*~~~ PUBLISHED SERVICE: /clear ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Service <tt>/clear</tt> publisher */
/** @{ */

/**
 * @brief   TCPROS <tt>/clear</tt> published service handler.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @return
 *          Error code.
 */
uros_err_t pub_srv__clear(UrosTcpRosStatus *tcpstp) {

  struct in_srv__std_srvs__Empty *inmsgp = NULL;
  struct out_srv__std_srvs__Empty *outmsgp = NULL;
  uint8_t okByte;
  uint32_t length;

  urosAssert(tcpstp != NULL);
  urosAssert(tcpstp->topicp != NULL);
  urosAssert(tcpstp->topicp->flags.service);
  urosAssert(urosConnIsValid(tcpstp->csp));

  /* Service messages allocation and initialization.*/
  inmsgp = urosNew(struct in_srv__std_srvs__Empty);
  if (inmsgp == NULL) { tcpstp->err = UROS_ERR_NOMEM; goto _finally; }
  outmsgp = urosNew(struct out_srv__std_srvs__Empty);
  if (outmsgp == NULL) { tcpstp->err = UROS_ERR_NOMEM; goto _finally; }
  init_in_srv__std_srvs__Empty(inmsgp);
  init_out_srv__std_srvs__Empty(outmsgp);

  /* Service message loop (if the service is persistent).*/
  while (!urosTcpRosStatusCheckExit(tcpstp)) {
    /* Receive the request message.*/
    if (urosTcpRosRecvRaw(tcpstp, length) != UROS_OK) { goto _finally; }
    recv_srv__std_srvs__Empty(tcpstp, inmsgp);
    if (tcpstp->err != UROS_OK) { goto _finally; }
    if ((size_t)length != length_in_srv__std_srvs__Empty(inmsgp)) {
      tcpstp->err = UROS_ERR_BADPARAM; goto _finally;
    }

    /* Process the request message.*/
    tcpstp->err = UROS_OK;
    urosStringClean(&tcpstp->errstr);
    okByte = 1;

    /* Dispose the contents of the request message.*/
    clean_in_srv__std_srvs__Empty(inmsgp);

    /* Send the response message.*/
    if (urosTcpRosSendRaw(tcpstp, okByte) != UROS_OK) { goto _finally; }
    if (okByte == 0) {
      /* On error, send the tcpstp->errstr error message (cleaned by the user).*/
      urosTcpRosSendString(tcpstp, &tcpstp->errstr);
      urosStringObjectInit(&tcpstp->errstr);
      goto _finally;
    }
    length = (uint32_t)length_out_srv__std_srvs__Empty(outmsgp);
    if (urosTcpRosSendRaw(tcpstp, length) != UROS_OK) { goto _finally; }
    send_srv__std_srvs__Empty(tcpstp, outmsgp);
    if (tcpstp->err != UROS_OK) { goto _finally; }

    /* Dispose the contents of the response message.*/
    clean_out_srv__std_srvs__Empty(outmsgp);
    if (!tcpstp->topicp->flags.persistent) { break; }
  }
  tcpstp->err = UROS_OK;

_finally:
  /* Service messages deinitialization and deallocation.*/
  clean_in_srv__std_srvs__Empty(inmsgp);
  clean_out_srv__std_srvs__Empty(outmsgp);
  urosFree(inmsgp);
  urosFree(outmsgp);
  return tcpstp->err;
}

/** @} */

/*~~~ PUBLISHED SERVICE: /kill ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Service <tt>/kill</tt> publisher */
/** @{ */

/**
 * @brief   TCPROS <tt>/kill</tt> published service handler.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @return
 *          Error code.
 */
uros_err_t pub_srv__kill(UrosTcpRosStatus *tcpstp) {

  struct in_srv__turtlesim__Kill *inmsgp = NULL;
  struct out_srv__turtlesim__Kill *outmsgp = NULL;
  uint8_t okByte;
  uint32_t length;

  urosAssert(tcpstp != NULL);
  urosAssert(tcpstp->topicp != NULL);
  urosAssert(tcpstp->topicp->flags.service);
  urosAssert(urosConnIsValid(tcpstp->csp));

  /* Service messages allocation and initialization.*/
  inmsgp = urosNew(struct in_srv__turtlesim__Kill);
  if (inmsgp == NULL) { tcpstp->err = UROS_ERR_NOMEM; goto _finally; }
  outmsgp = urosNew(struct out_srv__turtlesim__Kill);
  if (outmsgp == NULL) { tcpstp->err = UROS_ERR_NOMEM; goto _finally; }
  init_in_srv__turtlesim__Kill(inmsgp);
  init_out_srv__turtlesim__Kill(outmsgp);

  /* Service message loop (if the service is persistent).*/
  do {
    turtle_t *turtlep;

    /* Receive the request message.*/
    if (urosTcpRosRecvRaw(tcpstp, length) != UROS_OK) { goto _finally; }
    recv_srv__turtlesim__Kill(tcpstp, inmsgp);
    if (tcpstp->err != UROS_OK) { goto _finally; }
    if ((size_t)length != length_in_srv__turtlesim__Kill(inmsgp)) {
      tcpstp->err = UROS_ERR_BADPARAM; goto _finally;
    }

    /* Process the request message.*/
    tcpstp->err = UROS_OK;
    urosStringClean(&tcpstp->errstr);
    okByte = 1;

    turtlep = turtle_refbyname(&inmsgp->name);
    if (turtlep == NULL) { return UROS_ERR_BADPARAM; }
    turtle_kill(turtlep);
    urosMutexLock(&turtlep->lock);
    turtle_unref(turtlep);
    urosMutexUnlock(&turtlep->lock);

    /* Dispose the contents of the request message.*/
    clean_in_srv__turtlesim__Kill(inmsgp);

    /* Send the response message.*/
    if (urosTcpRosSendRaw(tcpstp, okByte) != UROS_OK) { goto _finally; }
    if (okByte == 0) {
      /* On error, send the tcpstp->errstr error message (cleaned by the user).*/
      urosTcpRosSendString(tcpstp, &tcpstp->errstr);
      urosStringObjectInit(&tcpstp->errstr);
      goto _finally;
    }
    length = (uint32_t)length_out_srv__turtlesim__Kill(outmsgp);
    if (urosTcpRosSendRaw(tcpstp, length) != UROS_OK) { goto _finally; }
    send_srv__turtlesim__Kill(tcpstp, outmsgp);
    if (tcpstp->err != UROS_OK) { goto _finally; }

    /* Dispose the contents of the response message.*/
    clean_out_srv__turtlesim__Kill(outmsgp);
  } while (tcpstp->topicp->flags.persistent && !urosTcpRosStatusCheckExit(tcpstp));
  tcpstp->err = UROS_OK;

_finally:
  /* Service messages deinitialization and deallocation.*/
  clean_in_srv__turtlesim__Kill(inmsgp);
  clean_out_srv__turtlesim__Kill(outmsgp);
  urosFree(inmsgp);
  urosFree(outmsgp);
  return tcpstp->err;
}

/** @} */

/*~~~ PUBLISHED SERVICE: /spawn ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Service <tt>/spawn</tt> publisher */
/** @{ */

/**
 * @brief   TCPROS <tt>/spawn</tt> published service handler.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @return
 *          Error code.
 */
uros_err_t pub_srv__spawn(UrosTcpRosStatus *tcpstp) {

  struct in_srv__turtlesim__Spawn *inmsgp = NULL;
  struct out_srv__turtlesim__Spawn *outmsgp = NULL;
  uint8_t okByte;
  uint32_t length;
  UrosString name;

  urosAssert(tcpstp != NULL);
  urosAssert(tcpstp->topicp != NULL);
  urosAssert(tcpstp->topicp->flags.service);
  urosAssert(urosConnIsValid(tcpstp->csp));

  /* Service messages allocation and initialization.*/
  inmsgp = urosNew(struct in_srv__turtlesim__Spawn);
  if (inmsgp == NULL) { tcpstp->err = UROS_ERR_NOMEM; goto _finally; }
  outmsgp = urosNew(struct out_srv__turtlesim__Spawn);
  if (outmsgp == NULL) { tcpstp->err = UROS_ERR_NOMEM; goto _finally; }
  init_in_srv__turtlesim__Spawn(inmsgp);
  init_out_srv__turtlesim__Spawn(outmsgp);

  /* Service message loop (if the service is persistent).*/
  do {
    turtle_t *turtlep;

    /* Receive the request message.*/
    if (urosTcpRosRecvRaw(tcpstp, length) != UROS_OK) { goto _finally; }
    recv_srv__turtlesim__Spawn(tcpstp, inmsgp);
    if (tcpstp->err != UROS_OK) { goto _finally; }
    if ((size_t)length != length_in_srv__turtlesim__Spawn(inmsgp)) {
      tcpstp->err = UROS_ERR_BADPARAM; goto _finally;
    }

    /* Process the request message.*/
    tcpstp->err = UROS_OK;
    urosStringClean(&tcpstp->errstr);
    okByte = 1;

    /* Spawn the new turtle, if possible.*/
    turtlep = turtle_spawn(&inmsgp->name, inmsgp->x, inmsgp->y, inmsgp->theta);
    if (turtlep == NULL) {
      tcpstp->errstr = inmsgp->name;
      okByte = 0;
    }

    /* Dispose the contents of the request message.*/
    name = inmsgp->name;
    urosStringObjectInit(&inmsgp->name);
    clean_in_srv__turtlesim__Spawn(inmsgp);

    /* Generate the response message.*/
    outmsgp->name = name;

    /* Send the response message.*/
    if (urosTcpRosSendRaw(tcpstp, okByte) != UROS_OK) { goto _finally; }
    if (okByte == 0) {
      /* On error, send the tcpstp->errstr error message (cleaned by the user).*/
      urosTcpRosSendString(tcpstp, &tcpstp->errstr);
      urosStringClean(&tcpstp->errstr);
      goto _finally;
    }
    length = (uint32_t)length_out_srv__turtlesim__Spawn(outmsgp);
    if (urosTcpRosSendRaw(tcpstp, length) != UROS_OK) { goto _finally; }
    send_srv__turtlesim__Spawn(tcpstp, outmsgp);
    if (tcpstp->err != UROS_OK) { goto _finally; }

    /* Dispose the contents of the response message.*/
    clean_out_srv__turtlesim__Spawn(outmsgp);
  } while (tcpstp->topicp->flags.persistent && !urosTcpRosStatusCheckExit(tcpstp));
  tcpstp->err = UROS_OK;

_finally:
  /* Service messages deinitialization and deallocation.*/
  clean_in_srv__turtlesim__Spawn(inmsgp);
  clean_out_srv__turtlesim__Spawn(outmsgp);
  urosFree(inmsgp);
  urosFree(outmsgp);
  return tcpstp->err;
}

/** @} */

/*~~~ PUBLISHED SERVICE: /turtleX/set_pen ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Service <tt>/turtleX/set_pen</tt> publisher */
/** @{ */

/**
 * @brief   TCPROS <tt>/turtleX/set_pen</tt> published service handler.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @return
 *          Error code.
 */
uros_err_t pub_srv__turtleX__set_pen(UrosTcpRosStatus *tcpstp) {

  struct in_srv__turtlesim__SetPen *inmsgp = NULL;
  struct out_srv__turtlesim__SetPen *outmsgp = NULL;
  uint8_t okByte;
  uint32_t length;
  turtle_t *turtlep;

  urosAssert(tcpstp != NULL);
  urosAssert(tcpstp->topicp != NULL);
  urosAssert(tcpstp->topicp->flags.service);
  urosAssert(urosConnIsValid(tcpstp->csp));

  /* Get the turtle slot.*/
  turtlep = turtle_refbypath(&tcpstp->topicp->name);
  if (turtlep == NULL) { return UROS_ERR_BADPARAM; }

  /* Service messages allocation and initialization.*/
  inmsgp = urosNew(struct in_srv__turtlesim__SetPen);
  if (inmsgp == NULL) { tcpstp->err = UROS_ERR_NOMEM; goto _finally; }
  outmsgp = urosNew(struct out_srv__turtlesim__SetPen);
  if (outmsgp == NULL) { tcpstp->err = UROS_ERR_NOMEM; goto _finally; }
  init_in_srv__turtlesim__SetPen(inmsgp);
  init_out_srv__turtlesim__SetPen(outmsgp);

  /* Service message loop (if the service is persistent).*/
  do {
    /* Receive the request message.*/
    if (urosTcpRosRecvRaw(tcpstp, length) != UROS_OK) { goto _finally; }
    recv_srv__turtlesim__SetPen(tcpstp, inmsgp);
    if (tcpstp->err != UROS_OK) { goto _finally; }
    if ((size_t)length != length_in_srv__turtlesim__SetPen(inmsgp)) {
      tcpstp->err = UROS_ERR_BADPARAM; goto _finally;
    }

    /* Process the request message.*/
    tcpstp->err = UROS_OK;
    urosStringClean(&tcpstp->errstr);
    okByte = 1;

    urosMutexLock(&turtlep->lock);
    turtlep->pen.r = inmsgp->r;
    turtlep->pen.g = inmsgp->g;
    turtlep->pen.b = inmsgp->b;
    turtlep->pen.width = inmsgp->width;
    turtlep->pen.off = inmsgp->off;
    urosMutexUnlock(&turtlep->lock);

    /* Dispose the contents of the request message.*/
    clean_in_srv__turtlesim__SetPen(inmsgp);

    /* Send the response message.*/
    if (urosTcpRosSendRaw(tcpstp, okByte) != UROS_OK) { goto _finally; }
    if (okByte == 0) {
      /* On error, send the tcpstp->errstr error message (cleaned by the user).*/
      urosTcpRosSendString(tcpstp, &tcpstp->errstr);
      urosStringObjectInit(&tcpstp->errstr);
      goto _finally;
    }
    length = (uint32_t)length_out_srv__turtlesim__SetPen(outmsgp);
    if (urosTcpRosSendRaw(tcpstp, length) != UROS_OK) { goto _finally; }
    send_srv__turtlesim__SetPen(tcpstp, outmsgp);
    if (tcpstp->err != UROS_OK) { goto _finally; }

    /* Dispose the contents of the response message.*/
    clean_out_srv__turtlesim__SetPen(outmsgp);
  } while (tcpstp->topicp->flags.persistent && !urosTcpRosStatusCheckExit(tcpstp));
  tcpstp->err = UROS_OK;

_finally:
  /* Service messages deinitialization and deallocation.*/
  clean_in_srv__turtlesim__SetPen(inmsgp);
  clean_out_srv__turtlesim__SetPen(outmsgp);
  urosFree(inmsgp);
  urosFree(outmsgp);
  urosMutexLock(&turtlep->lock);
  turtle_unref(turtlep);
  urosMutexUnlock(&turtlep->lock);
  return tcpstp->err;
}

/** @} */

/*~~~ PUBLISHED SERVICE: /turtleX/teleport_absolute ~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Service <tt>/turtleX/teleport_absolute</tt> publisher */
/** @{ */

/**
 * @brief   TCPROS <tt>/turtleX/teleport_absolute</tt> published service handler.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @return
 *          Error code.
 */
uros_err_t pub_srv__turtleX__teleport_absolute(UrosTcpRosStatus *tcpstp) {

  struct in_srv__turtlesim__TeleportAbsolute *inmsgp = NULL;
  struct out_srv__turtlesim__TeleportAbsolute *outmsgp = NULL;
  uint8_t okByte;
  uint32_t length;
  turtle_t *turtlep;
  struct msg__turtlesim__Pose *posep;

  urosAssert(tcpstp != NULL);
  urosAssert(tcpstp->topicp != NULL);
  urosAssert(tcpstp->topicp->flags.service);
  urosAssert(urosConnIsValid(tcpstp->csp));

  /* Get the turtle slot.*/
  turtlep = turtle_refbypath(&tcpstp->topicp->name);
  if (turtlep == NULL) { return UROS_ERR_BADPARAM; }
  posep = (struct msg__turtlesim__Pose *)&turtlep->pose;

  /* Service messages allocation and initialization.*/
  inmsgp = urosNew(struct in_srv__turtlesim__TeleportAbsolute);
  if (inmsgp == NULL) { tcpstp->err = UROS_ERR_NOMEM; goto _finally; }
  outmsgp = urosNew(struct out_srv__turtlesim__TeleportAbsolute);
  if (outmsgp == NULL) { tcpstp->err = UROS_ERR_NOMEM; goto _finally; }
  init_in_srv__turtlesim__TeleportAbsolute(inmsgp);
  init_out_srv__turtlesim__TeleportAbsolute(outmsgp);

  /* Service message loop (if the service is persistent).*/
  do {
    /* Receive the request message.*/
    if (urosTcpRosRecvRaw(tcpstp, length) != UROS_OK) { goto _finally; }
    recv_srv__turtlesim__TeleportAbsolute(tcpstp, inmsgp);
    if (tcpstp->err != UROS_OK) { goto _finally; }
    if ((size_t)length != length_in_srv__turtlesim__TeleportAbsolute(inmsgp)) {
      tcpstp->err = UROS_ERR_BADPARAM; goto _finally;
    }

    /* Process the request message.*/
    tcpstp->err = UROS_OK;
    urosStringClean(&tcpstp->errstr);
    okByte = 1;

    urosMutexLock(&turtlep->lock);
    posep->x = inmsgp->x;
    posep->y = inmsgp->y;
    posep->theta = inmsgp->theta;
    posep->linear_velocity = 0;
    posep->angular_velocity = 0;
    if (posep->x < 0 || posep->x > SANDBOX_WIDTH ||
        posep->y < 0 || posep->y > SANDBOX_WIDTH) {
      UrosString msg = urosStringAssignZ("Turtle outside the sandbox, repositioned");
      rosout_warn(&msg, UROS_TRUE);
    }
    posep->x = min(max(0, posep->x), SANDBOX_WIDTH);
    posep->y = min(max(0, posep->y), SANDBOX_HEIGHT);
    while (posep->theta < 0)         { posep->theta += 2 * M_PI; }
    while (posep->theta >= 2 * M_PI) { posep->theta -= 2 * M_PI; }
    urosMutexUnlock(&turtlep->lock);

    /* Dispose the contents of the request message.*/
    clean_in_srv__turtlesim__TeleportAbsolute(inmsgp);

    /* Send the response message.*/
    if (urosTcpRosSendRaw(tcpstp, okByte) != UROS_OK) { goto _finally; }
    if (okByte == 0) {
      /* On error, send the tcpstp->errstr error message (cleaned by the user).*/
      urosTcpRosSendString(tcpstp, &tcpstp->errstr);
      urosStringObjectInit(&tcpstp->errstr);
      goto _finally;
    }
    length = (uint32_t)length_out_srv__turtlesim__TeleportAbsolute(outmsgp);
    if (urosTcpRosSendRaw(tcpstp, length) != UROS_OK) { goto _finally; }
    send_srv__turtlesim__TeleportAbsolute(tcpstp, outmsgp);
    if (tcpstp->err != UROS_OK) { goto _finally; }

    /* Dispose the contents of the response message.*/
    clean_out_srv__turtlesim__TeleportAbsolute(outmsgp);
  } while (tcpstp->topicp->flags.persistent && !urosTcpRosStatusCheckExit(tcpstp));
  tcpstp->err = UROS_OK;

_finally:
  /* Service messages deinitialization and deallocation.*/
  clean_in_srv__turtlesim__TeleportAbsolute(inmsgp);
  clean_out_srv__turtlesim__TeleportAbsolute(outmsgp);
  urosFree(inmsgp);
  urosFree(outmsgp);
  urosMutexLock(&turtlep->lock);
  turtle_unref(turtlep);
  urosMutexUnlock(&turtlep->lock);
  return tcpstp->err;
}

/** @} */

/*~~~ PUBLISHED SERVICE: /turtleX/teleport_relative ~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Service <tt>/turtleX/teleport_relative</tt> publisher */
/** @{ */

/**
 * @brief   TCPROS <tt>/turtleX/teleport_relative</tt> published service handler.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @return
 *          Error code.
 */
uros_err_t pub_srv__turtleX__teleport_relative(UrosTcpRosStatus *tcpstp) {

  struct in_srv__turtlesim__TeleportRelative *inmsgp = NULL;
  struct out_srv__turtlesim__TeleportRelative *outmsgp = NULL;
  uint8_t okByte;
  uint32_t length;
  turtle_t *turtlep;
  struct msg__turtlesim__Pose *posep;

  urosAssert(tcpstp != NULL);
  urosAssert(tcpstp->topicp != NULL);
  urosAssert(tcpstp->topicp->flags.service);
  urosAssert(urosConnIsValid(tcpstp->csp));

  /* Get the turtle slot.*/
  turtlep = turtle_refbypath(&tcpstp->topicp->name);
  if (turtlep == NULL) { return UROS_ERR_BADPARAM; }
  posep = (struct msg__turtlesim__Pose *)&turtlep->pose;

  /* Service messages allocation and initialization.*/
  inmsgp = urosNew(struct in_srv__turtlesim__TeleportRelative);
  if (inmsgp == NULL) { tcpstp->err = UROS_ERR_NOMEM; goto _finally; }
  outmsgp = urosNew(struct out_srv__turtlesim__TeleportRelative);
  if (outmsgp == NULL) { tcpstp->err = UROS_ERR_NOMEM; goto _finally; }
  init_in_srv__turtlesim__TeleportRelative(inmsgp);
  init_out_srv__turtlesim__TeleportRelative(outmsgp);

  /* Service message loop (if the service is persistent).*/
  do {
    /* Receive the request message.*/
    if (urosTcpRosRecvRaw(tcpstp, length) != UROS_OK) { goto _finally; }
    recv_srv__turtlesim__TeleportRelative(tcpstp, inmsgp);
    if (tcpstp->err != UROS_OK) { goto _finally; }
    if ((size_t)length != length_in_srv__turtlesim__TeleportRelative(inmsgp)) {
      tcpstp->err = UROS_ERR_BADPARAM; goto _finally;
    }

    /* Process the request message.*/
    tcpstp->err = UROS_OK;
    urosStringClean(&tcpstp->errstr);
    okByte = 1;

    urosMutexLock(&turtlep->lock);
    posep->theta += inmsgp->angular;
    while (posep->theta < 0)         { posep->theta += 2 * M_PI; }
    while (posep->theta >= 2 * M_PI) { posep->theta -= 2 * M_PI; }
    posep->x += (float)cos(posep->theta) * inmsgp->linear;
    posep->y += (float)sin(posep->theta) * inmsgp->linear;
    if (posep->x < 0 || posep->x > SANDBOX_WIDTH ||
        posep->y < 0 || posep->y > SANDBOX_WIDTH) {
      UrosString msg = urosStringAssignZ("Turtle hit the wall");
      rosout_warn(&msg, UROS_TRUE);
    }
    posep->x = min(max(0, posep->x), SANDBOX_WIDTH);
    posep->y = min(max(0, posep->y), SANDBOX_HEIGHT);
    posep->linear_velocity = 0;
    posep->angular_velocity = 0;
    urosMutexUnlock(&turtlep->lock);

    /* Dispose the contents of the request message.*/
    clean_in_srv__turtlesim__TeleportRelative(inmsgp);

    /* Send the response message.*/
    if (urosTcpRosSendRaw(tcpstp, okByte) != UROS_OK) { goto _finally; }
    if (okByte == 0) {
      /* On error, send the tcpstp->errstr error message (cleaned by the user).*/
      urosTcpRosSendString(tcpstp, &tcpstp->errstr);
      urosStringObjectInit(&tcpstp->errstr);
      goto _finally;
    }
    length = (uint32_t)length_out_srv__turtlesim__TeleportRelative(outmsgp);
    if (urosTcpRosSendRaw(tcpstp, length) != UROS_OK) { goto _finally; }
    send_srv__turtlesim__TeleportRelative(tcpstp, outmsgp);
    if (tcpstp->err != UROS_OK) { goto _finally; }

    /* Dispose the contents of the response message.*/
    clean_out_srv__turtlesim__TeleportRelative(outmsgp);
  } while (tcpstp->topicp->flags.persistent && !urosTcpRosStatusCheckExit(tcpstp));
  tcpstp->err = UROS_OK;

_finally:
  /* Service messages deinitialization and deallocation.*/
  clean_in_srv__turtlesim__TeleportRelative(inmsgp);
  clean_out_srv__turtlesim__TeleportRelative(outmsgp);
  urosFree(inmsgp);
  urosFree(outmsgp);
  urosMutexLock(&turtlep->lock);
  turtle_unref(turtlep);
  urosMutexUnlock(&turtlep->lock);
  return tcpstp->err;
}

/** @} */

/** @} */

/*============================================================================*/
/* GLOBAL FUNCTIONS                                                           */
/*============================================================================*/

/** @addtogroup tcpros_funcs */
/** @{ */

/**
 * @brief   Registers all the published topics to the Master node.
 * @note    Should be called at node initialization.
 */
void urosTcpRosPublishTopics(void) {

  /* /rosout */
  urosNodePublishTopicSZ(
    "/rosout",
    "rosgraph_msgs/Log",
    (uros_proc_f)pub_tpc__rosout
  );
}

/**
 * @brief   Unregisters all the published topics to the Master node.
 * @note    Should be called at node shutdown.
 */
void urosTcpRosUnpublishTopics(void) {

  /* /rosout */
  urosNodeUnpublishTopicSZ(
    "/rosout"
  );
}

/**
 * @brief   Registers all the subscribed topics to the Master node.
 * @note    Should be called at node initialization.
 */
void urosTcpRosSubscribeTopics(void) {

  /* No topics to subscribe to.*/
}

/**
 * @brief   Unregisters all the subscribed topics to the Master node.
 * @note    Should be called at node shutdown.
 */
void urosTcpRosUnsubscribeTopics(void) {

  /* No topics to unsubscribe from.*/
}

/**
 * @brief   Registers all the published services to the Master node.
 * @note    Should be called at node initialization.
 */
void urosTcpRosPublishServices(void) {

  /* /clear */
  urosNodePublishServiceSZ(
    "/clear",
    "std_srvs/Empty",
    (uros_proc_f)pub_srv__clear
  );

  /* /kill */
  urosNodePublishServiceSZ(
    "/kill",
    "turtlesim/Kill",
    (uros_proc_f)pub_srv__kill
  );

  /* /spawn */
  urosNodePublishServiceSZ(
    "/spawn",
    "turtlesim/Spawn",
    (uros_proc_f)pub_srv__spawn
  );

  /* All the remaining services are turtle-specific.*/
}

/**
 * @brief   Unregisters all the published services to the Master node.
 * @note    Should be called at node shutdown.
 */
void urosTcpRosUnpublishServices(void) {

  /* /clear */
  urosNodeUnpublishServiceSZ(
    "/clear"
  );

  /* /kill */
  urosNodeUnpublishServiceSZ(
    "/kill"
  );

  /* /spawn */
  urosNodeUnpublishServiceSZ(
    "/spawn"
  );

  /* All the remaining services are turtle-specific.*/
}

/** @} */

