#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include <string>

class ServerManager {
 public:
  static ServerManager* GetInstance();
  bool connecting;
  std::string currentIP;

 private:
  ServerManager();
  static ServerManager* _instance;
};

#endif // SERVERMANAGER_HPP
