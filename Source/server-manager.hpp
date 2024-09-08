#ifndef SERVER_MANAGER_HPP
#define SERVER_MANAGER_HPP

class ServerManager {
 public:
  static ServerManager* GetInstance();
  static void ListenerHandler();

 private:
  static ServerManager* _instance;

 public:
  ServerManager(const ServerManager&) = delete;
  ServerManager& operator=(const ServerManager&) = delete;

 private:
  ServerManager();
};

#endif // SERVER_MANAGER_HPP
