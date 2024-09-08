#ifndef SERVER_HPP
#define SERVER_HPP

#include <mutex>
#include <string>
#include <thread>

#include "SQLiteCpp/SQLiteCpp.h"
#include "TCPServer.h"

#include "Room.hpp"

class Server {
 public:
  Server();
  ~Server();

 public:
  void Start();
  void Update(std::string ip, bool connecting);
  void ConnectClient(Client* client);
  void DisconnectClient(Client* client);

 private:
  void SMOListener();
  void SMOReader(Client* client);

 public:
  bool IsRunning() const;
  unsigned int GetPort() const;
  unsigned int GetMaxPlayers() const;
  std::string GetName() const;
  std::string GetIP() const;
  const Room& GetRoom() const;
  CTCPServer* GetConnection() const;

 private:
  bool _running;
  unsigned int _port;
  unsigned int _maxPlayers;
  unsigned int _version;
  unsigned int _protocolVersion;
  std::string _name;
  std::string _ip;
  std::string _password;
  std::string _serverDB;
  std::string _salt;
  std::mutex _mutex;
  std::thread* _mainThread;
  Room _room;
  CTCPServer* _connection;
  SQLite::Database* _database;
};

#endif // SERVER_HPP
