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
 * @author  Your Name <your.email@example.com>
 *
 * @brief   TCPROS message and service descriptors.
 */

#ifndef _UROSMSGTYPES_H_
#define _UROSMSGTYPES_H_

/*============================================================================*/
/* HEADER FILES                                                               */
/*============================================================================*/

#include <urosTcpRos.h>

/*============================================================================*/
/*  MESSAGE TYPES                                                             */
/*============================================================================*/

/** @addtogroup tcpros_msg_types */
/** @{ */

/*~~~ MESSAGE: dynamic_reconfigure/IntParameter ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/**
 * @brief   TCPROS <tt>dynamic_reconfigure/IntParameter</tt> message descriptor.
 * @details MD5 sum: <tt>65fedc7a0cbfb8db035e46194a350bf1</tt>.
 */
struct msg__dynamic_reconfigure__IntParameter {

  /** @brief TODO: @p name description.*/
  UrosString    name;

  /** @brief TODO: @p value description.*/
  int32_t       value;
};

/*~~~ MESSAGE: dynamic_reconfigure/GroupState ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/**
 * @brief   TCPROS <tt>dynamic_reconfigure/GroupState</tt> message descriptor.
 * @details MD5 sum: <tt>a2d87f51dc22930325041a2f8b1571f8</tt>.
 */
struct msg__dynamic_reconfigure__GroupState {

  /** @brief TODO: @p name description.*/
  UrosString    name;

  /** @brief TODO: @p state description.*/
  uint8_t       state;

  /** @brief TODO: @p id description.*/
  int32_t       id;

  /** @brief TODO: @p parent description.*/
  int32_t       parent;
};

/*~~~ MESSAGE: dynamic_reconfigure/BoolParameter ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/**
 * @brief   TCPROS <tt>dynamic_reconfigure/BoolParameter</tt> message descriptor.
 * @details MD5 sum: <tt>23f05028c1a699fb83e22401228c3a9e</tt>.
 */
struct msg__dynamic_reconfigure__BoolParameter {

  /** @brief TODO: @p name description.*/
  UrosString    name;

  /** @brief TODO: @p value description.*/
  uint8_t       value;
};

/*~~~ MESSAGE: dynamic_reconfigure/DoubleParameter ~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/**
 * @brief   TCPROS <tt>dynamic_reconfigure/DoubleParameter</tt> message descriptor.
 * @details MD5 sum: <tt>d8512f27253c0f65f928a67c329cd658</tt>.
 */
struct msg__dynamic_reconfigure__DoubleParameter {

  /** @brief TODO: @p name description.*/
  UrosString    name;

  /** @brief TODO: @p value description.*/
  double        value;
};

/*~~~ MESSAGE: dynamic_reconfigure/StrParameter ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/**
 * @brief   TCPROS <tt>dynamic_reconfigure/StrParameter</tt> message descriptor.
 * @details MD5 sum: <tt>bc6ccc4a57f61779c8eaae61e9f422e0</tt>.
 */
struct msg__dynamic_reconfigure__StrParameter {

  /** @brief TODO: @p name description.*/
  UrosString    name;

  /** @brief TODO: @p value description.*/
  UrosString    value;
};

/*~~~ MESSAGE: sensor_msgs/RegionOfInterest ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/**
 * @brief   TCPROS <tt>sensor_msgs/RegionOfInterest</tt> message descriptor.
 * @details MD5 sum: <tt>bdb633039d588fcccb441a4d43ccfe09</tt>.
 */
struct msg__sensor_msgs__RegionOfInterest {

  /** @brief TODO: @p x_offset description.*/
  uint32_t  x_offset;

  /** @brief TODO: @p y_offset description.*/
  uint32_t  y_offset;

  /** @brief TODO: @p height description.*/
  uint32_t  height;

  /** @brief TODO: @p width description.*/
  uint32_t  width;

  /** @brief TODO: @p do_rectify description.*/
  uint8_t   do_rectify;
};

/*~~~ MESSAGE: std_msgs/Header ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/**
 * @brief   TCPROS <tt>std_msgs/Header</tt> message descriptor.
 * @details MD5 sum: <tt>2176decaecbce78abc3b96ef049fabed</tt>.
 */
struct msg__std_msgs__Header {

  /** @brief TODO: @p seq description.*/
  uint32_t      seq;

  /** @brief TODO: @p stamp description.*/
  uros_time_t   stamp;

  /** @brief TODO: @p frame_id description.*/
  UrosString    frame_id;
};

/*~~~ MESSAGE: dynamic_reconfigure/Config ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/**
 * @brief   TCPROS <tt>dynamic_reconfigure/Config</tt> message descriptor.
 * @details MD5 sum: <tt>958f16a05573709014982821e6822580</tt>.
 */
struct msg__dynamic_reconfigure__Config {

  /** @brief TODO: @p bools description.*/
  UROS_VARARR(struct msg__dynamic_reconfigure__BoolParameter)   bools;

  /** @brief TODO: @p ints description.*/
  UROS_VARARR(struct msg__dynamic_reconfigure__IntParameter)    ints;

  /** @brief TODO: @p strs description.*/
  UROS_VARARR(struct msg__dynamic_reconfigure__StrParameter)    strs;

  /** @brief TODO: @p doubles description.*/
  UROS_VARARR(struct msg__dynamic_reconfigure__DoubleParameter) doubles;

  /** @brief TODO: @p groups description.*/
  UROS_VARARR(struct msg__dynamic_reconfigure__GroupState)      groups;
};

/*~~~ MESSAGE: rosgraph_msgs/Log ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/**
 * @brief   TCPROS <tt>rosgraph_msgs/Log</tt> message descriptor.
 * @details MD5 sum: <tt>acffd30cd6b6de30f120938c17c593fb</tt>.
 */
struct msg__rosgraph_msgs__Log {

  /** @brief TODO: @p header description.*/
  struct msg__std_msgs__Header  header;

  /** @brief TODO: @p level description.*/
  uint8_t                       level;

  /** @brief TODO: @p name description.*/
  UrosString                    name;

  /** @brief TODO: @p msg description.*/
  UrosString                    msg;

  /** @brief TODO: @p file description.*/
  UrosString                    file;

  /** @brief TODO: @p function description.*/
  UrosString                    function;

  /** @brief TODO: @p line description.*/
  uint32_t                      line;

  /** @brief TODO: @p topics description.*/
  UROS_VARARR(UrosString)       topics;
};

/*~~~ MESSAGE: sensor_msgs/Image ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/**
 * @brief   TCPROS <tt>sensor_msgs/Image</tt> message descriptor.
 * @details MD5 sum: <tt>060021388200f6f0f447d0fcd9c64743</tt>.
 */
struct msg__sensor_msgs__Image {

  /** @brief TODO: @p header description.*/
  struct msg__std_msgs__Header  header;

  /** @brief TODO: @p height description.*/
  uint32_t                      height;

  /** @brief TODO: @p width description.*/
  uint32_t                      width;

  /** @brief TODO: @p encoding description.*/
  UrosString                    encoding;

  /** @brief TODO: @p is_bigendian description.*/
  uint8_t                       is_bigendian;

  /** @brief TODO: @p step description.*/
  uint32_t                      step;

  /** @brief TODO: @p data description.*/
  UROS_VARARR(uint8_t)          data;
};

/*~~~ MESSAGE: stereo_msgs/DisparityImage ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/**
 * @brief   TCPROS <tt>stereo_msgs/DisparityImage</tt> message descriptor.
 * @details MD5 sum: <tt>04a177815f75271039fa21f16acad8c9</tt>.
 */
struct msg__stereo_msgs__DisparityImage {

  /** @brief TODO: @p header description.*/
  struct msg__std_msgs__Header              header;

  /** @brief TODO: @p image description.*/
  struct msg__sensor_msgs__Image            image;

  /** @brief TODO: @p f description.*/
  float                                     f;

  /** @brief TODO: @p T description.*/
  float                                     T;

  /** @brief TODO: @p valid_window description.*/
  struct msg__sensor_msgs__RegionOfInterest valid_window;

  /** @brief TODO: @p min_disparity description.*/
  float                                     min_disparity;

  /** @brief TODO: @p max_disparity description.*/
  float                                     max_disparity;

  /** @brief TODO: @p delta_d description.*/
  float                                     delta_d;
};

/*~~~ MESSAGE: bond/Constants ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/**
 * @brief   TCPROS <tt>bond/Constants</tt> message descriptor.
 * @details MD5 sum: <tt>6fc594dc1d7bd7919077042712f8c8b0</tt>.
 */
struct msg__bond__Constants {
  /* This message type has no fields.*/
  uint8_t _dummy;
};

/** @} */

/*============================================================================*/
/* SERVICE TYPES                                                              */
/*============================================================================*/

/** @addtogroup tcpros_srv_types */
/** @{ */

/*~~~ SERVICE: dynamic_reconfigure/Reconfigure ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/**
 * @brief   TCPROS <tt>dynamic_reconfigure/Reconfigure</tt> service request descriptor.
 */
struct in_srv__dynamic_reconfigure__Reconfigure {

  /** @brief TODO: @p config description.*/
  struct msg__dynamic_reconfigure__Config   config;
};

/**
 * @brief   TCPROS <tt>dynamic_reconfigure/Reconfigure</tt> service response descriptor.
 */
struct out_srv__dynamic_reconfigure__Reconfigure {

  /** @brief TODO: @p config description.*/
  struct msg__dynamic_reconfigure__Config   config;
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

/** @brief TODO: <tt>rosgraph_msgs/Log.DEBUG</tt> description.*/
#define msg__rosgraph_msgs__Log__DEBUG      ((uint8_t)1)

/** @brief TODO: <tt>rosgraph_msgs/Log.INFO</tt> description.*/
#define msg__rosgraph_msgs__Log__INFO       ((uint8_t)2)

/** @brief TODO: <tt>rosgraph_msgs/Log.WARN</tt> description.*/
#define msg__rosgraph_msgs__Log__WARN       ((uint8_t)4)

/** @brief TODO: <tt>rosgraph_msgs/Log.ERROR</tt> description.*/
#define msg__rosgraph_msgs__Log__ERROR      ((uint8_t)8)

/** @brief TODO: <tt>rosgraph_msgs/Log.FATAL</tt> description.*/
#define msg__rosgraph_msgs__Log__FATAL      ((uint8_t)16)

/** @} */

/*~~~ MESSAGE: bond/Constants ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Message <tt>bond/Constants</tt> */
/** @{ */

/** @see @p msg__bond__Constants__DISABLE_HEARTBEAT_TIMEOUT_PARAM */
#define msg__bond__Constants__DISABLE_HEARTBEAT_TIMEOUT_PARAM__SZ \
  "/bond_disable_heartbeat_timeout"
extern const UrosString msg__bond__Constants__DISABLE_HEARTBEAT_TIMEOUT_PARAM;

/** @brief TODO: <tt>bond/Constants.DEAD_PUBLISH_PERIOD</tt> description.*/
#define msg__bond__Constants__DEAD_PUBLISH_PERIOD               ((float)0.05)

/** @brief TODO: <tt>bond/Constants.DEFAULT_CONNECT_TIMEOUT</tt> description.*/
#define msg__bond__Constants__DEFAULT_CONNECT_TIMEOUT           ((float)10.0)

/** @brief TODO: <tt>bond/Constants.DEFAULT_HEARTBEAT_TIMEOUT</tt> description.*/
#define msg__bond__Constants__DEFAULT_HEARTBEAT_TIMEOUT         ((float)4.0)

/** @brief TODO: <tt>bond/Constants.DEFAULT_DISCONNECT_TIMEOUT</tt> description.*/
#define msg__bond__Constants__DEFAULT_DISCONNECT_TIMEOUT        ((float)2.0)

/** @brief TODO: <tt>bond/Constants.DEFAULT_HEARTBEAT_PERIOD</tt> description.*/
#define msg__bond__Constants__DEFAULT_HEARTBEAT_PERIOD          ((float)1.0)

/** @} */

/** @} */

/*============================================================================*/
/* SERVICE CONSTANTS                                                          */
/*============================================================================*/

/** @addtogroup tcpros_srv_consts */
/** @{ */

/* There are no service costants.*/

/** @} */

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*/
/* MESSAGE PROTOTYPES                                                         */
/*============================================================================*/

/*~~~ MESSAGE: dynamic_reconfigure/IntParameter ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

size_t length_msg__dynamic_reconfigure__IntParameter(
  struct msg__dynamic_reconfigure__IntParameter *objp
);
void init_msg__dynamic_reconfigure__IntParameter(
  struct msg__dynamic_reconfigure__IntParameter *objp
);
void clean_msg__dynamic_reconfigure__IntParameter(
  struct msg__dynamic_reconfigure__IntParameter *objp
);
uros_err_t recv_msg__dynamic_reconfigure__IntParameter(
  UrosTcpRosStatus *tcpstp,
  struct msg__dynamic_reconfigure__IntParameter *objp
);
uros_err_t send_msg__dynamic_reconfigure__IntParameter(
  UrosTcpRosStatus *tcpstp,
  struct msg__dynamic_reconfigure__IntParameter *objp
);

/*~~~ MESSAGE: dynamic_reconfigure/GroupState ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

size_t length_msg__dynamic_reconfigure__GroupState(
  struct msg__dynamic_reconfigure__GroupState *objp
);
void init_msg__dynamic_reconfigure__GroupState(
  struct msg__dynamic_reconfigure__GroupState *objp
);
void clean_msg__dynamic_reconfigure__GroupState(
  struct msg__dynamic_reconfigure__GroupState *objp
);
uros_err_t recv_msg__dynamic_reconfigure__GroupState(
  UrosTcpRosStatus *tcpstp,
  struct msg__dynamic_reconfigure__GroupState *objp
);
uros_err_t send_msg__dynamic_reconfigure__GroupState(
  UrosTcpRosStatus *tcpstp,
  struct msg__dynamic_reconfigure__GroupState *objp
);

/*~~~ MESSAGE: dynamic_reconfigure/BoolParameter ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

size_t length_msg__dynamic_reconfigure__BoolParameter(
  struct msg__dynamic_reconfigure__BoolParameter *objp
);
void init_msg__dynamic_reconfigure__BoolParameter(
  struct msg__dynamic_reconfigure__BoolParameter *objp
);
void clean_msg__dynamic_reconfigure__BoolParameter(
  struct msg__dynamic_reconfigure__BoolParameter *objp
);
uros_err_t recv_msg__dynamic_reconfigure__BoolParameter(
  UrosTcpRosStatus *tcpstp,
  struct msg__dynamic_reconfigure__BoolParameter *objp
);
uros_err_t send_msg__dynamic_reconfigure__BoolParameter(
  UrosTcpRosStatus *tcpstp,
  struct msg__dynamic_reconfigure__BoolParameter *objp
);

/*~~~ MESSAGE: dynamic_reconfigure/DoubleParameter ~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

size_t length_msg__dynamic_reconfigure__DoubleParameter(
  struct msg__dynamic_reconfigure__DoubleParameter *objp
);
void init_msg__dynamic_reconfigure__DoubleParameter(
  struct msg__dynamic_reconfigure__DoubleParameter *objp
);
void clean_msg__dynamic_reconfigure__DoubleParameter(
  struct msg__dynamic_reconfigure__DoubleParameter *objp
);
uros_err_t recv_msg__dynamic_reconfigure__DoubleParameter(
  UrosTcpRosStatus *tcpstp,
  struct msg__dynamic_reconfigure__DoubleParameter *objp
);
uros_err_t send_msg__dynamic_reconfigure__DoubleParameter(
  UrosTcpRosStatus *tcpstp,
  struct msg__dynamic_reconfigure__DoubleParameter *objp
);

/*~~~ MESSAGE: dynamic_reconfigure/StrParameter ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

size_t length_msg__dynamic_reconfigure__StrParameter(
  struct msg__dynamic_reconfigure__StrParameter *objp
);
void init_msg__dynamic_reconfigure__StrParameter(
  struct msg__dynamic_reconfigure__StrParameter *objp
);
void clean_msg__dynamic_reconfigure__StrParameter(
  struct msg__dynamic_reconfigure__StrParameter *objp
);
uros_err_t recv_msg__dynamic_reconfigure__StrParameter(
  UrosTcpRosStatus *tcpstp,
  struct msg__dynamic_reconfigure__StrParameter *objp
);
uros_err_t send_msg__dynamic_reconfigure__StrParameter(
  UrosTcpRosStatus *tcpstp,
  struct msg__dynamic_reconfigure__StrParameter *objp
);

/*~~~ MESSAGE: sensor_msgs/RegionOfInterest ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

size_t length_msg__sensor_msgs__RegionOfInterest(
  struct msg__sensor_msgs__RegionOfInterest *objp
);
void init_msg__sensor_msgs__RegionOfInterest(
  struct msg__sensor_msgs__RegionOfInterest *objp
);
void clean_msg__sensor_msgs__RegionOfInterest(
  struct msg__sensor_msgs__RegionOfInterest *objp
);
uros_err_t recv_msg__sensor_msgs__RegionOfInterest(
  UrosTcpRosStatus *tcpstp,
  struct msg__sensor_msgs__RegionOfInterest *objp
);
uros_err_t send_msg__sensor_msgs__RegionOfInterest(
  UrosTcpRosStatus *tcpstp,
  struct msg__sensor_msgs__RegionOfInterest *objp
);

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

/*~~~ MESSAGE: dynamic_reconfigure/Config ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

size_t length_msg__dynamic_reconfigure__Config(
  struct msg__dynamic_reconfigure__Config *objp
);
void init_msg__dynamic_reconfigure__Config(
  struct msg__dynamic_reconfigure__Config *objp
);
void clean_msg__dynamic_reconfigure__Config(
  struct msg__dynamic_reconfigure__Config *objp
);
uros_err_t recv_msg__dynamic_reconfigure__Config(
  UrosTcpRosStatus *tcpstp,
  struct msg__dynamic_reconfigure__Config *objp
);
uros_err_t send_msg__dynamic_reconfigure__Config(
  UrosTcpRosStatus *tcpstp,
  struct msg__dynamic_reconfigure__Config *objp
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

/*~~~ MESSAGE: sensor_msgs/Image ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

size_t length_msg__sensor_msgs__Image(
  struct msg__sensor_msgs__Image *objp
);
void init_msg__sensor_msgs__Image(
  struct msg__sensor_msgs__Image *objp
);
void clean_msg__sensor_msgs__Image(
  struct msg__sensor_msgs__Image *objp
);
uros_err_t recv_msg__sensor_msgs__Image(
  UrosTcpRosStatus *tcpstp,
  struct msg__sensor_msgs__Image *objp
);
uros_err_t send_msg__sensor_msgs__Image(
  UrosTcpRosStatus *tcpstp,
  struct msg__sensor_msgs__Image *objp
);

/*~~~ MESSAGE: stereo_msgs/DisparityImage ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

size_t length_msg__stereo_msgs__DisparityImage(
  struct msg__stereo_msgs__DisparityImage *objp
);
void init_msg__stereo_msgs__DisparityImage(
  struct msg__stereo_msgs__DisparityImage *objp
);
void clean_msg__stereo_msgs__DisparityImage(
  struct msg__stereo_msgs__DisparityImage *objp
);
uros_err_t recv_msg__stereo_msgs__DisparityImage(
  UrosTcpRosStatus *tcpstp,
  struct msg__stereo_msgs__DisparityImage *objp
);
uros_err_t send_msg__stereo_msgs__DisparityImage(
  UrosTcpRosStatus *tcpstp,
  struct msg__stereo_msgs__DisparityImage *objp
);

/*~~~ MESSAGE: bond/Constants ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

size_t length_msg__bond__Constants(
  struct msg__bond__Constants *objp
);
void init_msg__bond__Constants(
  struct msg__bond__Constants *objp
);
void clean_msg__bond__Constants(
  struct msg__bond__Constants *objp
);
uros_err_t recv_msg__bond__Constants(
  UrosTcpRosStatus *tcpstp,
  struct msg__bond__Constants *objp
);
uros_err_t send_msg__bond__Constants(
  UrosTcpRosStatus *tcpstp,
  struct msg__bond__Constants *objp
);

/*============================================================================*/
/* SERVICE PROTOTYPES                                                         */
/*============================================================================*/

/*~~~ SERVICE: bond/Constants ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

size_t length_in_srv__dynamic_reconfigure__Reconfigure(
  struct in_srv__dynamic_reconfigure__Reconfigure *objp
);
size_t length_out_srv__dynamic_reconfigure__Reconfigure(
  struct out_srv__dynamic_reconfigure__Reconfigure *objp
);
void init_in_srv__dynamic_reconfigure__Reconfigure(
  struct in_srv__dynamic_reconfigure__Reconfigure *objp
);
void init_out_srv__dynamic_reconfigure__Reconfigure(
  struct out_srv__dynamic_reconfigure__Reconfigure *objp
);
void clean_in_srv__dynamic_reconfigure__Reconfigure(
  struct in_srv__dynamic_reconfigure__Reconfigure *objp
);
void clean_out_srv__dynamic_reconfigure__Reconfigure(
  struct out_srv__dynamic_reconfigure__Reconfigure *objp
);
uros_err_t recv_in_srv__dynamic_reconfigure__Reconfigure(
  UrosTcpRosStatus *tcpstp,
  struct in_srv__dynamic_reconfigure__Reconfigure *objp
);
uros_err_t send_out_srv__dynamic_reconfigure__Reconfigure(
  UrosTcpRosStatus *tcpstp,
  struct out_srv__dynamic_reconfigure__Reconfigure *objp
);

/*============================================================================*/
/* GLOBAL PROTOTYPES                                                          */
/*============================================================================*/

void urosMsgTypesRegStaticTypes(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _UROSMSGTYPES_H_ */

