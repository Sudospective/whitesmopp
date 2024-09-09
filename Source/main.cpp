#include "server.hpp"

int main() {
  Server server;

  server.Start();
  while (server.IsRunning()) {
    server.Update();
  }
  
  return 0;
}
