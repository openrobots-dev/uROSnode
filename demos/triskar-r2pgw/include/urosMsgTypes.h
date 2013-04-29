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
 * @file    urosMsgTypes.h
 * @author  Andrea Zoppi <texzk@email.it>
 *
 * @brief   TCPROS message and service descriptors.
 */

#ifndef _UROSMSGTYPES_H_
#define _UROSMSGTYPES_H_

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include <urosTcpRos.h>

#ifdef __cplusplus
extern "C" {
#endif

/*===========================================================================*/
/*  MESSAGE TYPES                                                            */
/*===========================================================================*/

/** @addtogroup tcpros_msg_types */
/** @{ */

/*~~~ MESSAGE: triskar/Proximity ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/**
 * @brief   TCPROS <tt>triskar/Proximity</tt> message descriptor.
 * @details MD5 sum: <tt>e375dcd2b74602ba85b8ccd90a2e7d70</tt>.
 */
struct msg__triskar__Proximity {
  float proximities[4];
};

/*~~~ MESSAGE: triskar/Velocity ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/**
 * @brief   TCPROS <tt>triskar/Velocity</tt> message descriptor.
 * @details MD5 sum: <tt>23d55db697c48d93db1057083ac92653</tt>.
 */
struct msg__triskar__Velocity {
  float strafe;
  float forward;
  float angular;
};

/** @} */

/*===========================================================================*/
/* SERVICE TYPES                                                             */
/*===========================================================================*/

/** @addtogroup tcpros_srv_types */
/** @{ */

/* There are no service types.*/

/** @} */

/*===========================================================================*/
/* MESSAGE CONSTANTS                                                         */
/*===========================================================================*/

/** @addtogroup tcpros_msg_consts */
/** @{ */

/*~~~ MESSAGE: triskar/Proximity ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Message <tt>triskar/Proximity</tt> */
/** @{ */

#define msg__triskar__Proximity__NUM_SENSORS        ((uint32_t)4)
#define msg__triskar__Proximity__EAST               ((uint32_t)0)
#define msg__triskar__Proximity__NORTH              ((uint32_t)1)
#define msg__triskar__Proximity__WEST               ((uint32_t)2)
#define msg__triskar__Proximity__SOUTH              ((uint32_t)3)

/** @} */

/** @} */

/*===========================================================================*/
/* SERVICE CONSTANTS                                                         */
/*===========================================================================*/

/** @addtogroup tcpros_srv_consts */
/** @{ */

/* There are no service costants.*/

/** @} */

/*===========================================================================*/
/* MESSAGE PROTOTYPES                                                        */
/*===========================================================================*/

/*~~~ MESSAGE: triskar/Proximity ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

size_t length_msg__triskar__Proximity(
  struct msg__triskar__Proximity *objp
);
void init_msg__triskar__Proximity(
  struct msg__triskar__Proximity *objp
);
void clean_msg__triskar__Proximity(
  struct msg__triskar__Proximity *objp
);
uros_err_t recv_msg__triskar__Proximity(
  UrosTcpRosStatus *tcpstp,
  struct msg__triskar__Proximity *objp
);
uros_err_t send_msg__triskar__Proximity(
  UrosTcpRosStatus *tcpstp,
  struct msg__triskar__Proximity *objp
);

/*~~~ MESSAGE: triskar/Velocity ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

size_t length_msg__triskar__Velocity(
  struct msg__triskar__Velocity *objp
);
void init_msg__triskar__Velocity(
  struct msg__triskar__Velocity *objp
);
void clean_msg__triskar__Velocity(
  struct msg__triskar__Velocity *objp
);
uros_err_t recv_msg__triskar__Velocity(
  UrosTcpRosStatus *tcpstp,
  struct msg__triskar__Velocity *objp
);
uros_err_t send_msg__triskar__Velocity(
  UrosTcpRosStatus *tcpstp,
  struct msg__triskar__Velocity *objp
);

/*===========================================================================*/
/* SERVICE PROTOTYPES                                                        */
/*===========================================================================*/

/* There are no service types.*/

/*===========================================================================*/
/* GLOBAL PROTOTYPES                                                         */
/*===========================================================================*/

void urosMsgTypesRegStaticTypes(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _UROSMSGTYPES_H_ */

