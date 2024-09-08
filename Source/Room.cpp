#include "Room.hpp"

void Room::SetOwner(Client &player) {
  _owner = &player;
}
Client& Room::GetOwner() const {
  return *_owner;
}

void Room::SetName(std::string name) {
  _name = name;
}
std::string Room::GetName() const {
  return _name;
}
void Room::SetDescription(std::string description) {
  _description = description;
}
std::string Room::GetDescription() const {
  return _description;
}
void Room::SetPassword(std::string password) {
  _password = password;
}
std::string Room::GetPassword() const {
  return _password;
}
