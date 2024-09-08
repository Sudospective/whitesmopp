#ifndef SERVER_HPP
#define SERVER_HPP

#include <mutex>
#include <string>

#include "TCPServer.h"

#include "Room.hpp"

class Server {
 public:
  Server();
  ~Server();

 public:
  unsigned int GetPort() const;
  unsigned int GetMaxPlayers() const;
  std::string GetName() const;
  std::string GetIP() const;
  Room GetRoom() const;
  CTCPServer* GetConnection() const;

 private:
  unsigned int _port;
  unsigned int _maxPlayers;
  std::string _name;
  std::string _ip;
  std::string _password;
  std::string _serverDB;
  std::string _salt;
  std::mutex _mutex;  
  Room _room;
  CTCPServer* _connection;
};

#endif // SERVER_HPP
