#include <chrono>

#include "mini/ini.h"
#include "SQLiteCpp/SQLiteCpp.h"

#include "Server.hpp"
#include "ServerManager.hpp"

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
}

Server::~Server() {
  delete _connection;
  delete _mainThread;
}

bool Server::Start() {
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
  _mainThread = new std::thread(SMOListener);
  while (true) {
    Server::Update(IP, gotIP);
    _mutex.lock();
    gotIP = false;
    _mutex.unlock();
  }
}

void Server::Update(std::string ip, bool connecting) {
  _mutex.lock();
  ServerManager::GetInstance()->currentIP = ip;
  if (!ServerManager::GetInstance()->connecting)
    ServerManager::GetInstance()->connecting = connecting;
  std::vector<Client*> clients = _room.GetPlayers();
  _mutex.unlock();

  for (Client* client : clients) {
    _mutex.lock();
    Client* result = *std::find_if(clients.begin(), clients.end(), [&client](Client c) { return client->GetSocket() == c.GetSocket(); });
    std::vector<std::string> vInput = result->vInput;
    result->vInput.clear();
    _mutex.unlock();
    for (std::string input : vInput) {
      switch (input[4]) {
        case 3: { // Game Start
          break;
        }
        case 4: { // Game End
          break;
        }
      }
    }
  }
}

void Server::SMOListener() {
  std::vector<std::thread> renderThreads;
  while (_running) {
    ASocket::Socket socket;
    if (_connection->Listen(socket)) {
      char input[1024] = {};
      _connection->Receive(socket, input, 1024, false);
      if (input[4] == 2) {
        while (!ServerManager::GetInstance()->connecting)
          std::this_thread::sleep_for(std::chrono::milliseconds(50));
        _mutex.lock();
        std::cout << std::string(input, input[6]+2).erase(0,6) + " '" + ServerManager::GetInstance()->currentIP + "'"+ " Connected with StepManiaOnline Protocol: V" + std::to_string(input[5]) << std::endl;
      }
    }
  }
}

bool Server::IsRunning() const { return _running; }
unsigned int Server::GetPort() const { return _port; }
unsigned int Server::GetMaxPlayers() const { return _maxPlayers; }
std::string Server::GetName() const { return _name; }
std::string Server::GetIP() const { return _ip; }
const Room& Server::GetRoom() const { return _room; }
CTCPServer* Server::GetConnection() const { return _connection; }
