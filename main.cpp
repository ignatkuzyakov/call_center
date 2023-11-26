#include "configure.hpp"
#include "call_center.hpp"

#include <iostream>

int main(int argc, char *argv[])
{
    std::string fileName("../configure.json");
    Config Cfg(fileName);
    Cfg.parse();

    std::cout << "Rmin: " << Rmin << std::endl;
    std::cout << "Rmax: " << Rmax << std::endl;
    std::cout << "NCalls: " << NCalls << std::endl;
    std::cout << "NQueue: " << NQueue << std::endl;

    call_center cc("0.0.0.0", 81, 1);
    cc.start();

    return 0;
}