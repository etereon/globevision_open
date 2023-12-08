#pragma once

#include <datatypes/datatypes.hpp>
#include <vector>

struct player_data_local_t {
  bool updated;
  uint8_t health;
  uint8_t armor;
  uint16_t weapon;
  uint16_t model;
  position_t position;
  quat_t quaternion;
  uint32_t color;
  char nickname[21];
};

class exported {
  struct buffer_t {
    bool update;
    size_t server_id;
    player_data_t players[MAX_PLAYERS];
  };

  std::vector<buffer_t*> _buffers;

  static player_data_local_t convert_data_to_local(player_data_t const& data);
  static unsigned long __stdcall thread_routine(exported* exported);

  void update_buffer_internal(buffer_t* buffer);

  exported();
  ~exported();

public:

  static int __stdcall create_buffer(const char* ip);
  static void __stdcall update_buffer(int buffer_id);
  static player_data_local_t __stdcall get_player_data(int buffer_id, uint16_t player_id);

  exported(const exported&) = delete;
  exported& operator=(const exported&) = delete;

  static exported& instance();
};
