#pragma once

#include <cstdint>

#define MAX_PLAYERS 1005
#define INVALID_SERVER_INDEX size_t(-1)

enum packets_list {
  ID_GET_SERVERS_LIST,
  ID_GET_SERVER_BUFFER,
  ID_SET_SERVER_BUFFER,
  ID_SET_PLAYER_MISC,
  ID_SET_PLAYER_NAME
};

struct position_t {
  float x, y, z;
};

struct quat_t {
  float w, x, y, z;
};

struct quat_compressed_t {
  int8_t w, x, y, z;
};

struct player_data_t {
  uint8_t updated : 1;
  uint8_t health : 7;
  uint8_t armor;
  uint16_t weapon : 6;
  uint16_t model : 10;
  position_t position;
  quat_compressed_t quaternion;
  uint32_t color;
  char nickname[20];
};
