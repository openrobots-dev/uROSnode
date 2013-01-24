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
 * @file    urosHandlers.c
 * @author  Your Name <your.email@example.com>
 *
 * @brief   TCPROS topic and service handlers.
 */

/*============================================================================*/
/* HEADER FILES                                                               */
/*============================================================================*/

#include "urosHandlers.h"

#include <urosNode.h>
#include <urosTcpRos.h>
#include <urosUser.h>

/*============================================================================*/
/* PUBLISHED TOPIC FUNCTIONS                                                  */
/*============================================================================*/

/** @addtogroup tcpros_pubtopic_funcs */
/** @{ */

/*~~~ PUBLISHED TOPIC: /output ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Topic <tt>/output</tt> publisher */
/** @{ */

/**
 * @brief   TCPROS <tt>/output</tt> published topic handler.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @return
 *          Error code.
 */
uros_err_t pub_tpc__output(UrosTcpRosStatus *tcpstp) {

  /* Message allocation and initialization.*/
  UROS_TPC_INIT_H(msg__stereo_msgs__DisparityImage);

  /* Published messages loop.*/
  while (!urosTcpRosStatusCheckExit(tcpstp)) {
    /* TODO: Generate the contents of the message.*/
    urosThreadSleepSec(1); continue; /* TODO: Remove this dummy line.*/

    /* Send the message.*/
    UROS_MSG_SEND_LENGTH(msgp, msg__stereo_msgs__DisparityImage);
    UROS_MSG_SEND_BODY(msgp, msg__stereo_msgs__DisparityImage);

    /* Dispose the contents of the message.*/
    clean_msg__stereo_msgs__DisparityImage(msgp);
  }
  tcpstp->err = UROS_OK;

_finally:
  /* Message deinitialization and deallocation.*/
  UROS_TPC_UNINIT_H(msg__stereo_msgs__DisparityImage);
  return tcpstp->err;
}

/** @} */

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

  /* Message allocation and initialization.*/
  UROS_TPC_INIT_H(msg__rosgraph_msgs__Log);

  /* Published messages loop.*/
  while (!urosTcpRosStatusCheckExit(tcpstp)) {
    /* TODO: Generate the contents of the message.*/
    urosThreadSleepSec(1); continue; /* TODO: Remove this dummy line.*/

    /* Send the message.*/
    UROS_MSG_SEND_LENGTH(msgp, msg__rosgraph_msgs__Log);
    UROS_MSG_SEND_BODY(msgp, msg__rosgraph_msgs__Log);

    /* Dispose the contents of the message.*/
    clean_msg__rosgraph_msgs__Log(msgp);
  }
  tcpstp->err = UROS_OK;

_finally:
  /* Message deinitialization and deallocation.*/
  UROS_TPC_UNINIT_H(msg__rosgraph_msgs__Log);
  return tcpstp->err;
}

/** @} */

/** @} */

/*============================================================================*/
/* SUBSCRIBED TOPIC FUNCTIONS                                                 */
/*============================================================================*/

/** @addtogroup tcpros_subtopic_funcs */
/** @{ */

/*~~~ SUBSCRIBED TOPIC: /constants ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Topic <tt>/constants</tt> subscriber */
/** @{ */

/**
 * @brief   TCPROS <tt>/constants</tt> subscribed topic handler.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @return
 *          Error code.
 */
uros_err_t sub_tpc__constants(UrosTcpRosStatus *tcpstp) {

  /* Message allocation and initialization.*/
  UROS_TPC_INIT_H(msg__bond__Constants);

  /* Subscribed messages loop.*/
  while (!urosTcpRosStatusCheckExit(tcpstp)) {
    /* Receive the next message.*/
    UROS_MSG_RECV_LENGTH();
    UROS_MSG_RECV_BODY(msgp, msg__bond__Constants);

    /* TODO: Process the received message.*/

    /* Dispose the contents of the message.*/
    clean_msg__bond__Constants(msgp);
  }
  tcpstp->err = UROS_OK;

_finally:
  /* Message deinitialization and deallocation.*/
  UROS_TPC_UNINIT_H(msg__bond__Constants);
  return tcpstp->err;
}

/** @} */

/*~~~ SUBSCRIBED TOPIC: /input ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Topic <tt>/input</tt> subscriber */
/** @{ */

/**
 * @brief   TCPROS <tt>/input</tt> subscribed topic handler.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @return
 *          Error code.
 */
uros_err_t sub_tpc__input(UrosTcpRosStatus *tcpstp) {

  /* Message allocation and initialization.*/
  UROS_TPC_INIT_H(msg__stereo_msgs__DisparityImage);

  /* Subscribed messages loop.*/
  while (!urosTcpRosStatusCheckExit(tcpstp)) {
    /* Receive the next message.*/
    UROS_MSG_RECV_LENGTH();
    UROS_MSG_RECV_BODY(msgp, msg__stereo_msgs__DisparityImage);

    /* TODO: Process the received message.*/

    /* Dispose the contents of the message.*/
    clean_msg__stereo_msgs__DisparityImage(msgp);
  }
  tcpstp->err = UROS_OK;

_finally:
  /* Message deinitialization and deallocation.*/
  UROS_TPC_UNINIT_H(msg__stereo_msgs__DisparityImage);
  return tcpstp->err;
}

/** @} */

/** @} */

/*============================================================================*/
/* PUBLISHED SERVICE FUNCTIONS                                                */
/*============================================================================*/

/** @addtogroup tcpros_pubservice_funcs */
/** @{ */

/*~~~ PUBLISHED SERVICE: /reconfigure ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Service <tt>/reconfigure</tt> publisher */
/** @{ */

/**
 * @brief   TCPROS <tt>/reconfigure</tt> published service handler.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @return
 *          Error code.
 */
uros_err_t pub_srv__reconfigure(UrosTcpRosStatus *tcpstp) {

  /* Service messages allocation and initialization.*/
  UROS_SRV_INIT_HISO(in_srv__dynamic_reconfigure__Reconfigure,
                     out_srv__dynamic_reconfigure__Reconfigure);

  /* Service message loop (if the service is persistent).*/
  do {
    /* Receive the request message.*/
    UROS_MSG_RECV_LENGTH();
    UROS_MSG_RECV_BODY(inmsgp, in_srv__dynamic_reconfigure__Reconfigure);

    /* TODO: Process the request message.*/
    tcpstp->err = UROS_OK;
    urosStringClean(&tcpstp->errstr);
    okByte = 1;

    /* Dispose the contents of the request message.*/
    clean_in_srv__dynamic_reconfigure__Reconfigure(inmsgp);

    /* TODO: Generate the contents of the response message.*/

    /* Send the response message.*/
    UROS_SRV_SEND_OKBYTE_ERRSTR();
    UROS_MSG_SEND_LENGTH(&outmsg, out_srv__dynamic_reconfigure__Reconfigure);
    UROS_MSG_SEND_BODY(&outmsg, out_srv__dynamic_reconfigure__Reconfigure);

    /* Dispose the contents of the response message.*/
    clean_out_srv__dynamic_reconfigure__Reconfigure(&outmsg);
  } while (tcpstp->topicp->flags.persistent && !urosTcpRosStatusCheckExit(tcpstp));
  tcpstp->err = UROS_OK;

_finally:
  /* Service messages deinitialization and deallocation.*/
  UROS_SRV_UNINIT_HISO(in_srv__dynamic_reconfigure__Reconfigure,
                       out_srv__dynamic_reconfigure__Reconfigure);
  return tcpstp->err;
}

/** @} */

/** @} */

/*============================================================================*/
/* CALLED SERVICE FUNCTIONS                                                   */
/*============================================================================*/

/** @addtogroup tcpros_callservice_funcs */
/** @{ */

/*~~~ CALLED SERVICE: /reconfigure ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/** @name Service <tt>/reconfigure</tt> caller */
/** @{ */

/**
 * @brief   TCPROS <tt>/reconfigure</tt> called service handler.
 *
 * @param[in,out] tcpstp
 *          Pointer to a working @p UrosTcpRosStatus object.
 * @param[in] inmsgp
 *          Pointer to the initialized request message.
 * @param[out] outmsgp
 *          Pointer to the allocated response message. It will be initialized
 *          by this function. The service result will be written there only
 *          if the call is successful.
 * @return
 *          Error code.
 */
uros_err_t call_srv__reconfigure(
  UrosTcpRosStatus *tcpstp,
  struct in_srv__dynamic_reconfigure__Reconfigure *inmsgp,
  struct out_srv__dynamic_reconfigure__Reconfigure *outmsgp
) {

  /* Service messages allocation and initialization.*/
  UROS_SRVCALL_INIT(in_srv__dynamic_reconfigure__Reconfigure,
                    out_srv__dynamic_reconfigure__Reconfigure);

  /* Send the request message.*/
  UROS_MSG_SEND_LENGTH(inmsgp, in_srv__dynamic_reconfigure__Reconfigure);
  UROS_MSG_SEND_BODY(inmsgp, in_srv__dynamic_reconfigure__Reconfigure);

  /* TODO: Dispose the contents of the request message.*/

  /* Receive the response message.*/
  UROS_SRV_RECV_OKBYTE();
  UROS_MSG_RECV_LENGTH();
  UROS_MSG_RECV_BODY(outmsgp, out_srv__dynamic_reconfigure__Reconfigure);

  tcpstp->err = UROS_OK;
_finally:
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
void urosHandlersPublishTopics(void) {

  /* /output */
  urosNodePublishTopicSZ(
    "/output",
    "stereo_msgs/DisparityImage",
    (uros_proc_f)pub_tpc__output,
    uros_nulltopicflags
  );

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
void urosHandlersUnpublishTopics(void) {

  /* /output */
  urosNodeUnpublishTopicSZ(
    "/output"
  );

  /* /rosout */
  urosNodeUnpublishTopicSZ(
    "/rosout"
  );
}

/**
 * @brief   Registers all the subscribed topics to the Master node.
 * @note    Should be called at node initialization.
 */
void urosHandlersSubscribeTopics(void) {

  /* /constants */
  urosNodeSubscribeTopicSZ(
    "/constants",
    "bond/Constants",
    (uros_proc_f)sub_tpc__constants,
    uros_nulltopicflags
  );

  /* /input */
  urosNodeSubscribeTopicSZ(
    "/input",
    "stereo_msgs/DisparityImage",
    (uros_proc_f)sub_tpc__input,
    uros_nulltopicflags
  );
}

/**
 * @brief   Unregisters all the subscribed topics to the Master node.
 * @note    Should be called at node shutdown.
 */
void urosHandlersUnsubscribeTopics(void) {

  /* /constants */
  urosNodeUnsubscribeTopicSZ(
    "/constants"
  );

  /* /input */
  urosNodeUnsubscribeTopicSZ(
    "/input"
  );
}

/**
 * @brief   Registers all the published services to the Master node.
 * @note    Should be called at node initialization.
 */
void urosHandlersPublishServices(void) {

  /* /reconfigure */
  urosNodePublishServiceSZ(
    "/reconfigure",
    "dynamic_reconfigure/Reconfigure",
    (uros_proc_f)pub_srv__reconfigure,
    uros_nullserviceflags
  );
}

/**
 * @brief   Unregisters all the published services to the Master node.
 * @note    Should be called at node shutdown.
 */
void urosHandlersUnpublishServices(void) {

  /* /reconfigure */
  urosNodeUnpublishServiceSZ(
    "/reconfigure"
  );
}

/** @} */

