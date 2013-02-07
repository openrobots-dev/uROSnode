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

#include <boost/signals2/mutex.hpp>
#include <sys/time.h>
#include <sys/resource.h>
#include <ros/ros.h>
#include <std_msgs/String.h>

using namespace boost::signals2;

/**
 * @brief   Stream counters.
 */
struct streamcnt_t {
  unsigned long numMsgs;        /**< @brief Total number of exchanged messages.*/
  size_t        numBytes;       /**< @brief Total exchanged size.*/
  unsigned long deltaMsgs;      /**< @brief Incremental number of exchanged messages.*/
  size_t        deltaBytes;     /**< @brief Incremental exchanged size.*/

  streamcnt_t() :
    numMsgs(0),
    numBytes(0),
    deltaMsgs(0),
    deltaBytes(0)
  {}
};

/**
 * @brief   CPU usage counters (jiffies).
 */
struct cpucnt_t {
  unsigned long user;           /**< @brief User-level CPU count.*/
  unsigned long nice;           /**< @brief Niced user-level CPU count.*/
  unsigned long system;         /**< @brief System-level CPU count.*/
  unsigned long idle;           /**< @brief Idle CPU count.*/

  cpucnt_t() :
    user(0),
    nice(0),
    system(0),
    idle(0)
  {}
};

/**
 * @brief   Benchmark status.
 */
struct benchmark_t {
  mutex         lock;           /**< @brief Lock word.*/

  /* Configuration.*/
  unsigned long rate;           /**< @brief Packets/s.*/
  std::string   payload;        /**< @brief Packet payload string.*/

  /* Meters.*/
  cpucnt_t      curCpu;         /**< @brief Current CPU usages.*/
  cpucnt_t      oldCpu;         /**< @brief Previous CPU usages.*/
  streamcnt_t   inCount;        /**< @brief Incoming stream counters.*/
  streamcnt_t   outCount;       /**< @brief Outgoing stream counters.*/
  uint64_t      curTime;        /**< @brief Current timestamp.*/
  uint64_t      oldTime;        /**< @brief Old timestamp.*/

  benchmark_t() :
    rate(1),
    curTime(0),
    oldTime(0)
  {}
};

benchmark_t benchmark;

void app_print_cpu_state(void) {

  FILE *statfp =  fopen("/proc/stat", "rt");
  assert(statfp != NULL);
  cpucnt_t curCpu;
  int n = fscanf(statfp, "%*s %lu %lu %lu %lu ",
                 &curCpu.user, &curCpu.nice, &curCpu.system, &curCpu.idle);
  assert(n == 4); (void)n;
  fclose(statfp);
  benchmark.lock.lock();
  cpucnt_t oldCpu = benchmark.oldCpu = benchmark.curCpu;
  benchmark.curCpu = curCpu;
  benchmark.lock.unlock();

  double mult = 100.0 /
                ((curCpu.user + curCpu.nice + curCpu.system + curCpu.idle) -
                 (oldCpu.user + oldCpu.nice + oldCpu.system + oldCpu.idle));

  printf("CPU%%: user: %.3f nice: %.3f sys: %.3f idle: %.3f\n",
         (curCpu.user - oldCpu.user) * mult,
         (curCpu.nice - oldCpu.nice) * mult,
         (curCpu.system - oldCpu.system) * mult,
         (curCpu.idle - oldCpu.idle) * mult);
}

void app_print_cpu_usage(void) {

  struct timespec time;
  int err; (void)err;

#if defined(CLOCK_PROCESS_CPUTIME_ID)
  err = clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time);
#elif defined(CLOCK_VIRTUAL)
  err = clock_gettime(CLOCK_VIRTUAL, &time);
#else
#error "process' clock_gettime() not available"
#endif
  assert(err == 0);

  printf("USER: %lu.%9.9lu\n",
         (unsigned long)time.tv_sec, (unsigned long)time.tv_nsec);
}

uint64_t app_get_time() {

  struct timeval tv;
  struct timezone tz;
  gettimeofday(&tv, &tz);
  struct tm *tm = localtime(&tv.tv_sec);
  return ((uint64_t)((tm->tm_yday * 24u + tm->tm_hour) * 60u +
          tm->tm_min) * 60u + tm->tm_sec) * 1000000ull + tv.tv_usec;
}

void printEvent(const ros::TimerEvent &e) {

  uint64_t curTime = app_get_time();
  benchmark.lock.lock();
  streamcnt_t inCount = benchmark.inCount;
  benchmark.inCount.deltaMsgs = 0;
  benchmark.inCount.deltaBytes = 0;
  uint64_t oldTime = benchmark.curTime;
  benchmark.curTime = curTime;
  benchmark.lock.unlock();

  printf("@ %llu\n", curTime);
  unsigned long winTime = (unsigned long)(curTime - oldTime);
  printf("IN: %lu msg %lu B %lu msg/s %lu B/s\n",
         ((1000 * inCount.numMsgs + 499) / winTime),
         ((1000 * inCount.numBytes + 499) / winTime),
         ((1000 * inCount.deltaMsgs + 499) / winTime),
         ((1000 * inCount.deltaBytes + 499) / winTime));
  app_print_cpu_state();
}

void messageEvent(const std_msgs::String::ConstPtr& msg) {

  benchmark.lock.lock();
  ++benchmark.inCount.numBytes;
  benchmark.inCount.numBytes += 2 * sizeof(uint32_t) + msg->data.length();
  ++benchmark.inCount.deltaBytes;
  benchmark.inCount.deltaBytes += 2 * sizeof(uint32_t) + msg->data.length();
  benchmark.lock.unlock();
}

int main(int argc, char **argv) {

  ros::init(argc, argv, "benchmark_sub_output");

  ros::NodeHandle nh;
  ros::Timer printTimer = nh.createTimer(ros::Duration(1), printEvent);
  ros::Subscriber sub = nh.subscribe("benchmark/output", 256, messageEvent);

  ros::spin();
  return 0;
}
