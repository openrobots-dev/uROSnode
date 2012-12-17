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
 * @file    msg_types.h
 * @author  Andrea Zoppi <texzk@email.it>
 *
 * @brief   ROS message type descriptors (C structs) used by the project.
 */

#ifndef _MSG_TYPES_H_
#define _MSG_TYPES_H_

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include <urosBase.h>

/*===========================================================================*/
/* TYPES & MACROS                                                            */
/*===========================================================================*/

/**
 * @brief   ROS <tt>time</tt> basic data type.
 */
struct time_t {
  int32_t       sec;
  int32_t       nsec;
};

/**
 * @brief   ROS <tt>std_msgs/Header</tt> data type.
 */
struct std_msgs__Header {
  uint32_t      seq;
  struct time_t time;
  UrosString    frame_id;
};

/**
 * @brief   ROS <tt>(rosgraph_msgs/Log).level</tt> values.
 */
enum rosgraph_msgs__Log_level {
  rosgraph_msgs__Log_level_DEBUG    = 1,
  rosgraph_msgs__Log_level_INFO     = 2,
  rosgraph_msgs__Log_level_WARN     = 4,
  rosgraph_msgs__Log_level_ERROR    = 8,
  rosgraph_msgs__Log_level_FATAL    = 16
};

/**
 * @brief   ROS <tt>rosgraph_msgs/Log</tt> data type.
 */
struct rosgraph_msgs__Log {
  struct std_msgs__Header   header;
  uint8_t                   level;
  UrosString                name;
  UrosString                msg;
  UrosString                file;
  UrosString                function;
  uint32_t                  line;
  UrosList                  topics;     /**< @brief Topic names that the node publishes, <tt>str[]</tt>. */
};

/**
 * @brief   ROS <tt>turtlesim/Velocity</tt> topic data type.
 */
struct msg__turtlesim__Velocity {
  float     linear;
  float     angular;
};

/**
 * @brief   ROS <tt>turtlesim/Pose</tt> topic data type.
 */
struct msg__turtlesim__Pose {
  float     x;
  float     y;
  float     theta;

  float     linear_velocity;
  float     angular_velocity;
};

/**
 * @brief   ROS <tt>turtlesim/Kill</tt> service data type.
 */
struct srv__turtlesim__Kill {
  struct {
    UrosString  name;
  } in;
  struct {
  } out;
};

/**
 * @brief   ROS <tt>turtlesim/Spawn</tt> service data type.
 */
struct srv__turtlesim__Spawn {
  struct {
    float       x;
    float       y;
    float       theta;
    UrosString  name;
  } in;
  struct {
    UrosString  name;
  } out;
};

/**
 * @brief   ROS <tt>turtlesim/SetPen</tt> service data type.
 */
struct srv__turtlesim__SetPen {
  struct {
    uint8_t     r;
    uint8_t     g;
    uint8_t     b;
    uint8_t     width;
    uint8_t     off;
  } in;
  struct {
  } out;
};

#endif /* _MSG_TYPES_H_ */
