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
  _version = atol(ini["Server"]["ServerVersion"].c_str());
  if (_version <= 0) {
    ini["Server"]["ServerVersion"] = "128";
    _version = atol(ini["Server"]["ServerVersion"].c_str());
  }
  _protocolVersion = atol(ini["Server"]["ProtocolVersion"].c_str());
  if (_protocolVersion <= 0) {
    ini["Server"]["ProtocolVersion"] = "128";
    _protocolVersion = atol(ini["Server"]["ProtocolVersion"].c_str());
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
  _mainThread = new std::thread([&]() { SMOListener(); });
  while (true) {
    Server::Update(IP, gotIP);
    _mutex.lock();
    gotIP = false;
    _mutex.unlock();
  }
}

void Server::Update(std::string ip, bool connecting) {
  _mutex.lock();
  ServerManager::GetInstance().currentIP = ip;
  if (!ServerManager::GetInstance().connecting)
    ServerManager::GetInstance().connecting = connecting;
  std::vector<Client*> clients = _room.GetPlayers();
  _mutex.unlock();

  for (Client* client : clients) {
    _mutex.lock();
    Client* result = *std::find_if(clients.begin(), clients.end(), [&client](Client* c) { return client->GetSocket() == c->GetSocket(); });
    std::vector<std::string> vInput = result->vInput;
    result->vInput.clear();
    _mutex.unlock();
    for (std::string input : vInput) {
      switch (input[4]) {
        case 0: { // Ping
          break;
        }
        case 1: { // Ping Response
          break;
        }
        case 2: { // Hello(?)
          break;
        }
        case 3: { // Game Start
          break;
        }
        case 4: { // Game End
          break;
        }
        case 5: { // Game Status Update
          break;
        }
        case 6: { // Style Update
          break;
        }
        case 7: { // Chat Message
          break;
        }
        case 8: { // Request Start
          break;
        }
        case 10: { // Music Select
          break;
        }
        case 11: { // PlayerOptions
          break;
        }
      }
    }
  }
}
void Server::ConnectClient(Client* client) {
  for (Client* c : _room.GetPlayers()) {
    if (c->roomID != client->roomID || c->GetName() == client->GetName())
      continue;
    std::string out = std::string(1, static_cast<char>(_protocolVersion + 7)) + "User joined: " + c->GetName();
    std::string header = std::string(3, '\0') + std::string(1, static_cast<char>(out.size()));
    _connection->Send(c->GetSocket(), header + out);
  }
}
void Server::DisconnectClient(Client* client) {
  for (Client* c : _room.GetPlayers()) {
    if (c->roomID != client->roomID || c->GetName() == client->GetName())
      continue;
    std::string out = std::string(1, static_cast<char>(_protocolVersion + 7)) + "User Left: " + client->GetName();
    std::string header = std::string(3, '\0') + std::string(1, static_cast<char>(out.size()));
    _connection->Send(client->GetSocket(), header + out);
  }
}

void Server::SMOListener() {
  std::vector<std::thread> readerThreads;
  while (_running) {
    ASocket::Socket socket;
    if (_connection->Listen(socket)) {
      char input[1024] = {};
      _connection->Receive(socket, input, 1024, false);
      if (input[4] == 2) {
        while (!ServerManager::GetInstance().connecting)
          std::this_thread::sleep_for(std::chrono::milliseconds(50));
        
        _mutex.lock();
        std::cout << std::string(input, input[6]+2).erase(0,6) + " '" + ServerManager::GetInstance().currentIP + "'"+ " Connected with StepManiaOnline Protocol: V" + std::to_string(input[5]) << std::endl;
        std::string out = std::string(1, static_cast<char>(_protocolVersion + 2)) + std::string(1, static_cast<char>(_version)) + _name;
        std::string header = std::string(3, '\0') + std::string(1, static_cast<char>(out.size()));
        _connection->Send(socket, header + out);
        Client c;
        c.SetSocket(socket);
        c.SetIP(ServerManager::GetInstance().currentIP);
        _room.GetPlayers().push_back(&c);
        readerThreads.push_back(std::thread([&](Client* client) { SMOReader(client); }, &c));
        _mutex.unlock();

        ServerManager::GetInstance().connecting = false;
      }
      else {
        _connection->Disconnect(socket);
      }
    }
  }
  for (std::thread& thread : readerThreads)
    thread.join();
}
void Server::SMOReader(Client* client) {
  while (true) {
    _mutex.lock();
    Client* result = *std::find_if(_room.GetPlayers().begin(), _room.GetPlayers().end(), [&client](Client* c) { return client->GetSocket() == c->GetSocket(); });
    Client c = *result;
    _mutex.unlock();

    if (c.connected) {
      char input[1024] = {};
      int read = _connection->Receive(c.GetSocket(), input, 1024, false);
      if (read < 0)
        continue;
      
      _mutex.lock();
      result = *std::find_if(_room.GetPlayers().begin(), _room.GetPlayers().end(), [&client](Client* c) { return client->GetSocket() == c->GetSocket(); });
      if (read == 0) {
        std::cout << "User: " << c.GetName() << " (" << c.GetIP() << ") disconnected." << std::endl;
        _connection->Disconnect(c.GetSocket());
        if (result->loggedIn) {
          // leave player
          // leave room
        }
        result->connected = false;
      }
      if (read > 0) {
        result->vInput.push_back(std::string(input, 1024));
      }
      _mutex.unlock();
      continue;
    }
    _mutex.lock();
    _room.GetPlayers().erase(std::remove_if(_room.GetPlayers().begin(), _room.GetPlayers().end(), [&client](Client* c) { return client->GetSocket() == c->GetSocket(); }), _room.GetPlayers().end());
    _mutex.unlock();

    break;
  }
}

bool Server::IsRunning() const { return _running; }
unsigned int Server::GetPort() const { return _port; }
unsigned int Server::GetMaxPlayers() const { return _maxPlayers; }
std::string Server::GetName() const { return _name; }
std::string Server::GetIP() const { return _ip; }
const Room& Server::GetRoom() const { return _room; }
CTCPServer* Server::GetConnection() const { return _connection; }
