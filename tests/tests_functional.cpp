#include <boost/filesystem.hpp>
#include <boost/process.hpp>

#include "gtest/gtest.h"

#include <boost/process/io.hpp>
#include <chrono>
#include <random>
#include <string>
#include <thread>

namespace bp = boost::process;

std::string host = " 0.0.0.0 ";
std::string port = " 81 ";

TEST(oneAbonent, functionalTests) {
  bp::ipstream is;
  std::string result;
  std::string processName1 =
      "./debug/main/src/callCenter ./tests/test_configure.json" + host + port;
  std::string processName2 = "./debug/main/src/abonent" + host + port;
  bp::child c1(processName1);
  std::this_thread::sleep_for(std::chrono::seconds(1));
  bp::child c2(processName2, bp::std_out > is);

  c2.wait();
  c1.terminate();

  std::getline(is, result);
  EXPECT_TRUE(result.find("in queue") != result.npos);

  std::getline(is, result);
  EXPECT_TRUE(result.find("CallID") != result.npos);

  std::getline(is, result);
  EXPECT_TRUE(result.find("We are talking") != result.npos);
}

TEST(multiuser, functionalTests) {
  std::string processName1 =
      "./debug/main/src/callCenter ./tests/test_configure.json" + host + port;
  std::string processName2 = "./debug/main/src/abonent" + host + port;
  bp::child c1(processName1);
  std::this_thread::sleep_for(std::chrono::seconds(1));

  int N = 1000;
  std::vector<bp::child> childs;
  childs.reserve(N);

  std::random_device rd;
  std::default_random_engine reng(rd());
  std::uniform_int_distribution<std::size_t> dist(70000000000, 79999999999);

  while (N--) {
    // To make different numbers
    std::string pn =
        processName2 + '+' + std::to_string((std::size_t)dist(reng));
    childs.emplace_back(bp::child(pn, bp::std_out > bp::null));
  }

  for (auto &ch : childs)
    ch.join();
  c1.terminate();
}
