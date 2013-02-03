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

struct status_t {
  boost::signals2::mutex    lock;
  unsigned                  numPackets;
  size_t                    numBytes;

  status_t() :
    numPackets(0),
    numBytes(0)
  {}
};

status_t st;

void printEvent(const ros::TimerEvent &e) {

  st.lock.lock();
  unsigned numPackets = st.numPackets;
  size_t numBytes = st.numBytes;
  st.lock.unlock();

  double duration = (e.current_real - e.last_real).toSec();
  ROS_INFO("%u pkt/s @ %zu B/s",
           (unsigned)(numPackets / duration),
           (size_t)(numBytes / duration));
}

void messageEvent(const std_msgs::String::ConstPtr& msg) {

  st.lock.lock();
  ++st.numPackets;
  st.numBytes += 2 * sizeof(uint32_t) + msg->data.length();
  st.lock.unlock();
}

int main(int argc, char **argv) {

  ros::init(argc, argv, "benchmark_sub_output");

  ros::NodeHandle nh;
  ros::Timer printTimer = nh.createTimer(ros::Duration(1), printEvent);
  ros::Subscriber sub = nh.subscribe("benchmark/output", 256, messageEvent);

  ros::spin();
  return 0;
}
