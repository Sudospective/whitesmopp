#ifndef SERVER_HPP
#define SERVER_HPP

#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "TCPServer.h"

struct Client {
  bool connected;
  std::string name;
  std::string IP;
  std::string gift;
  ASocket::Socket* socket;
  std::vector<std::string> inputs;
};

class Server {
 public:
  Server();
  ~Server();

 public:
  void Start();
  void Update();
  void Stop();

 public:
  bool IsRunning() const;
  unsigned int GetPort() const;
  std::string GetName() const;
  std::vector<Client*> GetPlayers() const;
  CTCPServer* GetConnection() const;

 private:
  bool _running;
  std::mutex _mutex;
  std::thread* _thread;
  std::vector<Client*> _players;
  CTCPServer* _tcp;

  const unsigned int _serverOffset = 128;
  const unsigned int _serverVersion = 128;
  const unsigned int _port = 8765;
  const std::string _name = "White Elephant 2024";
};

#endif // SERVER_HPP
