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
#include <ros/ros.h>
#include <std_msgs/String.h>

static const std::string ratename = "/benchmark_rate";
static const std::string sizename = "/benchmark_size";

struct status_t {
  boost::signals2::mutex    lock;
  ros::NodeHandle           *nhp;
  uint32_t                  rate;
  ros::Rate                 looper;
  std_msgs::String          msg;

  status_t() :
    nhp(NULL),
    rate(1),
    looper(ros::Duration(1))
  {}
};

status_t *stp = NULL;

void pollParams(const ros::TimerEvent& e) {

  static const char hex[16] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
  };

  double rate;
  int size;
  stp->nhp->getParamCached(ratename, rate);
  stp->nhp->getParamCached(sizename, size);

  stp->lock.lock();
  if ((uint32_t)rate != stp->rate) {
    stp->looper = ros::Rate(::floor(rate));
  }
  int oldsize = (int)stp->msg.data.length();
  stp->msg.data.resize(size);
  for (int i = oldsize; i < size; ++i) {
    stp->msg.data[i] = hex[i & 0x0F];
  }
  stp->lock.unlock();
}

int main(int argc, char *argv[]) {

  ros::init(argc, argv, "benchmark_pub_input");

  ros::NodeHandle nh;
  status_t st;
  st.nhp = &nh;
  stp = &st;

  if (!nh.hasParam(ratename)) {
    nh.setParam(ratename, 1);
  }
  if (!nh.hasParam(sizename)) {
    nh.setParam(sizename, 0);
  }
  pollParams(ros::TimerEvent());
  ros::Timer paramTimer = nh.createTimer(ros::Duration(2), pollParams);

  ros::Publisher pub = nh.advertise<std_msgs::String>("benchmark/input", 256);
  while (ros::ok()) {
    st.lock.lock();
    pub.publish(st.msg);
    st.lock.unlock();

    ros::spinOnce();
    st.looper.sleep();
  }
  return 0;
}
