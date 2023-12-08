#include "events.hpp"

#include "process.hpp"
#include "storage.hpp"

#include <RakNet/PacketEnumerations.h>
#include <mh_wrap/mh_wrap.hpp>
#include <intrin.h>

void __fastcall events::on_register_raknet_callback(RakClientInterface* rci, void* edx, int* id, uintptr_t callback) {
  static auto installed = false;

  if (!installed) {
	int id = RPC_SetPlayerName;
	const auto callback = reinterpret_cast<uintptr_t>(&on_change_nickname);
	mh_wrap::instance().original<decltype(&on_register_raknet_callback)>("rrc")(rci, edx, &id, callback);
	installed = true;
  }

  switch (*id) {
  case RPC_WorldPlayerAdd:
	mh_wrap::instance().add("world_player_add", callback, &on_world_player_add);
	break;
  case RPC_ServerJoin:
	mh_wrap::instance().add("server_join", callback, &on_server_join);
	break;
  default:
	break;
  }

  return mh_wrap::instance().original<decltype(&on_register_raknet_callback)>("rrc")(rci, edx, id, callback);
}

void __cdecl events::on_world_player_add(RakNet::BitStream* bs) {
  uint16_t player_id = 0;
  uint32_t model = 0;
  uint32_t color = 0;

  bs->Read(player_id);
  bs->IgnoreBits(8);
  bs->Read(model);
  bs->IgnoreBits(128);
  bs->Read(color);

  storage::instance().update_player_misc(player_id, model, color);

  if (process::instance().is_raksamp()) {
	bs->ResetReadPointer();
	return mh_wrap::instance().original<decltype(&on_world_player_add)>("world_player_add")(bs);
  }
}

void __cdecl events::on_server_join(RakNet::BitStream* bs) {
  uint16_t player_id = 0;
  uint8_t name_len = 0;

  bs->Read(player_id);
  bs->IgnoreBits(40);
  bs->Read(name_len);

  if (name_len >= 3 && name_len < 21) {
	char nickname[21];

	bs->ReadBits(reinterpret_cast<uint8_t*>(nickname), name_len * 8);
	nickname[name_len] = 0;

	storage::instance().update_player_name(player_id, nickname);
  }

  if (process::instance().is_raksamp()) {
	bs->ResetReadPointer();
	return mh_wrap::instance().original<decltype(&on_server_join)>("server_join")(bs);
  }
}

void __cdecl events::on_change_nickname(RakNet::BitStream* bs) {
  uint16_t player_id = 0;
  uint8_t name_len = 0;

  bs->Read(player_id);
  bs->Read(name_len);

  if (!(name_len >= 3 && name_len < 21))
	return;

  char nickname[21];

  bs->ReadBits(reinterpret_cast<uint8_t*>(nickname), name_len * 8);
  nickname[name_len] = 0;

  storage::instance().update_player_name(player_id, nickname);
}

Packet* __fastcall events::on_receive_packet(RakClientInterface* rci, void* edx) {
  const auto packet = mh_wrap::instance().original<decltype(&on_receive_packet)>("receive_packet")(rci, edx);
  auto& storage = storage::instance();

  if (packet == nullptr)
	return packet;

  const auto packet_id = *packet->data;
  const auto required_packet = packet_id == ID_VEHICLE_SYNC || packet_id == ID_PLAYER_SYNC || packet_id == ID_PASSENGER_SYNC;

  if (!required_packet || !storage.is_ip_listed())
	return packet;

  RakNet::BitStream bs(packet->data, packet->bitSize / 8, false);
  const auto player_id = *reinterpret_cast<uint16_t*>(packet->data + 1);

  uint8_t health = 0, armor = 0, weapon = 0;
  position_t position{ 0.f, 0.f, 0.f };
  quat_t quaternion{ 0.f, 0.f, 0.f, 0.f };

  switch (packet_id) {
  case ID_PLAYER_SYNC: {
	auto has_keys = false;
	bs.SetReadOffset(24);
	bs.Read(has_keys);
	bs.IgnoreBits(has_keys ? 16 : 0);
	bs.Read(has_keys);
	bs.IgnoreBits(16 + (has_keys ? 16 : 0));
	bs.Read(position);
	bs.ReadNormQuat(quaternion.w, quaternion.x, quaternion.y, quaternion.z);
	bs.Read(health);
	bs.Read(weapon);
	break;
  }
  case ID_VEHICLE_SYNC: {
	bs.SetReadOffset(88);
	bs.ReadNormQuat(quaternion.w, quaternion.x, quaternion.y, quaternion.z);
	bs.Read(position);
	bs.SetReadOffset(332);
	bs.Read(health);
	break;
  }
  case ID_PASSENGER_SYNC: {
	bs.SetReadOffset(120);
	bs.Read(position);
	bs.SetReadOffset(56);
	bs.Read(health);
	break;
  }
  default:
	break;
  }

  if (packet_id == ID_PLAYER_SYNC || packet_id == ID_VEHICLE_SYNC) {
	const auto temp = health;
	health = (temp >> 4) * 7;
	armor = (temp & 0x0F) * 7;
  }
  else {
	bs.Read(armor);
  }

  storage.update_player_sync(player_id, health, armor, weapon, position, quaternion);
  return packet;
}

bool __fastcall events::on_receive_rpc(void* unk, void* edx, uint8_t* data, size_t size, PlayerID id) {
  const auto rpc_id = *(data + 1);

  if (rpc_id != RPC_SetPlayerName && rpc_id != RPC_WorldPlayerAdd && rpc_id != RPC_ServerJoin)
	return mh_wrap::instance().original<decltype(&on_receive_rpc)>("receive_rpc")(unk, edx, data, size, id);

  RakNet::BitStream bs(data, size, false);
  bs.SetReadOffset(rpc_id == RPC_WorldPlayerAdd ? 35 : 28);

  switch (rpc_id) {
  case RPC_SetPlayerName:
	on_change_nickname(&bs);
	break;
  case RPC_WorldPlayerAdd:
	on_world_player_add(&bs);
	break;
  case RPC_ServerJoin:
	on_server_join(&bs);
	break;
  default:
	break;
  }

  return mh_wrap::instance().original<decltype(&on_receive_rpc)>("receive_rpc")(unk, edx, data, size, id);
}

unsigned long __stdcall events::on_update_ip(const char* ip) {
  const auto result = mh_wrap::instance().original<decltype(&on_update_ip)>("update_ip")(ip);
  const auto return_address = reinterpret_cast<uintptr_t>(_ReturnAddress());

  if (return_address == process::instance().get_ia_ret_address())
	storage::instance().update_ip(result);

  return result;
}

events::events() {}

void events::deploy() {
  const auto& process = process::instance();
  const auto raksamp = process.is_raksamp();

  if (!raksamp && process.get_samp_version() == VER_UNKNOWN)
	return;

  auto& hooks = mh_wrap::instance();

  if (raksamp)
	hooks.add("rrc", process.get_raksamp_rrc_address(), &on_register_raknet_callback);
  else
	hooks.add("receive_rpc", process.get_rpc_handler_address(), &on_receive_rpc);

  hooks.add("receive_packet", process.get_receive_address(), &on_receive_packet);
  hooks.add("update_ip", L"ws2_32.dll", "inet_addr", &on_update_ip);
}

events& events::instance() {
  static events inst;
  return inst;
}
