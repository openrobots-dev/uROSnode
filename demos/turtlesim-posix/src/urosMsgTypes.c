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
 * @file    urosMsgTypes.c
 * @author  Andrea Zoppi <texzk@email.it>
 *
 * @brief   TCPROS message and service descriptor functions.
 */

/*============================================================================*/
/* HEADER FILES                                                               */
/*============================================================================*/

#include "urosMsgTypes.h"

/*============================================================================*/
/* MESSAGE CONSTANTS                                                          */
/*============================================================================*/

/** @addtogroup tcpros_msg_consts */
/** @{ */

/* There are no message constants.*/

/** @} */

/*============================================================================*/
/* SERVICE CONSTANTS                                                          */
/*============================================================================*/

/** @addtogroup tcpros_srv_consts */
/** @{ */

/* There are no service constants.*/

/** @} */

/*============================================================================*/
/* MESSAGE FUNCTIONS                                                          */
/*============================================================================*/

/** @addtogroup tcpros_msg_funcs */
/** @{ */

/*~~~ MESSAGE: std_msgs/Header ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Message <tt>std_msgs/Header</tt> */
/** @{ */

/**
 * @brief   Content length of a TCPROS <tt>std_msgs/Header</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct msg__std_msgs__Header</code> object.
 * @return
 *          Length of the TCPROS message contents, in bytes.
 */
size_t length_msg__std_msgs__Header(
  struct msg__std_msgs__Header *objp
) {
  size_t length = 0;

  urosAssert(objp != NULL);

  length += sizeof(uint32_t);
  length += sizeof(uros_time_t);
  length += sizeof(uint32_t) + objp->frame_id.length;

  return length;
}

/**
 * @brief   Initializes a TCPROS <tt>std_msgs/Header</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an allocated <code>struct msg__std_msgs__Header</code> object.
 * @return
 *          Error code.
 */
void init_msg__std_msgs__Header(
  struct msg__std_msgs__Header *objp
) {
  urosAssert(objp != NULL);

  urosStringObjectInit(&objp->frame_id);
}

/**
 * @brief   Cleans a TCPROS <tt>std_msgs/Header</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct msg__std_msgs__Header</code> object, or @p NULL.
 * @return
 *          Error code.
 */
void clean_msg__std_msgs__Header(
  struct msg__std_msgs__Header *objp
) {
  if (objp == NULL) { return; }

  urosStringClean(&objp->frame_id);
}

/**
 * @brief   Receives a TCPROS <tt>std_msgs/Header</tt> message.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[out] objp
 *          Pointer to an initialized <code>struct msg__std_msgs__Header</code> object.
 * @return
 *          Error code.
 */
uros_err_t recv_msg__std_msgs__Header(
  UrosTcpRosStatus *tcpstp,
  struct msg__std_msgs__Header *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);
#define _CHKOK { if (tcpstp->err != UROS_OK) { goto _error; } }

  urosTcpRosRecvRaw(tcpstp, objp->seq); _CHKOK
  urosTcpRosRecvRaw(tcpstp, objp->stamp); _CHKOK
  urosTcpRosRecvString(tcpstp, &objp->frame_id); _CHKOK

  return tcpstp->err = UROS_OK;
_error:
  clean_msg__std_msgs__Header(objp);
  return tcpstp->err;
#undef _CHKOK
}

/**
 * @brief   Sends a TCPROS <tt>std_msgs/Header</tt> message.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[in] objp
 *          Pointer to an initialized <code>struct msg__std_msgs__Header</code> object.
 * @return
 *          Error code.
 */
uros_err_t send_msg__std_msgs__Header(
  UrosTcpRosStatus *tcpstp,
  struct msg__std_msgs__Header *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);
#define _CHKOK { if (tcpstp->err != UROS_OK) { return tcpstp->err; } }

  urosTcpRosSendRaw(tcpstp, objp->seq); _CHKOK
  urosTcpRosSendRaw(tcpstp, objp->stamp); _CHKOK
  urosTcpRosSendString(tcpstp, &objp->frame_id); _CHKOK

  return tcpstp->err = UROS_OK;
#undef _CHKOK
}

/** @} */

/*~~~ MESSAGE: rosgraph_msgs/Log ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Message <tt>rosgraph_msgs/Log</tt> */
/** @{ */

/**
 * @brief   Content length of a TCPROS <tt>rosgraph_msgs/Log</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct msg__rosgraph_msgs__Log</code> object.
 * @return
 *          Length of the TCPROS message contents, in bytes.
 */
size_t length_msg__rosgraph_msgs__Log(
  struct msg__rosgraph_msgs__Log *objp
) {
  size_t length = 0;
  uint32_t i;

  urosAssert(objp != NULL);

  length += length_msg__std_msgs__Header(&objp->header);
  length += sizeof(uint8_t);
  length += sizeof(uint32_t) + objp->name.length;
  length += sizeof(uint32_t) + objp->msg.length;
  length += sizeof(uint32_t) + objp->file.length;
  length += sizeof(uint32_t) + objp->function.length;
  length += sizeof(uint32_t);
  length += sizeof(uint32_t);
  length += (size_t)objp->topics.length * sizeof(uint32_t);
  for (i = 0; i < objp->topics.length; ++i) {
    length += objp->topics.entriesp[i].length;
  }

  return length;
}

/**
 * @brief   Initializes a TCPROS <tt>rosgraph_msgs/Log</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an allocated <code>struct msg__rosgraph_msgs__Log</code> object.
 * @return
 *          Error code.
 */
void init_msg__rosgraph_msgs__Log(
  struct msg__rosgraph_msgs__Log *objp
) {
  uint32_t i;

  urosAssert(objp != NULL);

  init_msg__std_msgs__Header(&objp->header);
  urosStringObjectInit(&objp->name);
  urosStringObjectInit(&objp->msg);
  urosStringObjectInit(&objp->file);
  urosStringObjectInit(&objp->function);
  urosTcpRosArrayObjectInit((UrosTcpRosArray *)&objp->topics);
  for (i = 0; i < objp->topics.length; ++i) {
    urosStringObjectInit(&objp->topics.entriesp[i]);
  }
}

/**
 * @brief   Cleans a TCPROS <tt>rosgraph_msgs/Log</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct msg__rosgraph_msgs__Log</code> object, or @p NULL.
 * @return
 *          Error code.
 */
void clean_msg__rosgraph_msgs__Log(
  struct msg__rosgraph_msgs__Log *objp
) {
  uint32_t i;

  if (objp == NULL) { return; }

  clean_msg__std_msgs__Header(&objp->header);
  urosStringClean(&objp->name);
  urosStringClean(&objp->msg);
  urosStringClean(&objp->file);
  urosStringClean(&objp->function);
  for (i = 0; i < objp->topics.length; ++i) {
    urosStringClean(&objp->topics.entriesp[i]);
  }
  urosTcpRosArrayClean((UrosTcpRosArray *)&objp->topics);
}

/**
 * @brief   Receives a TCPROS <tt>rosgraph_msgs/Log</tt> message.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[out] objp
 *          Pointer to an initialized <code>struct msg__rosgraph_msgs__Log</code> object.
 * @return
 *          Error code.
 */
uros_err_t recv_msg__rosgraph_msgs__Log(
  UrosTcpRosStatus *tcpstp,
  struct msg__rosgraph_msgs__Log *objp
) {
  uint32_t i;

  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);
#define _CHKOK { if (tcpstp->err != UROS_OK) { goto _error; } }

  recv_msg__std_msgs__Header(tcpstp, &objp->header); _CHKOK
  urosTcpRosRecvRaw(tcpstp, objp->level); _CHKOK
  urosTcpRosRecvString(tcpstp, &objp->name); _CHKOK
  urosTcpRosRecvString(tcpstp, &objp->msg); _CHKOK
  urosTcpRosRecvString(tcpstp, &objp->file); _CHKOK
  urosTcpRosRecvString(tcpstp, &objp->function); _CHKOK
  urosTcpRosRecvRaw(tcpstp, objp->line); _CHKOK
  urosTcpRosArrayObjectInit((UrosTcpRosArray *)&objp->topics);
  urosTcpRosRecvRaw(tcpstp, objp->topics.length); _CHKOK
  objp->topics.entriesp = urosArrayNew(NULL, objp->topics.length,
                                       UrosString);
  if (objp->topics.entriesp == NULL) { tcpstp->err = UROS_ERR_NOMEM; goto _error; }
  for (i = 0; i < objp->topics.length; ++i) {
    urosTcpRosRecvString(tcpstp, &objp->topics.entriesp[i]); _CHKOK
  }

  return tcpstp->err = UROS_OK;
_error:
  clean_msg__rosgraph_msgs__Log(objp);
  return tcpstp->err;
#undef _CHKOK
}

/**
 * @brief   Sends a TCPROS <tt>rosgraph_msgs/Log</tt> message.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[in] objp
 *          Pointer to an initialized <code>struct msg__rosgraph_msgs__Log</code> object.
 * @return
 *          Error code.
 */
uros_err_t send_msg__rosgraph_msgs__Log(
  UrosTcpRosStatus *tcpstp,
  struct msg__rosgraph_msgs__Log *objp
) {
  uint32_t i;

  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);
#define _CHKOK { if (tcpstp->err != UROS_OK) { return tcpstp->err; } }

  send_msg__std_msgs__Header(tcpstp, &objp->header); _CHKOK
  urosTcpRosSendRaw(tcpstp, objp->level); _CHKOK
  urosTcpRosSendString(tcpstp, &objp->name); _CHKOK
  urosTcpRosSendString(tcpstp, &objp->msg); _CHKOK
  urosTcpRosSendString(tcpstp, &objp->file); _CHKOK
  urosTcpRosSendString(tcpstp, &objp->function); _CHKOK
  urosTcpRosSendRaw(tcpstp, objp->line); _CHKOK
  urosTcpRosSendRaw(tcpstp, objp->topics.length); _CHKOK
  for (i = 0; i < objp->topics.length; ++i) {
    urosTcpRosSendString(tcpstp, &objp->topics.entriesp[i]); _CHKOK
  }

  return tcpstp->err = UROS_OK;
#undef _CHKOK
}

/** @} */

/*~~~ MESSAGE: turtlesim/Pose ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Message <tt>turtlesim/Pose</tt> */
/** @{ */

/**
 * @brief   Content length of a TCPROS <tt>turtlesim/Pose</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct msg__turtlesim__Pose</code> object.
 * @return
 *          Length of the TCPROS message contents, in bytes.
 */
size_t length_msg__turtlesim__Pose(
  struct msg__turtlesim__Pose *objp
) {
  size_t length = 0;

  urosAssert(objp != NULL);

  length += sizeof(float);
  length += sizeof(float);
  length += sizeof(float);
  length += sizeof(float);
  length += sizeof(float);

  (void)objp;
  return length;
}

/**
 * @brief   Initializes a TCPROS <tt>turtlesim/Pose</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an allocated <code>struct msg__turtlesim__Pose</code> object.
 * @return
 *          Error code.
 */
void init_msg__turtlesim__Pose(
  struct msg__turtlesim__Pose *objp
) {
  urosAssert(objp != NULL);

  /* Nothing to initialize.*/
  (void)objp;
}

/**
 * @brief   Cleans a TCPROS <tt>turtlesim/Pose</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct msg__turtlesim__Pose</code> object, or @p NULL.
 * @return
 *          Error code.
 */
void clean_msg__turtlesim__Pose(
  struct msg__turtlesim__Pose *objp
) {
  /* Nothing to clean.*/
  (void)objp;
}

/**
 * @brief   Receives a TCPROS <tt>turtlesim/Pose</tt> message.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[out] objp
 *          Pointer to an initialized <code>struct msg__turtlesim__Pose</code> object.
 * @return
 *          Error code.
 */
uros_err_t recv_msg__turtlesim__Pose(
  UrosTcpRosStatus *tcpstp,
  struct msg__turtlesim__Pose *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);
#define _CHKOK { if (tcpstp->err != UROS_OK) { goto _error; } }

  urosTcpRosRecvRaw(tcpstp, objp->x); _CHKOK
  urosTcpRosRecvRaw(tcpstp, objp->y); _CHKOK
  urosTcpRosRecvRaw(tcpstp, objp->theta); _CHKOK
  urosTcpRosRecvRaw(tcpstp, objp->linear_velocity); _CHKOK
  urosTcpRosRecvRaw(tcpstp, objp->angular_velocity); _CHKOK

  return tcpstp->err = UROS_OK;
_error:
  clean_msg__turtlesim__Pose(objp);
  return tcpstp->err;
#undef _CHKOK
}

/**
 * @brief   Sends a TCPROS <tt>turtlesim/Pose</tt> message.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[in] objp
 *          Pointer to an initialized <code>struct msg__turtlesim__Pose</code> object.
 * @return
 *          Error code.
 */
uros_err_t send_msg__turtlesim__Pose(
  UrosTcpRosStatus *tcpstp,
  struct msg__turtlesim__Pose *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);
#define _CHKOK { if (tcpstp->err != UROS_OK) { return tcpstp->err; } }

  urosTcpRosSendRaw(tcpstp, objp->x); _CHKOK
  urosTcpRosSendRaw(tcpstp, objp->y); _CHKOK
  urosTcpRosSendRaw(tcpstp, objp->theta); _CHKOK
  urosTcpRosSendRaw(tcpstp, objp->linear_velocity); _CHKOK
  urosTcpRosSendRaw(tcpstp, objp->angular_velocity); _CHKOK

  return tcpstp->err = UROS_OK;
#undef _CHKOK
}

/** @} */

/*~~~ MESSAGE: turtlesim/Color ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Message <tt>turtlesim/Color</tt> */
/** @{ */

/**
 * @brief   Content length of a TCPROS <tt>turtlesim/Color</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct msg__turtlesim__Color</code> object.
 * @return
 *          Length of the TCPROS message contents, in bytes.
 */
size_t length_msg__turtlesim__Color(
  struct msg__turtlesim__Color *objp
) {
  size_t length = 0;

  urosAssert(objp != NULL);

  length += sizeof(uint8_t);
  length += sizeof(uint8_t);
  length += sizeof(uint8_t);

  (void)objp;
  return length;
}

/**
 * @brief   Initializes a TCPROS <tt>turtlesim/Color</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an allocated <code>struct msg__turtlesim__Color</code> object.
 * @return
 *          Error code.
 */
void init_msg__turtlesim__Color(
  struct msg__turtlesim__Color *objp
) {
  urosAssert(objp != NULL);

  /* Nothing to initialize.*/
  (void)objp;
}

/**
 * @brief   Cleans a TCPROS <tt>turtlesim/Color</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct msg__turtlesim__Color</code> object, or @p NULL.
 * @return
 *          Error code.
 */
void clean_msg__turtlesim__Color(
  struct msg__turtlesim__Color *objp
) {
  /* Nothing to clean.*/
  (void)objp;
}

/**
 * @brief   Receives a TCPROS <tt>turtlesim/Color</tt> message.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[out] objp
 *          Pointer to an initialized <code>struct msg__turtlesim__Color</code> object.
 * @return
 *          Error code.
 */
uros_err_t recv_msg__turtlesim__Color(
  UrosTcpRosStatus *tcpstp,
  struct msg__turtlesim__Color *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);
#define _CHKOK { if (tcpstp->err != UROS_OK) { goto _error; } }

  urosTcpRosRecvRaw(tcpstp, objp->r); _CHKOK
  urosTcpRosRecvRaw(tcpstp, objp->g); _CHKOK
  urosTcpRosRecvRaw(tcpstp, objp->b); _CHKOK

  return tcpstp->err = UROS_OK;
_error:
  clean_msg__turtlesim__Color(objp);
  return tcpstp->err;
#undef _CHKOK
}

/**
 * @brief   Sends a TCPROS <tt>turtlesim/Color</tt> message.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[in] objp
 *          Pointer to an initialized <code>struct msg__turtlesim__Color</code> object.
 * @return
 *          Error code.
 */
uros_err_t send_msg__turtlesim__Color(
  UrosTcpRosStatus *tcpstp,
  struct msg__turtlesim__Color *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);
#define _CHKOK { if (tcpstp->err != UROS_OK) { return tcpstp->err; } }

  urosTcpRosSendRaw(tcpstp, objp->r); _CHKOK
  urosTcpRosSendRaw(tcpstp, objp->g); _CHKOK
  urosTcpRosSendRaw(tcpstp, objp->b); _CHKOK

  return tcpstp->err = UROS_OK;
#undef _CHKOK
}

/** @} */

/*~~~ MESSAGE: turtlesim/Velocity ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Message <tt>turtlesim/Velocity</tt> */
/** @{ */

/**
 * @brief   Content length of a TCPROS <tt>turtlesim/Velocity</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct msg__turtlesim__Velocity</code> object.
 * @return
 *          Length of the TCPROS message contents, in bytes.
 */
size_t length_msg__turtlesim__Velocity(
  struct msg__turtlesim__Velocity *objp
) {
  size_t length = 0;

  urosAssert(objp != NULL);

  length += sizeof(float);
  length += sizeof(float);

  (void)objp;
  return length;
}

/**
 * @brief   Initializes a TCPROS <tt>turtlesim/Velocity</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an allocated <code>struct msg__turtlesim__Velocity</code> object.
 * @return
 *          Error code.
 */
void init_msg__turtlesim__Velocity(
  struct msg__turtlesim__Velocity *objp
) {
  urosAssert(objp != NULL);

  /* Nothing to initialize.*/
  (void)objp;
}

/**
 * @brief   Cleans a TCPROS <tt>turtlesim/Velocity</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct msg__turtlesim__Velocity</code> object, or @p NULL.
 * @return
 *          Error code.
 */
void clean_msg__turtlesim__Velocity(
  struct msg__turtlesim__Velocity *objp
) {
  /* Nothing to clean.*/
  (void)objp;
}

/**
 * @brief   Receives a TCPROS <tt>turtlesim/Velocity</tt> message.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[out] objp
 *          Pointer to an initialized <code>struct msg__turtlesim__Velocity</code> object.
 * @return
 *          Error code.
 */
uros_err_t recv_msg__turtlesim__Velocity(
  UrosTcpRosStatus *tcpstp,
  struct msg__turtlesim__Velocity *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);
#define _CHKOK { if (tcpstp->err != UROS_OK) { goto _error; } }

  urosTcpRosRecvRaw(tcpstp, objp->linear); _CHKOK
  urosTcpRosRecvRaw(tcpstp, objp->angular); _CHKOK

  return tcpstp->err = UROS_OK;
_error:
  clean_msg__turtlesim__Velocity(objp);
  return tcpstp->err;
#undef _CHKOK
}

/**
 * @brief   Sends a TCPROS <tt>turtlesim/Velocity</tt> message.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[in] objp
 *          Pointer to an initialized <code>struct msg__turtlesim__Velocity</code> object.
 * @return
 *          Error code.
 */
uros_err_t send_msg__turtlesim__Velocity(
  UrosTcpRosStatus *tcpstp,
  struct msg__turtlesim__Velocity *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);
#define _CHKOK { if (tcpstp->err != UROS_OK) { return tcpstp->err; } }

  urosTcpRosSendRaw(tcpstp, objp->linear); _CHKOK
  urosTcpRosSendRaw(tcpstp, objp->angular); _CHKOK

  return tcpstp->err = UROS_OK;
#undef _CHKOK
}

/** @} */

/** @} */

/*============================================================================*/
/* SERVICE FUNCTIONS                                                          */
/*============================================================================*/

/** @addtogroup tcpros_srv_funcs */
/** @{ */

/*~~~ SERVICE: turtlesim/SetPen ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Service <tt>turtlesim/SetPen</tt> */
/** @{ */

/**
 * @brief   Content length of a TCPROS <tt>turtlesim/SetPen</tt> service request.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct srv__turtlesim__SetPen</code> object.
 * @return
 *          Length of the TCPROS service request contents, in bytes.
 */
size_t length_in_srv__turtlesim__SetPen(
  struct in_srv__turtlesim__SetPen *objp
) {
  size_t length = 0;

  urosAssert(objp != NULL);

  length += sizeof(uint8_t);
  length += sizeof(uint8_t);
  length += sizeof(uint8_t);
  length += sizeof(uint8_t);
  length += sizeof(uint8_t);

  (void)objp;
  return length;
}

/**
 * @brief   Initializes a TCPROS <tt>turtlesim/SetPen</tt> service request.
 *
 * @param[in,out] objp
 *          Pointer to an allocated <code>struct in_srv__turtlesim__SetPen</code> object.
 * @return
 *          Error code.
 */
void init_in_srv__turtlesim__SetPen(
  struct in_srv__turtlesim__SetPen *objp
) {
  urosAssert(objp != NULL);

  /* Nothing to initialize.*/
  (void)objp;
}

/**
 * @brief   Cleans a TCPROS <tt>turtlesim/SetPen</tt> service request.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct in_srv__turtlesim__SetPen</code>object.
 * @return
 *          Error code.
 */
void clean_in_srv__turtlesim__SetPen(
  struct in_srv__turtlesim__SetPen *objp
) {
  urosAssert(objp != NULL);

  /* Nothing to clean.*/
  (void)objp;
}

/**
 * @brief   Content length of a TCPROS <tt>turtlesim/SetPen</tt> service response.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct srv__turtlesim__SetPen</code> object.
 * @return
 *          Length of the TCPROS service response contents, in bytes.
 */
size_t length_out_srv__turtlesim__SetPen(
  struct out_srv__turtlesim__SetPen *objp
) {
  size_t length = 0;

  urosAssert(objp != NULL);

  /* Nothing to measure.*/
  (void)objp;
  return length;
}

/**
 * @brief   Initializes a TCPROS <tt>turtlesim/SetPen</tt> service request.
 *
 * @param[in,out] objp
 *          Pointer to an allocated <code>struct out_srv__turtlesim__SetPen</code> object.
 * @return
 *          Error code.
 */
void init_out_srv__turtlesim__SetPen(
  struct out_srv__turtlesim__SetPen *objp
) {
  urosAssert(objp != NULL);

  /* Nothing to initialize.*/
  (void)objp;
}

/**
 * @brief   Cleans a TCPROS <tt>turtlesim/SetPen</tt> service request.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct out_srv__turtlesim__SetPen</code> object.
 * @return
 *          Error code.
 */
void clean_out_srv__turtlesim__SetPen(
  struct out_srv__turtlesim__SetPen *objp
) {
  urosAssert(objp != NULL);

  /* Nothing to clean.*/
  (void)objp;
}

/**
 * @brief   Receives a TCPROS <tt>turtlesim/SetPen</tt> service request.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[out] objp
 *          Pointer to an initialized <code>struct in_srv__turtlesim__SetPen</code> object.
 * @return
 *          Error code.
 */
uros_err_t recv_in_srv__turtlesim__SetPen(
  UrosTcpRosStatus *tcpstp,
  struct in_srv__turtlesim__SetPen *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);
#define _CHKOK { if (tcpstp->err) { goto _error; } }

  urosTcpRosRecvRaw(tcpstp, objp->r); _CHKOK
  urosTcpRosRecvRaw(tcpstp, objp->g); _CHKOK
  urosTcpRosRecvRaw(tcpstp, objp->b); _CHKOK
  urosTcpRosRecvRaw(tcpstp, objp->width); _CHKOK
  urosTcpRosRecvRaw(tcpstp, objp->off); _CHKOK

  return tcpstp->err = UROS_OK;
_error:
  clean_in_srv__turtlesim__SetPen(objp);
  return tcpstp->err;
#undef _CHKOK
}

/**
 * @brief   Sends a TCPROS <tt>turtlesim/SetPen</tt> service response.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[in] objp
 *          Pointer to an initialized <code>struct out_srv__turtlesim__SetPen</code> object.
 * @return
 *          Error code.
 */
uros_err_t send_out_srv__turtlesim__SetPen(
  UrosTcpRosStatus *tcpstp,
  struct out_srv__turtlesim__SetPen *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);

  /* Nothing to send.*/
  (void)objp;
  return tcpstp->err = UROS_OK;
}

/** @} */

/*~~~ SERVICE: turtlesim/Spawn ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Service <tt>turtlesim/Spawn</tt> */
/** @{ */

/**
 * @brief   Content length of a TCPROS <tt>turtlesim/Spawn</tt> service request.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct srv__turtlesim__Spawn</code> object.
 * @return
 *          Length of the TCPROS service request contents, in bytes.
 */
size_t length_in_srv__turtlesim__Spawn(
  struct in_srv__turtlesim__Spawn *objp
) {
  size_t length = 0;

  urosAssert(objp != NULL);

  length += sizeof(float);
  length += sizeof(float);
  length += sizeof(float);
  length += sizeof(uint32_t) + objp->name.length;

  return length;
}

/**
 * @brief   Initializes a TCPROS <tt>turtlesim/Spawn</tt> service request.
 *
 * @param[in,out] objp
 *          Pointer to an allocated <code>struct in_srv__turtlesim__Spawn</code> object.
 * @return
 *          Error code.
 */
void init_in_srv__turtlesim__Spawn(
  struct in_srv__turtlesim__Spawn *objp
) {
  urosAssert(objp != NULL);

  urosStringObjectInit(&objp->name);
}

/**
 * @brief   Cleans a TCPROS <tt>turtlesim/Spawn</tt> service request.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct in_srv__turtlesim__Spawn</code>object.
 * @return
 *          Error code.
 */
void clean_in_srv__turtlesim__Spawn(
  struct in_srv__turtlesim__Spawn *objp
) {
  urosAssert(objp != NULL);

  urosStringClean(&objp->name);
}

/**
 * @brief   Content length of a TCPROS <tt>turtlesim/Spawn</tt> service response.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct srv__turtlesim__Spawn</code> object.
 * @return
 *          Length of the TCPROS service response contents, in bytes.
 */
size_t length_out_srv__turtlesim__Spawn(
  struct out_srv__turtlesim__Spawn *objp
) {
  size_t length = 0;

  urosAssert(objp != NULL);

  length += sizeof(uint32_t) + objp->name.length;

  return length;
}

/**
 * @brief   Initializes a TCPROS <tt>turtlesim/Spawn</tt> service request.
 *
 * @param[in,out] objp
 *          Pointer to an allocated <code>struct out_srv__turtlesim__Spawn</code> object.
 * @return
 *          Error code.
 */
void init_out_srv__turtlesim__Spawn(
  struct out_srv__turtlesim__Spawn *objp
) {
  urosAssert(objp != NULL);

  urosStringObjectInit(&objp->name);
}

/**
 * @brief   Cleans a TCPROS <tt>turtlesim/Spawn</tt> service request.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct out_srv__turtlesim__Spawn</code> object.
 * @return
 *          Error code.
 */
void clean_out_srv__turtlesim__Spawn(
  struct out_srv__turtlesim__Spawn *objp
) {
  urosAssert(objp != NULL);

  urosStringClean(&objp->name);
}

/**
 * @brief   Receives a TCPROS <tt>turtlesim/Spawn</tt> service request.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[out] objp
 *          Pointer to an initialized <code>struct in_srv__turtlesim__Spawn</code> object.
 * @return
 *          Error code.
 */
uros_err_t recv_in_srv__turtlesim__Spawn(
  UrosTcpRosStatus *tcpstp,
  struct in_srv__turtlesim__Spawn *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);
#define _CHKOK { if (tcpstp->err) { goto _error; } }

  urosTcpRosRecvRaw(tcpstp, objp->x); _CHKOK
  urosTcpRosRecvRaw(tcpstp, objp->y); _CHKOK
  urosTcpRosRecvRaw(tcpstp, objp->theta); _CHKOK
  urosTcpRosRecvString(tcpstp, &objp->name); _CHKOK

  return tcpstp->err = UROS_OK;
_error:
  clean_in_srv__turtlesim__Spawn(objp);
  return tcpstp->err;
#undef _CHKOK
}

/**
 * @brief   Sends a TCPROS <tt>turtlesim/Spawn</tt> service response.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[in] objp
 *          Pointer to an initialized <code>struct out_srv__turtlesim__Spawn</code> object.
 * @return
 *          Error code.
 */
uros_err_t send_out_srv__turtlesim__Spawn(
  UrosTcpRosStatus *tcpstp,
  struct out_srv__turtlesim__Spawn *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);
#define _CHKOK { if (tcpstp->err) { return tcpstp->err; } }

  urosTcpRosSendString(tcpstp, &objp->name); _CHKOK

  return tcpstp->err = UROS_OK;
#undef _CHKOK
}

/** @} */

/*~~~ SERVICE: turtlesim/Kill ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Service <tt>turtlesim/Kill</tt> */
/** @{ */

/**
 * @brief   Content length of a TCPROS <tt>turtlesim/Kill</tt> service request.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct srv__turtlesim__Kill</code> object.
 * @return
 *          Length of the TCPROS service request contents, in bytes.
 */
size_t length_in_srv__turtlesim__Kill(
  struct in_srv__turtlesim__Kill *objp
) {
  size_t length = 0;

  urosAssert(objp != NULL);

  length += sizeof(uint32_t) + objp->name.length;

  return length;
}

/**
 * @brief   Initializes a TCPROS <tt>turtlesim/Kill</tt> service request.
 *
 * @param[in,out] objp
 *          Pointer to an allocated <code>struct in_srv__turtlesim__Kill</code> object.
 * @return
 *          Error code.
 */
void init_in_srv__turtlesim__Kill(
  struct in_srv__turtlesim__Kill *objp
) {
  urosAssert(objp != NULL);

  urosStringObjectInit(&objp->name);
}

/**
 * @brief   Cleans a TCPROS <tt>turtlesim/Kill</tt> service request.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct in_srv__turtlesim__Kill</code>object.
 * @return
 *          Error code.
 */
void clean_in_srv__turtlesim__Kill(
  struct in_srv__turtlesim__Kill *objp
) {
  urosAssert(objp != NULL);

  /* Nothing to clean.*/
  (void)objp;
}

/**
 * @brief   Content length of a TCPROS <tt>turtlesim/Kill</tt> service response.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct srv__turtlesim__Kill</code> object.
 * @return
 *          Length of the TCPROS service response contents, in bytes.
 */
size_t length_out_srv__turtlesim__Kill(
  struct out_srv__turtlesim__Kill *objp
) {
  size_t length = 0;

  urosAssert(objp != NULL);

  /* Nothing to measure.*/
  (void)objp;
  return length;
}

/**
 * @brief   Initializes a TCPROS <tt>turtlesim/Kill</tt> service request.
 *
 * @param[in,out] objp
 *          Pointer to an allocated <code>struct out_srv__turtlesim__Kill</code> object.
 * @return
 *          Error code.
 */
void init_out_srv__turtlesim__Kill(
  struct out_srv__turtlesim__Kill *objp
) {
  urosAssert(objp != NULL);

  /* Nothing to initialize.*/
  (void)objp;
}

/**
 * @brief   Cleans a TCPROS <tt>turtlesim/Kill</tt> service request.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct out_srv__turtlesim__Kill</code> object.
 * @return
 *          Error code.
 */
void clean_out_srv__turtlesim__Kill(
  struct out_srv__turtlesim__Kill *objp
) {
  urosAssert(objp != NULL);

  /* Nothing to clean.*/
  (void)objp;
}

/**
 * @brief   Receives a TCPROS <tt>turtlesim/Kill</tt> service request.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[out] objp
 *          Pointer to an initialized <code>struct in_srv__turtlesim__Kill</code> object.
 * @return
 *          Error code.
 */
uros_err_t recv_in_srv__turtlesim__Kill(
  UrosTcpRosStatus *tcpstp,
  struct in_srv__turtlesim__Kill *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);
#define _CHKOK { if (tcpstp->err) { goto _error; } }

  urosTcpRosRecvString(tcpstp, &objp->name); _CHKOK

  return tcpstp->err = UROS_OK;
_error:
  clean_in_srv__turtlesim__Kill(objp);
  return tcpstp->err;
#undef _CHKOK
}

/**
 * @brief   Sends a TCPROS <tt>turtlesim/Kill</tt> service response.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[in] objp
 *          Pointer to an initialized <code>struct out_srv__turtlesim__Kill</code> object.
 * @return
 *          Error code.
 */
uros_err_t send_out_srv__turtlesim__Kill(
  UrosTcpRosStatus *tcpstp,
  struct out_srv__turtlesim__Kill *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);

  /* Nothing to send.*/
  (void)objp;
  return tcpstp->err = UROS_OK;
}

/** @} */

/*~~~ SERVICE: turtlesim/TeleportAbsolute ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Service <tt>turtlesim/TeleportAbsolute</tt> */
/** @{ */

/**
 * @brief   Content length of a TCPROS <tt>turtlesim/TeleportAbsolute</tt> service request.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct srv__turtlesim__TeleportAbsolute</code> object.
 * @return
 *          Length of the TCPROS service request contents, in bytes.
 */
size_t length_in_srv__turtlesim__TeleportAbsolute(
  struct in_srv__turtlesim__TeleportAbsolute *objp
) {
  size_t length = 0;

  urosAssert(objp != NULL);

  length += sizeof(float);
  length += sizeof(float);
  length += sizeof(float);

  (void)objp;
  return length;
}

/**
 * @brief   Initializes a TCPROS <tt>turtlesim/TeleportAbsolute</tt> service request.
 *
 * @param[in,out] objp
 *          Pointer to an allocated <code>struct in_srv__turtlesim__TeleportAbsolute</code> object.
 * @return
 *          Error code.
 */
void init_in_srv__turtlesim__TeleportAbsolute(
  struct in_srv__turtlesim__TeleportAbsolute *objp
) {
  urosAssert(objp != NULL);

  /* Nothing to initialize.*/
  (void)objp;
}

/**
 * @brief   Cleans a TCPROS <tt>turtlesim/TeleportAbsolute</tt> service request.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct in_srv__turtlesim__TeleportAbsolute</code>object.
 * @return
 *          Error code.
 */
void clean_in_srv__turtlesim__TeleportAbsolute(
  struct in_srv__turtlesim__TeleportAbsolute *objp
) {
  urosAssert(objp != NULL);

  /* Nothing to clean.*/
  (void)objp;
}

/**
 * @brief   Content length of a TCPROS <tt>turtlesim/TeleportAbsolute</tt> service response.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct srv__turtlesim__TeleportAbsolute</code> object.
 * @return
 *          Length of the TCPROS service response contents, in bytes.
 */
size_t length_out_srv__turtlesim__TeleportAbsolute(
  struct out_srv__turtlesim__TeleportAbsolute *objp
) {
  size_t length = 0;

  urosAssert(objp != NULL);

  /* Nothing to measure.*/
  (void)objp;
  return length;
}

/**
 * @brief   Initializes a TCPROS <tt>turtlesim/TeleportAbsolute</tt> service request.
 *
 * @param[in,out] objp
 *          Pointer to an allocated <code>struct out_srv__turtlesim__TeleportAbsolute</code> object.
 * @return
 *          Error code.
 */
void init_out_srv__turtlesim__TeleportAbsolute(
  struct out_srv__turtlesim__TeleportAbsolute *objp
) {
  urosAssert(objp != NULL);

  /* Nothing to initialize.*/
  (void)objp;
}

/**
 * @brief   Cleans a TCPROS <tt>turtlesim/TeleportAbsolute</tt> service request.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct out_srv__turtlesim__TeleportAbsolute</code> object.
 * @return
 *          Error code.
 */
void clean_out_srv__turtlesim__TeleportAbsolute(
  struct out_srv__turtlesim__TeleportAbsolute *objp
) {
  urosAssert(objp != NULL);

  /* Nothing to clean.*/
  (void)objp;
}

/**
 * @brief   Receives a TCPROS <tt>turtlesim/TeleportAbsolute</tt> service request.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[out] objp
 *          Pointer to an initialized <code>struct in_srv__turtlesim__TeleportAbsolute</code> object.
 * @return
 *          Error code.
 */
uros_err_t recv_in_srv__turtlesim__TeleportAbsolute(
  UrosTcpRosStatus *tcpstp,
  struct in_srv__turtlesim__TeleportAbsolute *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);
#define _CHKOK { if (tcpstp->err) { goto _error; } }

  urosTcpRosRecvRaw(tcpstp, objp->x); _CHKOK
  urosTcpRosRecvRaw(tcpstp, objp->y); _CHKOK
  urosTcpRosRecvRaw(tcpstp, objp->theta); _CHKOK

  return tcpstp->err = UROS_OK;
_error:
  clean_in_srv__turtlesim__TeleportAbsolute(objp);
  return tcpstp->err;
#undef _CHKOK
}

/**
 * @brief   Sends a TCPROS <tt>turtlesim/TeleportAbsolute</tt> service response.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[in] objp
 *          Pointer to an initialized <code>struct out_srv__turtlesim__TeleportAbsolute</code> object.
 * @return
 *          Error code.
 */
uros_err_t send_out_srv__turtlesim__TeleportAbsolute(
  UrosTcpRosStatus *tcpstp,
  struct out_srv__turtlesim__TeleportAbsolute *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);

  /* Nothing to send.*/
  (void)objp;
  return tcpstp->err = UROS_OK;
}

/** @} */

/*~~~ SERVICE: std_srvs/Empty ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Service <tt>std_srvs/Empty</tt> */
/** @{ */

/**
 * @brief   Content length of a TCPROS <tt>std_srvs/Empty</tt> service request.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct srv__std_srvs__Empty</code> object.
 * @return
 *          Length of the TCPROS service request contents, in bytes.
 */
size_t length_in_srv__std_srvs__Empty(
  struct in_srv__std_srvs__Empty *objp
) {
  size_t length = 0;

  urosAssert(objp != NULL);

  /* Nothing to measure.*/
  (void)objp;
  return length;
}

/**
 * @brief   Initializes a TCPROS <tt>std_srvs/Empty</tt> service request.
 *
 * @param[in,out] objp
 *          Pointer to an allocated <code>struct in_srv__std_srvs__Empty</code> object.
 * @return
 *          Error code.
 */
void init_in_srv__std_srvs__Empty(
  struct in_srv__std_srvs__Empty *objp
) {
  urosAssert(objp != NULL);

  /* Nothing to initialize.*/
  (void)objp;
}

/**
 * @brief   Cleans a TCPROS <tt>std_srvs/Empty</tt> service request.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct in_srv__std_srvs__Empty</code>object.
 * @return
 *          Error code.
 */
void clean_in_srv__std_srvs__Empty(
  struct in_srv__std_srvs__Empty *objp
) {
  urosAssert(objp != NULL);

  /* Nothing to clean.*/
  (void)objp;
}

/**
 * @brief   Content length of a TCPROS <tt>std_srvs/Empty</tt> service response.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct srv__std_srvs__Empty</code> object.
 * @return
 *          Length of the TCPROS service response contents, in bytes.
 */
size_t length_out_srv__std_srvs__Empty(
  struct out_srv__std_srvs__Empty *objp
) {
  size_t length = 0;

  urosAssert(objp != NULL);

  /* Nothing to measure.*/
  (void)objp;
  return length;
}

/**
 * @brief   Initializes a TCPROS <tt>std_srvs/Empty</tt> service request.
 *
 * @param[in,out] objp
 *          Pointer to an allocated <code>struct out_srv__std_srvs__Empty</code> object.
 * @return
 *          Error code.
 */
void init_out_srv__std_srvs__Empty(
  struct out_srv__std_srvs__Empty *objp
) {
  urosAssert(objp != NULL);

  /* Nothing to initialize.*/
  (void)objp;
}

/**
 * @brief   Cleans a TCPROS <tt>std_srvs/Empty</tt> service request.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct out_srv__std_srvs__Empty</code> object.
 * @return
 *          Error code.
 */
void clean_out_srv__std_srvs__Empty(
  struct out_srv__std_srvs__Empty *objp
) {
  urosAssert(objp != NULL);

  /* Nothing to clean.*/
  (void)objp;
}

/**
 * @brief   Receives a TCPROS <tt>std_srvs/Empty</tt> service request.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[out] objp
 *          Pointer to an initialized <code>struct in_srv__std_srvs__Empty</code> object.
 * @return
 *          Error code.
 */
uros_err_t recv_in_srv__std_srvs__Empty(
  UrosTcpRosStatus *tcpstp,
  struct in_srv__std_srvs__Empty *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);

  /* Nothing to receive.*/
  (void)objp;
  return tcpstp->err = UROS_OK;
}

/**
 * @brief   Sends a TCPROS <tt>std_srvs/Empty</tt> service response.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[in] objp
 *          Pointer to an initialized <code>struct out_srv__std_srvs__Empty</code> object.
 * @return
 *          Error code.
 */
uros_err_t send_out_srv__std_srvs__Empty(
  UrosTcpRosStatus *tcpstp,
  struct out_srv__std_srvs__Empty *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);

  /* Nothing to send.*/
  (void)objp;
  return tcpstp->err = UROS_OK;
}

/** @} */

/*~~~ SERVICE: turtlesim/TeleportRelative ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Service <tt>turtlesim/TeleportRelative</tt> */
/** @{ */

/**
 * @brief   Content length of a TCPROS <tt>turtlesim/TeleportRelative</tt> service request.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct srv__turtlesim__TeleportRelative</code> object.
 * @return
 *          Length of the TCPROS service request contents, in bytes.
 */
size_t length_in_srv__turtlesim__TeleportRelative(
  struct in_srv__turtlesim__TeleportRelative *objp
) {
  size_t length = 0;

  urosAssert(objp != NULL);

  length += sizeof(float);
  length += sizeof(float);

  (void)objp;
  return length;
}

/**
 * @brief   Initializes a TCPROS <tt>turtlesim/TeleportRelative</tt> service request.
 *
 * @param[in,out] objp
 *          Pointer to an allocated <code>struct in_srv__turtlesim__TeleportRelative</code> object.
 * @return
 *          Error code.
 */
void init_in_srv__turtlesim__TeleportRelative(
  struct in_srv__turtlesim__TeleportRelative *objp
) {
  urosAssert(objp != NULL);

  /* Nothing to initialize.*/
  (void)objp;
}

/**
 * @brief   Cleans a TCPROS <tt>turtlesim/TeleportRelative</tt> service request.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct in_srv__turtlesim__TeleportRelative</code>object.
 * @return
 *          Error code.
 */
void clean_in_srv__turtlesim__TeleportRelative(
  struct in_srv__turtlesim__TeleportRelative *objp
) {
  urosAssert(objp != NULL);

  /* Nothing to clean.*/
  (void)objp;
}

/**
 * @brief   Content length of a TCPROS <tt>turtlesim/TeleportRelative</tt> service response.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct srv__turtlesim__TeleportRelative</code> object.
 * @return
 *          Length of the TCPROS service response contents, in bytes.
 */
size_t length_out_srv__turtlesim__TeleportRelative(
  struct out_srv__turtlesim__TeleportRelative *objp
) {
  size_t length = 0;

  urosAssert(objp != NULL);

  /* Nothing to measure.*/
  (void)objp;
  return length;
}

/**
 * @brief   Initializes a TCPROS <tt>turtlesim/TeleportRelative</tt> service request.
 *
 * @param[in,out] objp
 *          Pointer to an allocated <code>struct out_srv__turtlesim__TeleportRelative</code> object.
 * @return
 *          Error code.
 */
void init_out_srv__turtlesim__TeleportRelative(
  struct out_srv__turtlesim__TeleportRelative *objp
) {
  urosAssert(objp != NULL);

  /* Nothing to initialize.*/
  (void)objp;
}

/**
 * @brief   Cleans a TCPROS <tt>turtlesim/TeleportRelative</tt> service request.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct out_srv__turtlesim__TeleportRelative</code> object.
 * @return
 *          Error code.
 */
void clean_out_srv__turtlesim__TeleportRelative(
  struct out_srv__turtlesim__TeleportRelative *objp
) {
  urosAssert(objp != NULL);

  /* Nothing to clean.*/
  (void)objp;
}

/**
 * @brief   Receives a TCPROS <tt>turtlesim/TeleportRelative</tt> service request.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[out] objp
 *          Pointer to an initialized <code>struct in_srv__turtlesim__TeleportRelative</code> object.
 * @return
 *          Error code.
 */
uros_err_t recv_in_srv__turtlesim__TeleportRelative(
  UrosTcpRosStatus *tcpstp,
  struct in_srv__turtlesim__TeleportRelative *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);
#define _CHKOK { if (tcpstp->err) { goto _error; } }

  urosTcpRosRecvRaw(tcpstp, objp->linear); _CHKOK
  urosTcpRosRecvRaw(tcpstp, objp->angular); _CHKOK

  return tcpstp->err = UROS_OK;
_error:
  clean_in_srv__turtlesim__TeleportRelative(objp);
  return tcpstp->err;
#undef _CHKOK
}

/**
 * @brief   Sends a TCPROS <tt>turtlesim/TeleportRelative</tt> service response.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[in] objp
 *          Pointer to an initialized <code>struct out_srv__turtlesim__TeleportRelative</code> object.
 * @return
 *          Error code.
 */
uros_err_t send_out_srv__turtlesim__TeleportRelative(
  UrosTcpRosStatus *tcpstp,
  struct out_srv__turtlesim__TeleportRelative *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);

  /* Nothing to send.*/
  (void)objp;
  return tcpstp->err = UROS_OK;
}

/** @} */

/** @} */

/*============================================================================*/
/* GLOBAL FUNCTIONS                                                           */
/*============================================================================*/

/** @addtogroup tcpros_funcs */
/** @{ */

/**
 * @brief   Static TCPROS types registration.
 * @details Statically registers all the TCPROS message and service types used
 *          within this source file.
 * @note    Should be called by @p urosUserRegisterStaticMsgTypes().
 * @see     urosUserRegisterStaticMsgTypes()
 */
void urosMsgTypesRegStaticTypes(void) {

  /* MESSAGE TYPES */

  /* rosgraph_msgs/Log */
  urosRegisterStaticMsgTypeSZ("rosgraph_msgs/Log",
                              NULL, "acffd30cd6b6de30f120938c17c593fb");

  /* std_msgs/Header */
  urosRegisterStaticMsgTypeSZ("std_msgs/Header",
                              NULL, "2176decaecbce78abc3b96ef049fabed");

  /* turtlesim/Color */
  urosRegisterStaticMsgTypeSZ("turtlesim/Color",
                              NULL, "353891e354491c51aabe32df673fb446");

  /* turtlesim/Pose */
  urosRegisterStaticMsgTypeSZ("turtlesim/Pose",
                              NULL, "863b248d5016ca62ea2e895ae5265cf9");

  /* turtlesim/Velocity */
  urosRegisterStaticMsgTypeSZ("turtlesim/Velocity",
                              NULL, "9d5c2dcd348ac8f76ce2a4307bd63a13");

  /* SERVICE TYPES */

  /* std_srvs/Empty */
  urosRegisterStaticSrvTypeSZ("std_srvs/Empty",
                              NULL, "d41d8cd98f00b204e9800998ecf8427e");

  /* turtlesim/Kill */
  urosRegisterStaticSrvTypeSZ("turtlesim/Kill",
                              NULL, "c1f3d28f1b044c871e6eff2e9fc3c667");

  /* turtlesim/SetPen */
  urosRegisterStaticSrvTypeSZ("turtlesim/SetPen",
                              NULL, "9f452acce566bf0c0954594f69a8e41b");

  /* turtlesim/Spawn */
  urosRegisterStaticSrvTypeSZ("turtlesim/Spawn",
                              NULL, "0b2d2e872a8e2887d5ed626f2bf2c561");

  /* turtlesim/TeleportAbsolute */
  urosRegisterStaticSrvTypeSZ("turtlesim/TeleportAbsolute",
                              NULL, "a130bc60ee6513855dc62ea83fcc5b20");

  /* turtlesim/TeleportRelative */
  urosRegisterStaticSrvTypeSZ("turtlesim/TeleportRelative",
                              NULL, "9d5c2dcd348ac8f76ce2a4307bd63a13");
}

/** @} */

