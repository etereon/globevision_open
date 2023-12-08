#include "network.hpp"
#include "storage.hpp"

#include <Windows.h>

unsigned long __stdcall storage::thread_routine(storage* storage) {
  simple_packet packet(MAX_BUFFER_SIZE);

  while (true) {
	Sleep(300);

	if (!storage->is_ip_listed())
	  continue;

	size_type final_size = 3;

	packet.reset(false);
	packet.write<uint8_t>(ID_SET_SERVER_BUFFER);
	packet.write(static_cast<uint8_t>(storage->_index));

	for (uint16_t i = 0; i < MAX_PLAYERS; ++i) {
	  Sleep(1);

	  auto& player = storage->_players[i];
	  const auto& data = player.data;

	  if (!player.updated_sync)
		continue;

	  if (player.updated_misc || player.updated_nick) {
		storage->send_misc(i);
		storage->send_nick(i);
	  }

	  packet.write(i);
	  packet.write(data.health);
	  packet.write(data.armor);
	  packet.write<uint8_t>(data.weapon);
	  packet.write(data.position);
	  packet.write(data.quaternion);

	  player.updated_sync = false;
	  final_size += 21;
	}

	if (final_size == 3)
	  continue;

	packet.set_data_size(final_size);
	network::instance().send(packet);
  }
  return 0;
}

void storage::send_misc(uint16_t player_id) {
  auto& player = _players[player_id];

  if (!player.updated_misc)
	return;

  const auto& data = player.data;
  simple_packet packet(10);

  packet.write<uint8_t>(ID_SET_PLAYER_MISC);
  packet.write(static_cast<uint8_t>(_index));
  packet.write(player_id);

  packet.write(data.model);
  packet.write(data.color);

  network::instance().send(packet);
  player.updated_misc = false;
}

void storage::send_nick(uint16_t player_id) {
  auto& player = _players[player_id];

  if (!player.updated_nick)
	return;

  const auto& data = player.data;
  simple_packet packet(24);

  packet.write<uint8_t>(ID_SET_PLAYER_NAME);
  packet.write(static_cast<uint8_t>(_index));
  packet.write(player_id);

  packet.write_bytes(reinterpret_cast<const uint8_t*>(data.nickname), sizeof(data.nickname));

  network::instance().send(packet);
  player.updated_nick = false;
}

void storage::send(uint16_t player_id) {
  auto& player = _players[player_id];

  // send_sync(player_id, player);
}

storage::storage() {
  memset(_players, 0, sizeof(_players));
  const auto thread = CreateThread(0, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(&thread_routine), this, 0, 0);
  if (thread != NULL) {
	CloseHandle(thread);
  }
}

void storage::update_player_sync(uint16_t player_id, uint8_t health, uint8_t armor, uint8_t weapon, position_t position, quat_t quaternion) {
  if (player_id >= MAX_PLAYERS || fabsf(position.x) > 3000.f || fabsf(position.y) > 3000.f)
	return;

  auto& player = _players[player_id];
  auto& data = player.data;

  data.health = health <= 100 ? health : 100;
  data.armor = armor <= 100 ? armor : 100;
  data.weapon = weapon;
  data.position = position;

  data.quaternion = {
	static_cast<int8_t>(quaternion.w * 127.f),
	static_cast<int8_t>(quaternion.x * 127.f),
	static_cast<int8_t>(quaternion.y * 127.f),
	static_cast<int8_t>(quaternion.z * 127.f),
  };

  player.updated_sync = true;
}

void storage::update_player_misc(uint16_t player_id, uint16_t model, uint32_t color) {
  if (player_id >= MAX_PLAYERS)
	return;

  auto& player = _players[player_id];

  player.data.model = model;
  player.data.color = color;

  player.updated_misc = true;
}

void storage::update_player_name(uint16_t player_id, char* nickname) {
  if (player_id >= MAX_PLAYERS || nickname == nullptr)
	return;

  auto& player = _players[player_id];

  memcpy(player.data.nickname, nickname, sizeof(player.data.nickname));
  player.updated_nick = true;
}

void storage::update_ip(uint32_t ip) {
  _index = get_server_index(ip);
  _ip = ip;
}

bool storage::is_ip_listed() const {
  return _index != INVALID_SERVER_INDEX;
}

size_t storage::get_server_index(uint32_t ip) const {
  for (size_t i = 0; i < _ips.size(); ++i)
	if (_ips[i] == ip)
	  return i;

  return INVALID_SERVER_INDEX;
}

void storage::initialize() {
  auto& network = network::instance();

  simple_packet packet(1);
  packet.write<uint8_t>(ID_GET_SERVERS_LIST);
  network.send(packet);

  const auto answer = network.receive();

  if (answer != nullptr && answer->get_data_size() != 0) {
	_ips.resize(answer->get_data_size() / sizeof(uint32_t));
	memcpy(_ips.data(), answer->get_data_ptr(), answer->get_data_size());
  }

  delete answer;
}

storage& storage::instance() {
  static storage inst;
  return inst;
}
