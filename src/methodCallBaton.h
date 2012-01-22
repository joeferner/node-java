
#ifndef _methodcallbaton_h_
#define _methodcallbaton_h_

#include "utils.h"
#include <v8.h>
#include <node.h>
#include <jni.h>
#include <list>

class Java;
class JavaObject;

class MethodCallBaton {
public:
  MethodCallBaton(Java* java, jobject method, jarray args, v8::Handle<v8::Value>& callback);
  virtual ~MethodCallBaton();

  static void EIO_MethodCall(uv_work_t* req);
  static void EIO_AfterMethodCall(uv_work_t* req);
  void run();
  v8::Handle<v8::Value> runSync();

protected:
  virtual void execute(JNIEnv *env) = 0;
  virtual void after(JNIEnv *env);
  v8::Handle<v8::Value> resultsToV8(JNIEnv *env);

  Java* m_java;
  v8::Persistent<v8::Value> m_callback;
  jthrowable m_error;
  std::string m_errorString;
  jarray m_args;
  jobject m_result;
  jobject m_method;
  jvalueType m_resultType;
};

class InstanceMethodCallBaton : public MethodCallBaton {
public:
  InstanceMethodCallBaton(Java* java, JavaObject* obj, jobject method, jarray args, v8::Handle<v8::Value>& callback);
  virtual ~InstanceMethodCallBaton();

protected:
  virtual void execute(JNIEnv *env);

  JavaObject* m_javaObject;
};

class NewInstanceBaton : public MethodCallBaton {
public:
  NewInstanceBaton(Java* java, jclass clazz, jobject method, jarray args, v8::Handle<v8::Value>& callback);
  virtual ~NewInstanceBaton();

protected:
  virtual void execute(JNIEnv *env);

  jclass m_clazz;
};

class StaticMethodCallBaton : public MethodCallBaton {
public:
  StaticMethodCallBaton(Java* java, jclass clazz, jobject method, jarray args, v8::Handle<v8::Value>& callback);
  virtual ~StaticMethodCallBaton();

protected:
  virtual void execute(JNIEnv *env);

  jclass m_clazz;
};

#endif
