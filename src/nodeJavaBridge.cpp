
#include "java.h"
#include "javaGlobalData.h"
#include "javaObject.h"

NAN_MODULE_INIT(init) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();
  if (isolate == nullptr) {
    return;
  }
  JavaGlobalData *data = new JavaGlobalData(isolate);
  Nan::SetIsolateData<JavaGlobalData>(isolate, data);

  Java::Init(target, data);
  JavaObject::Init(target);
}

NAN_MODULE_WORKER_ENABLED(nodejavabridge_bindings, init);

#ifdef WIN32
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) { return TRUE; }
#endif
