#pragma once

#include <map>

class mh_wrap {
  std::map<const char*, void*> _originals;

  mh_wrap();
  ~mh_wrap();

public:

  void add(const char* name, void* target, void* detour);
  void add(const char* name, uintptr_t target, void* detour);

  void add(const char* name, const wchar_t* mod, const char* target, void* detour);

  template<typename Type>
  Type original(const char* name) {
	return reinterpret_cast<Type>(_originals[name]);
  }

  mh_wrap(const mh_wrap&) = delete;
  mh_wrap& operator=(const mh_wrap&) = delete;

  static mh_wrap& instance();
};
