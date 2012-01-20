
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
  MethodCallBaton(Java* java, std::list<jobject> args, v8::Handle<v8::Value> &callback);
  virtual ~MethodCallBaton();

  static void EIO_MethodCall(eio_req* req);
  static int EIO_AfterMethodCall(eio_req* req);
  void run();
  
protected:
  virtual void execute(JNIEnv *env) = 0;
  virtual void after(JNIEnv *env);
  
  Java* m_java;
  v8::Persistent<v8::Value> m_callback;
  std::list<jobject> m_args;
  jobject m_result;
  int m_resultType;
};

class InstanceMethodCallBaton : public MethodCallBaton {
public:
  InstanceMethodCallBaton(Java* java, JavaObject* obj, jobject method, std::list<jobject> args, v8::Handle<v8::Value> &callback);
  virtual ~InstanceMethodCallBaton();

protected:
  virtual void execute(JNIEnv *env);
  
private:
  jobject m_method;
  JavaObject* m_javaObject;
};

class NewInstanceBaton : public MethodCallBaton {
public:
  NewInstanceBaton(Java* java, jclass clazz, jmethodID method, std::list<jobject> args, v8::Handle<v8::Value> &callback);
  virtual ~NewInstanceBaton();
  
protected:
  virtual void execute(JNIEnv *env);
  
  jclass m_clazz;
  jmethodID m_method;
};

#endif
