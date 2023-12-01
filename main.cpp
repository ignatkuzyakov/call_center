#include "configure.hpp"
#include "call_center.hpp"
#include <gtest/gtest.h>


#include <iostream>


int main() {
      std::string fileName("../configure.json");
    Config Cfg(fileName);
    Cfg.parse();

    call_center cc("0.0.0.0", 81, 1);
    cc.start();
}

#if 0
TEST(test1, ConfigureParser)
{
    std::string fileName("../configure.json");
    Config Cfg(fileName);
    Cfg.parse();

    EXPECT_EQ(Rmin, 5);
    EXPECT_EQ(Rmax, 15);
    EXPECT_EQ(NCalls, 2);
    EXPECT_EQ(NQueue, 2);
}

TEST(test2, Server)
{
    call_center cc("0.0.0.0", 81, 1);
    cc.start();
}
#endif