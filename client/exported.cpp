#include "network.hpp"
#include "storage.hpp"
#include "exported.hpp"

#include <Windows.h>

player_data_local_t exported::convert_data_to_local(player_data_t const& data) {
  player_data_local_t result;

  result.updated = data.updated;
  result.health = data.health;
  result.armor = data.armor;
  result.weapon = data.weapon;
  result.model = data.model;
  result.color = data.color;
  result.position = data.position;

  result.quaternion = {
	data.quaternion.w / 127.f,
	data.quaternion.x / 127.f,
	data.quaternion.y / 127.f,
	data.quaternion.z / 127.f,
  };

  memset(result.nickname, 0, sizeof(result.nickname));
  memcpy(result.nickname, data.nickname, sizeof(data.nickname));

  return result;
}

unsigned long __stdcall exported::thread_routine(exported* exported) {
  while (true) {
	Sleep(300);

	const auto buffers = exported->_buffers;

	for (auto& buffer : buffers) {
	  if (!buffer->update)
		continue;

	  exported->update_buffer_internal(buffer);
	}
  }
  return 0;
}

void exported::update_buffer_internal(buffer_t* buffer) {
  auto& network = network::instance();

  if (buffer->server_id == INVALID_SERVER_INDEX)
	return;

  simple_packet packet(2);

  packet.write<uint8_t>(ID_GET_SERVER_BUFFER);
  packet.write(static_cast<uint8_t>(buffer->server_id));

  network.send(packet);

  const auto answer = network.receive(MAX_SIZE_FOR_BUFFER);

  if (answer == nullptr) {
	buffer->update = false;
	return;
  }

  const auto players_count = answer->get_data_size() / (sizeof(uint16_t) + sizeof(player_data_t));
  memset(buffer->players, 0, sizeof(buffer->players));

  for (size_t i = 0; i < players_count; ++i) {
	const auto player_id = answer->read<uint16_t>();

	if (player_id >= MAX_PLAYERS)
	  continue;

	buffer->players[player_id] = answer->read<player_data_t>();
  }
  
  buffer->update = false;
  delete answer;
}

exported::exported() {
  const auto thread = CreateThread(0, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(&thread_routine), this, 0, 0);
  if (thread != NULL) {
	CloseHandle(thread);
  }
}

exported::~exported() {
  for (const auto& buffer : _buffers) {
	delete buffer;
  }
}

int __stdcall exported::create_buffer(const char* ip) {
  auto& buffers = instance()._buffers;

  const auto num_ip = network::instance().ip_to_num(ip).S_un.S_addr;
  const auto server_id = storage::instance().get_server_index(num_ip);

  if (server_id == INVALID_SERVER_INDEX)
	return -1;

  for (size_t i = 0; i < buffers.size(); ++i)
	if (buffers[i]->server_id == server_id)
	  return static_cast<int>(i);

  int result = static_cast<int>(buffers.size());
  buffers.resize(result + 1);

  buffers[result] = new buffer_t;

  memset(buffers[result], 0, sizeof(buffer_t));
  buffers[result]->server_id = server_id;
  buffers[result]->update = false;

  return result;
}

void __stdcall exported::update_buffer(int buffer_id) {
  auto& buffers = instance()._buffers;

  if (static_cast<int>(buffers.size()) <= buffer_id)
	return;

  buffers[buffer_id]->update = true;
}

player_data_local_t __stdcall exported::get_player_data(int buffer_id, uint16_t player_id) {
  auto& buffers = instance()._buffers;

  if (static_cast<int>(buffers.size()) <= buffer_id || player_id >= MAX_PLAYERS)
	return player_data_local_t();

  return convert_data_to_local(buffers[buffer_id]->players[player_id]);
}

exported& exported::instance() {
  static exported inst;
  return inst;
}
