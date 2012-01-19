
#ifndef _javaobject_h_
#define _javaobject_h_

#include <v8.h>
#include <node.h>
#include <jni.h>
#include <list>
#include "methodCallBaton.h"

class Java;
class MethodCallBaton;

class JavaObject : node::ObjectWrap {
public:
  static void Init(v8::Handle<v8::Object> target);
  static v8::Local<v8::Object> New(Java* java, jobject obj);

  friend class MethodCallBaton;

private:
  JavaObject(Java* java, jobject obj);
  ~JavaObject();
  static v8::Handle<v8::Value> methodCall(const v8::Arguments& args);

  static v8::Persistent<v8::FunctionTemplate> s_ct;
  Java* m_java;
  jobject m_obj;
  jclass m_class;
  std::list<jobject> m_methods;
};

#endif
