#include "server.hpp"

#include <chrono>
#include <iostream>
#include <sstream>

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
        if (input[4] == 2) {
          while (!ServerManager::GetInstance()->isConnecting)
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
          _mutex.lock();
          Client c;
          c.socket = socket;
          c.IP = ServerManager::GetInstance()->connectingIP;
          c.connected = true;
          std::cout << "New player from IP address " << c.IP << std::endl;
          std::string out = (
            std::string(1, static_cast<char>(_serverOffset + 2)) +
            std::string(1, static_cast<char>(_serverVersion)) +
            _name
          );
          std::string header = (
            std::string(3, '\0') +
            std::string(1, static_cast<char>(out.size()))
          );
          _tcp->Send(socket, header + out);
          _players.push_back(&c);
          threads.push_back(std::thread(reader, &c));
          ServerManager::GetInstance()->isConnecting = false;
          _mutex.unlock();
        }
        else {
          _tcp->Disconnect(socket);
        }
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
  _mutex.lock();
  ServerManager::GetInstance()->isConnecting = false;
  _mutex.unlock();
  for (Client* player : _players) {
    _mutex.lock();
    std::vector<std::string> inputs = player->inputs;
    player->inputs.clear();
    _mutex.unlock();
    Read(player, inputs);
  }
}

void Server::Read(Client* player, std::vector<std::string> inputs) {
  for (std::string input : inputs) {
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
      case 9: { // Reserved
        break;
      }
      case 10: { // Music Select
        break;
      }
      case 11: { // Player Options
        break;
      }
      case 12: { // StepManiaOnline
        std::stringstream in(input.erase(0, 8));
        std::string val;
        std::vector<std::string> vals;

        while (std::getline(in, val, '\0')) {
          vals.push_back(val);
        }

        vals.erase(std::remove(vals.begin() + 2, vals.end(), "\0"), vals.end());
        player->name = vals[0];
        player->ID = vals[1];

        break;
      }
      case 13: { // Reserved
        break;
      }
      case 14: { // Reserved
        break;
      }
      case 15: { // Reserved
        break;
      }
      case 16: { // Friend List Update
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
CTCPServer* Server::GetConnection() const { return _tcp; }
