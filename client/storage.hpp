#pragma once

#include <datatypes/datatypes.hpp>

#include <vector>

class storage {
  size_t _index = INVALID_SERVER_INDEX;
  uint32_t _ip = 0;

  std::vector<uint32_t> _ips;

  struct player_t {
	bool updated_sync;
	bool updated_misc;
	bool updated_nick;
	player_data_t data;
  } _players[MAX_PLAYERS];

  static unsigned long __stdcall thread_routine(storage* storage);

  void send_misc(uint16_t player_id);
  void send_nick(uint16_t player_id);

  void send(uint16_t player_id);

  storage();

public:

  void update_player_sync(uint16_t player_id, uint8_t health, uint8_t armor, 
	uint8_t weapon, position_t position, quat_t quaternion);

  void update_player_misc(uint16_t player_id, uint16_t model, uint32_t color);
  void update_player_name(uint16_t player_id, char* nickname);

  void update_ip(uint32_t ip);
  bool is_ip_listed() const;

  size_t get_server_index(uint32_t ip) const;

  void initialize();

  storage(const storage&) = delete;
  storage& operator=(const storage&) = delete;

  static storage& instance();
};

