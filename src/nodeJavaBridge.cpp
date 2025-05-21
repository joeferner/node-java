
#include "java.h"
#include "javaObject.h"

NAN_MODULE_INIT(init) {
  Java::Init(target);
  JavaObject::Init(target);
}

NAN_MODULE_WORKER_ENABLED(nodejavabridge_bindings, init);

#ifdef WIN32
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) { return TRUE; }
#endif
