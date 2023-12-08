#pragma once

#include <simple_packet/simple_packet.hpp>

class handling {

  handling();

public:

  void requests_handler(simple_packet& request, simple_packet& answer);

  handling(const handling&) = delete;
  handling& operator=(const handling&) = delete;

  static handling& instance();
};
