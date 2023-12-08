#pragma once

#include <WS2tcpip.h>
#include <iostream>

class server {
  SOCKET _socket = INVALID_SOCKET;

  bool bind_server_addr();

  server();
  ~server();
   
public:
  in_addr ip_to_num(std::string const& ip);

  bool start();
  void loop();

  server(const server&) = delete;
  server& operator=(const server&) = delete;

  static server& instance();
};
