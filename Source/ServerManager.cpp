#include "ServerManager.hpp"

ServerManager::ServerManager() {

}

ServerManager* ServerManager::GetInstance() {
  if (_instance == nullptr)
    _instance = new ServerManager;
  return _instance;
}
