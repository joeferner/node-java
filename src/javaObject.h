
#ifndef _javaobject_h_
#define _javaobject_h_

#include <v8.h>
#include <node.h>
#include <jni.h>

class Java;

class JavaObject : node::ObjectWrap {
public:
  static void Init(v8::Handle<v8::Object> target);
  static v8::Local<v8::Object> New(Java* java, jobject obj);

private:
  JavaObject(Java* java, jobject obj);
  ~JavaObject();

  static v8::Persistent<v8::FunctionTemplate> s_ct;
  Java* m_java;
  jobject m_obj;
};

#endif
