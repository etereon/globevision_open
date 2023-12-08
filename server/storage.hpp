#pragma once

#include <cstdint>
#include <vector>

#include <datatypes/datatypes.hpp>

struct server_data_t {
  time_t update_time[MAX_PLAYERS];
  player_data_t players[MAX_PLAYERS];
};

class storage {
  std::vector<server_data_t> _buffers;
  std::vector<uint32_t> _ips;

  storage();

public:

  bool update_player_sync(uint8_t server_id, uint16_t player_id, uint8_t health, uint8_t armor,
	uint8_t weapon, position_t position, quat_compressed_t quaternion);

  void update_player_misc(uint8_t server_id, uint16_t player_id, uint16_t model, uint32_t color);
  void update_player_name(uint8_t server_id, uint16_t player_id, char* nickname);

  player_data_t const& get_player_data(size_t server_id, uint16_t player_id);

  const std::vector<uint32_t>& get_ips() const;
  size_t get_server_id(uint32_t ip) const;

  bool initialize();

  storage(const storage&) = delete;
  storage& operator=(const storage&) = delete;

  static storage& instance();
};
