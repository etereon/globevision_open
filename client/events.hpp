#pragma once

#include <RakNet/RakClientInterface.h>
#include <cstdint>

enum eRPCs {
  RPC_SetPlayerName = 11,
  RPC_WorldPlayerAdd = 32,
  RPC_ServerJoin = 137,
};

class events {
  static void __fastcall on_register_raknet_callback(RakClientInterface* rci, void* edx, int* id, uintptr_t callback);

  static void __cdecl on_world_player_add(RakNet::BitStream* bs);
  static void __cdecl on_server_join(RakNet::BitStream* bs);
  static void __cdecl on_change_nickname(RakNet::BitStream* bs);

  static Packet* __fastcall on_receive_packet(RakClientInterface* rci, void* edx);
  static bool __fastcall on_receive_rpc(void* unk, void* edx, uint8_t* data, size_t size, PlayerID id);

  static unsigned long __stdcall on_update_ip(const char* ip);

  events();

public:

  void deploy();

  events(const events&) = delete;
  events& operator=(const events&) = delete;

  static events& instance();
};
