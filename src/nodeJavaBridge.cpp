
#include "java.h"

extern "C" {
  static void init(v8::Handle<v8::Object> target) {
    Java::Init(target);
  }

  NODE_MODULE(nodejavabridge_bindings, init);
}
