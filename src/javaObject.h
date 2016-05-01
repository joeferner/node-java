
#ifndef _javaobject_h_
#define _javaobject_h_

#include <v8.h>
#include <node.h>
#include <jni.h>
#include <list>
#include <map>
#include "methodCallBaton.h"

class Java;

class JavaObject : public Nan::ObjectWrap {
public:
  static void Init(v8::Handle<v8::Object> target);
  static v8::Local<v8::Object> New(Java* java, jobject obj);
  static v8::Local<v8::Object> NewProxy(Java* java, jobject obj, DynamicProxyData* dynamicProxyData);

  jobject getObject() { return m_obj; }
  jclass getClass() { return m_class; }

  void Ref() { Nan::ObjectWrap::Ref(); }
  void Unref() { Nan::ObjectWrap::Unref(); }

protected:
  JavaObject(Java* java, jobject obj);
  ~JavaObject();

private:
  static NAN_METHOD(methodCall);
  static NAN_METHOD(methodCallSync);
  static NAN_METHOD(methodCallPromise);
  static NAN_GETTER(fieldGetter);
  static NAN_SETTER(fieldSetter);
  static NAN_INDEX_GETTER(indexGetter);

  static std::map<std::string, Nan::Persistent<v8::FunctionTemplate>*> sFunctionTemplates;
  Java* m_java;
  jobject m_obj;
  jclass m_class;
};

class JavaProxyObject : public JavaObject {
public:
  static void init();
  static v8::Local<v8::Object> New(Java* java, jobject obj, DynamicProxyData* dynamicProxyData);

private:
  JavaProxyObject(Java* java, jobject obj, DynamicProxyData* dynamicProxyData);
  ~JavaProxyObject();
  static NAN_METHOD(doUnref);
  static NAN_GETTER(invocationHandlerGetter);

  static Nan::Persistent<v8::FunctionTemplate> s_proxyCt;
  DynamicProxyData* m_dynamicProxyData;
};

#endif

