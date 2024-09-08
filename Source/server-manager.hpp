#ifndef SERVER_MANAGER_HPP
#define SERVER_MANAGER_HPP

#include <string>

class ServerManager {
 public:
  static ServerManager* GetInstance();

 public:
  bool isConnecting;
  std::string connectingIP;

 private:
  static ServerManager* _instance;

 public:
  ServerManager(const ServerManager&) = delete;
  ServerManager& operator=(const ServerManager&) = delete;

 private:
  ServerManager();
};

#endif // SERVER_MANAGER_HPP
