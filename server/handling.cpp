#include "handling.hpp"
#include "storage.hpp"

#include <datatypes/datatypes.hpp>

handling::handling() {}

void handling::requests_handler(simple_packet& request, simple_packet& answer) {
  const auto request_id = request.read<uint8_t>();
  auto& storage = storage::instance();

  answer.reset(false);
  answer.set_data_size(0);

  switch (request_id) {
  case ID_GET_SERVERS_LIST: {
	const auto& ips = storage.get_ips();
	const auto size = static_cast<size_type>(ips.size() * 4);
	answer.set_data_size(size);
	answer.write_bytes(reinterpret_cast<const uint8_t*>(ips.data()), size);
	break;
  }
  case ID_GET_SERVER_BUFFER: {
	if (request.get_data_size() != 2)
	  break;

	const auto server_id = request.read<uint8_t>();
	size_type final_size = 0;

	for (uint16_t i = 0; i < MAX_PLAYERS; ++i) {
	  const auto player = storage.get_player_data(server_id, i);
	  
	  if (!player.updated)
		continue;

	  answer.write(i);
	  answer.write(player);
	  final_size += sizeof(i) + sizeof(player);
	}

	answer.set_data_size(final_size);
	break;
  }
  case ID_SET_PLAYER_SYNC: {
	if (request.get_data_size() != 23)
	  break;

	const auto server_id = request.read<uint8_t>();
	const auto player_id = request.read<uint16_t>();

	const auto health = request.read<uint8_t>();
	const auto armor = request.read<uint8_t>();
	const auto weapon = request.read<uint8_t>();
	const auto position = request.read<position_t>();
	const auto quaternion = request.read<quat_compressed_t>();

	storage.update_player_sync(server_id, player_id, health, armor, weapon, position, quaternion);
	break;
  }
  case ID_SET_PLAYER_MISC: {
	if (request.get_data_size() != 10)
	  break;

	const auto server_id = request.read<uint8_t>();
	const auto player_id = request.read<uint16_t>();

	const auto model = request.read<uint16_t>();
	const auto color = request.read<uint32_t>();

	storage.update_player_misc(server_id, player_id, model, color);
	break;
  }
  case ID_SET_PLAYER_NAME: {
	if (request.get_data_size() != 24)
	  break;

	const auto server_id = request.read<uint8_t>();
	const auto player_id = request.read<uint16_t>();

	char nickname[20];
	request.read_bytes(reinterpret_cast<uint8_t*>(nickname), sizeof(nickname));

	storage.update_player_name(server_id, player_id, nickname);
	break;
  }
  default:
	break;
  }
}

handling& handling::instance() {
  static handling inst;
  return inst;
}
