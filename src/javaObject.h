
#ifndef _javaobject_h_
#define _javaobject_h_

#include <v8.h>
#include <node.h>
#include <jni.h>
#include <list>
#include <map>
#include "methodCallBaton.h"

class Java;

class JavaObject : public node::ObjectWrap {
public:
  static void Init(v8::Handle<v8::Object> target);
  static v8::Local<v8::Object> New(Java* java, jobject obj);

  jobject getObject() { return m_obj; }
  jclass getClass() { return m_class; }

  void Ref() { node::ObjectWrap::Ref(); }
  void Unref() { node::ObjectWrap::Unref(); }

private:
  JavaObject(Java* java, jobject obj);
  ~JavaObject();
  static NAN_METHOD(methodCall);
  static NAN_METHOD(methodCallSync);
  static NAN_GETTER(fieldGetter);
  static NAN_SETTER(fieldSetter);

  static std::map<std::string, v8::Persistent<v8::FunctionTemplate>*> sFunctionTemplates;
  Java* m_java;
  jobject m_obj;
  jclass m_class;
};

#endif
