#include "../lib/include/notify_interface.hpp"
#include "../lib/include/session.hpp"
#include "../lib/include/ts_queue.hpp"

#include "gtest/gtest.h"

#include <thread>

TEST(pushAndSize1, queueInterface) {
  ts_queue<int> q;
  q.push(5);

  EXPECT_EQ(1, q.size());
}

TEST(pushAndSize2, queueInterface) {
  ts_queue<std::size_t> q;

  for (int i = 0; i < 10; ++i)
    q.push(i);
  EXPECT_EQ(10, q.size());
}

TEST(notifyOneAwaiterOnePusher1, queueInterface) {
  notify_interface n;
  ts_queue<int> q(n);

  std::thread t1([&q]() {
    int x;
    q.wait_and_pop(x);
    EXPECT_EQ(15, x);
  });

  std::thread t2([&q]() { q.push(15); });

  t1.join();
  t2.join();
}

TEST(notifyOneAwaiterOnePusher2, queueInterface) {
  notify_interface n;
  ts_queue<int> q(n);

  std::thread t1([&q]() {
    int x;
    q.wait_and_pop(x);
    EXPECT_EQ(20, x);
    q.wait_and_pop(x);
    EXPECT_EQ(25, x);
  });

  std::thread t2([&q]() {
    q.push(20);
    q.push(25);
  });

  t1.join();
  t2.join();
}

TEST(notifyOneAwaiterTwoPushers1, queueInterface) {

  notify_interface n;
  ts_queue<int> q(n);

  std::thread t1([&q]() {
    int x;
    q.wait_and_pop(x);
    EXPECT_EQ(20, x);
    q.wait_and_pop(x);
    EXPECT_EQ(20, x);
  });

  std::thread t2([&q]() { q.push(20); });

  std::thread t3([&q]() { q.push(20); });

  t1.join();
  t2.join();
  t3.join();
}

TEST(eraseAndSize1, queueInterface) {

  ts_queue<int> q;
  q.push(100);
  q.push(200);
  q.push(300);

  q.erase(200);

  EXPECT_EQ(2, q.size());
}
