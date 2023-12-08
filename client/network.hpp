#pragma once

#include <simple_packet/simple_packet.hpp>

class network {
  SOCKET _socket = NULL;
  sockaddr_in _server_addr{};

  network();
  ~network();

public:

  in_addr ip_to_num(const char* ip);

  int send(simple_packet& data);
  simple_packet* receive(size_type size = MAX_REQUEST_SIZE);

  network(const network&) = delete;
  network& operator=(const network&) = delete;

  static network& instance();
};
