#include "server.hpp"

#include "config.hpp"
#include "storage.hpp"
#include "handling.hpp"

in_addr server::ip_to_num(std::string const& ip) {
  in_addr result{};
  
  if (inet_pton(AF_INET, ip.c_str(), &result) == SOCKET_ERROR) {
	printf("%s - inet_pton error: %d\n", __FUNCTION__, WSAGetLastError());
	result.S_un.S_addr = 0;
  }

  return result;
}

bool server::bind_server_addr() {
  const auto& config = config::instance();

  sockaddr_in address{};

  const auto ip = config.get_value<std::string>("ip");
  const auto port = config.get_value<uint16_t>("port");

  ZeroMemory(&address, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_addr = ip_to_num(ip);
  address.sin_port = htons(port);

  if (bind(_socket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR) {
	printf("%s - bind error: %d\n", __FUNCTION__, WSAGetLastError());
	return false;
  }

  printf("%s - server started %s:%d\n", __FUNCTION__, ip.c_str(), port);

  return true;
}

server::server() {
  WSAData data{};

  if (const auto error = WSAStartup(MAKEWORD(2, 2), &data) != 0) {
	printf("%s - WSAStartup error: %d\n", __FUNCTION__, error);
	return;
  }

  _socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  if (_socket == INVALID_SOCKET) {
	printf("%s - socket creation error: %d\n", __FUNCTION__, WSAGetLastError());
	return;
  }
}

server::~server() {
  closesocket(_socket);
  WSACleanup();
}

server& server::instance() {
  static server inst;
  return inst;
}

bool server::start() {
  return _socket != INVALID_SOCKET && bind_server_addr() && storage::instance().initialize();
}

void server::loop() {
  simple_packet incoming(MAX_REQUEST_SIZE);
  simple_packet answer(MAX_SIZE_FOR_BUFFER);
  while (true) {
	incoming.reset();

	sockaddr_in client_addr;
	ZeroMemory(&client_addr, sizeof(client_addr));

	const auto received = incoming.receive(_socket, client_addr);

	if (received == SOCKET_ERROR || received != incoming.get_data_size() + sizeof(size_type))
	  continue;

	handling::instance().requests_handler(incoming, answer);

	if (answer.get_data_size() != 0) {
	  // answer.print();
	  answer.send(_socket, client_addr);
	}
  }
}
