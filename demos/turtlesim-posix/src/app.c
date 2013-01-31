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
 * @brief   Application source code.
 */

/*===========================================================================*/
/* HEADER FILES                                                              */
/*===========================================================================*/

#include <urosBase.h>
#include <urosUser.h>
#include <urosNode.h>

#include <math.h>

#include "app.h"
#include "urosHandlers.h"

/*===========================================================================*/
/* TYPES & MACROS                                                            */
/*===========================================================================*/

#define min(a,b)    (((a) <= (b)) ? (a) : (b))
#define max(a,b)    (((a) >= (b)) ? (a) : (b))
#define _2PI        ((float)(2.0 * M_PI))

/*===========================================================================*/
/* GLOBAL VARIABLES                                                          */
/*===========================================================================*/

fifo_t rosoutQueue;

turtle_t turtles[MAX_TURTLES];

static UROS_STACKPOOL(turtlesThreadStacks, TURTLE_THREAD_STKSIZE, MAX_TURTLES);
UrosMemPool turtlesMemPool;
UrosThreadPool turtlesThreadPool;

uros_bool_t turtleCanSpawn = UROS_FALSE;
UrosMutex turtleCanSpawnLock;

UrosString backcolparnameR;
UrosString backcolparnameG;
UrosString backcolparnameB;
struct msg__turtlesim__Color backgroundColor = { 123, 132, 213 };
UrosMutex backgroundColorLock;

/*===========================================================================*/
/* GLOBAL FUNCTIONS                                                          */
/*===========================================================================*/

/*~~~ FIFO MESSAGE QUEUE ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

void fifo_init(fifo_t *queuep, unsigned length) {

  urosAssert(queuep != NULL);
  urosAssert(length > 0);

  urosSemObjectInit(&queuep->freeSem, length);
  urosSemObjectInit(&queuep->usedSem, 0);
  queuep->length = length;
  queuep->head = 0;
  queuep->tail = 0;
  urosMutexObjectInit(&queuep->slotsMtx);
  queuep->slots = urosArrayNew(NULL, length, void *);
  urosAssert(queuep->slots != NULL);
}

void fifo_enqueue(fifo_t *queuep, void *msgp) {

  urosAssert(queuep != NULL);
  urosAssert(msgp != NULL);

  urosSemWait(&queuep->freeSem);
  urosMutexLock(&queuep->slotsMtx);
  queuep->slots[queuep->tail] = msgp;
  if (++queuep->tail >= queuep->length) {
    queuep->tail = 0;
  }
  urosMutexUnlock(&queuep->slotsMtx);
  urosSemSignal(&queuep->usedSem);
}

void *fifo_dequeue(fifo_t *queuep) {

  void *msgp;

  urosAssert(queuep != NULL);

  urosSemWait(&queuep->usedSem);
  urosMutexLock(&queuep->slotsMtx);
  msgp = queuep->slots[queuep->head];
  if (++queuep->head >= queuep->length) {
    queuep->head = 0;
  }
  urosMutexUnlock(&queuep->slotsMtx);
  urosSemSignal(&queuep->freeSem);
  return msgp;
}

/*~~~ ROSOUT ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

void rosout_post(UrosString *strp, uros_bool_t costant, uint8_t level,
                 const char *fileszp, int line, const char *funcp) {

  static uint32_t seq = 0;

  struct msg__rosgraph_msgs__Log *msgp;

  urosAssert(urosStringIsValid(strp));

  msgp = urosNew(NULL, struct msg__rosgraph_msgs__Log);
  urosAssert(msgp != NULL);
  init_msg__rosgraph_msgs__Log(msgp);

  msgp->header.frame_id = urosStringAssignZ(costant ? "1" : "0");
  msgp->header.seq = seq++;
  msgp->header.stamp.sec = urosGetTimestampMsec();
  msgp->header.stamp.nsec = (msgp->header.stamp.sec % 1000) * 1000000;
  msgp->header.stamp.sec /= 1000;
  msgp->level = level;
  msgp->name = urosNode.config.nodeName;
  msgp->msg = *strp;
  msgp->file = urosStringAssignZ(fileszp);
  msgp->function = urosStringAssignZ(funcp);
  msgp->line = line;

  fifo_enqueue(&rosoutQueue, (void *)msgp);
}

void rosout_fetch(struct msg__rosgraph_msgs__Log **msgpp) {

  urosAssert(msgpp != NULL);

  *msgpp = (struct msg__rosgraph_msgs__Log *)fifo_dequeue(&rosoutQueue);
}

/*~~~ APPLICATION ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

void app_initialize(void) {

  static const UrosString turtle1 = { 7, "turtle1" };
  static const UrosNodeConfig *const cfgp = &urosNode.config;

  unsigned i;

  /* Initialize the uROS system.*/
  urosInit();
  fifo_init(&rosoutQueue, 8);

  /* Initialize variables related to the background color.*/
  urosMutexObjectInit(&backgroundColorLock);

  backcolparnameR.length = cfgp->nodeName.length + 13;
  backcolparnameR.datap = (char*)urosAlloc(NULL, backcolparnameR.length);
  urosAssert(backcolparnameR.datap != NULL);
  memcpy(backcolparnameR.datap, cfgp->nodeName.datap, cfgp->nodeName.length);
  memcpy(backcolparnameR.datap + cfgp->nodeName.length, "/background_r", 13);

  backcolparnameG.length = cfgp->nodeName.length + 13;
  backcolparnameG.datap = (char*)urosAlloc(NULL, backcolparnameG.length);
  urosAssert(backcolparnameG.datap != NULL);
  memcpy(backcolparnameG.datap, cfgp->nodeName.datap, cfgp->nodeName.length);
  memcpy(backcolparnameG.datap + cfgp->nodeName.length, "/background_g", 13);

  backcolparnameB.length = cfgp->nodeName.length + 13;
  backcolparnameB.datap = (char*)urosAlloc(NULL, backcolparnameB.length);
  urosAssert(backcolparnameB.datap != NULL);
  memcpy(backcolparnameB.datap, cfgp->nodeName.datap, cfgp->nodeName.length);
  memcpy(backcolparnameB.datap + cfgp->nodeName.length, "/background_b", 13);

  /* Initialize the turtle slots.*/
  urosMutexObjectInit(&turtleCanSpawnLock);
  turtleCanSpawn = UROS_TRUE;
  turtle_init_pools();
  for (i = 0; i < MAX_TURTLES; ++i) {
    turtle_init(&turtles[i], i);
  }

  /* Create the Node thread.*/
  urosNodeCreateThread();

  /* Spawn the first turtle.*/
  turtle_spawn(&turtle1, 0.5f * SANDBOX_WIDTH, 0.5f * SANDBOX_HEIGHT, 0.0f);
}

/*~~~ TURTLE ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

void turtle_init_pools(void) {

  urosMemPoolObjectInit(&turtlesMemPool,
                        sizeof(void*) + TURTLE_THREAD_STKSIZE,
                        NULL);

  urosMemPoolLoadArray(&turtlesMemPool, turtlesThreadStacks, MAX_TURTLES);

  urosThreadPoolObjectInit(&turtlesThreadPool, &turtlesMemPool,
                           (uros_proc_f)turtle_brain_thread, "turtle_brain",
                           TURTLE_THREAD_PRIO);

  urosThreadPoolCreateAll(&turtlesThreadPool);
}

void turtle_init(turtle_t *turtlep, unsigned id) {

  urosAssert(turtlep != NULL);

  urosMutexObjectInit(&turtlep->lock);
  turtlep->id = id;
  urosStringObjectInit(&turtlep->name);
  urosStringObjectInit(&turtlep->poseTopic);
  urosStringObjectInit(&turtlep->velTopic);
  urosStringObjectInit(&turtlep->setpenService);
  urosStringObjectInit(&turtlep->telabsService);
  urosStringObjectInit(&turtlep->telrelService);
  turtlep->pose.x = 0;
  turtlep->pose.y = 0;
  turtlep->pose.theta = 0;
  turtlep->pose.linear_velocity = 0;
  turtlep->pose.angular_velocity = 0;
  turtlep->countdown = 0;
  turtlep->status = TURTLE_EMPTY;
  turtlep->refCnt = 0;
}

uros_err_t turtle_brain_thread(turtle_t *turtlep) {

  struct msg__turtlesim__Pose *posep;

  urosAssert(turtlep != NULL);

  /* Simple integration.*/
  posep = (struct msg__turtlesim__Pose *)&turtlep->pose;
  urosMutexLock(&turtlep->lock);
  while (turtlep->status == TURTLE_ALIVE) {
    /* Execute commands until their deadline.*/
    if (turtlep->countdown > 0) {
      --turtlep->countdown;
      posep->x += (float)cos(posep->theta) * posep->linear_velocity
                  * (0.001f * TURTLE_THREAD_PERIOD_MS);
      posep->y += (float)sin(posep->theta) * posep->linear_velocity
                  * (0.001f * TURTLE_THREAD_PERIOD_MS);
      posep->theta += posep->angular_velocity
                      * (0.001f * TURTLE_THREAD_PERIOD_MS);

      /* Clamp values.*/
      if (posep->x < 0 || posep->x > SANDBOX_WIDTH ||
          posep->y < 0 || posep->y > SANDBOX_WIDTH) {
        static const UrosString msg = { 19, "Turtle hit the wall" };
        rosout_warn((UrosString*)&msg, UROS_TRUE);
      }
      posep->x = min(max(0, posep->x), SANDBOX_WIDTH);
      posep->y = min(max(0, posep->y), SANDBOX_HEIGHT);
      while (posep->theta < 0)     { posep->theta += _2PI; }
      while (posep->theta >= _2PI) { posep->theta -= _2PI; }
    } else {
      posep->linear_velocity = 0;
      posep->angular_velocity = 0;
    }
    urosMutexUnlock(&turtlep->lock);
    urosThreadSleepMsec(TURTLE_THREAD_PERIOD_MS);
    urosMutexLock(&turtlep->lock);
  }
  turtle_unref(turtlep);
  urosMutexUnlock(&turtlep->lock);
  return UROS_OK;
}

turtle_t *turtle_spawn(const UrosString *namep,
                       float x, float y, float theta) {

  static const char *const posend    = "/pose";
  static const char *const colsenend = "/color_sensor";
  static const char *const velend    = "/command_velocity";
  static const char *const setpenend = "/set_pen";
  static const char *const telabsend = "/teleport_absolute";
  static const char *const telrelend = "/teleport_relative";

  turtle_t *turtlep;
  unsigned i, numAlive;
  uros_err_t err;

  urosAssert(urosStringNotEmpty(namep));

  /* Check if the turtle can be spawned.*/
  urosMutexLock(&turtleCanSpawnLock);
  if (!turtleCanSpawn) {
    urosMutexUnlock(&turtleCanSpawnLock);
    return NULL;
  }
  urosMutexUnlock(&turtleCanSpawnLock);

  /* Fill an empty slot.*/
  for (turtlep = NULL, numAlive = 0; turtlep == NULL;) {
    for (i = 0, turtlep = turtles; i < MAX_TURTLES; ++turtlep, ++i) {
      urosMutexLock(&turtlep->lock);
      if (turtlep->status == TURTLE_ALIVE) {
        urosError(0 == urosStringCmp(&turtlep->name, namep),
                  { urosMutexUnlock(&turtlep->lock); return NULL; },
                  ("A turtle named [%.*s] is alive\n", UROS_STRARG(namep)));
        ++numAlive;
      }
      if (turtlep->status == TURTLE_EMPTY) {
        break;
      }
      urosMutexUnlock(&turtlep->lock);
    }
    if (numAlive == MAX_TURTLES) {
      /* All the turtles are alive, sorry.*/
      return NULL;
    }
    if (i == MAX_TURTLES) {
      /* Wait for 10ms to let referencing threads release a slot.*/
      urosThreadSleepMsec(10);
    }
  }

  /* Build the topic names.*/
#define _ALLOCFIELD(field, endsz) \
  { urosStringObjectInit(&turtlep->field); \
    turtlep->field.length = 1 + namep->length + strlen(endsz); \
    turtlep->field.datap = (char*)urosAlloc(NULL, turtlep->field.length + 1); }

  _ALLOCFIELD(poseTopic, posend);
  _ALLOCFIELD(colsenTopic, colsenend);
  _ALLOCFIELD(velTopic, velend);
  _ALLOCFIELD(setpenService, setpenend);
  _ALLOCFIELD(telabsService, telabsend);
  _ALLOCFIELD(telrelService, telrelend);

#undef _ALLOCFIELD

  if (turtlep->poseTopic.datap == NULL ||
      turtlep->colsenTopic.datap == NULL ||
      turtlep->velTopic.datap == NULL ||
      turtlep->setpenService.datap == NULL ||
      turtlep->telabsService.datap == NULL ||
      turtlep->telrelService.datap == NULL) {
    urosStringClean(&turtlep->poseTopic);
    urosStringClean(&turtlep->colsenTopic);
    urosStringClean(&turtlep->velTopic);
    urosStringClean(&turtlep->setpenService);
    urosStringClean(&turtlep->telabsService);
    urosStringClean(&turtlep->telrelService);
    urosMutexUnlock(&turtlep->lock);
    return NULL;
  }

#define _BUILDFIELD(field, endsz) \
  { turtlep->poseTopic.datap[0] = '/'; \
    memcpy(1 + turtlep->field.datap, namep->datap, namep->length); \
    memcpy(1 + turtlep->field.datap + namep->length, endsz, strlen(endsz) + 1); }

  _BUILDFIELD(poseTopic, posend);
  _BUILDFIELD(colsenTopic, colsenend);
  _BUILDFIELD(velTopic, velend);
  _BUILDFIELD(setpenService, setpenend);
  _BUILDFIELD(telabsService, telabsend);
  _BUILDFIELD(telrelService, telrelend);

#undef _BUILDFIELD

  /* Assign the new attributes.*/
  turtlep->name = urosStringClone(namep);
  turtlep->pose.x = min(max(0, x), SANDBOX_WIDTH);
  turtlep->pose.y = min(max(0, y), SANDBOX_HEIGHT);
  turtlep->pose.theta = theta;
  while (turtlep->pose.theta < 0)     { turtlep->pose.theta += _2PI; }
  while (turtlep->pose.theta >= _2PI) { turtlep->pose.theta -= _2PI; }
  turtlep->pose.linear_velocity = 0;
  turtlep->pose.angular_velocity = 0;
  turtlep->countdown = 0;
  turtlep->status = TURTLE_ALIVE;
  turtlep->refCnt = 1; /* For the brain thread only.*/

  /* Publish "<turtle>/pose" .*/
  err = urosNodePublishTopicSZ(
    turtlep->poseTopic.datap,
    "turtlesim/Pose",
    (uros_proc_f)pub_tpc__turtleX__pose,
    uros_nulltopicflags
  );
  urosError(err != UROS_OK,
            goto _error,
            ("Error %s while publishing topic [%s]\n",
             urosErrorText(err), turtlep->poseTopic.datap));

  /* Publish "<turtle>/color_sensor" .*/
  err = urosNodePublishTopicSZ(
    turtlep->colsenTopic.datap,
    "turtlesim/Color",
    (uros_proc_f)pub_tpc__turtleX__color_sensor,
    uros_nulltopicflags
  );
  urosError(err != UROS_OK,
            { urosNodeUnpublishTopic(&turtlep->poseTopic);
              goto _error; },
            ("Error %s while publishing topic [%s]\n",
             urosErrorText(err), turtlep->colsenTopic.datap));

  /* Subscribe to "<turtle>/command_velocity".*/
  err = urosNodeSubscribeTopicSZ(
    turtlep->velTopic.datap,
    "turtlesim/Velocity",
    (uros_proc_f)sub_tpc__turtleX__command_velocity,
    uros_nulltopicflags
  );
  urosError(err != UROS_OK,
            { urosNodeUnpublishTopic(&turtlep->poseTopic);
              urosNodeUnpublishTopic(&turtlep->colsenTopic);
              goto _error; },
            ("Error %s while subscribing to topic [%s]\n",
             urosErrorText(err), turtlep->velTopic.datap));

  /* Publish "<turtle>/set_pen".*/
  err = urosNodePublishServiceSZ(
    turtlep->setpenService.datap,
    "turtlesim/SetPen",
    (uros_proc_f)pub_srv__turtleX__set_pen,
    uros_nullserviceflags
  );
  urosError(err != UROS_OK,
            { urosNodeUnpublishTopic(&turtlep->poseTopic);
              urosNodeUnpublishTopic(&turtlep->colsenTopic);
              urosNodeUnpublishTopic(&turtlep->velTopic);
              goto _error; },
            ("Error %s while publishing service [%s]\n",
             urosErrorText(err), turtlep->setpenService.datap));

  /* Publish "<turtle>/teleport_absolute".*/
  err = urosNodePublishServiceSZ(
    turtlep->telabsService.datap,
    "turtlesim/TeleportAbsolute",
    (uros_proc_f)pub_srv__turtleX__teleport_absolute,
    uros_nullserviceflags
  );
  urosError(err != UROS_OK,
            { urosNodeUnpublishTopic(&turtlep->poseTopic);
              urosNodeUnpublishTopic(&turtlep->colsenTopic);
              urosNodeUnpublishTopic(&turtlep->velTopic);
              urosNodeUnpublishService(&turtlep->setpenService);
              goto _error; },
            ("Error %s while publishing service [%s]\n",
             urosErrorText(err), turtlep->telabsService.datap));

  /* Publish "<turtle>/teleport_relative".*/
  err = urosNodePublishServiceSZ(
    turtlep->telrelService.datap,
    "turtlesim/TeleportRelative",
    (uros_proc_f)pub_srv__turtleX__teleport_relative,
    uros_nullserviceflags
  );
  urosError(err != UROS_OK,
            { urosNodeUnpublishTopic(&turtlep->poseTopic);
              urosNodeUnpublishTopic(&turtlep->colsenTopic);
              urosNodeUnpublishTopic(&turtlep->velTopic);
              urosNodeUnpublishService(&turtlep->setpenService);
              urosNodeUnpublishService(&turtlep->telabsService);
              goto _error; },
            ("Error %s while publishing service [%s]\n",
             urosErrorText(err), turtlep->telrelService.datap));

  /* Start its new brain.*/
  err = urosThreadPoolStartWorker(&turtlesThreadPool, (void*)turtlep);
  urosAssert(err == UROS_OK);
  urosMutexUnlock(&turtlep->lock);
  return turtlep;

_error:
  turtlep->status = TURTLE_EMPTY;
  turtlep->refCnt = 0;
  urosStringClean(&turtlep->poseTopic);
  urosStringClean(&turtlep->colsenTopic);
  urosStringClean(&turtlep->velTopic);
  urosStringClean(&turtlep->setpenService);
  urosStringClean(&turtlep->telabsService);
  urosStringClean(&turtlep->telrelService);
  urosMutexUnlock(&turtlep->lock);
  return NULL;
}

void turtle_kill(turtle_t *turtlep) {

  uros_err_t err;

  urosAssert(turtlep != NULL);
  urosAssert(turtlep->status == TURTLE_ALIVE);

  /* Unpublish its topics.*/
  err = urosNodeUnpublishTopic(&turtlep->poseTopic);
  urosError(err != UROS_OK, UROS_NOP,
            ("Error %s while unpublishing topic [%.*s]\n",
             urosErrorText(err), UROS_STRARG(&turtlep->poseTopic)));

  err = urosNodeUnpublishTopic(&turtlep->colsenTopic);
  urosError(err != UROS_OK, UROS_NOP,
            ("Error %s while unpublishing topic [%.*s]\n",
             urosErrorText(err), UROS_STRARG(&turtlep->colsenTopic)));

  /* Unsubscribe its topics.*/
  err = urosNodeUnsubscribeTopic(&turtlep->velTopic);
  urosError(err != UROS_OK, UROS_NOP,
            ("Error %s while unsubscribing topic [%.*s]\n",
             urosErrorText(err), UROS_STRARG(&turtlep->velTopic)));

  /* Unpublish its services.*/
  err = urosNodeUnpublishService(&turtlep->setpenService);
  urosError(err != UROS_OK, UROS_NOP,
            ("Error %s while unpublishing service [%.*s]\n",
             urosErrorText(err), UROS_STRARG(&turtlep->setpenService)));

  err = urosNodeUnpublishService(&turtlep->telabsService);
  urosError(err != UROS_OK, UROS_NOP,
            ("Error %s while unpublishing service [%.*s]\n",
             urosErrorText(err), UROS_STRARG(&turtlep->telabsService)));

  err = urosNodeUnpublishService(&turtlep->telrelService);
  urosError(err != UROS_OK, UROS_NOP,
            ("Error %s while unpublishing service [%.*s]\n",
             urosErrorText(err), UROS_STRARG(&turtlep->telrelService)));

  /* Cleanup fields.*/
  urosMutexLock(&turtlep->lock);
  urosStringClean(&turtlep->name);
  urosStringClean(&turtlep->poseTopic);
  urosStringClean(&turtlep->colsenTopic);
  urosStringClean(&turtlep->velTopic);
  urosStringClean(&turtlep->setpenService);
  urosStringClean(&turtlep->telabsService);
  urosStringClean(&turtlep->telrelService);
  turtlep->status = TURTLE_DEAD;
  urosMutexUnlock(&turtlep->lock);
}

turtle_t *turtle_refbyname(const UrosString *name) {

  turtle_t *turtlep;
  unsigned i;

  urosAssert(urosStringNotEmpty(name));

  /* Find the turtle by its name.*/
  for (turtlep = turtles, i = 0; i < MAX_TURTLES; ++turtlep, ++i) {
    urosMutexLock(&turtlep->lock);
    if (0 == urosStringCmp(name, &turtlep->name)) {
      ++turtlep->refCnt;
      urosMutexUnlock(&turtlep->lock);
      return turtlep;
    }
    urosMutexUnlock(&turtlep->lock);
  }
  return NULL;
}

turtle_t *turtle_refbypath(const UrosString *topicName) {

  turtle_t *turtlep;
  unsigned i;

  urosAssert(urosStringNotEmpty(topicName));
  urosAssert(topicName->datap[0] == '/');

  /* Find the turtle by its topic/service path.*/
  for (turtlep = turtles, i = 0; i < MAX_TURTLES; ++turtlep, ++i) {
    urosMutexLock(&turtlep->lock);
    if (topicName->length >= 1 + turtlep->name.length + 1 &&
        topicName->datap[1 + turtlep->name.length] == '/' &&
        0 == memcmp(topicName->datap + 1, turtlep->name.datap,
                    turtlep->name.length)) {
      ++turtlep->refCnt;
      urosMutexUnlock(&turtlep->lock);
      return turtlep;
    }
    urosMutexUnlock(&turtlep->lock);
  }
  return NULL;
}

void turtle_unref(turtle_t *turtlep) {

  urosAssert(turtlep != NULL);
  urosAssert(turtlep->refCnt > 0);

  /* Note: Must be locked! */
  if (--turtlep->refCnt == 0) {
    urosAssert(turtlep->status == TURTLE_DEAD);
    turtlep->status = TURTLE_EMPTY;
  }
}
