#ifdef WIN32
#define WINDOWS
#endif

#define SCALE(x, l1, h1, l2, h2)	(((x) - (l1)) * ((h2) - (l2)) / ((h1) - (l1)) + (l2))

#include <iostream>

#include "Server.hpp"

int main() {
  Server server;
  server.Start();
  return 0;
}
