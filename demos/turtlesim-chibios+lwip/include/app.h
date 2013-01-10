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
 * @file    app.c
 * @author  Andrea Zoppi <texzk@email.it>
 *
 * @brief   Application source header.
 */

#ifndef _APP_H_
#define _APP_H_

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include <urosBase.h>
#include "urosHandlers.h"

/*===========================================================================*/
/* TYPES & MACROS                                                            */
/*===========================================================================*/

#define SANDBOX_WIDTH           11
#define SANDBOX_HEIGHT          11

#define MAX_TURTLES             4
#define TURTLE_THREAD_STKSIZE   512
#define TURTLE_THREAD_PRIO      HIGHPRIO
#define TURTLE_THREAD_PERIOD_MS 1

/**
 * @brief   Thread-safe message queue.
 */
typedef struct queue_t {
  UrosSem       freeSem;
  UrosSem       usedSem;
  unsigned      length;
  unsigned      head;
  unsigned      tail;
  UrosMutex     slotsMtx;
  void          **slots;
} fifo_t;

/**
 * @brief   Turtle slot status.
 */
typedef enum turtlestatus_t {
  TURTLE_EMPTY,     /**< @brief Empty slot, a new turtle can be spawned here.*/
  TURTLE_ALIVE,     /**< @brief A living turtle, do not touch.*/
  TURTLE_DEAD       /**< @brief A dead turtle, wait until its related threads are done.*/
} turtlestatus_t;

/**
 * @brief   Turtle descriptor and status.
 */
typedef struct turtle_t {
  UrosMutex         lock;           /**< @brief Guard lock.*/
  unsigned          id;             /**< @brief Turtle ID.*/
  UrosString        name;           /**< @brief Turtle name.*/
  UrosString        poseTopic;      /**< @brief <code><em>turtle</em>/pose</code> topic name.*/
  UrosString        colsenTopic;    /**< @brief <code><em>turtle</em>/color_sensor</code> topic name.*/
  UrosString        velTopic;       /**< @brief <code><em>turtle</em>/command_velocity</code> topic name.*/
  UrosString        setpenService;  /**< @brief <code><em>turtle</em>/set_pen</code> service name.*/
  UrosString        telabsService;  /**< @brief <code><em>turtle</em>/teleport_absolute</code> service name.*/
  UrosString        telrelService;  /**< @brief <code><em>turtle</em>/teleport_relative</code> service name.*/
  struct {
    float x;
    float y;
    float theta;
    float linear_velocity;
    float angular_velocity;
  }                 pose;           /**< @brief Current turtle pose.*/
  uros_cnt_t        countdown;      /**< @brief Command countdown, in milliseconds.*/
  turtlestatus_t    status;         /**< @brief Turtle slot status.*/
  uros_cnt_t        refCnt;         /**< @brief Reference counter.*/
  struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t width;
    uint8_t off;
  }                 pen;            /**< @brief Pen configuration.*/
} turtle_t;

/*===========================================================================*/
/* GLOBAL VARIABLES                                                          */
/*===========================================================================*/

extern fifo_t rosoutQueue;

extern turtle_t turtles[MAX_TURTLES];
extern UrosThreadPool turtlesThreadPool;
extern uros_bool_t turtleCanSpawn;
extern UrosMutex turtleCanSpawnLock;

/*===========================================================================*/
/* GLOBAL PROTOTYPES                                                         */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*~~~ FIFO MESSAGE QUEUE ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

void fifo_init(fifo_t *queuep, unsigned length);
void fifo_enqueue(fifo_t *queuep, void *msgp);
void *fifo_dequeue(fifo_t *queuep);

/*~~~ ROSOUT ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

void rosout_post(UrosString *strp, uros_bool_t constant, uint8_t level,
                 const char *fileszp, int line, const char *funcp);
void rosout_fetch(struct msg__rosgraph_msgs__Log **msgpp);

#define rosout_debug(strp, constant) \
  rosout_post((strp), (constant), msg__rosgraph_msgs__Log__DEBUG, \
              __FILE__, __LINE__, __PRETTY_FUNCTION__);

#define rosout_info(strp, constant) \
  rosout_post((strp), (constant), msg__rosgraph_msgs__Log__INFO, \
              __FILE__, __LINE__, __PRETTY_FUNCTION__);

#define rosout_warn(strp, constant) \
  rosout_post((strp), (constant), msg__rosgraph_msgs__Log__WARN, \
              __FILE__, __LINE__, __PRETTY_FUNCTION__);

#define rosout_error(strp, constant) \
  rosout_post((strp), (constant), msg__rosgraph_msgs__Log__ERROR, \
              __FILE__, __LINE__, __PRETTY_FUNCTION__);

#define rosout_fatal(strp, constant) \
  rosout_post((strp), (constant), msg__rosgraph_msgs__Log__FATAL, \
              __FILE__, __LINE__, __PRETTY_FUNCTION__);

/*~~~ APPLICATION ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

void app_initialize(void);

/*~~~ TURTLE ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

void turtle_init_pools(void);
void turtle_init(turtle_t *turtlep, unsigned id);
uros_err_t turtle_brain_thread(turtle_t *turtlep);
turtle_t *turtle_spawn(const UrosString *namep,
                       float x, float y, float theta);
void turtle_kill(turtle_t *turtlep);
turtle_t *turtle_refbyname(const UrosString *name);
turtle_t *turtle_refbypath(const UrosString *path);
void turtle_unref(turtle_t *turtlep);

#ifdef __cplusplus
}
#endif

#endif /* _APP_H_ */

