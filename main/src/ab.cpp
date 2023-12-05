#include "../../lib/include/abonent.hpp"

int main(int argc, char **argv) {
  const char *host;
  const char *port;
  const char *number;

  if (argc == 4) {
    host = argv[1];
    port = argv[2];
    number = argv[3];
  } else {
    host = "0.0.0.0";
    port = "81";
    number = "1234567";
  }

  abonent ab(host, port, number, 11);

  ab.call();
}
