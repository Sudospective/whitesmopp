#ifndef ROOM_HPP
#define ROOM_HPP

#include <string>
#include <vector>

#include "Client.hpp"

class Room {
 public:
  /// @brief Set the owner of the room.
  /// @param player
  void SetOwner(Client& player);
  /// @brief Get the owner of the room.
  /// @return 
  std::string GetOwner() const;
  /// @brief Set the name of the room.
  /// @param name 
  void SetName(std::string name);
  /// @brief Get the name of the room.
  /// @return 
  std::string GetName() const;
  /// @brief Set the description of the room.
  /// @param description 
  void SetDescription(std::string description);
  /// @brief Get the description of the room.
  /// @return 
  std::string GetDescription() const;
  /// @brief Set the password for administrator privileges.
  /// @param password 
  void SetPassword(std::string password);
  /// @brief Get the password for administrator privileges.
  /// @return 
  std::string GetPassword() const;

 private:
  Client* _owner;
  std::string _name;
  std::string _description;
  std::string _password;
  std::vector<Client> _players;
};

#endif // ROOM_HPP
