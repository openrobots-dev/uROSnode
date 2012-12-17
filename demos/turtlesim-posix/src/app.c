/*
 * app.c
 *
 *  Created on: 19/ott/2012
 *      Author: texzk
 */

#include <urosBase.h>
#include <urosUser.h>
#include <urosNode.h>

#include <math.h>

#include "app.h"
#include "urosTcpRosHandlers.h"

#define min(a,b)    (((a) <= (b)) ? (a) : (b))
#define max(a,b)    (((a) >= (b)) ? (a) : (b))

fifo_t rosoutQueue;

turtle_t turtles[MAX_TURTLES];

UrosThreadPool turtlesThreadPool;
UrosMemPool turtlesMemPool;
static uint8_t turtlesThreadStacks[MAX_TURTLES]
                                  [sizeof(void*) + TURTLE_THREAD_STKSIZE];

void fifo_init(fifo_t *queuep, unsigned length) {

  urosAssert(queuep != NULL);
  urosAssert(length > 0);

  urosSemObjectInit(&queuep->freeSem, length);
  urosSemObjectInit(&queuep->usedSem, 0);
  queuep->length = length;
  queuep->head = 0;
  queuep->tail = 0;
  queuep->slots = urosArrayAlloc(length, void *);
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


void rosout_post(UrosString *strp, uros_bool_t costant, uint8_t level,
                 const char *fileszp, int line, const char *funcp) {

  static uint32_t seq = 0;

  struct msg__rosgraph_msgs__Log *msgp;

  urosAssert(urosStringIsValid(strp));

  msgp = urosNew(struct msg__rosgraph_msgs__Log);
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

/* Sets its own paramaters.*/
void app_set_params(void) {

  UrosString name;
  UrosRpcParam value;
  UrosRpcResponse response;
  uros_err_t err;

  urosRpcParamObjectInit(&value, UROS_RPCP_INT);
  urosRpcResponseObjectInit(&response);

  /* Red background color component.*/
  name.datap = "/turtlesim/background_r";
  name.length = strlen(name.datap);
  value.value.int32 = 123;
  err = urosRpcCallSetParam(&urosNode.config.masterAddr,
                            &urosNode.config.nodeName,
                            &name, &value, &response);
  urosAssert(err == UROS_OK);
  urosRpcResponseClean(&response);

  /* Green background color component.*/
  name.datap = "/turtlesim/background_g";
  name.length = strlen(name.datap);
  value.value.int32 = 123;
  err = urosRpcCallSetParam(&urosNode.config.masterAddr,
                            &urosNode.config.nodeName,
                            &name, &value, &response);
  urosAssert(err == UROS_OK);
  urosRpcResponseClean(&response);

  /* Blue background color component.*/
  name.datap = "/turtlesim/background_b";
  name.length = strlen(name.datap);
  value.value.int32 = 123;
  err = urosRpcCallSetParam(&urosNode.config.masterAddr,
                            &urosNode.config.nodeName,
                            &name, &value, &response);
  urosAssert(err == UROS_OK);
  urosRpcResponseClean(&response);
}

/* Register supported subscribed parameters.*/
void app_register_subscribed_params(void) {

  /* Subscribe to background color components.*/
  urosNodeSubscribeParamSZ("/turtlesim/background_r");
  urosNodeSubscribeParamSZ("/turtlesim/background_g");
  urosNodeSubscribeParamSZ("/turtlesim/background_b");
}

void app_initialize(void) {

  static const UrosString turtle1 = { 7, "turtle1" };

  unsigned i;

  /* Initialize the uROS system.*/
  urosInit();

  /* Initialize the turtle slots.*/
  turtle_init_pools();
  for (i = 0; i < MAX_TURTLES; ++i) {
    turtle_init(&turtles[i], i);
  }

  /* Spawn the first turtle.*/
  turtle_spawn(&turtle1, 0.5f * SANDBOX_WIDTH, 0.5f * SANDBOX_HEIGHT, 0.0f);

  /* Register topics and services.*/
  urosTcpRosRegPubTopics();
  urosTcpRosRegSubTopics();
  urosTcpRosRegPubServices();

  /* Set its own parameters.*/
  app_set_params();

  /* Register supported subscribed topics.*/
  app_register_subscribed_params();

}

void app_background_service(void) {

  /* Sleep forever.*/
  do { urosThreadSleepSec(1); } while (UROS_TRUE);
}

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

  /* Simple integration every 1ms.*/
  urosMutexLock(&turtlep->lock);
  posep = &turtlep->pose;
  while (turtlep->status == TURTLE_ALIVE) {
    /* Execute commands until their deadline.*/
    if (turtlep->countdown > 0) {
      --turtlep->countdown;
      posep->x += (float)cos(posep->theta) * posep->linear_velocity * 0.001f;
      posep->y += (float)sin(posep->theta) * posep->linear_velocity * 0.001f;
      posep->theta += posep->angular_velocity * 0.001f;

      /* Clamp values.*/
      if (posep->x < 0 || posep->x > SANDBOX_WIDTH ||
          posep->y < 0 || posep->y > SANDBOX_WIDTH) {
        UrosString msg = urosStringAssignZ("Turtle hit the wall");
        rosout_warn(&msg, UROS_TRUE);
      }
      posep->x = min(max(0, posep->x), SANDBOX_WIDTH);
      posep->y = min(max(0, posep->y), SANDBOX_HEIGHT);
      while (posep->theta < 0)         { posep->theta += 2 * M_PI; }
      while (posep->theta >= 2 * M_PI) { posep->theta -= 2 * M_PI; }
    } else {
      posep->linear_velocity = 0;
      posep->angular_velocity = 0;
    }
    urosMutexUnlock(&turtlep->lock);
    urosThreadSleepMsec(1);
    urosMutexLock(&turtlep->lock);
  }
  turtle_unref(turtlep);
  urosMutexUnlock(&turtlep->lock);
  return UROS_OK;
}

turtle_t *turtle_spawn(const UrosString *namep,
                       float x, float y, float theta) {

  static const char *posend = "/pose";
  static const char *velend = "/command_velocity";
  static const char *setpenend = "/set_pen";
  static const char *telabsend = "/teleport_absolute";
  static const char *telrelend = "/teleport_relative";

  turtle_t *turtlep, *curp;
  unsigned i, numAlive;
  uros_err_t err;
  UrosString name, posname, velname, setpenname, telabsname, telrelname;

  urosAssert(urosStringNotEmpty(namep));

  /* Fill an empty slot.*/
  for (turtlep = NULL, numAlive = 0; turtlep == NULL;) {
    for (i = 0, curp = turtles; i < MAX_TURTLES; ++curp, ++i) {
      urosMutexLock(&curp->lock);
      if (curp->status == TURTLE_ALIVE) {
        urosError(0 == urosStringCmp(&curp->name, namep),
                  { urosMutexUnlock(&curp->lock); return NULL; },
                  ("A turtle named [%.*s] is alive\n", UROS_STRARG(namep)));
        ++numAlive;
      }
      if (curp->status == TURTLE_EMPTY) {
        turtlep = curp;
        break;
      }
      urosMutexUnlock(&curp->lock);
    }
    if (numAlive == MAX_TURTLES) {
      /* All the turtles are alive, sorry.*/
      return NULL;
    }
    if (turtlep == NULL) {
      /* Wait for 10ms to let referencing threads release a slot.*/
      urosThreadSleepMsec(10);
    }
  }

  /* Build the topic names.*/
  name = urosStringClone(namep);
  if (name.datap == NULL) { return NULL; }
  urosStringObjectInit(&posname);
  urosStringObjectInit(&velname);
  urosStringObjectInit(&setpenname);
  urosStringObjectInit(&telabsname);
  urosStringObjectInit(&telrelname);

  posname.length = 1 + namep->length + strlen(posend);
  posname.datap = (char*)urosAlloc(posname.length + 1);
  velname.length = 1 + namep->length + strlen(velend);
  velname.datap = (char*)urosAlloc(velname.length + 1);
  setpenname.length = 1 + namep->length + strlen(setpenend);
  setpenname.datap = (char*)urosAlloc(setpenname.length + 1);
  telabsname.length = 1 + namep->length + strlen(telabsend);
  telabsname.datap = (char*)urosAlloc(telabsname.length + 1);
  telrelname.length = 1 + namep->length + strlen(telrelend);
  telrelname.datap = (char*)urosAlloc(telrelname.length + 1);
  if (posname.datap == NULL || velname.datap == NULL ||
      setpenname.datap == NULL || telabsname.datap == NULL ||
      telrelname.datap == NULL) {
    urosStringClean(&posname);
    urosStringClean(&velname);
    urosStringClean(&setpenname);
    urosStringClean(&telabsname);
    urosStringClean(&telrelname);
    return NULL;
  }

  posname.datap[0] = '/';
  memcpy(1 + posname.datap, namep->datap, namep->length);
  memcpy(1 + posname.datap + namep->length, posend, strlen(posend) + 1);
  velname.datap[0] = '/';
  memcpy(1 + velname.datap, namep->datap, namep->length);
  memcpy(1 + velname.datap + namep->length, velend, strlen(velend) + 1);
  setpenname.datap[0] = '/';
  memcpy(1 + setpenname.datap, namep->datap, namep->length);
  memcpy(1 + setpenname.datap + namep->length, setpenend, strlen(setpenend) + 1);
  telabsname.datap[0] = '/';
  memcpy(1 + telabsname.datap, namep->datap, namep->length);
  memcpy(1 + telabsname.datap + namep->length, telabsend, strlen(telabsend) + 1);
  telrelname.datap[0] = '/';
  memcpy(1 + telrelname.datap, namep->datap, namep->length);
  memcpy(1 + telrelname.datap + namep->length, telrelend, strlen(telrelend) + 1);

  /* Assign the new attributes.*/
  turtlep->name = urosStringClone(namep);
  turtlep->poseTopic = posname;
  turtlep->velTopic = velname;
  turtlep->setpenService = setpenname;
  turtlep->telabsService = telabsname;
  turtlep->telrelService = telrelname;
  turtlep->pose.x = min(max(0, x), SANDBOX_WIDTH);
  turtlep->pose.y = min(max(0, y), SANDBOX_HEIGHT);
  turtlep->pose.theta = theta;
  while (turtlep->pose.theta < 0)         { turtlep->pose.theta += 2 * M_PI; }
  while (turtlep->pose.theta >= 2 * M_PI) { turtlep->pose.theta -= 2 * M_PI; }
  turtlep->pose.linear_velocity = 0;
  turtlep->pose.angular_velocity = 0;
  turtlep->countdown = 0;
  turtlep->status = TURTLE_ALIVE;
  turtlep->refCnt = 1; /* For the brain only */

  /* Publish "<turtle>/pose" .*/
  err = urosNodePublishTopicSZ(posname.datap,
                               "turtlesim/Pose",
                               (uros_proc_f)pub_tpc__turtleX__pose);
  urosError(err != UROS_OK,
            goto _error,
            ("Error %s while publishing topic [%s]\n",
             urosErrorText(err), posname.datap));

  /* Subscribe to "<turtle>/command_velocity".*/
  err = urosNodeSubscribeTopicSZ(velname.datap,
                                 "turtlesim/Velocity",
                                 (uros_proc_f)sub_tpc__turtleX__command_velocity);
  urosError(err != UROS_OK,
            { urosNodeUnpublishTopic(&posname);
              goto _error; },
            ("Error %s while subscribing to topic [%s]\n",
             urosErrorText(err), velname.datap));

  /* Publish "<turtle>/set_pen".*/
  err = urosNodePublishServiceSZ(setpenname.datap,
                                 "turtlesim/SetPen",
                                 (uros_proc_f)pub_srv__turtleX__set_pen);
  urosError(err != UROS_OK,
            { urosNodeUnpublishTopic(&posname);
              urosNodeUnpublishTopic(&velname);
              goto _error; },
            ("Error %s while publishing service [%s]\n",
             urosErrorText(err), setpenname.datap));

  /* Publish "<turtle>/teleport_absolute".*/
  err = urosNodePublishServiceSZ(telabsname.datap,
                                 "turtlesim/TeleportAbsolute",
                                 (uros_proc_f)pub_srv__turtleX__teleport_absolute);
  urosError(err != UROS_OK,
            { urosNodeUnpublishTopic(&posname);
              urosNodeUnpublishTopic(&velname);
              urosNodeUnpublishService(&setpenname);
              goto _error; },
            ("Error %s while publishing service [%s]\n",
             urosErrorText(err), telabsname.datap));

  /* Publish "<turtle>/teleport_relative".*/
  err = urosNodePublishServiceSZ(telrelname.datap,
                                 "turtlesim/TeleportRelative",
                                 (uros_proc_f)pub_srv__turtleX__teleport_relative);
  urosError(err != UROS_OK,
            { urosNodeUnpublishTopic(&posname);
              urosNodeUnpublishTopic(&velname);
              urosNodeUnpublishService(&setpenname);
              urosNodeUnpublishService(&telabsname);
              goto _error; },
            ("Error %s while publishing service [%s]\n",
             urosErrorText(err), telrelname.datap));

  /* Start its new brain.*/
  err = urosThreadPoolStartWorker(&turtlesThreadPool, (void*)turtlep);
  urosAssert(err == UROS_OK);
  urosMutexUnlock(&turtlep->lock);
  return turtlep;

_error:
  turtlep->status = TURTLE_EMPTY;
  turtlep->refCnt = 0;
  urosStringClean(&posname);
  urosStringClean(&velname);
  urosStringClean(&setpenname);
  urosStringClean(&telabsname);
  urosStringClean(&telrelname);
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
             UROS_STRARG(&turtlep->poseTopic)));

  err = urosNodeUnpublishTopic(&turtlep->velTopic);
  urosError(err != UROS_OK, UROS_NOP,
            ("Error %s while unpublishing topic [%.*s]\n",
             UROS_STRARG(&turtlep->velTopic)));

  /* Unpublish its services.*/
  err = urosNodeUnpublishService(&turtlep->setpenService);
  urosError(err != UROS_OK, UROS_NOP,
            ("Error %s while unpublishing service [%.*s]\n",
             UROS_STRARG(&turtlep->setpenService)));

  err = urosNodeUnpublishService(&turtlep->telabsService);
  urosError(err != UROS_OK, UROS_NOP,
            ("Error %s while unpublishing service [%.*s]\n",
             UROS_STRARG(&turtlep->telabsService)));

  err = urosNodeUnpublishService(&turtlep->telrelService);
  urosError(err != UROS_OK, UROS_NOP,
            ("Error %s while unpublishing service [%.*s]\n",
             UROS_STRARG(&turtlep->telrelService)));

  /* Cleanup fields.*/
  urosMutexLock(&turtlep->lock);
  urosStringClean(&turtlep->name);
  urosStringClean(&turtlep->poseTopic);
  urosStringClean(&turtlep->velTopic);
  urosStringClean(&turtlep->setpenService);
  urosStringClean(&turtlep->telabsService);
  urosStringClean(&turtlep->telrelService);
  turtlep->status = TURTLE_DEAD;
  urosMutexUnlock(&turtlep->lock);
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
