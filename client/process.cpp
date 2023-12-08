#include "process.hpp"

#include <Windows.h>

uintptr_t process::get_address(uintptr_t offset) const {
  return _base + offset;
}

process::process() {
  if (is_raksamp())
	return;

  _base = reinterpret_cast<uintptr_t>(GetModuleHandleA("samp.dll"));

  if (!_base)
	return;

  const auto header = reinterpret_cast<PIMAGE_NT_HEADERS32>(_base + reinterpret_cast<PIMAGE_DOS_HEADER>(_base)->e_lfanew);

  switch (header->OptionalHeader.AddressOfEntryPoint) {
  case 0x31DF13:
	_version = VER_037_R1;
	break;
  case 0xCC4D0:
	_version = VER_037_R3_1;
	break;
  default:
	_version = VER_UNKNOWN;
  }
}

samp_version process::get_samp_version() const {
  return _version;
}

bool process::is_raksamp() const {
  const auto base = reinterpret_cast<uintptr_t>(GetModuleHandleA(NULL));
  const auto header = reinterpret_cast<PIMAGE_NT_HEADERS32>(base + reinterpret_cast<PIMAGE_DOS_HEADER>(base)->e_lfanew);
  return header->OptionalHeader.AddressOfEntryPoint == _raksamp_offsets.entry;
}

uintptr_t process::get_receive_address() const {
  return get_address(is_raksamp() ? _raksamp_offsets.receive : (_version == VER_037_R3_1 ? 0x34AC0 : 0x31710));
}

uintptr_t process::get_ia_ret_address() const {
  return get_address(is_raksamp() ? _raksamp_offsets.ia_ret : (_version == VER_037_R3_1 ? 0x3857F : 0x351CF));
}

uintptr_t process::get_rpc_handler_address() const {
  return get_address(_version == VER_037_R3_1 ? 0x3a6a0 : 0x372F0);
}

uintptr_t process::get_raksamp_rrc_address() const {
  return get_address(_raksamp_offsets.rrc);
}

process& process::instance() {
  static process inst;
  return inst;
}
