#ifndef SERVER_HPP
#define SERVER_HPP

#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "TCPServer.h"

struct Client {
  bool connected = false;
  bool loggedIn = false;
  bool inRoom = false;
  std::string name;
  std::string ID;
  std::string IP;
  std::string gift;
  std::vector<std::string> inputs;
  ASocket::Socket socket;
};

struct Room {
  int state;
  std::string name;
  std::string description;
  std::string password;
  std::vector<Client*> players;
};

class Server {
 public:
  Server();
  ~Server();

 public:
  void Start();
  void Update();
  void ListPlayers(Client* player, std::vector<Client*> allPlayers);
  void SendChat(Client* player, std::string msg);
  void Read(Client* player, std::vector<std::string> inputs);
  void Stop();

 // just in case
 public:
  bool IsRunning() const;
  unsigned int GetPort() const;
  std::string GetName() const;
  std::vector<Client*> GetPlayers() const;
  Room GetRoom() const;
  CTCPServer* GetConnection() const;

 private:
  bool _running;
  std::mutex _mutex;
  std::thread* _thread;
  std::vector<Client*> _players;
  Room _room;
  CTCPServer* _tcp;

  const unsigned int _serverOffset = 128;
  const unsigned int _serverVersion = 128;
  const unsigned int _port = 8765;
  const std::string _name = "White Elephant 2024";
};

#endif // SERVER_HPP
