#include "mh_wrap.hpp"

#include <MinHook/MinHook.h>

mh_wrap::mh_wrap() {
  MH_Initialize();
}

mh_wrap::~mh_wrap() {
  MH_Uninitialize();
}

void mh_wrap::add(const char* name, void* target, void* detour) {
  _originals[name] = nullptr;
  MH_CreateHook(target, detour, reinterpret_cast<void**>(&_originals[name]));
  MH_EnableHook(MH_ALL_HOOKS);
}

void mh_wrap::add(const char* name, uintptr_t target, void* detour) {
  _originals[name] = nullptr;
  MH_CreateHook(reinterpret_cast<void*>(target), detour, reinterpret_cast<void**>(&_originals[name]));
  MH_EnableHook(MH_ALL_HOOKS);
}

void mh_wrap::add(const char* name, const wchar_t* mod, const char* target, void* detour) {
  _originals[name] = nullptr;
  MH_CreateHookApi(mod, target, detour, reinterpret_cast<void**>(&_originals[name]));
  MH_EnableHook(MH_ALL_HOOKS);
}

mh_wrap& mh_wrap::instance() {
  static mh_wrap inst;
  return inst;
}
