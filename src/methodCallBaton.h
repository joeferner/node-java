
#ifndef _methodcallbaton_h_
#define _methodcallbaton_h_

#include <v8.h>
#include <node.h>
#include <jni.h>
#include <list>
#include "utils.h"

class Java;
class JavaObject;

class MethodCallBaton {
public:
  MethodCallBaton(Java* java, JavaObject* obj, jobject method, std::list<jobject> args, v8::Handle<v8::Value> &callback);
  ~MethodCallBaton();

  static void EIO_MethodCall(eio_req* req);
  static int EIO_AfterMethodCall(eio_req* req);

private:
  Java* m_java;
  JavaObject* m_javaObject;
  v8::Persistent<v8::Value> m_callback;
  jobject m_method;
  std::list<jobject> m_args;
  jobject m_result;
  int m_resultType;
};

#endif
