#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include <string>

class ServerManager {
 public:
  ServerManager(const ServerManager&) = delete;

 public:
  static ServerManager& GetInstance() {
    if (_instance == nullptr)
      _instance = new ServerManager;
    return *_instance;
  }
  bool connecting;
  std::string currentIP;

 public:
  ServerManager& operator=(const ServerManager&) = delete;

 private:
  ServerManager() {}
  ~ServerManager() {}

 private:
  inline static ServerManager* _instance = nullptr;
};

#endif // SERVERMANAGER_HPP
