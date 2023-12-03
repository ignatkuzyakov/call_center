#include "../../lib/include/abonent.hpp"
#include "../../lib/include/call_center.hpp"
#include "../../lib/include/configure.hpp"

#include <chrono>
#include <string>
#include <thread>

int main(int argc, char **argv) {
  std::string fileName;
  if (argc == 1)
    fileName = "./configure.json";
  else if (argc == 2)
    fileName = argv[1];

  Config Cfg(fileName);
  Cfg.parse();

  std::thread serv([]() {
    call_center cc("0.0.0.0", 81, 1);
    cc.start();
  });

  std::thread ab1([]() {
    // straight-forward block while server is start listening
    std::this_thread::sleep_for(std::chrono::seconds(1));
    abonent ab("0.0.0.0", "81", "90200202", 11);
    ab.call();
  });
  std::thread ab2([]() {
    // straight-forward block while server is start listening
    std::this_thread::sleep_for(std::chrono::seconds(1));
    abonent ab("0.0.0.0", "81", "9032200202", 11);
    ab.call();
  });

  std::thread ab3([]() {
    // straight-forward block while server is start listening
    std::this_thread::sleep_for(std::chrono::seconds(1));
    abonent ab("0.0.0.0", "81", "921032200202", 11);
    ab.call();
  });
  serv.join();
  ab1.join();
  ab2.join();
  ab3.join();
}
