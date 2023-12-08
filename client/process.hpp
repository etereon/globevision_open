#pragma once

#include <Windows.h>

enum samp_version {
  VER_037_R1,
  VER_037_R3_1,
  VER_UNKNOWN,
};

class process {
  uintptr_t _base = reinterpret_cast<uintptr_t>(GetModuleHandleA(NULL));
  samp_version _version = VER_UNKNOWN;

  struct {
	uintptr_t entry = 0x5FB68;
	uintptr_t receive = 0x50C4;
	uintptr_t ia_ret = 0x8C0F;
	uintptr_t rrc = 0x50CF;
  } _raksamp_offsets;

  uintptr_t get_address(uintptr_t offset) const;

  process();

public:

  samp_version get_samp_version() const;
  bool is_raksamp() const;

  uintptr_t get_receive_address() const;
  uintptr_t get_ia_ret_address() const;
  uintptr_t get_rpc_handler_address() const;
  uintptr_t get_raksamp_rrc_address() const;

  process(const process&) = delete;
  process& operator=(const process&) = delete;

  static process& instance();
};
