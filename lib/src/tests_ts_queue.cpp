#include "ts_queue.hpp"
#include "gtest/gtest.h"

class QueueTest : public ::testing::Test {
protected:
  ts_queue<int> q;
  virtual void SetUp() {}

  virtual void TearDown() {}
};

TEST_F(QueueTest, OperatorTest) {}
