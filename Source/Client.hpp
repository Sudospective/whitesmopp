#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

#include "Socket.h"

enum class ClientType {
  Guest,
  User,
  Admin,
};

class Client {
 public:
  /// @brief Set the client IP.
  /// @param ip 
  void SetIP(std::string ip);
  /// @brief Get the client IP.
  /// @return 
  std::string GetIP() const;
  /// @brief Set the client name.
  /// @param name 
  void SetName(std::string name);
  /// @brief Get the client name.
  /// @return 
  std::string GetName() const;
  /// @brief Set the client socket.
  /// @param socket 
  void SetSocket(ASocket::Socket socket);
  /// @brief Get the client socket.
  /// @return 
  ASocket::Socket GetSocket() const;
  /// @brief Set the client type.
  ///        Client types are `Guest`, `User`, and `Admin`.
  /// @param type 
  void SetType(ClientType type);
  /// @brief Get the client type.
  ///        Client types are `Guest`, `User`, and `Admin`.
  /// @return 
  ClientType GetType() const;

 public:
  bool connected;
  bool loggedIn;
  unsigned int roomID;
  std::vector<std::string> vInput;

 private:
  std::string _ip;
  std::string _name;
  ASocket::Socket _socket;
  ClientType _type;
};

#endif // CLIENT_HPP
