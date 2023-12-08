#pragma once

#include <WS2tcpip.h>
#include <iostream>
#include <cstdint>

#define MAX_REQUEST_SIZE 1024
#define MAX_SIZE_FOR_BUFFER 64512

using size_type = uint16_t;

class simple_packet {
  const size_type _reserved = sizeof(size_type);

  uint8_t* _buffer = nullptr;
  size_type _size = 0;

  size_type _write_offset = 0;
  size_type _read_offset = 0;

public:

  template<typename Type>
  bool write(Type value) {
	const auto value_size = static_cast<size_type>(sizeof(Type));

	if (_write_offset + value_size > _size - _reserved) {
	  return false;
	}

	*reinterpret_cast<Type*>(_buffer + _reserved + _write_offset) = value;
	_write_offset += value_size;

	return true;
  }

  template<typename Type>
  Type read() {
	const auto value_size = static_cast<size_type>(sizeof(Type));

	if (_read_offset + value_size > get_data_size()) {
	  return Type();
	}
	
	const auto result = *reinterpret_cast<Type*>(_buffer + _reserved + _read_offset);
	_read_offset += value_size;

	return result;
  }

  bool write_bytes(const uint8_t* source, size_type size);
  bool read_bytes(uint8_t* destination, size_type size);

  void set_data_size(size_type new_size);

  size_type get_allocated_size() const;
  size_type get_data_size() const;

  uint8_t* get_data_ptr() const;

  void reset(bool zero_memory = true);
  void print() const;

  int send(SOCKET socket, sockaddr_in address) const;
  int receive(SOCKET socket, sockaddr_in& client) const;

  simple_packet(size_type size);
  ~simple_packet();
};
