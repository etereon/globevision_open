#include "simple_packet.hpp"

void simple_packet::set_data_size(size_type new_size) {
  *reinterpret_cast<size_type*>(_buffer) = new_size;
}

bool simple_packet::write_bytes(const uint8_t* source, size_type size) {
  if (_write_offset + size > _size - _reserved) {
	return false;
  }

  memcpy(_buffer + _reserved + _write_offset, source, size);
  _write_offset += size;

  return true;
}

bool simple_packet::read_bytes(uint8_t* destination, size_type size) {
  if (_read_offset + size > get_data_size()) {
	return false;
  }

  memcpy(destination, _buffer + _reserved + _read_offset, size);
  _read_offset += size;

  return true;
}

size_type simple_packet::get_allocated_size() const {
  return _size;
}

size_type simple_packet::get_data_size() const {
  return *reinterpret_cast<size_type*>(_buffer);
}

uint8_t* simple_packet::get_data_ptr() const {
  return _buffer + _reserved;
}

void simple_packet::reset(bool zero_memory) {
  if (zero_memory)
	memset(_buffer, 0, _size);

  _write_offset = 0;
  _read_offset = 0;
  set_data_size(_size - _reserved);
}

void simple_packet::print() const {
  for (size_t i = 1; i <= get_data_size(); ++i) {
	printf("%02X ", _buffer[i]);
  }
  printf("| SIZE: %d\n", get_data_size());
}

int simple_packet::send(SOCKET socket, sockaddr_in address) const {
  int from_len = sizeof(address);
  return sendto(socket, reinterpret_cast<const char*>(_buffer), get_data_size() + _reserved, 0, reinterpret_cast<sockaddr*>(&address), from_len);
}

int simple_packet::receive(SOCKET socket, sockaddr_in& client) const {
  int len = sizeof(client);
  return recvfrom(socket, reinterpret_cast<char*>(_buffer), _size, 0, reinterpret_cast<sockaddr*>(&client), &len);
}

simple_packet::simple_packet(size_type size) {
  _size = _reserved + size;
  _buffer = new uint8_t[_size];
  reset();
}

simple_packet::~simple_packet() {
  delete[] _buffer;
}
