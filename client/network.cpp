#include "network.hpp"
#include "config.hpp"

#include <iostream>

network::network() {
  WSAData data{};

  const auto& config = config::instance();

  const auto ip = config.get_value<std::string>("ip");
  const auto port = config.get_value<uint16_t>("port");

  printf("%s - network startup on %s:%d\n", __FUNCTION__, ip.c_str(), port);

  if (const auto error = WSAStartup(MAKEWORD(2, 2), &data) != 0) {
	printf("%s - WSAStartup error: %d\n", __FUNCTION__, error);
	return;
  }

  _socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  uint32_t timeout = 2000;
  setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));

  ZeroMemory(&_server_addr, sizeof(_server_addr));
  _server_addr.sin_family = AF_INET;
  _server_addr.sin_port = htons(port);
  _server_addr.sin_addr = ip_to_num(ip.c_str());

  sockaddr_in client_addr{};

  ZeroMemory(&client_addr, sizeof(client_addr));
  client_addr.sin_family = AF_INET;
  client_addr.sin_port = htons(0);
  client_addr.sin_addr.S_un.S_addr = INADDR_ANY;

  if (bind(_socket, reinterpret_cast<sockaddr*>(&client_addr), sizeof(client_addr)) == SOCKET_ERROR) {
	printf("%s - bind error: %d\n", __FUNCTION__, WSAGetLastError());
	return;
  }
}

network::~network() {
  closesocket(_socket);
  WSACleanup();
}

in_addr network::ip_to_num(const char* ip) {
  in_addr result{};

  if (inet_pton(AF_INET, ip, &result) == SOCKET_ERROR) {
	result.S_un.S_addr = 0;
  }

  return result;
}

int network::send(simple_packet& data) {
  return data.send(_socket, _server_addr);
}

simple_packet* network::receive(size_type size) {
  auto answer = new simple_packet(size);

  const auto received = answer->receive(_socket, _server_addr);

  if (received == SOCKET_ERROR || received != answer->get_data_size() + sizeof(size_type)) {
	delete answer;
	answer = nullptr;
  }

  return answer;
}

network& network::instance() {
  static network inst;
  return inst;
}
