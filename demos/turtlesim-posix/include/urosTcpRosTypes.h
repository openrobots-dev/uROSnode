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
 * @file    urosTcpRosTypes.h
 * @author  Andrea Zoppi <texzk@email.it>
 *
 * @brief   TCPROS message and service descriptors.
 */

#ifndef _UROSTCPROSTYPES_H_
#define _UROSTCPROSTYPES_H_

/*============================================================================*/
/* HEADER FILES                                                               */
/*============================================================================*/

#include <urosTcpRos.h>

/*============================================================================*/
/*  MESSAGE TYPES                                                             */
/*============================================================================*/

/** @addtogroup tcpros_msg_types */
/** @{ */

/*~~~ MESSAGE: std_msgs/Header ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/**
 * @brief   TCPROS <tt>std_msgs/Header</tt> message descriptor.
 * @details MD5 sum: <tt>2176decaecbce78abc3b96ef049fabed</tt>.
 */
struct msg__std_msgs__Header {
  uint32_t      seq;
  uros_time_t   stamp;
  UrosString    frame_id;
};

/*~~~ MESSAGE: rosgraph_msgs/Log ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/**
 * @brief   TCPROS <tt>rosgraph_msgs/Log</tt> message descriptor.
 * @details MD5 sum: <tt>acffd30cd6b6de30f120938c17c593fb</tt>.
 */
struct msg__rosgraph_msgs__Log {
  struct msg__std_msgs__Header  header;
  uint8_t                       level;
  UrosString                    name;
  UrosString                    msg;
  UrosString                    file;
  UrosString                    function;
  uint32_t                      line;
  UROS_VARARR(UrosString)       topics;
};

/*~~~ MESSAGE: turtlesim/Pose ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/**
 * @brief   TCPROS <tt>turtlesim/Pose</tt> message descriptor.
 * @details MD5 sum: <tt>863b248d5016ca62ea2e895ae5265cf9</tt>.
 */
struct msg__turtlesim__Pose {
  float x;
  float y;
  float theta;
  float linear_velocity;
  float angular_velocity;
};

/*~~~ MESSAGE: turtlesim/Velocity ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/**
 * @brief   TCPROS <tt>turtlesim/Velocity</tt> message descriptor.
 * @details MD5 sum: <tt>9d5c2dcd348ac8f76ce2a4307bd63a13</tt>.
 */
struct msg__turtlesim__Velocity {
  float linear;
  float angular;
};

/** @} */

/*============================================================================*/
/* SERVICE TYPES                                                              */
/*============================================================================*/

/** @addtogroup tcpros_srv_types */
/** @{ */

/*~~~ SERVICE: turtlesim/SetPen ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/**
 * @brief   TCPROS <tt>turtlesim/SetPen</tt> service request descriptor.
 */
struct in_srv__turtlesim__SetPen {
  uint8_t   r;
  uint8_t   g;
  uint8_t   b;
  uint8_t   width;
  uint8_t   off;
};

/**
 * @brief   TCPROS <tt>turtlesim/SetPen</tt> service response descriptor.
 */
struct out_srv__turtlesim__SetPen {
  /* This message type has no fields.*/
  uint8_t _dummy;
};

/*~~~ SERVICE: turtlesim/Spawn ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/**
 * @brief   TCPROS <tt>turtlesim/Spawn</tt> service request descriptor.
 */
struct in_srv__turtlesim__Spawn {
  float         x;
  float         y;
  float         theta;
  UrosString    name;
};

/**
 * @brief   TCPROS <tt>turtlesim/Spawn</tt> service response descriptor.
 */
struct out_srv__turtlesim__Spawn {
  UrosString    name;
};

/*~~~ SERVICE: turtlesim/Kill ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/**
 * @brief   TCPROS <tt>turtlesim/Kill</tt> service request descriptor.
 */
struct in_srv__turtlesim__Kill {
  UrosString    name;
};

/**
 * @brief   TCPROS <tt>turtlesim/Kill</tt> service response descriptor.
 */
struct out_srv__turtlesim__Kill {
  /* This message type has no fields.*/
  uint8_t _dummy;
};

/*~~~ SERVICE: turtlesim/TeleportAbsolute ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/**
 * @brief   TCPROS <tt>turtlesim/TeleportAbsolute</tt> service request descriptor.
 */
struct in_srv__turtlesim__TeleportAbsolute {
  float x;
  float y;
  float theta;
};

/**
 * @brief   TCPROS <tt>turtlesim/TeleportAbsolute</tt> service response descriptor.
 */
struct out_srv__turtlesim__TeleportAbsolute {
  /* This message type has no fields.*/
  uint8_t _dummy;
};

/*~~~ SERVICE: std_srvs/Empty ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/**
 * @brief   TCPROS <tt>std_srvs/Empty</tt> service request descriptor.
 */
struct in_srv__std_srvs__Empty {
  /* This message type has no fields.*/
  uint8_t _dummy;
};

/**
 * @brief   TCPROS <tt>std_srvs/Empty</tt> service response descriptor.
 */
struct out_srv__std_srvs__Empty {
  /* This message type has no fields.*/
  uint8_t _dummy;
};

/*~~~ SERVICE: turtlesim/TeleportRelative ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/**
 * @brief   TCPROS <tt>turtlesim/TeleportRelative</tt> service request descriptor.
 */
struct in_srv__turtlesim__TeleportRelative {
  float linear;
  float angular;
};

/**
 * @brief   TCPROS <tt>turtlesim/TeleportRelative</tt> service response descriptor.
 */
struct out_srv__turtlesim__TeleportRelative {
  /* This message type has no fields.*/
  uint8_t _dummy;
};

/** @} */

/*============================================================================*/
/* MESSAGE CONSTANTS                                                          */
/*============================================================================*/

/** @addtogroup tcpros_msg_consts */
/** @{ */

/*~~~ MESSAGE: rosgraph_msgs/Log ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Message <tt>rosgraph_msgs/Log</tt> */
/** @{ */

#define msg__rosgraph_msgs__Log__DEBUG      ((uint8_t)1)
#define msg__rosgraph_msgs__Log__INFO       ((uint8_t)2)
#define msg__rosgraph_msgs__Log__WARN       ((uint8_t)4)
#define msg__rosgraph_msgs__Log__ERROR      ((uint8_t)8)
#define msg__rosgraph_msgs__Log__FATAL      ((uint8_t)16)

/** @} */

/** @} */

/*============================================================================*/
/* SERVICE CONSTANTS                                                          */
/*============================================================================*/

/** @addtogroup tcpros_srv_consts */
/** @{ */

/** @} */

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*/
/* MESSAGE PROTOTYPES                                                         */
/*============================================================================*/

/*~~~ MESSAGE: std_msgs/Header ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

size_t length_msg__std_msgs__Header(
  struct msg__std_msgs__Header *objp
);
void init_msg__std_msgs__Header(
  struct msg__std_msgs__Header *objp
);
void clean_msg__std_msgs__Header(
  struct msg__std_msgs__Header *objp
);
uros_err_t recv_msg__std_msgs__Header(
  UrosTcpRosStatus *tcpstp,
  struct msg__std_msgs__Header *objp
);
uros_err_t send_msg__std_msgs__Header(
  UrosTcpRosStatus *tcpstp,
  struct msg__std_msgs__Header *objp
);

/*~~~ MESSAGE: rosgraph_msgs/Log ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

size_t length_msg__rosgraph_msgs__Log(
  struct msg__rosgraph_msgs__Log *objp
);
void init_msg__rosgraph_msgs__Log(
  struct msg__rosgraph_msgs__Log *objp
);
void clean_msg__rosgraph_msgs__Log(
  struct msg__rosgraph_msgs__Log *objp
);
uros_err_t recv_msg__rosgraph_msgs__Log(
  UrosTcpRosStatus *tcpstp,
  struct msg__rosgraph_msgs__Log *objp
);
uros_err_t send_msg__rosgraph_msgs__Log(
  UrosTcpRosStatus *tcpstp,
  struct msg__rosgraph_msgs__Log *objp
);

/*~~~ MESSAGE: turtlesim/Pose ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

size_t length_msg__turtlesim__Pose(
  struct msg__turtlesim__Pose *objp
);
void init_msg__turtlesim__Pose(
  struct msg__turtlesim__Pose *objp
);
void clean_msg__turtlesim__Pose(
  struct msg__turtlesim__Pose *objp
);
uros_err_t recv_msg__turtlesim__Pose(
  UrosTcpRosStatus *tcpstp,
  struct msg__turtlesim__Pose *objp
);
uros_err_t send_msg__turtlesim__Pose(
  UrosTcpRosStatus *tcpstp,
  struct msg__turtlesim__Pose *objp
);

/*~~~ MESSAGE: turtlesim/Velocity ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

size_t length_msg__turtlesim__Velocity(
  struct msg__turtlesim__Velocity *objp
);
void init_msg__turtlesim__Velocity(
  struct msg__turtlesim__Velocity *objp
);
void clean_msg__turtlesim__Velocity(
  struct msg__turtlesim__Velocity *objp
);
uros_err_t recv_msg__turtlesim__Velocity(
  UrosTcpRosStatus *tcpstp,
  struct msg__turtlesim__Velocity *objp
);
uros_err_t send_msg__turtlesim__Velocity(
  UrosTcpRosStatus *tcpstp,
  struct msg__turtlesim__Velocity *objp
);

/*============================================================================*/
/* SERVICE PROTOTYPES                                                         */
/*============================================================================*/

/*~~~ SERVICE: turtlesim/Velocity ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

size_t length_in_srv__turtlesim__SetPen(
  struct in_srv__turtlesim__SetPen *objp
);
size_t length_out_srv__turtlesim__SetPen(
  struct out_srv__turtlesim__SetPen *objp
);
void init_in_srv__turtlesim__SetPen(
  struct in_srv__turtlesim__SetPen *objp
);
void init_out_srv__turtlesim__SetPen(
  struct out_srv__turtlesim__SetPen *objp
);
void clean_in_srv__turtlesim__SetPen(
  struct in_srv__turtlesim__SetPen *objp
);
void clean_out_srv__turtlesim__SetPen(
  struct out_srv__turtlesim__SetPen *objp
);
uros_err_t recv_srv__turtlesim__SetPen(
  UrosTcpRosStatus *tcpstp,
  struct in_srv__turtlesim__SetPen *objp
);
uros_err_t send_srv__turtlesim__SetPen(
  UrosTcpRosStatus *tcpstp,
  struct out_srv__turtlesim__SetPen *objp
);

/*~~~ SERVICE: turtlesim/Velocity ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

size_t length_in_srv__turtlesim__Spawn(
  struct in_srv__turtlesim__Spawn *objp
);
size_t length_out_srv__turtlesim__Spawn(
  struct out_srv__turtlesim__Spawn *objp
);
void init_in_srv__turtlesim__Spawn(
  struct in_srv__turtlesim__Spawn *objp
);
void init_out_srv__turtlesim__Spawn(
  struct out_srv__turtlesim__Spawn *objp
);
void clean_in_srv__turtlesim__Spawn(
  struct in_srv__turtlesim__Spawn *objp
);
void clean_out_srv__turtlesim__Spawn(
  struct out_srv__turtlesim__Spawn *objp
);
uros_err_t recv_srv__turtlesim__Spawn(
  UrosTcpRosStatus *tcpstp,
  struct in_srv__turtlesim__Spawn *objp
);
uros_err_t send_srv__turtlesim__Spawn(
  UrosTcpRosStatus *tcpstp,
  struct out_srv__turtlesim__Spawn *objp
);

/*~~~ SERVICE: turtlesim/Velocity ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

size_t length_in_srv__turtlesim__Kill(
  struct in_srv__turtlesim__Kill *objp
);
size_t length_out_srv__turtlesim__Kill(
  struct out_srv__turtlesim__Kill *objp
);
void init_in_srv__turtlesim__Kill(
  struct in_srv__turtlesim__Kill *objp
);
void init_out_srv__turtlesim__Kill(
  struct out_srv__turtlesim__Kill *objp
);
void clean_in_srv__turtlesim__Kill(
  struct in_srv__turtlesim__Kill *objp
);
void clean_out_srv__turtlesim__Kill(
  struct out_srv__turtlesim__Kill *objp
);
uros_err_t recv_srv__turtlesim__Kill(
  UrosTcpRosStatus *tcpstp,
  struct in_srv__turtlesim__Kill *objp
);
uros_err_t send_srv__turtlesim__Kill(
  UrosTcpRosStatus *tcpstp,
  struct out_srv__turtlesim__Kill *objp
);

/*~~~ SERVICE: turtlesim/Velocity ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

size_t length_in_srv__turtlesim__TeleportAbsolute(
  struct in_srv__turtlesim__TeleportAbsolute *objp
);
size_t length_out_srv__turtlesim__TeleportAbsolute(
  struct out_srv__turtlesim__TeleportAbsolute *objp
);
void init_in_srv__turtlesim__TeleportAbsolute(
  struct in_srv__turtlesim__TeleportAbsolute *objp
);
void init_out_srv__turtlesim__TeleportAbsolute(
  struct out_srv__turtlesim__TeleportAbsolute *objp
);
void clean_in_srv__turtlesim__TeleportAbsolute(
  struct in_srv__turtlesim__TeleportAbsolute *objp
);
void clean_out_srv__turtlesim__TeleportAbsolute(
  struct out_srv__turtlesim__TeleportAbsolute *objp
);
uros_err_t recv_srv__turtlesim__TeleportAbsolute(
  UrosTcpRosStatus *tcpstp,
  struct in_srv__turtlesim__TeleportAbsolute *objp
);
uros_err_t send_srv__turtlesim__TeleportAbsolute(
  UrosTcpRosStatus *tcpstp,
  struct out_srv__turtlesim__TeleportAbsolute *objp
);

/*~~~ SERVICE: turtlesim/Velocity ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

size_t length_in_srv__std_srvs__Empty(
  struct in_srv__std_srvs__Empty *objp
);
size_t length_out_srv__std_srvs__Empty(
  struct out_srv__std_srvs__Empty *objp
);
void init_in_srv__std_srvs__Empty(
  struct in_srv__std_srvs__Empty *objp
);
void init_out_srv__std_srvs__Empty(
  struct out_srv__std_srvs__Empty *objp
);
void clean_in_srv__std_srvs__Empty(
  struct in_srv__std_srvs__Empty *objp
);
void clean_out_srv__std_srvs__Empty(
  struct out_srv__std_srvs__Empty *objp
);
uros_err_t recv_srv__std_srvs__Empty(
  UrosTcpRosStatus *tcpstp,
  struct in_srv__std_srvs__Empty *objp
);
uros_err_t send_srv__std_srvs__Empty(
  UrosTcpRosStatus *tcpstp,
  struct out_srv__std_srvs__Empty *objp
);

/*~~~ SERVICE: turtlesim/Velocity ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

size_t length_in_srv__turtlesim__TeleportRelative(
  struct in_srv__turtlesim__TeleportRelative *objp
);
size_t length_out_srv__turtlesim__TeleportRelative(
  struct out_srv__turtlesim__TeleportRelative *objp
);
void init_in_srv__turtlesim__TeleportRelative(
  struct in_srv__turtlesim__TeleportRelative *objp
);
void init_out_srv__turtlesim__TeleportRelative(
  struct out_srv__turtlesim__TeleportRelative *objp
);
void clean_in_srv__turtlesim__TeleportRelative(
  struct in_srv__turtlesim__TeleportRelative *objp
);
void clean_out_srv__turtlesim__TeleportRelative(
  struct out_srv__turtlesim__TeleportRelative *objp
);
uros_err_t recv_srv__turtlesim__TeleportRelative(
  UrosTcpRosStatus *tcpstp,
  struct in_srv__turtlesim__TeleportRelative *objp
);
uros_err_t send_srv__turtlesim__TeleportRelative(
  UrosTcpRosStatus *tcpstp,
  struct out_srv__turtlesim__TeleportRelative *objp
);

/*============================================================================*/
/* GLOBAL PROTOTYPES                                                          */
/*============================================================================*/

void urosTcpRosRegStaticTypes(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _UROSTCPROSTYPES_H_ */

