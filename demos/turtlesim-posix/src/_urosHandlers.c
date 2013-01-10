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
#include <urosTcpRos.h>
#include <urosUser.h>
#include <math.h>

/*============================================================================*/
/* TYPES & MACROS                                                             */
/*============================================================================*/

#define min(a, b)   (((a) <= (b)) ? (a) : (b))
#define max(a, b)   (((a) >= (b)) ? (a) : (b))

/*============================================================================*/
/* PUBLISHED TOPIC FUNCTIONS                                                  */
/*============================================================================*/

/** @addtogroup tcpros_pubtopic_funcs */
/** @{ */

/*~~~ PUBLISHED TOPIC: /rosout ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Topic <code>/rosout</code> publisher */
/** @{ */

/**
 * @brief   TCPROS <code>/rosout</code> published topic handler.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @return
 *          Error code.
 */
uros_err_t pub_tpc__rosout(UrosTcpRosStatus *tcpstp) {

  uros_bool_t constant = UROS_TRUE;

  /* Message allocation and initialization.*/
  UROS_TPC_PROLOGUE_H(msg__rosgraph_msgs__Log);

  /* Published messages loop.*/
  while (!urosTcpRosStatusCheckExit(tcpstp)) {
    /* Get the next message from the queue.*/
    rosout_fetch(&msgp);
    constant = msgp->header.frame_id.datap[0] != '0';
    msgp->header.frame_id.datap = "0";

    /* Send the message.*/
    UROS_MSG_SEND_LENGTH(msgp, msg__rosgraph_msgs__Log);
    UROS_MSG_SEND_BODY(msgp, msg__rosgraph_msgs__Log);

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

/*~~~ PUBLISHED TOPIC: /turtleX/color_sensor ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Topic <code>/turtleX/color_sensor</code> publisher */
/** @{ */

/**
 * @brief   TCPROS <code>/turtleX/color_sensor</code> published topic handler.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @return
 *          Error code.
 */
uros_err_t pub_tpc__turtleX__color_sensor(UrosTcpRosStatus *tcpstp) {

  /* Message allocation and initialization.*/
  UROS_TPC_PROLOGUE_H(msg__turtlesim__Color);

  /* Published messages loop.*/
  while (!urosTcpRosStatusCheckExit(tcpstp)) {
    /* TODO: Generate the contents of the message.*/
    urosThreadSleepSec(1); continue; /* TODO: Remove this dummy line.*/

    /* Send the message.*/
    UROS_MSG_SEND_LENGTH(msgp, msg__turtlesim__Color);
    UROS_MSG_SEND_BODY(msgp, msg__turtlesim__Color);

    /* Dispose the contents of the message.*/
    clean_msg__turtlesim__Color(msgp);
  }
  tcpstp->err = UROS_OK;

_finally:
  /* Message deinitialization and deallocation.*/
  UROS_TPC_EPILOGUE_H(msg__turtlesim__Color);
  return tcpstp->err;
}

/** @} */

/*~~~ PUBLISHED TOPIC: /turtleX/pose ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Topic <code>/turtleX/pose</code> publisher */
/** @{ */

/**
 * @brief   TCPROS <code>/turtleX/pose</code> published topic handler.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @return
 *          Error code.
 */
uros_err_t pub_tpc__turtleX__pose(UrosTcpRosStatus *tcpstp) {

  turtle_t *turtlep;

  /* Message allocation and initialization.*/
  UROS_TPC_PROLOGUE_H(msg__turtlesim__Pose);

  /* Get the turtle slot.*/
  turtlep = turtle_refbypath(&tcpstp->topicp->name);
  if (turtlep == NULL) { return UROS_ERR_BADPARAM; }

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
    UROS_MSG_SEND_LENGTH(msgp, msg__turtlesim__Pose);
    UROS_MSG_SEND_BODY(msgp, msg__turtlesim__Pose);

    /* Dispose the contents of the message.*/
    clean_msg__turtlesim__Pose(msgp);

    /* Send the next pose every 10ms.*/
    urosThreadSleepMsec(10);
  }
  tcpstp->err = UROS_OK;

_finally:
  /* Message deinitialization and deallocation.*/
  UROS_TPC_EPILOGUE_H(msg__turtlesim__Pose);
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

/** @name Topic <code>/turtleX/command_velocity</code> subscriber */
/** @{ */

/**
 * @brief   TCPROS <code>/turtleX/command_velocity</code> subscribed topic handler.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @return
 *          Error code.
 */
uros_err_t sub_tpc__turtleX__command_velocity(UrosTcpRosStatus *tcpstp) {

  turtle_t *turtlep;

  /* Message allocation and initialization.*/
  UROS_TPC_PROLOGUE_H(msg__turtlesim__Velocity);

  /* Get the turtle slot.*/
  turtlep = turtle_refbypath(&tcpstp->topicp->name);
  if (turtlep == NULL) { return UROS_ERR_BADPARAM; }

  urosMutexLock(&turtlep->lock);
  if (turtlep->status != TURTLE_ALIVE) {
    return tcpstp->err = UROS_OK;
  }
  ++turtlep->refCnt;
  urosMutexUnlock(&turtlep->lock);

  /* Subscribed messages loop.*/
  while (!urosTcpRosStatusCheckExit(tcpstp)) {
    /* Receive the next message.*/
    UROS_MSG_RECV_LENGTH();
    UROS_MSG_RECV_BODY(msgp, msg__turtlesim__Velocity);

    /* Start a new turtle movement for 1s.*/
    urosMutexLock(&turtlep->lock);
    turtlep->pose.linear_velocity = msgp->linear;
    turtlep->pose.angular_velocity = msgp->angular;
    turtlep->countdown = 1000 / TURTLE_THREAD_PERIOD_MS;
    urosMutexUnlock(&turtlep->lock);

    /* Dispose the contents of the message.*/
    clean_msg__turtlesim__Velocity(msgp);
  }
  tcpstp->err = UROS_OK;

_finally:
  /* Message deinitialization and deallocation.*/
  UROS_TPC_EPILOGUE_H(msg__turtlesim__Velocity);
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

/** @name Service <code>/clear</code> publisher */
/** @{ */

/**
 * @brief   TCPROS <code>/clear</code> published service handler.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @return
 *          Error code.
 */
uros_err_t pub_srv__clear(UrosTcpRosStatus *tcpstp) {

  /* Service messages allocation and initialization.*/
  UROS_SRV_PROLOGUE_HIHO(in_srv__std_srvs__Empty,
                         out_srv__std_srvs__Empty);

  /* Service message loop (if the service is persistent).*/
  do {
    /* Receive the request message.*/
    UROS_MSG_RECV_LENGTH();
    UROS_MSG_RECV_BODY(inmsgp, in_srv__std_srvs__Empty);

    /* Process the request message.*/
    tcpstp->err = UROS_OK;
    urosStringClean(&tcpstp->errstr);
    okByte = 1;

    /* Dispose the contents of the request message.*/
    clean_in_srv__std_srvs__Empty(inmsgp);

    /* Send the response message.*/
    UROS_MSG_SEND_LENGTH(outmsgp, out_srv__std_srvs__Empty);
    UROS_SRV_SEND_OKBYTE_ERRSTR();
    UROS_MSG_SEND_BODY(outmsgp, out_srv__std_srvs__Empty);

    /* Dispose the contents of the response message.*/
    clean_out_srv__std_srvs__Empty(outmsgp);
  } while (tcpstp->topicp->flags.persistent && !urosTcpRosStatusCheckExit(tcpstp));
  tcpstp->err = UROS_OK;

_finally:
  /* Service messages deinitialization and deallocation.*/
  UROS_SRV_EPILOGUE_HIHO(in_srv__std_srvs__Empty,
                         out_srv__std_srvs__Empty);
  return tcpstp->err;
}

/** @} */

/*~~~ PUBLISHED SERVICE: /kill ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Service <code>/kill</code> publisher */
/** @{ */

/**
 * @brief   TCPROS <code>/kill</code> published service handler.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @return
 *          Error code.
 */
uros_err_t pub_srv__kill(UrosTcpRosStatus *tcpstp) {

  turtle_t *turtlep;

  /* Service messages allocation and initialization.*/
  UROS_SRV_PROLOGUE_HIHO(in_srv__turtlesim__Kill,
                         out_srv__turtlesim__Kill);

  /* Receive the request message.*/
  UROS_MSG_RECV_LENGTH();
  UROS_MSG_RECV_BODY(inmsgp, in_srv__turtlesim__Kill);

  /* Process the request message.*/
  tcpstp->err = UROS_OK;
  urosStringClean(&tcpstp->errstr);
  okByte = 1;

  /* Kill the turtle.*/
  turtlep = turtle_refbyname(&inmsgp->name);
  if (turtlep == NULL) { return UROS_ERR_BADPARAM; }
  turtle_kill(turtlep);
  urosMutexLock(&turtlep->lock);
  turtle_unref(turtlep);
  urosMutexUnlock(&turtlep->lock);

  /* Dispose the contents of the request message.*/
  clean_in_srv__turtlesim__Kill(inmsgp);

  /* Send the response message.*/
  UROS_MSG_SEND_LENGTH(outmsgp, out_srv__turtlesim__Kill);
  UROS_SRV_SEND_OKBYTE_ERRSTR();
  UROS_MSG_SEND_BODY(outmsgp, out_srv__turtlesim__Kill);

  tcpstp->err = UROS_OK;

_finally:
  /* Service messages deinitialization and deallocation.*/
  UROS_SRV_EPILOGUE_HIHO(in_srv__turtlesim__Kill,
                         out_srv__turtlesim__Kill);
  return tcpstp->err;
}

/** @} */

/*~~~ PUBLISHED SERVICE: /spawn ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Service <code>/spawn</code> publisher */
/** @{ */

/**
 * @brief   TCPROS <code>/spawn</code> published service handler.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @return
 *          Error code.
 */
uros_err_t pub_srv__spawn(UrosTcpRosStatus *tcpstp) {

  turtle_t *turtlep;
  UrosString name;

  /* Service messages allocation and initialization.*/
  UROS_SRV_PROLOGUE_HIHO(in_srv__turtlesim__Spawn,
                         out_srv__turtlesim__Spawn);

  /* Service message loop (if the service is persistent).*/
  do {

    /* Receive the request message.*/
    UROS_MSG_RECV_LENGTH();
    UROS_MSG_RECV_BODY(inmsgp, in_srv__turtlesim__Spawn);

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
    UROS_MSG_SEND_LENGTH(outmsgp, out_srv__turtlesim__Spawn);
    UROS_SRV_SEND_OKBYTE_ERRSTR();
    UROS_MSG_SEND_BODY(outmsgp, out_srv__turtlesim__Spawn);

    /* Dispose the contents of the response message.*/
    clean_out_srv__turtlesim__Spawn(outmsgp);
  } while (tcpstp->topicp->flags.persistent && !urosTcpRosStatusCheckExit(tcpstp));
  tcpstp->err = UROS_OK;

_finally:
  /* Service messages deinitialization and deallocation.*/
  UROS_SRV_EPILOGUE_HIHO(in_srv__turtlesim__Spawn,
                         out_srv__turtlesim__Spawn);
  return tcpstp->err;
}

/** @} */

/*~~~ PUBLISHED SERVICE: /turtleX/set_pen ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Service <code>/turtleX/set_pen</code> publisher */
/** @{ */

/**
 * @brief   TCPROS <code>/turtleX/set_pen</code> published service handler.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @return
 *          Error code.
 */
uros_err_t pub_srv__turtleX__set_pen(UrosTcpRosStatus *tcpstp) {

  turtle_t *turtlep;

  /* Service messages allocation and initialization.*/
  UROS_SRV_PROLOGUE_HIHO(in_srv__turtlesim__SetPen,
                         out_srv__turtlesim__SetPen);

  /* Get the turtle slot.*/
  turtlep = turtle_refbypath(&tcpstp->topicp->name);
  if (turtlep == NULL) { return UROS_ERR_BADPARAM; }

  /* Service message loop (if the service is persistent).*/
  do {
    /* Receive the request message.*/
    UROS_MSG_RECV_LENGTH();
    UROS_MSG_RECV_BODY(inmsgp, in_srv__turtlesim__SetPen);

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

    /* TODO: Generate the contents of the response message.*/

    /* Send the response message.*/
    UROS_MSG_SEND_LENGTH(outmsgp, out_srv__turtlesim__SetPen);
    UROS_SRV_SEND_OKBYTE_ERRSTR();
    UROS_MSG_SEND_BODY(outmsgp, out_srv__turtlesim__SetPen);

    /* Dispose the contents of the response message.*/
    clean_out_srv__turtlesim__SetPen(outmsgp);
  } while (tcpstp->topicp->flags.persistent && !urosTcpRosStatusCheckExit(tcpstp));
  tcpstp->err = UROS_OK;

_finally:
  /* Service messages deinitialization and deallocation.*/
  UROS_SRV_EPILOGUE_HIHO(in_srv__turtlesim__SetPen,
                         out_srv__turtlesim__SetPen);
  urosMutexLock(&turtlep->lock);
  turtle_unref(turtlep);
  urosMutexUnlock(&turtlep->lock);
  return tcpstp->err;
}

/** @} */

/*~~~ PUBLISHED SERVICE: /turtleX/teleport_absolute ~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Service <code>/turtleX/teleport_absolute</code> publisher */
/** @{ */

/**
 * @brief   TCPROS <code>/turtleX/teleport_absolute</code> published service handler.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @return
 *          Error code.
 */
uros_err_t pub_srv__turtleX__teleport_absolute(UrosTcpRosStatus *tcpstp) {

  turtle_t *turtlep;
  struct msg__turtlesim__Pose *posep;

  /* Service messages allocation and initialization.*/
  UROS_SRV_PROLOGUE_HIHO(in_srv__turtlesim__TeleportAbsolute,
                         out_srv__turtlesim__TeleportAbsolute);

  /* Get the turtle slot.*/
  turtlep = turtle_refbypath(&tcpstp->topicp->name);
  if (turtlep == NULL) { return UROS_ERR_BADPARAM; }
  posep = (struct msg__turtlesim__Pose *)&turtlep->pose;

  /* Service message loop (if the service is persistent).*/
  do {
    /* Receive the request message.*/
    UROS_MSG_RECV_LENGTH();
    UROS_MSG_RECV_BODY(inmsgp, in_srv__turtlesim__TeleportAbsolute);

    /* Process the request message.*/
    tcpstp->err = UROS_OK;
    urosStringClean(&tcpstp->errstr);
    okByte = 1;

    /* Set the new pose.*/
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

    /* TODO: Generate the contents of the response message.*/

    /* Send the response message.*/
    UROS_MSG_SEND_LENGTH(outmsgp, out_srv__turtlesim__TeleportAbsolute);
    UROS_SRV_SEND_OKBYTE_ERRSTR();
    UROS_MSG_SEND_BODY(outmsgp, out_srv__turtlesim__TeleportAbsolute);

    /* Dispose the contents of the response message.*/
    clean_out_srv__turtlesim__TeleportAbsolute(outmsgp);
  } while (tcpstp->topicp->flags.persistent && !urosTcpRosStatusCheckExit(tcpstp));
  tcpstp->err = UROS_OK;

_finally:
  /* Service messages deinitialization and deallocation.*/
  UROS_SRV_EPILOGUE_HIHO(in_srv__turtlesim__TeleportAbsolute,
                         out_srv__turtlesim__TeleportAbsolute);
  urosMutexLock(&turtlep->lock);
  turtle_unref(turtlep);
  urosMutexUnlock(&turtlep->lock);
  return tcpstp->err;
}

/** @} */

/*~~~ PUBLISHED SERVICE: /turtleX/teleport_relative ~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Service <code>/turtleX/teleport_relative</code> publisher */
/** @{ */

/**
 * @brief   TCPROS <code>/turtleX/teleport_relative</code> published service handler.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @return
 *          Error code.
 */
uros_err_t pub_srv__turtleX__teleport_relative(UrosTcpRosStatus *tcpstp) {

  turtle_t *turtlep;
  struct msg__turtlesim__Pose *posep;

  /* Service messages allocation and initialization.*/
  UROS_SRV_PROLOGUE_HIHO(in_srv__turtlesim__TeleportRelative,
                         out_srv__turtlesim__TeleportRelative);

  /* Get the turtle slot.*/
  turtlep = turtle_refbypath(&tcpstp->topicp->name);
  if (turtlep == NULL) { return UROS_ERR_BADPARAM; }
  posep = (struct msg__turtlesim__Pose *)&turtlep->pose;

  /* Service message loop (if the service is persistent).*/
  do {
    /* Receive the request message.*/
    UROS_MSG_RECV_LENGTH();
    UROS_MSG_RECV_BODY(inmsgp, in_srv__turtlesim__TeleportRelative);

    /* Process the request message.*/
    tcpstp->err = UROS_OK;
    urosStringClean(&tcpstp->errstr);
    okByte = 1;

    /* Move to the new pose.*/
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

    /* TODO: Generate the contents of the response message.*/

    /* Send the response message.*/
    UROS_MSG_SEND_LENGTH(outmsgp, out_srv__turtlesim__TeleportRelative);
    UROS_SRV_SEND_OKBYTE_ERRSTR();
    UROS_MSG_SEND_BODY(outmsgp, out_srv__turtlesim__TeleportRelative);

    /* Dispose the contents of the response message.*/
    clean_out_srv__turtlesim__TeleportRelative(outmsgp);
  } while (tcpstp->topicp->flags.persistent && !urosTcpRosStatusCheckExit(tcpstp));
  tcpstp->err = UROS_OK;

_finally:
  /* Service messages deinitialization and deallocation.*/
  UROS_SRV_EPILOGUE_HIHO(in_srv__turtlesim__TeleportRelative,
                         out_srv__turtlesim__TeleportRelative);
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
    (uros_proc_f)pub_tpc__rosout,
    uros_nulltopicflags
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
    (uros_proc_f)pub_srv__clear,
    uros_nullserviceflags
  );

  /* /kill */
  urosNodePublishServiceSZ(
    "/kill",
    "turtlesim/Kill",
    (uros_proc_f)pub_srv__kill,
    uros_nullserviceflags
  );

  /* /spawn */
  urosNodePublishServiceSZ(
    "/spawn",
    "turtlesim/Spawn",
    (uros_proc_f)pub_srv__spawn,
    uros_nullserviceflags
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

