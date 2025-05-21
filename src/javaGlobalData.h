#ifndef _javaGlobalData_h_
#define _javaGlobalData_h_

#include <jni.h>
#include <nan.h>
#include <node.h>
#include <string>
#include <v8.h>
#include <queue>
#include "utils.h"

#ifdef WIN32
typedef long threadId;
#else
typedef pthread_t threadId;
#endif

class JavaGlobalData {
public:
  threadId v8ThreadId;
  bool isDefaultLoopRunning = false;

  std::queue<DynamicProxyJsCallData *> queue_dynamicProxyJsCallData;
  uv_mutex_t uvMutex_dynamicProxyJsCall;
  uv_async_t uvAsync_dynamicProxyJsCall;

  Nan::Persistent<v8::FunctionTemplate> ct;
  std::string nativeBindingLocation;

  explicit JavaGlobalData(v8::Isolate *isolate) {
    // Ensure this per-addon-instance data is deleted at environment cleanup.
    node::AddEnvironmentCleanupHook(isolate, DeleteInstance, this);
  }

  static void DeleteInstance(void *data) { delete static_cast<JavaGlobalData *>(data); }
};

#endif
