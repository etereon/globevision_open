#include "storage.hpp"
#include "config.hpp"
#include "server.hpp"

storage::storage() {}

void storage::update_player_sync(uint8_t server_id, uint16_t player_id, uint8_t health, uint8_t armor, uint8_t weapon, position_t position, quat_compressed_t quaternion) {
  if (server_id >= _buffers.size() || player_id >= MAX_PLAYERS || fabsf(position.x) > 3000.f || fabsf(position.y) > 3000.f)
	return;

  auto& buffer = _buffers[server_id];
  auto& data = buffer.players[player_id];

  data.health = health;
  data.armor = armor;
  data.weapon = weapon;
  data.position = position;
  data.quaternion = quaternion;

  printf("SERVER_ID: %d | PLAYER: %s (%d) | POS: %.2f %.2f %.2f | MODEL: %d | COLOR: %X | HP: %d | ARMOR: %d | WEAPON: %d\n", 
	server_id, data.nickname, player_id, data.position.x, data.position.y, data.position.z,
	data.model, data.color, data.health, data.armor, data.weapon);

  buffer.update_time[player_id] = std::time(nullptr);
}

void storage::update_player_misc(uint8_t server_id, uint16_t player_id, uint16_t model, uint32_t color) {
  if (server_id >= _buffers.size() || player_id >= MAX_PLAYERS)
	return;

  auto& data = _buffers[server_id].players[player_id];

  data.model = model;
  data.color = color;
}

void storage::update_player_name(uint8_t server_id, uint16_t player_id, char* nickname) {
  if (server_id >= _buffers.size() || player_id >= MAX_PLAYERS)
	return;

  auto& data = _buffers[server_id].players[player_id];
  memcpy(data.nickname, nickname, sizeof(data.nickname));
}

player_data_t const& storage::get_player_data(size_t server_id, uint16_t player_id) {
  server_id = server_id >= _ips.size() ? 0 : server_id;
  player_id = player_id >= MAX_PLAYERS ? 0 : player_id;

  auto& data = _buffers[server_id];
  data.players[player_id].updated = (std::time(nullptr) - data.update_time[player_id]) < 5;
  return data.players[player_id];
}

const std::vector<uint32_t>& storage::get_ips() const {
  return _ips;
}

size_t storage::get_server_id(uint32_t ip) const {
  for (size_t i = 0; i < _ips.size(); ++i)
	if (_ips[i] == ip)
	  return i;

  return INVALID_SERVER_INDEX;
}

bool storage::initialize() {
  const auto& config = config::instance();

  if (config.get_value_size("servers") == 0) {
	printf("%s - error: zero servers in list\n", __FUNCTION__);
	return false;
  }

  const auto str_list = config.get_value<std::vector<std::string>>("servers");

  for (size_t i = 0; i < str_list.size(); ++i) {
	printf("%s - %s added to list with id %zd\n", __FUNCTION__, str_list[i].c_str(), i);
	_ips.push_back(server::instance().ip_to_num(str_list[i]).S_un.S_addr);
  }

  _buffers.resize(_ips.size());
  memset(_buffers.data(), 0, _buffers.size() * sizeof(server_data_t));
  return true;
}

storage& storage::instance() {
  static storage inst;
  return inst;
}
