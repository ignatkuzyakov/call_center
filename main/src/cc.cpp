#include "../../lib/include/call_center.hpp"
#include "../../lib/include/configure.hpp"

#include <string>

int main(int argc, char **argv) {
  std::string fileName;
  if (argc == 1)
    fileName = "./configure.json";
  else if (argc > 3 || argc == 2)
    fileName = argv[1];

  const char *host;
  int port;

  if (argc >= 3) {
    if (argc == 3) {
      host = argv[1];
      port = std::stoi(argv[2]);
    } else {
      host = argv[2];
      port = std::stoi(argv[3]);
    }
  } else {
    host = "0.0.0.0";
    port = 81;
  }

  Config Cfg(fileName);
  Cfg.parse();

  // host, port, number of threads of working on io handler
  call_center cc("0.0.0.0", 81, 1);
  cc.start();
}
