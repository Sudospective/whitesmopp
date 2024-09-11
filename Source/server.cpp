#include "server.hpp"

#include <chrono>
#include <iostream>
#include <sstream>

#include <nlohmann/json.hpp>

#include "server-manager.hpp"

Server::Server() {
  _tcp = nullptr;
  _thread = nullptr;
}

Server::~Server() {
  if (_tcp != nullptr)
    delete _tcp;
  if (_thread != nullptr)
    delete _thread;
}

void Server::Start() {
  std::cout << "Starting server..." << std::endl;
  ServerManager::GetInstance()->isConnecting = false;
  ServerManager::GetInstance()->connectingIP = "unknown";

  // th lambda
  std::function logger = [&](const std::string& msg) {
    if (msg.find("Incoming connection") != std::string::npos) {
      _mutex.lock();
      ServerManager::GetInstance()->isConnecting = true;
      ServerManager::GetInstance()->connectingIP = msg;
      ServerManager::GetInstance()->connectingIP.erase(
        0,
        ServerManager::GetInstance()->connectingIP.find_first_of('\'') + 1
      );
      ServerManager::GetInstance()->connectingIP.erase(
        ServerManager::GetInstance()->connectingIP.find_first_of('\''),
        ServerManager::GetInstance()->connectingIP.length()
      );
      _mutex.unlock();
    }
    else {
      std::cout << msg << std::endl;
    }
  };

  _tcp = new CTCPServer(logger, std::to_string(_port).c_str());

  _running = true;

  std::cout << "Server started." << std::endl;

  _room.name = "White Elephant 2024";
  _room.description = "Merry Christmas!";

  // th antilambda
  std::function reader = [&](Client* player) {
    while (_running) {
      if (player->connected) {
        char input[1024] = {};
        int read = _tcp->Receive(player->socket, input, 1024, false);
        if (read < 0)
          continue;
        _mutex.lock();
        if (read == 0) {
          std::cout << "User " << player->name << " (" << player->IP << ") disconnected." << std::endl;
          _tcp->Disconnect(player->socket);
          player->connected = false;
        }
        if (read > 0) {
          player->inputs.push_back(std::string(input, 1024));
        }
        _mutex.unlock();
        continue;
      }
      _mutex.lock();
      _players.erase(std::remove_if(
        _players.begin(),
        _players.end(),
        [&](Client* c) { return player->socket == c->socket; }
      ), _players.end());
      _mutex.unlock();
      break;
    }
  };

  // th omnilambda
  std::function listener = [&]() {
    std::vector<std::thread> threads;
    while (_running) {
      ASocket::Socket socket;
      if (_tcp->Listen(socket)) {
        char input[1024] = {};
        _tcp->Receive(socket, input, 1024, false);
        std::string inputStr = std::string(input, 1024);
        inputStr = inputStr.erase(inputStr.find_first_of('\0'));
        nlohmann::json jRecv = nlohmann::json::parse(inputStr);
        if (jRecv["command"] == 2) {
          while (!ServerManager::GetInstance()->isConnecting)
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
          _mutex.lock();
          Client c;
          c.socket = socket;
          c.IP = ServerManager::GetInstance()->connectingIP;
          c.connected = true;
          std::cout << "New player from IP address " << c.IP << std::endl;
          nlohmann::json jSend;
          jSend["command"] = _serverOffset + 2;
          jSend["offset"] = _serverOffset;
          jSend["message"] = "Hello";
          _tcp->Send(socket, jSend.dump());
          _players.push_back(&c);
          threads.push_back(std::thread(reader, &c));
          ServerManager::GetInstance()->isConnecting = false;
          _mutex.unlock();
        }
        else
          _tcp->Disconnect(socket);
      }
    }
    for (std::thread& thread : threads)
      thread.join();
  };

  _thread = new std::thread(listener);

  while (_running)
    Update();

  _thread->join();
}

void Server::Update() {
  //_mutex.lock();
  //ServerManager::GetInstance()->isConnecting = false;
  //_mutex.unlock();
  for (Client* player : _players) {
    _mutex.lock();
    std::vector<std::string> inputs = player->inputs;
    player->inputs.clear();
    _mutex.unlock();
    Read(player, inputs);
  }
}

void Server::ListPlayers(Client* player, std::vector<Client*> allPlayers) {
  std::string users;
  for (Client* plr : allPlayers)
    users += std::string(1, '\n') + plr->name;
  SendChat(player, "Player list:" + users);
}

void Server::SendChat(Client* player, std::string msg) {
  std::string out = (
    std::string(1, static_cast<char>(_serverOffset + 7)) +
    msg
  );
  std::string header = (
    std::string(3, '\0') +
    std::string(1, static_cast<char>(out.size()))
  );
  _tcp->Send(player->socket, header + out);
}

void Server::Read(Client* player, std::vector<std::string> inputs) {
  for (std::string input : inputs) {
    std::cout << player->IP << " sent message on protocol " << static_cast<int>(input[4]) << std::endl;
    switch (input[4]) {
      case 0: { // Ping
        break;
      }
      case 1: { // Ping Response
        break;
      }
      case 2: { // Hello
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
      case 7: { // Chat
        break;
      }
      case 8: { // Request Start
        break;
      }
      case 9: { // White Elephant
        break;
      }
      case 10: { // User Status
        break;
      }
      case 11: { // Player Options
        break;
      }
      case 12: { // SMOnline Packet
        break;
      }
      case 13: { // Reserved
        break;
      }
      case 14: { // Send Attack
        break;
      }
      case 15: { // XML Packet
        break;
      }
    }
  }
}

void Server::Stop() {
  _running = false;
}

bool Server::IsRunning() const { return _running; }
std::string Server::GetName() const { return _name; }
unsigned int Server::GetPort() const { return _port; }
std::vector<Client*> Server::GetPlayers() const { return _players; }
Room Server::GetRoom() const { return _room; }
CTCPServer* Server::GetConnection() const { return _tcp; }
