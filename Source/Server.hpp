#ifndef SERVER_HPP
#define SERVER_HPP

#include <mutex>
#include <string>

#include "TCPServer.h"

#include "Room.hpp"

class Server {
 public:
  Server();
  Server(unsigned int port);
  Room GetRoom() const;

 private:
  unsigned int _version;
  unsigned int _protocolVersion;
  unsigned int _port;
  unsigned int _maxPlayers;
  Room _room;
  std::string _ip;
  std::string _serverDB;
  std::string _salt;
  std::mutex _mutex;
  CTCPServer* _tcpServer;
};

#endif // SERVER_HPP
