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

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include "urosMsgTypes.h"

/*===========================================================================*/
/* MESSAGE CONSTANTS                                                         */
/*===========================================================================*/

/** @addtogroup tcpros_msg_consts */
/** @{ */

/* There are no message constants.*/

/** @} */

/*===========================================================================*/
/* SERVICE CONSTANTS                                                         */
/*===========================================================================*/

/** @addtogroup tcpros_srv_consts */
/** @{ */

/* There are no service constants.*/

/** @} */

/*===========================================================================*/
/* MESSAGE FUNCTIONS                                                         */
/*===========================================================================*/

/** @addtogroup tcpros_msg_funcs */
/** @{ */

/*~~~ MESSAGE: triskar/Proximity ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Message <tt>triskar/Proximity</tt> */
/** @{ */

/**
 * @brief   Content length of a TCPROS <tt>triskar/Proximity</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct msg__triskar__Proximity</code> object.
 * @return
 *          Length of the TCPROS message contents, in bytes.
 */
size_t length_msg__triskar__Proximity(
  struct msg__triskar__Proximity *objp
) {
  size_t length = 0;

  urosAssert(objp != NULL);

  length += (size_t)4 * sizeof(float);

  (void)objp;
  return length;
}

/**
 * @brief   Initializes a TCPROS <tt>triskar/Proximity</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an allocated <code>struct msg__triskar__Proximity</code> object.
 * @return
 *          Error code.
 */
void init_msg__triskar__Proximity(
  struct msg__triskar__Proximity *objp
) {
  urosAssert(objp != NULL);

  /* Nothing to initialize.*/
  (void)objp;
}

/**
 * @brief   Cleans a TCPROS <tt>triskar/Proximity</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct msg__triskar__Proximity</code> object, or @p NULL.
 * @return
 *          Error code.
 */
void clean_msg__triskar__Proximity(
  struct msg__triskar__Proximity *objp
) {
  /* Nothing to clean.*/
  (void)objp;
}

/**
 * @brief   Receives a TCPROS <tt>triskar/Proximity</tt> message.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[out] objp
 *          Pointer to an initialized <code>struct msg__triskar__Proximity</code> object.
 * @return
 *          Error code.
 */
uros_err_t recv_msg__triskar__Proximity(
  UrosTcpRosStatus *tcpstp,
  struct msg__triskar__Proximity *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);
#define _CHKOK { if (tcpstp->err != UROS_OK) { goto _error; } }

  urosTcpRosRecv(tcpstp, objp->proximities,
                 (size_t)4 * sizeof(float)); _CHKOK

  return tcpstp->err = UROS_OK;
_error:
  clean_msg__triskar__Proximity(objp);
  return tcpstp->err;
#undef _CHKOK
}

/**
 * @brief   Sends a TCPROS <tt>triskar/Proximity</tt> message.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[in] objp
 *          Pointer to an initialized <code>struct msg__triskar__Proximity</code> object.
 * @return
 *          Error code.
 */
uros_err_t send_msg__triskar__Proximity(
  UrosTcpRosStatus *tcpstp,
  struct msg__triskar__Proximity *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);
#define _CHKOK { if (tcpstp->err != UROS_OK) { return tcpstp->err; } }

  urosTcpRosSend(tcpstp, objp->proximities,
                 (size_t)4 * sizeof(float)); _CHKOK

  return tcpstp->err = UROS_OK;
#undef _CHKOK
}

/** @} */

/*~~~ MESSAGE: triskar/Velocity ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Message <tt>triskar/Velocity</tt> */
/** @{ */

/**
 * @brief   Content length of a TCPROS <tt>triskar/Velocity</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct msg__triskar__Velocity</code> object.
 * @return
 *          Length of the TCPROS message contents, in bytes.
 */
size_t length_msg__triskar__Velocity(
  struct msg__triskar__Velocity *objp
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
 * @brief   Initializes a TCPROS <tt>triskar/Velocity</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an allocated <code>struct msg__triskar__Velocity</code> object.
 * @return
 *          Error code.
 */
void init_msg__triskar__Velocity(
  struct msg__triskar__Velocity *objp
) {
  urosAssert(objp != NULL);

  /* Nothing to initialize.*/
  (void)objp;
}

/**
 * @brief   Cleans a TCPROS <tt>triskar/Velocity</tt> message.
 *
 * @param[in,out] objp
 *          Pointer to an initialized <code>struct msg__triskar__Velocity</code> object, or @p NULL.
 * @return
 *          Error code.
 */
void clean_msg__triskar__Velocity(
  struct msg__triskar__Velocity *objp
) {
  /* Nothing to clean.*/
  (void)objp;
}

/**
 * @brief   Receives a TCPROS <tt>triskar/Velocity</tt> message.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[out] objp
 *          Pointer to an initialized <code>struct msg__triskar__Velocity</code> object.
 * @return
 *          Error code.
 */
uros_err_t recv_msg__triskar__Velocity(
  UrosTcpRosStatus *tcpstp,
  struct msg__triskar__Velocity *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);
#define _CHKOK { if (tcpstp->err != UROS_OK) { goto _error; } }

  urosTcpRosRecvRaw(tcpstp, objp->strafe); _CHKOK
  urosTcpRosRecvRaw(tcpstp, objp->forward); _CHKOK
  urosTcpRosRecvRaw(tcpstp, objp->angular); _CHKOK

  return tcpstp->err = UROS_OK;
_error:
  clean_msg__triskar__Velocity(objp);
  return tcpstp->err;
#undef _CHKOK
}

/**
 * @brief   Sends a TCPROS <tt>triskar/Velocity</tt> message.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[in] objp
 *          Pointer to an initialized <code>struct msg__triskar__Velocity</code> object.
 * @return
 *          Error code.
 */
uros_err_t send_msg__triskar__Velocity(
  UrosTcpRosStatus *tcpstp,
  struct msg__triskar__Velocity *objp
) {
  urosAssert(tcpstp != NULL);
  urosAssert(urosConnIsValid(tcpstp->csp));
  urosAssert(objp != NULL);
#define _CHKOK { if (tcpstp->err != UROS_OK) { return tcpstp->err; } }

  urosTcpRosSendRaw(tcpstp, objp->strafe); _CHKOK
  urosTcpRosSendRaw(tcpstp, objp->forward); _CHKOK
  urosTcpRosSendRaw(tcpstp, objp->angular); _CHKOK

  return tcpstp->err = UROS_OK;
#undef _CHKOK
}

/** @} */

/** @} */

/*===========================================================================*/
/* SERVICE FUNCTIONS                                                         */
/*===========================================================================*/

/** @addtogroup tcpros_srv_funcs */
/** @{ */

/* There are no service types.*/

/** @} */

/*===========================================================================*/
/* GLOBAL FUNCTIONS                                                          */
/*===========================================================================*/

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

  /* triskar/Proximity */
  urosRegisterStaticMsgTypeSZ("triskar/Proximity",
                              NULL, "e375dcd2b74602ba85b8ccd90a2e7d70");

  /* triskar/Velocity */
  urosRegisterStaticMsgTypeSZ("triskar/Velocity",
                              NULL, "23d55db697c48d93db1057083ac92653");
}

/** @} */

