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
 * @author  Your Name <your.email@example.com>
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

/*~~~ MESSAGE: bond/Constants ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Message <tt>bond/Constants</tt> */
/** @{ */

/** @brief TODO: <tt>bond/Constants.DISABLE_HEARTBEAT_TIMEOUT_PARAM</tt> description.*/
const UrosString msg__bond__Constants__DISABLE_HEARTBEAT_TIMEOUT_PARAM = 
  { 31, "/bond_disable_heartbeat_timeout" };

/** @} */

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

/*~~~ MESSAGE: dynamic_reconfigure/IntParameter ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Message <tt>dynamic_reconfigure/IntParameter</tt> */
/** @{ */

/**
 * @brief   Content length of a TCPROS <tt>dynamic_reconfigure/IntParameter</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct msg__dynamic_reconfigure__IntParameter</code> object.
 * @return
 *          Length of the TCPROS message contents, in bytes.
 */
size_t length_msg__dynamic_reconfigure__IntParameter(
  struct msg__dynamic_reconfigure__IntParameter *objp
) {
  size_t length = 0;

  urosAssert(objp != NULL);

  length += sizeof(uint32_t) + objp->name.length;
  length += sizeof(int32_t);

  return length;
}

/**
 * @brief   Initializes a TCPROS <tt>dynamic_reconfigure/IntParameter</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an allocated <code>struct msg__dynamic_reconfigure__IntParameter</code> object.
 * @return
 *          Error code.
 */
void init_msg__dynamic_reconfigure__IntParameter(
  struct msg__dynamic_reconfigure__IntParameter *objp
) {
  urosAssert(objp != NULL);

  urosStringObjectInit(&objp->name);
}

/**
 * @brief   Cleans a TCPROS <tt>dynamic_reconfigure/IntParameter</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct msg__dynamic_reconfigure__IntParameter</code> object, or @p NULL.
 * @return
 *          Error code.
 */
void clean_msg__dynamic_reconfigure__IntParameter(
  struct msg__dynamic_reconfigure__IntParameter *objp
) {
  if (objp == NULL) { return; }

  urosStringClean(&objp->name);
}

/**
 * @brief   Receives a TCPROS <tt>dynamic_reconfigure/IntParameter</tt> message.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[out] objp
 *          Pointer to an initialized <code>struct msg__dynamic_reconfigure__IntParameter</code> object.
 * @return
 *          Error code.
 */
uros_err_t recv_msg__dynamic_reconfigure__IntParameter(
  UrosTcpRosStatus *tcpstp,
  struct msg__dynamic_reconfigure__IntParameter *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);
#define _CHKOK { if (tcpstp->err != UROS_OK) { goto _error; } }

  urosTcpRosRecvString(tcpstp, &objp->name); _CHKOK
  urosTcpRosRecvRaw(tcpstp, objp->value); _CHKOK

  return tcpstp->err = UROS_OK;
_error:
  clean_msg__dynamic_reconfigure__IntParameter(objp);
  return tcpstp->err;
#undef _CHKOK
}

/**
 * @brief   Sends a TCPROS <tt>dynamic_reconfigure/IntParameter</tt> message.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[in] objp
 *          Pointer to an initialized <code>struct msg__dynamic_reconfigure__IntParameter</code> object.
 * @return
 *          Error code.
 */
uros_err_t send_msg__dynamic_reconfigure__IntParameter(
  UrosTcpRosStatus *tcpstp,
  struct msg__dynamic_reconfigure__IntParameter *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);
#define _CHKOK { if (tcpstp->err != UROS_OK) { return tcpstp->err; } }

  urosTcpRosSendString(tcpstp, &objp->name); _CHKOK
  urosTcpRosSendRaw(tcpstp, objp->value); _CHKOK

  return tcpstp->err = UROS_OK;
#undef _CHKOK
}

/** @} */

/*~~~ MESSAGE: dynamic_reconfigure/GroupState ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Message <tt>dynamic_reconfigure/GroupState</tt> */
/** @{ */

/**
 * @brief   Content length of a TCPROS <tt>dynamic_reconfigure/GroupState</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct msg__dynamic_reconfigure__GroupState</code> object.
 * @return
 *          Length of the TCPROS message contents, in bytes.
 */
size_t length_msg__dynamic_reconfigure__GroupState(
  struct msg__dynamic_reconfigure__GroupState *objp
) {
  size_t length = 0;

  urosAssert(objp != NULL);

  length += sizeof(uint32_t) + objp->name.length;
  length += sizeof(uint8_t);
  length += sizeof(int32_t);
  length += sizeof(int32_t);

  return length;
}

/**
 * @brief   Initializes a TCPROS <tt>dynamic_reconfigure/GroupState</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an allocated <code>struct msg__dynamic_reconfigure__GroupState</code> object.
 * @return
 *          Error code.
 */
void init_msg__dynamic_reconfigure__GroupState(
  struct msg__dynamic_reconfigure__GroupState *objp
) {
  urosAssert(objp != NULL);

  urosStringObjectInit(&objp->name);
}

/**
 * @brief   Cleans a TCPROS <tt>dynamic_reconfigure/GroupState</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct msg__dynamic_reconfigure__GroupState</code> object, or @p NULL.
 * @return
 *          Error code.
 */
void clean_msg__dynamic_reconfigure__GroupState(
  struct msg__dynamic_reconfigure__GroupState *objp
) {
  if (objp == NULL) { return; }

  urosStringClean(&objp->name);
}

/**
 * @brief   Receives a TCPROS <tt>dynamic_reconfigure/GroupState</tt> message.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[out] objp
 *          Pointer to an initialized <code>struct msg__dynamic_reconfigure__GroupState</code> object.
 * @return
 *          Error code.
 */
uros_err_t recv_msg__dynamic_reconfigure__GroupState(
  UrosTcpRosStatus *tcpstp,
  struct msg__dynamic_reconfigure__GroupState *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);
#define _CHKOK { if (tcpstp->err != UROS_OK) { goto _error; } }

  urosTcpRosRecvString(tcpstp, &objp->name); _CHKOK
  urosTcpRosRecvRaw(tcpstp, objp->state); _CHKOK
  urosTcpRosRecvRaw(tcpstp, objp->id); _CHKOK
  urosTcpRosRecvRaw(tcpstp, objp->parent); _CHKOK

  return tcpstp->err = UROS_OK;
_error:
  clean_msg__dynamic_reconfigure__GroupState(objp);
  return tcpstp->err;
#undef _CHKOK
}

/**
 * @brief   Sends a TCPROS <tt>dynamic_reconfigure/GroupState</tt> message.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[in] objp
 *          Pointer to an initialized <code>struct msg__dynamic_reconfigure__GroupState</code> object.
 * @return
 *          Error code.
 */
uros_err_t send_msg__dynamic_reconfigure__GroupState(
  UrosTcpRosStatus *tcpstp,
  struct msg__dynamic_reconfigure__GroupState *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);
#define _CHKOK { if (tcpstp->err != UROS_OK) { return tcpstp->err; } }

  urosTcpRosSendString(tcpstp, &objp->name); _CHKOK
  urosTcpRosSendRaw(tcpstp, objp->state); _CHKOK
  urosTcpRosSendRaw(tcpstp, objp->id); _CHKOK
  urosTcpRosSendRaw(tcpstp, objp->parent); _CHKOK

  return tcpstp->err = UROS_OK;
#undef _CHKOK
}

/** @} */

/*~~~ MESSAGE: dynamic_reconfigure/BoolParameter ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Message <tt>dynamic_reconfigure/BoolParameter</tt> */
/** @{ */

/**
 * @brief   Content length of a TCPROS <tt>dynamic_reconfigure/BoolParameter</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct msg__dynamic_reconfigure__BoolParameter</code> object.
 * @return
 *          Length of the TCPROS message contents, in bytes.
 */
size_t length_msg__dynamic_reconfigure__BoolParameter(
  struct msg__dynamic_reconfigure__BoolParameter *objp
) {
  size_t length = 0;

  urosAssert(objp != NULL);

  length += sizeof(uint32_t) + objp->name.length;
  length += sizeof(uint8_t);

  return length;
}

/**
 * @brief   Initializes a TCPROS <tt>dynamic_reconfigure/BoolParameter</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an allocated <code>struct msg__dynamic_reconfigure__BoolParameter</code> object.
 * @return
 *          Error code.
 */
void init_msg__dynamic_reconfigure__BoolParameter(
  struct msg__dynamic_reconfigure__BoolParameter *objp
) {
  urosAssert(objp != NULL);

  urosStringObjectInit(&objp->name);
}

/**
 * @brief   Cleans a TCPROS <tt>dynamic_reconfigure/BoolParameter</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct msg__dynamic_reconfigure__BoolParameter</code> object, or @p NULL.
 * @return
 *          Error code.
 */
void clean_msg__dynamic_reconfigure__BoolParameter(
  struct msg__dynamic_reconfigure__BoolParameter *objp
) {
  if (objp == NULL) { return; }

  urosStringClean(&objp->name);
}

/**
 * @brief   Receives a TCPROS <tt>dynamic_reconfigure/BoolParameter</tt> message.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[out] objp
 *          Pointer to an initialized <code>struct msg__dynamic_reconfigure__BoolParameter</code> object.
 * @return
 *          Error code.
 */
uros_err_t recv_msg__dynamic_reconfigure__BoolParameter(
  UrosTcpRosStatus *tcpstp,
  struct msg__dynamic_reconfigure__BoolParameter *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);
#define _CHKOK { if (tcpstp->err != UROS_OK) { goto _error; } }

  urosTcpRosRecvString(tcpstp, &objp->name); _CHKOK
  urosTcpRosRecvRaw(tcpstp, objp->value); _CHKOK

  return tcpstp->err = UROS_OK;
_error:
  clean_msg__dynamic_reconfigure__BoolParameter(objp);
  return tcpstp->err;
#undef _CHKOK
}

/**
 * @brief   Sends a TCPROS <tt>dynamic_reconfigure/BoolParameter</tt> message.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[in] objp
 *          Pointer to an initialized <code>struct msg__dynamic_reconfigure__BoolParameter</code> object.
 * @return
 *          Error code.
 */
uros_err_t send_msg__dynamic_reconfigure__BoolParameter(
  UrosTcpRosStatus *tcpstp,
  struct msg__dynamic_reconfigure__BoolParameter *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);
#define _CHKOK { if (tcpstp->err != UROS_OK) { return tcpstp->err; } }

  urosTcpRosSendString(tcpstp, &objp->name); _CHKOK
  urosTcpRosSendRaw(tcpstp, objp->value); _CHKOK

  return tcpstp->err = UROS_OK;
#undef _CHKOK
}

/** @} */

/*~~~ MESSAGE: dynamic_reconfigure/DoubleParameter ~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Message <tt>dynamic_reconfigure/DoubleParameter</tt> */
/** @{ */

/**
 * @brief   Content length of a TCPROS <tt>dynamic_reconfigure/DoubleParameter</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct msg__dynamic_reconfigure__DoubleParameter</code> object.
 * @return
 *          Length of the TCPROS message contents, in bytes.
 */
size_t length_msg__dynamic_reconfigure__DoubleParameter(
  struct msg__dynamic_reconfigure__DoubleParameter *objp
) {
  size_t length = 0;

  urosAssert(objp != NULL);

  length += sizeof(uint32_t) + objp->name.length;
  length += sizeof(double);

  return length;
}

/**
 * @brief   Initializes a TCPROS <tt>dynamic_reconfigure/DoubleParameter</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an allocated <code>struct msg__dynamic_reconfigure__DoubleParameter</code> object.
 * @return
 *          Error code.
 */
void init_msg__dynamic_reconfigure__DoubleParameter(
  struct msg__dynamic_reconfigure__DoubleParameter *objp
) {
  urosAssert(objp != NULL);

  urosStringObjectInit(&objp->name);
}

/**
 * @brief   Cleans a TCPROS <tt>dynamic_reconfigure/DoubleParameter</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct msg__dynamic_reconfigure__DoubleParameter</code> object, or @p NULL.
 * @return
 *          Error code.
 */
void clean_msg__dynamic_reconfigure__DoubleParameter(
  struct msg__dynamic_reconfigure__DoubleParameter *objp
) {
  if (objp == NULL) { return; }

  urosStringClean(&objp->name);
}

/**
 * @brief   Receives a TCPROS <tt>dynamic_reconfigure/DoubleParameter</tt> message.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[out] objp
 *          Pointer to an initialized <code>struct msg__dynamic_reconfigure__DoubleParameter</code> object.
 * @return
 *          Error code.
 */
uros_err_t recv_msg__dynamic_reconfigure__DoubleParameter(
  UrosTcpRosStatus *tcpstp,
  struct msg__dynamic_reconfigure__DoubleParameter *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);
#define _CHKOK { if (tcpstp->err != UROS_OK) { goto _error; } }

  urosTcpRosRecvString(tcpstp, &objp->name); _CHKOK
  urosTcpRosRecvRaw(tcpstp, objp->value); _CHKOK

  return tcpstp->err = UROS_OK;
_error:
  clean_msg__dynamic_reconfigure__DoubleParameter(objp);
  return tcpstp->err;
#undef _CHKOK
}

/**
 * @brief   Sends a TCPROS <tt>dynamic_reconfigure/DoubleParameter</tt> message.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[in] objp
 *          Pointer to an initialized <code>struct msg__dynamic_reconfigure__DoubleParameter</code> object.
 * @return
 *          Error code.
 */
uros_err_t send_msg__dynamic_reconfigure__DoubleParameter(
  UrosTcpRosStatus *tcpstp,
  struct msg__dynamic_reconfigure__DoubleParameter *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);
#define _CHKOK { if (tcpstp->err != UROS_OK) { return tcpstp->err; } }

  urosTcpRosSendString(tcpstp, &objp->name); _CHKOK
  urosTcpRosSendRaw(tcpstp, objp->value); _CHKOK

  return tcpstp->err = UROS_OK;
#undef _CHKOK
}

/** @} */

/*~~~ MESSAGE: dynamic_reconfigure/StrParameter ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Message <tt>dynamic_reconfigure/StrParameter</tt> */
/** @{ */

/**
 * @brief   Content length of a TCPROS <tt>dynamic_reconfigure/StrParameter</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct msg__dynamic_reconfigure__StrParameter</code> object.
 * @return
 *          Length of the TCPROS message contents, in bytes.
 */
size_t length_msg__dynamic_reconfigure__StrParameter(
  struct msg__dynamic_reconfigure__StrParameter *objp
) {
  size_t length = 0;

  urosAssert(objp != NULL);

  length += sizeof(uint32_t) + objp->name.length;
  length += sizeof(uint32_t) + objp->value.length;

  return length;
}

/**
 * @brief   Initializes a TCPROS <tt>dynamic_reconfigure/StrParameter</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an allocated <code>struct msg__dynamic_reconfigure__StrParameter</code> object.
 * @return
 *          Error code.
 */
void init_msg__dynamic_reconfigure__StrParameter(
  struct msg__dynamic_reconfigure__StrParameter *objp
) {
  urosAssert(objp != NULL);

  urosStringObjectInit(&objp->name);
  urosStringObjectInit(&objp->value);
}

/**
 * @brief   Cleans a TCPROS <tt>dynamic_reconfigure/StrParameter</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct msg__dynamic_reconfigure__StrParameter</code> object, or @p NULL.
 * @return
 *          Error code.
 */
void clean_msg__dynamic_reconfigure__StrParameter(
  struct msg__dynamic_reconfigure__StrParameter *objp
) {
  if (objp == NULL) { return; }

  urosStringClean(&objp->name);
  urosStringClean(&objp->value);
}

/**
 * @brief   Receives a TCPROS <tt>dynamic_reconfigure/StrParameter</tt> message.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[out] objp
 *          Pointer to an initialized <code>struct msg__dynamic_reconfigure__StrParameter</code> object.
 * @return
 *          Error code.
 */
uros_err_t recv_msg__dynamic_reconfigure__StrParameter(
  UrosTcpRosStatus *tcpstp,
  struct msg__dynamic_reconfigure__StrParameter *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);
#define _CHKOK { if (tcpstp->err != UROS_OK) { goto _error; } }

  urosTcpRosRecvString(tcpstp, &objp->name); _CHKOK
  urosTcpRosRecvString(tcpstp, &objp->value); _CHKOK

  return tcpstp->err = UROS_OK;
_error:
  clean_msg__dynamic_reconfigure__StrParameter(objp);
  return tcpstp->err;
#undef _CHKOK
}

/**
 * @brief   Sends a TCPROS <tt>dynamic_reconfigure/StrParameter</tt> message.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[in] objp
 *          Pointer to an initialized <code>struct msg__dynamic_reconfigure__StrParameter</code> object.
 * @return
 *          Error code.
 */
uros_err_t send_msg__dynamic_reconfigure__StrParameter(
  UrosTcpRosStatus *tcpstp,
  struct msg__dynamic_reconfigure__StrParameter *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);
#define _CHKOK { if (tcpstp->err != UROS_OK) { return tcpstp->err; } }

  urosTcpRosSendString(tcpstp, &objp->name); _CHKOK
  urosTcpRosSendString(tcpstp, &objp->value); _CHKOK

  return tcpstp->err = UROS_OK;
#undef _CHKOK
}

/** @} */

/*~~~ MESSAGE: sensor_msgs/RegionOfInterest ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Message <tt>sensor_msgs/RegionOfInterest</tt> */
/** @{ */

/**
 * @brief   Content length of a TCPROS <tt>sensor_msgs/RegionOfInterest</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct msg__sensor_msgs__RegionOfInterest</code> object.
 * @return
 *          Length of the TCPROS message contents, in bytes.
 */
size_t length_msg__sensor_msgs__RegionOfInterest(
  struct msg__sensor_msgs__RegionOfInterest *objp
) {
  size_t length = 0;

  urosAssert(objp != NULL);

  length += sizeof(uint32_t);
  length += sizeof(uint32_t);
  length += sizeof(uint32_t);
  length += sizeof(uint32_t);
  length += sizeof(uint8_t);

  (void)objp;
  return length;
}

/**
 * @brief   Initializes a TCPROS <tt>sensor_msgs/RegionOfInterest</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an allocated <code>struct msg__sensor_msgs__RegionOfInterest</code> object.
 * @return
 *          Error code.
 */
void init_msg__sensor_msgs__RegionOfInterest(
  struct msg__sensor_msgs__RegionOfInterest *objp
) {
  urosAssert(objp != NULL);

  /* Nothing to initialize.*/
  (void)objp;
}

/**
 * @brief   Cleans a TCPROS <tt>sensor_msgs/RegionOfInterest</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct msg__sensor_msgs__RegionOfInterest</code> object, or @p NULL.
 * @return
 *          Error code.
 */
void clean_msg__sensor_msgs__RegionOfInterest(
  struct msg__sensor_msgs__RegionOfInterest *objp
) {
  /* Nothing to clean.*/
  (void)objp;
}

/**
 * @brief   Receives a TCPROS <tt>sensor_msgs/RegionOfInterest</tt> message.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[out] objp
 *          Pointer to an initialized <code>struct msg__sensor_msgs__RegionOfInterest</code> object.
 * @return
 *          Error code.
 */
uros_err_t recv_msg__sensor_msgs__RegionOfInterest(
  UrosTcpRosStatus *tcpstp,
  struct msg__sensor_msgs__RegionOfInterest *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);
#define _CHKOK { if (tcpstp->err != UROS_OK) { goto _error; } }

  urosTcpRosRecvRaw(tcpstp, objp->x_offset); _CHKOK
  urosTcpRosRecvRaw(tcpstp, objp->y_offset); _CHKOK
  urosTcpRosRecvRaw(tcpstp, objp->height); _CHKOK
  urosTcpRosRecvRaw(tcpstp, objp->width); _CHKOK
  urosTcpRosRecvRaw(tcpstp, objp->do_rectify); _CHKOK

  return tcpstp->err = UROS_OK;
_error:
  clean_msg__sensor_msgs__RegionOfInterest(objp);
  return tcpstp->err;
#undef _CHKOK
}

/**
 * @brief   Sends a TCPROS <tt>sensor_msgs/RegionOfInterest</tt> message.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[in] objp
 *          Pointer to an initialized <code>struct msg__sensor_msgs__RegionOfInterest</code> object.
 * @return
 *          Error code.
 */
uros_err_t send_msg__sensor_msgs__RegionOfInterest(
  UrosTcpRosStatus *tcpstp,
  struct msg__sensor_msgs__RegionOfInterest *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);
#define _CHKOK { if (tcpstp->err != UROS_OK) { return tcpstp->err; } }

  urosTcpRosSendRaw(tcpstp, objp->x_offset); _CHKOK
  urosTcpRosSendRaw(tcpstp, objp->y_offset); _CHKOK
  urosTcpRosSendRaw(tcpstp, objp->height); _CHKOK
  urosTcpRosSendRaw(tcpstp, objp->width); _CHKOK
  urosTcpRosSendRaw(tcpstp, objp->do_rectify); _CHKOK

  return tcpstp->err = UROS_OK;
#undef _CHKOK
}

/** @} */

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

/*~~~ MESSAGE: dynamic_reconfigure/Config ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Message <tt>dynamic_reconfigure/Config</tt> */
/** @{ */

/**
 * @brief   Content length of a TCPROS <tt>dynamic_reconfigure/Config</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct msg__dynamic_reconfigure__Config</code> object.
 * @return
 *          Length of the TCPROS message contents, in bytes.
 */
size_t length_msg__dynamic_reconfigure__Config(
  struct msg__dynamic_reconfigure__Config *objp
) {
  size_t length = 0;
  uint32_t i;

  urosAssert(objp != NULL);

  length += sizeof(uint32_t);
  for (i = 0; i < objp->bools.length; ++i) {
    length += length_msg__dynamic_reconfigure__BoolParameter(&objp->bools.entriesp[i]);
  }
  length += sizeof(uint32_t);
  for (i = 0; i < objp->ints.length; ++i) {
    length += length_msg__dynamic_reconfigure__IntParameter(&objp->ints.entriesp[i]);
  }
  length += sizeof(uint32_t);
  for (i = 0; i < objp->strs.length; ++i) {
    length += length_msg__dynamic_reconfigure__StrParameter(&objp->strs.entriesp[i]);
  }
  length += sizeof(uint32_t);
  for (i = 0; i < objp->doubles.length; ++i) {
    length += length_msg__dynamic_reconfigure__DoubleParameter(&objp->doubles.entriesp[i]);
  }
  length += sizeof(uint32_t);
  for (i = 0; i < objp->groups.length; ++i) {
    length += length_msg__dynamic_reconfigure__GroupState(&objp->groups.entriesp[i]);
  }

  return length;
}

/**
 * @brief   Initializes a TCPROS <tt>dynamic_reconfigure/Config</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an allocated <code>struct msg__dynamic_reconfigure__Config</code> object.
 * @return
 *          Error code.
 */
void init_msg__dynamic_reconfigure__Config(
  struct msg__dynamic_reconfigure__Config *objp
) {
  uint32_t i;

  urosAssert(objp != NULL);

  urosTcpRosArrayObjectInit((UrosTcpRosArray *)&objp->bools);
  for (i = 0; i < objp->bools.length; ++i) {
    init_msg__dynamic_reconfigure__BoolParameter(&objp->bools.entriesp[i]);
  }
  urosTcpRosArrayObjectInit((UrosTcpRosArray *)&objp->ints);
  for (i = 0; i < objp->ints.length; ++i) {
    init_msg__dynamic_reconfigure__IntParameter(&objp->ints.entriesp[i]);
  }
  urosTcpRosArrayObjectInit((UrosTcpRosArray *)&objp->strs);
  for (i = 0; i < objp->strs.length; ++i) {
    init_msg__dynamic_reconfigure__StrParameter(&objp->strs.entriesp[i]);
  }
  urosTcpRosArrayObjectInit((UrosTcpRosArray *)&objp->doubles);
  for (i = 0; i < objp->doubles.length; ++i) {
    init_msg__dynamic_reconfigure__DoubleParameter(&objp->doubles.entriesp[i]);
  }
  urosTcpRosArrayObjectInit((UrosTcpRosArray *)&objp->groups);
  for (i = 0; i < objp->groups.length; ++i) {
    init_msg__dynamic_reconfigure__GroupState(&objp->groups.entriesp[i]);
  }
}

/**
 * @brief   Cleans a TCPROS <tt>dynamic_reconfigure/Config</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct msg__dynamic_reconfigure__Config</code> object, or @p NULL.
 * @return
 *          Error code.
 */
void clean_msg__dynamic_reconfigure__Config(
  struct msg__dynamic_reconfigure__Config *objp
) {
  uint32_t i;

  if (objp == NULL) { return; }

  for (i = 0; i < objp->bools.length; ++i) {
    clean_msg__dynamic_reconfigure__BoolParameter(&objp->bools.entriesp[i]);
  }
  urosTcpRosArrayClean((UrosTcpRosArray *)&objp->bools);
  for (i = 0; i < objp->ints.length; ++i) {
    clean_msg__dynamic_reconfigure__IntParameter(&objp->ints.entriesp[i]);
  }
  urosTcpRosArrayClean((UrosTcpRosArray *)&objp->ints);
  for (i = 0; i < objp->strs.length; ++i) {
    clean_msg__dynamic_reconfigure__StrParameter(&objp->strs.entriesp[i]);
  }
  urosTcpRosArrayClean((UrosTcpRosArray *)&objp->strs);
  for (i = 0; i < objp->doubles.length; ++i) {
    clean_msg__dynamic_reconfigure__DoubleParameter(&objp->doubles.entriesp[i]);
  }
  urosTcpRosArrayClean((UrosTcpRosArray *)&objp->doubles);
  for (i = 0; i < objp->groups.length; ++i) {
    clean_msg__dynamic_reconfigure__GroupState(&objp->groups.entriesp[i]);
  }
  urosTcpRosArrayClean((UrosTcpRosArray *)&objp->groups);
}

/**
 * @brief   Receives a TCPROS <tt>dynamic_reconfigure/Config</tt> message.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[out] objp
 *          Pointer to an initialized <code>struct msg__dynamic_reconfigure__Config</code> object.
 * @return
 *          Error code.
 */
uros_err_t recv_msg__dynamic_reconfigure__Config(
  UrosTcpRosStatus *tcpstp,
  struct msg__dynamic_reconfigure__Config *objp
) {
  uint32_t i;

  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);
#define _CHKOK { if (tcpstp->err != UROS_OK) { goto _error; } }

  urosTcpRosArrayObjectInit((UrosTcpRosArray *)&objp->bools);
  urosTcpRosRecvRaw(tcpstp, objp->bools.length); _CHKOK
  objp->bools.entriesp = urosArrayNew(NULL, objp->bools.length,
                                      struct msg__dynamic_reconfigure__BoolParameter);
  if (objp->bools.entriesp == NULL) { tcpstp->err = UROS_ERR_NOMEM; goto _error; }
  for (i = 0; i < objp->bools.length; ++i) {
    recv_msg__dynamic_reconfigure__BoolParameter(tcpstp, &objp->bools.entriesp[i]); _CHKOK
  }
  urosTcpRosArrayObjectInit((UrosTcpRosArray *)&objp->ints);
  urosTcpRosRecvRaw(tcpstp, objp->ints.length); _CHKOK
  objp->ints.entriesp = urosArrayNew(NULL, objp->ints.length,
                                     struct msg__dynamic_reconfigure__IntParameter);
  if (objp->ints.entriesp == NULL) { tcpstp->err = UROS_ERR_NOMEM; goto _error; }
  for (i = 0; i < objp->ints.length; ++i) {
    recv_msg__dynamic_reconfigure__IntParameter(tcpstp, &objp->ints.entriesp[i]); _CHKOK
  }
  urosTcpRosArrayObjectInit((UrosTcpRosArray *)&objp->strs);
  urosTcpRosRecvRaw(tcpstp, objp->strs.length); _CHKOK
  objp->strs.entriesp = urosArrayNew(NULL, objp->strs.length,
                                     struct msg__dynamic_reconfigure__StrParameter);
  if (objp->strs.entriesp == NULL) { tcpstp->err = UROS_ERR_NOMEM; goto _error; }
  for (i = 0; i < objp->strs.length; ++i) {
    recv_msg__dynamic_reconfigure__StrParameter(tcpstp, &objp->strs.entriesp[i]); _CHKOK
  }
  urosTcpRosArrayObjectInit((UrosTcpRosArray *)&objp->doubles);
  urosTcpRosRecvRaw(tcpstp, objp->doubles.length); _CHKOK
  objp->doubles.entriesp = urosArrayNew(NULL, objp->doubles.length,
                                        struct msg__dynamic_reconfigure__DoubleParameter);
  if (objp->doubles.entriesp == NULL) { tcpstp->err = UROS_ERR_NOMEM; goto _error; }
  for (i = 0; i < objp->doubles.length; ++i) {
    recv_msg__dynamic_reconfigure__DoubleParameter(tcpstp, &objp->doubles.entriesp[i]); _CHKOK
  }
  urosTcpRosArrayObjectInit((UrosTcpRosArray *)&objp->groups);
  urosTcpRosRecvRaw(tcpstp, objp->groups.length); _CHKOK
  objp->groups.entriesp = urosArrayNew(NULL, objp->groups.length,
                                       struct msg__dynamic_reconfigure__GroupState);
  if (objp->groups.entriesp == NULL) { tcpstp->err = UROS_ERR_NOMEM; goto _error; }
  for (i = 0; i < objp->groups.length; ++i) {
    recv_msg__dynamic_reconfigure__GroupState(tcpstp, &objp->groups.entriesp[i]); _CHKOK
  }

  return tcpstp->err = UROS_OK;
_error:
  clean_msg__dynamic_reconfigure__Config(objp);
  return tcpstp->err;
#undef _CHKOK
}

/**
 * @brief   Sends a TCPROS <tt>dynamic_reconfigure/Config</tt> message.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[in] objp
 *          Pointer to an initialized <code>struct msg__dynamic_reconfigure__Config</code> object.
 * @return
 *          Error code.
 */
uros_err_t send_msg__dynamic_reconfigure__Config(
  UrosTcpRosStatus *tcpstp,
  struct msg__dynamic_reconfigure__Config *objp
) {
  uint32_t i;

  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);
#define _CHKOK { if (tcpstp->err != UROS_OK) { return tcpstp->err; } }

  urosTcpRosSendRaw(tcpstp, objp->bools.length); _CHKOK
  for (i = 0; i < objp->bools.length; ++i) {
    send_msg__dynamic_reconfigure__BoolParameter(tcpstp, &objp->bools.entriesp[i]); _CHKOK
  }
  urosTcpRosSendRaw(tcpstp, objp->ints.length); _CHKOK
  for (i = 0; i < objp->ints.length; ++i) {
    send_msg__dynamic_reconfigure__IntParameter(tcpstp, &objp->ints.entriesp[i]); _CHKOK
  }
  urosTcpRosSendRaw(tcpstp, objp->strs.length); _CHKOK
  for (i = 0; i < objp->strs.length; ++i) {
    send_msg__dynamic_reconfigure__StrParameter(tcpstp, &objp->strs.entriesp[i]); _CHKOK
  }
  urosTcpRosSendRaw(tcpstp, objp->doubles.length); _CHKOK
  for (i = 0; i < objp->doubles.length; ++i) {
    send_msg__dynamic_reconfigure__DoubleParameter(tcpstp, &objp->doubles.entriesp[i]); _CHKOK
  }
  urosTcpRosSendRaw(tcpstp, objp->groups.length); _CHKOK
  for (i = 0; i < objp->groups.length; ++i) {
    send_msg__dynamic_reconfigure__GroupState(tcpstp, &objp->groups.entriesp[i]); _CHKOK
  }

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

/*~~~ MESSAGE: sensor_msgs/Image ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Message <tt>sensor_msgs/Image</tt> */
/** @{ */

/**
 * @brief   Content length of a TCPROS <tt>sensor_msgs/Image</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct msg__sensor_msgs__Image</code> object.
 * @return
 *          Length of the TCPROS message contents, in bytes.
 */
size_t length_msg__sensor_msgs__Image(
  struct msg__sensor_msgs__Image *objp
) {
  size_t length = 0;

  urosAssert(objp != NULL);

  length += length_msg__std_msgs__Header(&objp->header);
  length += sizeof(uint32_t);
  length += sizeof(uint32_t);
  length += sizeof(uint32_t) + objp->encoding.length;
  length += sizeof(uint8_t);
  length += sizeof(uint32_t);
  length += sizeof(uint32_t);
  length += (size_t)objp->data.length * sizeof(uint8_t);

  return length;
}

/**
 * @brief   Initializes a TCPROS <tt>sensor_msgs/Image</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an allocated <code>struct msg__sensor_msgs__Image</code> object.
 * @return
 *          Error code.
 */
void init_msg__sensor_msgs__Image(
  struct msg__sensor_msgs__Image *objp
) {
  urosAssert(objp != NULL);

  init_msg__std_msgs__Header(&objp->header);
  urosStringObjectInit(&objp->encoding);
  urosTcpRosArrayObjectInit((UrosTcpRosArray *)&objp->data);
}

/**
 * @brief   Cleans a TCPROS <tt>sensor_msgs/Image</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct msg__sensor_msgs__Image</code> object, or @p NULL.
 * @return
 *          Error code.
 */
void clean_msg__sensor_msgs__Image(
  struct msg__sensor_msgs__Image *objp
) {
  if (objp == NULL) { return; }

  clean_msg__std_msgs__Header(&objp->header);
  urosStringClean(&objp->encoding);
  urosTcpRosArrayClean((UrosTcpRosArray *)&objp->data);
}

/**
 * @brief   Receives a TCPROS <tt>sensor_msgs/Image</tt> message.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[out] objp
 *          Pointer to an initialized <code>struct msg__sensor_msgs__Image</code> object.
 * @return
 *          Error code.
 */
uros_err_t recv_msg__sensor_msgs__Image(
  UrosTcpRosStatus *tcpstp,
  struct msg__sensor_msgs__Image *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);
#define _CHKOK { if (tcpstp->err != UROS_OK) { goto _error; } }

  recv_msg__std_msgs__Header(tcpstp, &objp->header); _CHKOK
  urosTcpRosRecvRaw(tcpstp, objp->height); _CHKOK
  urosTcpRosRecvRaw(tcpstp, objp->width); _CHKOK
  urosTcpRosRecvString(tcpstp, &objp->encoding); _CHKOK
  urosTcpRosRecvRaw(tcpstp, objp->is_bigendian); _CHKOK
  urosTcpRosRecvRaw(tcpstp, objp->step); _CHKOK
  urosTcpRosArrayObjectInit((UrosTcpRosArray *)&objp->data);
  urosTcpRosRecvRaw(tcpstp, objp->data.length); _CHKOK
  objp->data.entriesp = urosArrayNew(NULL, objp->data.length,
                                     uint8_t);
  if (objp->data.entriesp == NULL) { tcpstp->err = UROS_ERR_NOMEM; goto _error; }
  urosTcpRosRecv(tcpstp, objp->data.entriesp,
                 (size_t)objp->data.length * sizeof(uint8_t)); _CHKOK

  return tcpstp->err = UROS_OK;
_error:
  clean_msg__sensor_msgs__Image(objp);
  return tcpstp->err;
#undef _CHKOK
}

/**
 * @brief   Sends a TCPROS <tt>sensor_msgs/Image</tt> message.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[in] objp
 *          Pointer to an initialized <code>struct msg__sensor_msgs__Image</code> object.
 * @return
 *          Error code.
 */
uros_err_t send_msg__sensor_msgs__Image(
  UrosTcpRosStatus *tcpstp,
  struct msg__sensor_msgs__Image *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);
#define _CHKOK { if (tcpstp->err != UROS_OK) { return tcpstp->err; } }

  send_msg__std_msgs__Header(tcpstp, &objp->header); _CHKOK
  urosTcpRosSendRaw(tcpstp, objp->height); _CHKOK
  urosTcpRosSendRaw(tcpstp, objp->width); _CHKOK
  urosTcpRosSendString(tcpstp, &objp->encoding); _CHKOK
  urosTcpRosSendRaw(tcpstp, objp->is_bigendian); _CHKOK
  urosTcpRosSendRaw(tcpstp, objp->step); _CHKOK
  urosTcpRosSendRaw(tcpstp, objp->data.length); _CHKOK
  urosTcpRosSend(tcpstp, objp->data.entriesp,
                 (size_t)objp->data.length * sizeof(uint8_t)); _CHKOK

  return tcpstp->err = UROS_OK;
#undef _CHKOK
}

/** @} */

/*~~~ MESSAGE: stereo_msgs/DisparityImage ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Message <tt>stereo_msgs/DisparityImage</tt> */
/** @{ */

/**
 * @brief   Content length of a TCPROS <tt>stereo_msgs/DisparityImage</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct msg__stereo_msgs__DisparityImage</code> object.
 * @return
 *          Length of the TCPROS message contents, in bytes.
 */
size_t length_msg__stereo_msgs__DisparityImage(
  struct msg__stereo_msgs__DisparityImage *objp
) {
  size_t length = 0;

  urosAssert(objp != NULL);

  length += length_msg__std_msgs__Header(&objp->header);
  length += length_msg__sensor_msgs__Image(&objp->image);
  length += sizeof(float);
  length += sizeof(float);
  length += length_msg__sensor_msgs__RegionOfInterest(&objp->valid_window);
  length += sizeof(float);
  length += sizeof(float);
  length += sizeof(float);

  return length;
}

/**
 * @brief   Initializes a TCPROS <tt>stereo_msgs/DisparityImage</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an allocated <code>struct msg__stereo_msgs__DisparityImage</code> object.
 * @return
 *          Error code.
 */
void init_msg__stereo_msgs__DisparityImage(
  struct msg__stereo_msgs__DisparityImage *objp
) {
  urosAssert(objp != NULL);

  init_msg__std_msgs__Header(&objp->header);
  init_msg__sensor_msgs__Image(&objp->image);
  init_msg__sensor_msgs__RegionOfInterest(&objp->valid_window);
}

/**
 * @brief   Cleans a TCPROS <tt>stereo_msgs/DisparityImage</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct msg__stereo_msgs__DisparityImage</code> object, or @p NULL.
 * @return
 *          Error code.
 */
void clean_msg__stereo_msgs__DisparityImage(
  struct msg__stereo_msgs__DisparityImage *objp
) {
  if (objp == NULL) { return; }

  clean_msg__std_msgs__Header(&objp->header);
  clean_msg__sensor_msgs__Image(&objp->image);
  clean_msg__sensor_msgs__RegionOfInterest(&objp->valid_window);
}

/**
 * @brief   Receives a TCPROS <tt>stereo_msgs/DisparityImage</tt> message.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[out] objp
 *          Pointer to an initialized <code>struct msg__stereo_msgs__DisparityImage</code> object.
 * @return
 *          Error code.
 */
uros_err_t recv_msg__stereo_msgs__DisparityImage(
  UrosTcpRosStatus *tcpstp,
  struct msg__stereo_msgs__DisparityImage *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);
#define _CHKOK { if (tcpstp->err != UROS_OK) { goto _error; } }

  recv_msg__std_msgs__Header(tcpstp, &objp->header); _CHKOK
  recv_msg__sensor_msgs__Image(tcpstp, &objp->image); _CHKOK
  urosTcpRosRecvRaw(tcpstp, objp->f); _CHKOK
  urosTcpRosRecvRaw(tcpstp, objp->T); _CHKOK
  recv_msg__sensor_msgs__RegionOfInterest(tcpstp, &objp->valid_window); _CHKOK
  urosTcpRosRecvRaw(tcpstp, objp->min_disparity); _CHKOK
  urosTcpRosRecvRaw(tcpstp, objp->max_disparity); _CHKOK
  urosTcpRosRecvRaw(tcpstp, objp->delta_d); _CHKOK

  return tcpstp->err = UROS_OK;
_error:
  clean_msg__stereo_msgs__DisparityImage(objp);
  return tcpstp->err;
#undef _CHKOK
}

/**
 * @brief   Sends a TCPROS <tt>stereo_msgs/DisparityImage</tt> message.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[in] objp
 *          Pointer to an initialized <code>struct msg__stereo_msgs__DisparityImage</code> object.
 * @return
 *          Error code.
 */
uros_err_t send_msg__stereo_msgs__DisparityImage(
  UrosTcpRosStatus *tcpstp,
  struct msg__stereo_msgs__DisparityImage *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);
#define _CHKOK { if (tcpstp->err != UROS_OK) { return tcpstp->err; } }

  send_msg__std_msgs__Header(tcpstp, &objp->header); _CHKOK
  send_msg__sensor_msgs__Image(tcpstp, &objp->image); _CHKOK
  urosTcpRosSendRaw(tcpstp, objp->f); _CHKOK
  urosTcpRosSendRaw(tcpstp, objp->T); _CHKOK
  send_msg__sensor_msgs__RegionOfInterest(tcpstp, &objp->valid_window); _CHKOK
  urosTcpRosSendRaw(tcpstp, objp->min_disparity); _CHKOK
  urosTcpRosSendRaw(tcpstp, objp->max_disparity); _CHKOK
  urosTcpRosSendRaw(tcpstp, objp->delta_d); _CHKOK

  return tcpstp->err = UROS_OK;
#undef _CHKOK
}

/** @} */

/*~~~ MESSAGE: bond/Constants ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Message <tt>bond/Constants</tt> */
/** @{ */

/**
 * @brief   Content length of a TCPROS <tt>bond/Constants</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct msg__bond__Constants</code> object.
 * @return
 *          Length of the TCPROS message contents, in bytes.
 */
size_t length_msg__bond__Constants(
  struct msg__bond__Constants *objp
) {
  size_t length = 0;

  urosAssert(objp != NULL);

  /* Nothing to measure.*/
  (void)objp;
  return length;
}

/**
 * @brief   Initializes a TCPROS <tt>bond/Constants</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an allocated <code>struct msg__bond__Constants</code> object.
 * @return
 *          Error code.
 */
void init_msg__bond__Constants(
  struct msg__bond__Constants *objp
) {
  urosAssert(objp != NULL);

  /* Nothing to initialize.*/
  (void)objp;
}

/**
 * @brief   Cleans a TCPROS <tt>bond/Constants</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct msg__bond__Constants</code> object, or @p NULL.
 * @return
 *          Error code.
 */
void clean_msg__bond__Constants(
  struct msg__bond__Constants *objp
) {
  /* Nothing to clean.*/
  (void)objp;
}

/**
 * @brief   Receives a TCPROS <tt>bond/Constants</tt> message.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[out] objp
 *          Pointer to an initialized <code>struct msg__bond__Constants</code> object.
 * @return
 *          Error code.
 */
uros_err_t recv_msg__bond__Constants(
  UrosTcpRosStatus *tcpstp,
  struct msg__bond__Constants *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);

  /* Nothing to receive.*/
  (void)objp;
  return tcpstp->err = UROS_OK;
}

/**
 * @brief   Sends a TCPROS <tt>bond/Constants</tt> message.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[in] objp
 *          Pointer to an initialized <code>struct msg__bond__Constants</code> object.
 * @return
 *          Error code.
 */
uros_err_t send_msg__bond__Constants(
  UrosTcpRosStatus *tcpstp,
  struct msg__bond__Constants *objp
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
/* SERVICE FUNCTIONS                                                          */
/*============================================================================*/

/** @addtogroup tcpros_srv_funcs */
/** @{ */

/*~~~ SERVICE: dynamic_reconfigure/Reconfigure ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Service <tt>dynamic_reconfigure/Reconfigure</tt> */
/** @{ */

/**
 * @brief   Content length of a TCPROS <tt>dynamic_reconfigure/Reconfigure</tt> service request.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct srv__dynamic_reconfigure__Reconfigure</code> object.
 * @return
 *          Length of the TCPROS service request contents, in bytes.
 */
size_t length_in_srv__dynamic_reconfigure__Reconfigure(
  struct in_srv__dynamic_reconfigure__Reconfigure *objp
) {
  size_t length = 0;

  urosAssert(objp != NULL);

  length += length_msg__dynamic_reconfigure__Config(&objp->config);

  return length;
}

/**
 * @brief   Initializes a TCPROS <tt>dynamic_reconfigure/Reconfigure</tt> service request.
 *
 * @param[in,out] objp
 *          Pointer to an allocated <code>struct in_srv__dynamic_reconfigure__Reconfigure</code> object.
 * @return
 *          Error code.
 */
void init_in_srv__dynamic_reconfigure__Reconfigure(
  struct in_srv__dynamic_reconfigure__Reconfigure *objp
) {
  urosAssert(objp != NULL);

  init_msg__dynamic_reconfigure__Config(&objp->config);
}

/**
 * @brief   Cleans a TCPROS <tt>dynamic_reconfigure/Reconfigure</tt> service request.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct in_srv__dynamic_reconfigure__Reconfigure</code>object.
 * @return
 *          Error code.
 */
void clean_in_srv__dynamic_reconfigure__Reconfigure(
  struct in_srv__dynamic_reconfigure__Reconfigure *objp
) {
  urosAssert(objp != NULL);

  clean_msg__dynamic_reconfigure__Config(&objp->config);
}

/**
 * @brief   Content length of a TCPROS <tt>dynamic_reconfigure/Reconfigure</tt> service response.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct srv__dynamic_reconfigure__Reconfigure</code> object.
 * @return
 *          Length of the TCPROS service response contents, in bytes.
 */
size_t length_out_srv__dynamic_reconfigure__Reconfigure(
  struct out_srv__dynamic_reconfigure__Reconfigure *objp
) {
  size_t length = 0;

  urosAssert(objp != NULL);

  length += length_msg__dynamic_reconfigure__Config(&objp->config);

  return length;
}

/**
 * @brief   Initializes a TCPROS <tt>dynamic_reconfigure/Reconfigure</tt> service request.
 *
 * @param[in,out] objp
 *          Pointer to an allocated <code>struct out_srv__dynamic_reconfigure__Reconfigure</code> object.
 * @return
 *          Error code.
 */
void init_out_srv__dynamic_reconfigure__Reconfigure(
  struct out_srv__dynamic_reconfigure__Reconfigure *objp
) {
  urosAssert(objp != NULL);

  init_msg__dynamic_reconfigure__Config(&objp->config);
}

/**
 * @brief   Cleans a TCPROS <tt>dynamic_reconfigure/Reconfigure</tt> service request.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct out_srv__dynamic_reconfigure__Reconfigure</code> object.
 * @return
 *          Error code.
 */
void clean_out_srv__dynamic_reconfigure__Reconfigure(
  struct out_srv__dynamic_reconfigure__Reconfigure *objp
) {
  urosAssert(objp != NULL);

  clean_msg__dynamic_reconfigure__Config(&objp->config);
}

/**
 * @brief   Receives a TCPROS <tt>dynamic_reconfigure/Reconfigure</tt> service request.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[out] objp
 *          Pointer to an initialized <code>struct in_srv__dynamic_reconfigure__Reconfigure</code> object.
 * @return
 *          Error code.
 */
uros_err_t recv_in_srv__dynamic_reconfigure__Reconfigure(
  UrosTcpRosStatus *tcpstp,
  struct in_srv__dynamic_reconfigure__Reconfigure *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);
#define _CHKOK { if (tcpstp->err) { goto _error; } }

  recv_msg__dynamic_reconfigure__Config(tcpstp, &objp->config); _CHKOK

  return tcpstp->err = UROS_OK;
_error:
  clean_in_srv__dynamic_reconfigure__Reconfigure(objp);
  return tcpstp->err;
#undef _CHKOK
}

/**
 * @brief   Sends a TCPROS <tt>dynamic_reconfigure/Reconfigure</tt> service response.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[in] objp
 *          Pointer to an initialized <code>struct out_srv__dynamic_reconfigure__Reconfigure</code> object.
 * @return
 *          Error code.
 */
uros_err_t send_out_srv__dynamic_reconfigure__Reconfigure(
  UrosTcpRosStatus *tcpstp,
  struct out_srv__dynamic_reconfigure__Reconfigure *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);
#define _CHKOK { if (tcpstp->err) { return tcpstp->err; } }

  send_msg__dynamic_reconfigure__Config(tcpstp, &objp->config); _CHKOK

  return tcpstp->err = UROS_OK;
#undef _CHKOK
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

  /* bond/Constants */
  urosRegisterStaticMsgTypeSZ("bond/Constants",
                              NULL, "6fc594dc1d7bd7919077042712f8c8b0");

  /* dynamic_reconfigure/BoolParameter */
  urosRegisterStaticMsgTypeSZ("dynamic_reconfigure/BoolParameter",
                              NULL, "23f05028c1a699fb83e22401228c3a9e");

  /* dynamic_reconfigure/Config */
  urosRegisterStaticMsgTypeSZ("dynamic_reconfigure/Config",
                              NULL, "958f16a05573709014982821e6822580");

  /* dynamic_reconfigure/DoubleParameter */
  urosRegisterStaticMsgTypeSZ("dynamic_reconfigure/DoubleParameter",
                              NULL, "d8512f27253c0f65f928a67c329cd658");

  /* dynamic_reconfigure/GroupState */
  urosRegisterStaticMsgTypeSZ("dynamic_reconfigure/GroupState",
                              NULL, "a2d87f51dc22930325041a2f8b1571f8");

  /* dynamic_reconfigure/IntParameter */
  urosRegisterStaticMsgTypeSZ("dynamic_reconfigure/IntParameter",
                              NULL, "65fedc7a0cbfb8db035e46194a350bf1");

  /* dynamic_reconfigure/StrParameter */
  urosRegisterStaticMsgTypeSZ("dynamic_reconfigure/StrParameter",
                              NULL, "bc6ccc4a57f61779c8eaae61e9f422e0");

  /* rosgraph_msgs/Log */
  urosRegisterStaticMsgTypeSZ("rosgraph_msgs/Log",
                              NULL, "acffd30cd6b6de30f120938c17c593fb");

  /* sensor_msgs/Image */
  urosRegisterStaticMsgTypeSZ("sensor_msgs/Image",
                              NULL, "060021388200f6f0f447d0fcd9c64743");

  /* sensor_msgs/RegionOfInterest */
  urosRegisterStaticMsgTypeSZ("sensor_msgs/RegionOfInterest",
                              NULL, "bdb633039d588fcccb441a4d43ccfe09");

  /* std_msgs/Header */
  urosRegisterStaticMsgTypeSZ("std_msgs/Header",
                              NULL, "2176decaecbce78abc3b96ef049fabed");

  /* stereo_msgs/DisparityImage */
  urosRegisterStaticMsgTypeSZ("stereo_msgs/DisparityImage",
                              NULL, "04a177815f75271039fa21f16acad8c9");

  /* SERVICE TYPES */

  /* dynamic_reconfigure/Reconfigure */
  urosRegisterStaticSrvTypeSZ("dynamic_reconfigure/Reconfigure",
                              NULL, "bb125d226a21982a4a98760418dc2672");
}

/** @} */

