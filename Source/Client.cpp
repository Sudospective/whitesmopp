#include "Client.hpp"

void Client::SetIP(std::string ip) {
  _ip = ip;
}
std::string Client::GetIP() const {
  return _ip;
}
void Client::SetName(std::string name) {
  _name = name;
}
std::string Client::GetName() const {
  return _name;
}
void Client::SetSocket(ASocket::Socket socket) {
  _socket = socket;
}
ASocket::Socket Client::GetSocket() const {
  return _socket;
}
void Client::SetType(ClientType type) {
  _type = type;
}
ClientType Client::GetType() const {
  return _type;
}
