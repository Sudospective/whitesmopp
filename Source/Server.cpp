#include "mini/ini.h"
#include "SQLiteCpp/SQLiteCpp.h"

#include "Server.hpp"

Server::Server() {
  mINI::INIFile file("Config.ini");
  mINI::INIStructure ini;
  file.read(ini);

  _name = ini["Server"]["Name"];
  if (_name.empty()) {
    ini["Server"]["Name"] = "White Elephant 2024";
    _name = ini["Server"]["Name"];
  }
  _port = atol(ini["Server"]["Port"].c_str());
  if (_port <= 0) {
    ini["Server"]["Port"] = "8765";
    _port = atol(ini["Server"]["Port"].c_str());
  }
  _maxPlayers = atol(ini["Server"]["MaxPlayers"].c_str());
  if (_maxPlayers <= 0) {
    ini["Server"]["MaxPlayers"] = "255";
    _maxPlayers = atol(ini["Server"]["MaxPlayers"].c_str());
  }
  _password = ini["Server"]["Password"];
  if (_password.empty()) {
    ini["Server"]["Password"] = "ChangeMe";
    _password = ini["Server"]["Password"];
  }
  _serverDB = ini["ServerDB"]["File"];
  if (_serverDB.empty()) {
    ini["ServerDB"]["File"] = "ServerDB.db";
    _serverDB = ini["ServerDB"]["File"];
  }
  _salt = ini["ServerDB"]["Salt"];
  if (_salt.empty()) {
    ini["ServerDB"]["Salt"] = "Salt";
    _salt = ini["ServerDB"]["Salt"];
  }

  file.write(ini);

  SQLite::Database db(_serverDB.c_str(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
  SQLite::Transaction transaction(db);
  db.exec("CREATE TABLE IF NOT EXISTS Users(UserName CHAR, Password CHAR, Banned int)");
  transaction.commit();

  std::string IP = "unknown";
  bool gotIP = false;
  std::mutex* mutex = &_mutex;

  auto logPrinter = [&IP, &gotIP, &mutex](const std::string& strLogMsg) {
    if (strLogMsg.find("Incoming connection from") != std::string::npos) {
      mutex->lock();
      IP = strLogMsg;
      IP.erase(0, IP.find_first_of('\'') + 1);
      IP.erase(IP.find_first_of('\''), IP.length());
      gotIP = true;
      mutex->unlock();
    }
    else {
      //std::cout << strLogMsg << std::endl;
    }
  };

  _connection = new CTCPServer(logPrinter, std::to_string(_port).c_str());
}

Server::~Server() {
  delete _connection;
}

unsigned int Server::GetPort() const {
  return _port;
}
unsigned int Server::GetMaxPlayers() const {
  return _maxPlayers;
}
std::string Server::GetName() const {
  return _name;
}
std::string Server::GetIP() const {
  return _ip;
}
const Room& Server::GetRoom() const {
  return _room;
}
CTCPServer* Server::GetConnection() const {
  return _connection;
}
