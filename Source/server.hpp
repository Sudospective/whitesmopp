#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>

#include "SQLiteCpp/SQLiteCpp.h"
#include "TCPServer.h"

#include "client.hpp"

class Server {
 public:
  void Start();
  void Update();

 public:
  std::string name;
  std::string port;
  std::vector<Client*> clients;

 private:
  SQLite::Database* _db;
  CTCPServer* _tcp;
};

#endif // SERVER_HPP
