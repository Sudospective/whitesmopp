#include "server.hpp"

#include <chrono>
#include <fstream>
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

void Server::ListPlayersInRoom(Client* player, std::vector<Client*> allPlayers) {
  nlohmann::json j;
  j["command"] = _serverOffset + 4;
  j["data"]["action"] = 0;
  j["data"]["message"] = "Player List";
  int numPlayers = 0;
  for (Client* other : allPlayers) {
    if (other->inRoom) {
      j["data"]["players"][other->ID] = other->name;
      numPlayers++;
    }
  }
  j["data"]["player_count"] = numPlayers;
  _tcp->Send(player->socket, j.dump());
}

void Server::ListGifts(Client* player, std::vector<Gift*> allGifts) {
  nlohmann::json j;
  j["command"] = _serverOffset + 4;
  j["data"]["action"] = 1;
  j["data"]["message"] = "Gift List";
  int numGifts = 0;
  for (Gift* gift : allGifts) {
    j["data"]["gifts"][std::to_string(gift->ID)] = gift->author;
    numGifts++;
  }
  j["data"]["gift_count"] = numGifts;
  _tcp->Send(player->socket, j.dump());
}

void Server::ListPlayersReady(Client* player, std::vector<Client*> allPlayers) {
  nlohmann::json j;
  j["command"] = _serverOffset + 4;
  j["data"]["action"] = 2;
  j["data"]["message"] = "Ready List";
  int numReady = 0;
  for (Client* other : allPlayers) {
    if (other->ready) {
      j["data"]["players"][player->ID] = player->name;
      numReady++;
    }
  }
  j["data"]["player_count"] = numReady;
  _tcp->Send(player->socket, j.dump());
}

void Server::StartGame(Client* player) {
  nlohmann::json j;
  j["command"] = _serverOffset + 5;
  j["data"]["action"] = 0;
  j["data"]["message"] = "Game Start";
  _tcp->Send(player->socket, j.dump());
}

void Server::EndGame(Client* player) {
  nlohmann::json j;
  j["command"] = _serverOffset + 5;
  j["data"]["action"] = 1;
  j["data"]["message"] = "Game End";
  _tcp->Send(player->socket, j.dump());
}

void Server::SelectGift(Client* player, Client* other, Gift* gift) {
  nlohmann::json j;
  j["command"] = _serverOffset + 5;
  j["data"]["action"] = 2;
  j["data"]["message"] = "Select Gift";
  j["data"]["player"] = other->ID;
  j["data"]["gift"] = gift->ID;
  _tcp->Send(player->socket, j.dump());
}

void Server::OpenGift(Client* player, Gift* gift) {
  nlohmann::json j;
  j["command"] = _serverOffset + 5;
  j["data"]["action"] = 3;
  j["data"]["message"] = "Open Gift";
  j["data"]["player"] = player->ID;
  j["data"]["gift"] = gift->ID;
  _tcp->Send(player->socket, j.dump());
}

void Server::Read(Client* player, std::vector<std::string> inputs) {
  for (std::string input : inputs) {
    nlohmann::json jRecv;
    try {
      jRecv = nlohmann::json::parse(input.erase(input.find_first_of('\0')));
    }
    catch (...) {
      std::cout << "Error: invalid or malformed JSON." << std::endl;
      continue;
    }
    if (jRecv == nullptr)
      continue;
    int cmd = jRecv["command"];
    std::cout << player->IP << " sent command " << cmd << std::endl;
    switch (cmd) {
      case 0: { // Ping
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        nlohmann::json jSend;
        jSend["command"] = _serverOffset + 1;
        jSend["data"]["message"] = "Pong";
        jSend["data"]["timestamp"] = ms;
        _tcp->Send(player->socket, jSend.dump());
        break;
      }
      case 1: { // Ping Response
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        int64_t time = ms - static_cast<int64_t>(jRecv["data"]["timestamp"]);
        std::cout << "Ping took " << time << "ms" << std::endl;
        break;
      }
      case 2: { // Hello
        if (!player->name.empty()) {
          std::cout << "Player already signed in as " << player->name << "." << std::endl;
          nlohmann::json jSend;
          jSend["command"] = _serverOffset - 1;
          jSend["data"]["message"] = "Already connected";
          _tcp->Send(player->socket, jSend.dump());
        }
        break;
      }
      case 3: { // Client Event
        int action = jRecv["data"]["action"];
        switch (action) {
          case 0: { // Join Room
            player->inRoom = true;
            std::cout << player->name << " has joined the room." << std::endl;
            nlohmann::json jSend;
            jSend["command"] = _serverOffset + 3;
            jSend["data"]["action"] = 0;
            jSend["data"]["message"] = "Success";
            _tcp->Send(player->socket, jSend.dump());
            for (Client* other : _players) {
              if (other->inRoom) {
                ListPlayersInRoom(other, _players);
                ListGifts(other, _gifts);
              }
            }
            break;
          }
          case 1: { // Leave Room
            player->inRoom = false;
            std::cout << player->name << " has left the room." << std::endl;
            nlohmann::json jSend;
            jSend["command"] = _serverOffset + 3;
            jSend["data"]["action"] = 1;
            jSend["data"]["message"] = "Success";
            _tcp->Send(player->socket, jSend.dump());
            for (Client* other : _players) {
              if (other->inRoom) {
                ListPlayersInRoom(other, _players);
                ListGifts(other, _gifts);
              }
            }
            break;
          }
          case 2: { // Ready
            player->ready = true;
            std::cout << player->name << " is ready." << std::endl;
            nlohmann::json jSend;
            jSend["command"] = _serverOffset + 3;
            jSend["data"]["action"] = 2;
            jSend["data"]["message"] = "Success";
            _tcp->Send(player->socket, jSend.dump());
            for (Client* other : _players) {
              ListPlayersReady(other, _players);
            }
            break;
          }
          case 3: { // Unready
            player->ready = false;
            std::cout << player->name << " is unready." << std::endl;
            nlohmann::json jSend;
            jSend["command"] = _serverOffset + 3;
            jSend["data"]["action"] = 3;
            jSend["data"]["message"] = "Success";
            _tcp->Send(player->socket, jSend.dump());
            for (Client* other : _players) {
              ListPlayersReady(other, _players);
            }
            break;
          }
        }
        break;
      }
      case 4: { // Server Event (DO NOT SEND PACKET HERE)
        int action = jRecv["data"]["action"];
        switch (action) {
          case 0: { // List Players
            break;
          }
          case 1: { // List Gifts
            break;
          }
          case 2: { // List Ready
            break;
          }
        }
        break;
      }
      case 5: { // White Elephant Event
        int action = jRecv["data"]["action"];
        switch (action) {
          case 0: { // Game Start
            break;
          }
          case 1: { // Game End
            break;
          }
          case 2: { // Select Gift
            int id = jRecv["data"]["gift"];
            auto result = std::find_if(
              _gifts.begin(),
              _gifts.end(),
              [&id](Gift* g) { return g->ID == id; }
            );
            if (result != _gifts.end()) {
              Gift* gift = *result;
              player->gift = gift;
              for (Client* other : _players) {
                SelectGift(other, player, player->gift);
              }
            }
            break;
          }
          case 3: { // Open Gift
            break;
          }
        }
        break;
      }
      case 6: { // StepMania Event
        int action = jRecv["data"]["action"];
        switch (action) {
          case 0: { // Enter Screen
            break;
          }
          case 1: { // Exit Screen
            break;
          }
        }
        break;
      }
    }
  }
}

void Server::Start() {
  std::cout << "Starting server..." << std::endl;
  ServerManager::GetInstance()->isConnecting = false;
  ServerManager::GetInstance()->connectingIP = "unknown";

  std::ifstream songlist("songlist.txt");
  std::string song;
  int numSongs = 0;
  while (std::getline(songlist, song)) {
    Gift g;
    g.author = song;
    g.ID = numSongs;
    _gifts.push_back(&g);
    numSongs++;
  }

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
  
  std::vector<std::thread> threads;

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
          std::cout << "User " << player->name << " disconnected." << std::endl;
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
    while (_running) {
      ASocket::Socket socket;
      if (_tcp->Listen(socket)) {
        std::cout << "New connection." << std::endl;
        char input[1024] = {};
        _tcp->Receive(socket, input, 1024, false);
        std::string inputStr = std::string(input, 1024);
        inputStr = inputStr.erase(inputStr.find_first_of('\0'));
        nlohmann::json jRecv;
        try {
          jRecv = nlohmann::json::parse(inputStr);
        }
        catch (...) {
          std::cout << "Error: invalid or malformed JSON." << std::endl;
          continue;
        }
        if (jRecv["command"] == 2) {
          while (!ServerManager::GetInstance()->isConnecting)
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
          _mutex.lock();
          Client c;
          c.socket = socket;
          c.IP = std::string(ServerManager::GetInstance()->connectingIP);
          c.name = std::string(jRecv["data"]["name"]);
          c.ID = std::string(jRecv["data"]["ID"]);
          c.connected = true;
          std::cout << "New player connected: " << c.name << std::endl;
          nlohmann::json jSend;
          jSend["command"] = _serverOffset + 2;
          jSend["status"] = "connected";
          jSend["data"]["message"] = "Hello";
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
    _mutex.lock();
    for (std::thread& thread : threads)
      thread.join();
    _mutex.unlock();
  };

  _thread = new std::thread(listener);

  while (_running)
    Update();

  _thread->join();
}

void Server::Update() {
  for (Client* player : _players) {
    _mutex.lock();
    std::vector<std::string> inputs = player->inputs;
    player->inputs.clear();
    _mutex.unlock();
    Read(player, inputs);
  }
}

void Server::Stop() {
  _running = false;
}

bool Server::IsRunning() const { return _running; }
std::string Server::GetName() const { return _name; }
unsigned int Server::GetPort() const { return _port; }
std::vector<Client*> Server::GetPlayers() const { return _players; }
std::vector<Gift*> Server::GetGifts() const { return _gifts; }
CTCPServer* Server::GetConnection() const { return _tcp; }
