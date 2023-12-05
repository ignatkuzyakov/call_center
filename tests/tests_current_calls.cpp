#include "../lib/include/current_calls.hpp"
#include "../lib/include/ts_queue.hpp"
#include "gtest/gtest.h"

#include <thread>

TEST(SizeAndJoin, currentCallsInterface) {
  current_calls<int> cc(10);
  cc.join(5);
  cc.join(10);
  ASSERT_EQ(2, cc.size());
}

TEST(SizeAndLeave, currentCallsInterface) {
  current_calls<int> cc(10);
  cc.join(5);
  cc.join(10);
  cc.leave(0);
  ASSERT_EQ(1, cc.size());
}

// We pop when we join
TEST(InterfaceWithQueue, currentCallsInterface) {
  current_calls<int> cc(10);
  ts_queue<int> q;
  q.push(10);
  std::thread qt([&]() {
    int x;
    q.wait_and_pop(x);
    EXPECT_EQ(x, 10);
  });

  std::thread cct([&]() {
    while (q.size())
      cc.join(10);
  });
  qt.join();
  cct.join();
}
