#include "dllmain.hpp"

BOOL WINAPI DllMain(HMODULE mod, DWORD reason, LPVOID) {
  DisableThreadLibraryCalls(mod);
  if (reason == DLL_PROCESS_ATTACH) {
	storage::instance().initialize();
	events::instance().deploy();
  }
  return TRUE;
}
