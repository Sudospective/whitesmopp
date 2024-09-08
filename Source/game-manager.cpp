#include "game-manager.hpp"

GameManager* GameManager::_instance = nullptr;

GameManager::GameManager() {}

GameManager* GameManager::GetInstance() {
  if (_instance == nullptr)
    _instance = new GameManager;
  return _instance;
}
