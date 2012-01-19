
#ifndef _node_java_h_
#define _node_java_h_

#include <v8.h>
#include <node.h>
#include <jni.h>

class NewInstanceBaton;

class Java : public node::ObjectWrap {
public:
  static void Init(v8::Handle<v8::Object> target);

  friend class NewInstanceBaton;

private:
  Java();
  ~Java();
  static JNIEnv* createJVM();

  static v8::Handle<v8::Value> New(const v8::Arguments& args);
  static v8::Handle<v8::Value> newInstance(const v8::Arguments& args);
  static void EIO_NewInstance(eio_req* req);
  static int EIO_AfterNewInstance(eio_req* req);

  static v8::Persistent<v8::FunctionTemplate> s_ct;
  JNIEnv* m_env;
};

class NewInstanceBaton {
public:
  NewInstanceBaton(Java* java, const char *className, v8::Handle<v8::Value> &callback);
  ~NewInstanceBaton();
  void doCallback();

private:
  Java* m_java;
  v8::Persistent<v8::Value> m_callback;
  char* m_className;
};

#endif
