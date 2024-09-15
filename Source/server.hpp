#ifndef SERVER_HPP
#define SERVER_HPP

#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "TCPServer.h"

struct Gift {
  bool selectable;
  int ID;
  std::string author;
};

struct Client {
  bool connected = false;
  bool inRoom = false;
  bool ready = false;
  std::string ID;
  std::string IP;
  std::string name;
  std::vector<std::string> inputs;
  ASocket::Socket socket;
  Gift* gift;
};

class Server {
 public:
  Server();
  ~Server();

 public:
  void ListPlayersInRoom(Client* player, std::vector<Client*> allPlayers);
  void ListGifts(Client* player, std::vector<Gift*> allGifts);
  void ListPlayersReady(Client* player, std::vector<Client*> allPlayers);

  void StartGame(Client* player);
  void EndGame(Client* player);
  void SelectGift(Client* player, Client* other, Gift* gift);
  void OpenGift(Client* player, Gift* gift);

  void Read(Client* player, std::vector<std::string> inputs);

  void Start();
  void Update();
  void Stop();

 // just in case
 public:
  bool IsRunning() const;
  unsigned int GetPort() const;
  std::string GetName() const;
  std::vector<Client*> GetPlayers() const;
  std::vector<Gift*> GetGifts() const;
  CTCPServer* GetConnection() const;

 private:
  bool _running;
  bool _locked;
  std::mutex _mutex;
  std::thread* _thread;
  std::vector<Client*> _players;
  std::vector<Gift*> _gifts;
  Gift* _currentGift;
  CTCPServer* _tcp;

  const unsigned int _serverOffset = 128;
  const unsigned int _serverVersion = 128;
  const unsigned int _port = 9876;
  const std::string _name = "White Elephant 2024";
};

#endif // SERVER_HPP
