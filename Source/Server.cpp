#include <chrono>
#include <sstream>

#include "mini/ini.h"

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

  std::cout << "WhiteSMO++ 1.0" << std::endl;
  std::cout << "written by Sudospective" << std::endl;
  std::cout << "Original code by Jousway" << std::endl << std::endl;

  _database = new SQLite::Database(_serverDB.c_str(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
  SQLite::Transaction transaction(*_database);
  _database->exec("CREATE TABLE IF NOT EXISTS Users(UserName CHAR, Password CHAR, Banned int)");
  transaction.commit();
}

Server::~Server() {
  delete _connection;
  delete _mainThread;
  delete _database;
}

void Server::Start() {
  std::cout << "Starting server..." << std::endl;

  auto logPrinter = [&](const std::string& strLogMsg) {
    if (strLogMsg.find("Incoming connection from") != std::string::npos) {
      _mutex.lock();
      ServerManager::GetInstance().currentIP = strLogMsg;
      ServerManager::GetInstance().currentIP.erase(0, ServerManager::GetInstance().currentIP.find_first_of('\'') + 1);
      ServerManager::GetInstance().currentIP.erase(ServerManager::GetInstance().currentIP.find_first_of('\''), ServerManager::GetInstance().currentIP.length());
      ServerManager::GetInstance().connecting = true;
      _mutex.unlock();
    }
    else {
      //std::cout << strLogMsg << std::endl;
    }
  };

  _connection = new CTCPServer(logPrinter, std::to_string(_port).c_str());
  _mainThread = new std::thread([&]() { SMOListener(); });

  std::cout << "Server started." << std::endl;

  _running = true;

  while (true) {
    Server::Update(ServerManager::GetInstance().currentIP, ServerManager::GetInstance().connecting);
  }

  _running = false;
  _mainThread->join();
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
        case 12: { // SMOnline
          if (!client->loggedIn) {
            std::stringstream in(input.erase(0, 8));
            std::string val;
            std::vector<std::string> vals;
            while (std::getline(in, val, '\0')) {
              vals.push_back(val);
            }
            vals.erase(std::remove(vals.begin() + 2, vals.end(), "\0"), vals.end());
            bool foundUser = false;
            try {
              SQLite::Statement query(*_database, (std::string("SELECT * FROM Users WHERE UserName =") + "\"" + vals[0] + "\"").c_str());
              bool invalidPass = false;
              while (query.executeStep()) {
                foundUser = true;
                const char* name = query.getColumn(0);
                client->SetName(name);
                const char* password = query.getColumn(1);
                int banned = query.getColumn(2);
                if (banned == 1) {
                  std::cout << "User: " << client->GetName() << " (" << client->GetIP() << ") Is Banned, Disconnecting\n";
                  _connection->Disconnect(client->GetSocket());
                  client->connected = false;
                  invalidPass = true;
                  break;
                }
                if (password != vals[1]) {
                  std::string out = std::string(1, static_cast<char>(_protocolVersion + 12)) + std::string(1, '\0') + std::string(1, '\1') + "Wrong password.";
                  std::string header = std::string(3, '\0') + std::string(1, static_cast<char>(out.size()));
                  _connection->Send(client->GetSocket(), header + out);
                  invalidPass = true;
                  break;
                }
              }
              if (invalidPass)
                continue;
            }
            catch (...) {}
            if (!foundUser) {
              std::cout << "Creating new user: " << vals[0] << std::endl;
              SQLite::Transaction transaction(*_database);
              _database->exec(("INSERT INTO Users VALUES (\"" + vals[0] + "\", \"" + vals[1] + "\", 0)").c_str());
              transaction.commit();
              client->SetName(vals[0]);
            }
            std::string out = std::string(1, static_cast<char>(_protocolVersion + 12)) + std::string(2, '\0') + "Correct password.";
            std::string header = std::string(3, '\0') + std::string(1, static_cast<char>(out.size()));
            _connection->Send(client->GetSocket(), header + out);
            client->loggedIn = true;
          }
          break;
        }
      }
    }
  }
  for (Client* client : clients) {
    _mutex.lock();
    auto result = std::find_if(clients.begin(), clients.end(), [&client](Client* c) { return client->GetSocket() == c->GetSocket(); });
    Client* c = *result;
    if (result != clients.end()) {
      c->connected = client->connected;
      c->loggedIn = client->loggedIn;
      c->roomID = client->roomID;
      c->SetName(client->GetName());
      c->SetType(client->GetType());
    }
    _mutex.unlock();
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
  std::cout << "Listening on port " << _port << std::endl;
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
        c.loggedIn = false;
        c.SetIP(ServerManager::GetInstance().currentIP);
        c.roomID = 0;
        _room.GetPlayers().push_back(&c);
        readerThreads.push_back(std::thread([&](Client* client) { SMOReader(client); }, &c));
        ServerManager::GetInstance().connecting = false;

        _mutex.unlock();

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
    auto search = std::find_if(_room.GetPlayers().begin(), _room.GetPlayers().end(), [&client](Client* c) { return client->GetSocket() == c->GetSocket(); });
    Client* result = nullptr;
    if (search != _room.GetPlayers().end())
      result = *search;
    _mutex.unlock();
    if (result == nullptr)
      continue;

    if (result->connected) {
      char input[1024] = {};
      int read = _connection->Receive(result->GetSocket(), input, 1024, false);
      if (read < 0)
        continue;
      
      _mutex.lock();
      search = std::find_if(_room.GetPlayers().begin(), _room.GetPlayers().end(), [&client](Client* c) { return client->GetSocket() == c->GetSocket(); });
      if (search != _room.GetPlayers().end())
        result = *search;
      if (result == nullptr) {
        _mutex.unlock();
        continue;
      }
      if (read == 0) {
        std::cout << "User: " << result->GetName() << " (" << result->GetIP() << ") disconnected." << std::endl;
        _connection->Disconnect(result->GetSocket());
        if (result->loggedIn) {
          DisconnectClient(result);
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
