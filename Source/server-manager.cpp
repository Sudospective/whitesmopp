#include "server-manager.hpp"

ServerManager* ServerManager::_instance = nullptr;

ServerManager::ServerManager() {}

ServerManager* ServerManager::GetInstance() {
  if (_instance == nullptr)
    _instance = new ServerManager;
  return _instance;
}
