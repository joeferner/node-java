
#ifndef _javaobject_h_
#define _javaobject_h_

#include <v8.h>
#include <node.h>
#include <jni.h>
#include <list>
#include "methodCallBaton.h"

class Java;

class JavaObject : public node::ObjectWrap {
public:
  static void Init(v8::Handle<v8::Object> target);
  static v8::Local<v8::Object> New(Java* java, jobject obj);

  jobject getObject() { return m_obj; }
  
  void Ref() { node::ObjectWrap::Ref(); }
  void Unref() { node::ObjectWrap::Unref(); }

private:
  JavaObject(Java* java, jobject obj);
  ~JavaObject();
  static v8::Handle<v8::Value> methodCall(const v8::Arguments& args);
  static v8::Handle<v8::Value> methodCallSync(const v8::Arguments& args);

  static v8::Persistent<v8::FunctionTemplate> s_ct;
  Java* m_java;
  jobject m_obj;
  jclass m_class;
  std::list<jobject> m_methods;
};

#endif
