#include "server.hpp"

#include <chrono>
#include <iostream>

#include "server-manager.hpp"
#include "game-manager.hpp"

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

  _thread = new std::thread([&]() {
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
          c.socket = &socket;
          c.IP = ServerManager::GetInstance()->connectingIP;
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

          ServerManager::GetInstance()->isConnecting = false;

          _mutex.unlock();
        }
        else {
          _tcp->Disconnect(socket);
        }
      }
    }
  });

  _thread->join();
}

void Server::Stop() {
  _running = false;
}

bool Server::IsRunning() const { return _running; }
std::string Server::GetName() const { return _name; }
unsigned int Server::GetPort() const { return _port; }
std::vector<Client*> Server::GetPlayers() const { return _players; }
CTCPServer* Server::GetConnection() const { return _tcp; }
