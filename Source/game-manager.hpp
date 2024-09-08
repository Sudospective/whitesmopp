#ifndef GAME_MANAGER_HPP
#define GAME_MANAGER_HPP

class GameManager {
 public:
  static GameManager* GetInstance();

 private:
  static GameManager* _instance;

 public:
  GameManager(const GameManager&) = delete;
  GameManager& operator=(const GameManager&) = delete;

 private:
  GameManager();
};

#endif // GAME_MANAGER_HPP
