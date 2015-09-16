
#ifndef _methodcallbaton_h_
#define _methodcallbaton_h_

#include "utils.h"
#include <v8.h>
#include <node.h>
#include <node_version.h>
#include <jni.h>
#include <list>

class Java;
class JavaObject;

class MethodCallBaton : public Nan::AsyncWorker {
public:
  MethodCallBaton(Java* java, jobject method, jarray args, v8::Local<v8::Value>& callback);
  virtual ~MethodCallBaton();

  void run();
  v8::Local<v8::Value> runSync();

protected:
  v8::Local<v8::Value> resultsToV8(JNIEnv *env);
  virtual void Execute();
  virtual void WorkComplete();
  virtual void ExecuteInternal(JNIEnv* env) = 0;
  static jmethodID getMethodInvokeMethodId(JNIEnv *env);

  Java* m_java;
  jthrowable m_error;
  std::string m_errorString;
  jarray m_args;
  jobject m_result;
  jobject m_method;

private:
  static jmethodID m_methodInvokeMethodId;
};

class InstanceMethodCallBaton : public MethodCallBaton {
public:
  InstanceMethodCallBaton(Java* java, JavaObject* obj, jobject method, jarray args, v8::Local<v8::Value>& callback);
  virtual ~InstanceMethodCallBaton();

protected:
  virtual void ExecuteInternal(JNIEnv* env);

  JavaObject* m_javaObject;
};

class NewInstanceBaton : public MethodCallBaton {
public:
  NewInstanceBaton(Java* java, jclass clazz, jobject method, jarray args, v8::Local<v8::Value>& callback);
  virtual ~NewInstanceBaton();

protected:
  virtual void ExecuteInternal(JNIEnv* env);

  jclass m_clazz;
};

class StaticMethodCallBaton : public MethodCallBaton {
public:
  StaticMethodCallBaton(Java* java, jclass clazz, jobject method, jarray args, v8::Local<v8::Value>& callback);
  virtual ~StaticMethodCallBaton();

protected:
  virtual void ExecuteInternal(JNIEnv* env);

  jclass m_clazz;
};

#endif
