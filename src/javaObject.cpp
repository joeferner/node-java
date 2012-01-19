
#include "javaObject.h"
#include "java.h"

/*static*/ v8::Persistent<v8::FunctionTemplate> JavaObject::s_ct;

/*static*/ void JavaObject::Init(v8::Handle<v8::Object> target) {
  v8::HandleScope scope;

  v8::Local<v8::FunctionTemplate> t = v8::FunctionTemplate::New();
  s_ct = v8::Persistent<v8::FunctionTemplate>::New(t);
  s_ct->InstanceTemplate()->SetInternalFieldCount(1);
  s_ct->SetClassName(v8::String::NewSymbol("JavaObject"));

  target->Set(v8::String::NewSymbol("JavaObject"), s_ct->GetFunction());
}

/*static*/ v8::Local<v8::Object> JavaObject::New(Java *java, jobject obj) {
	v8::HandleScope scope;
  v8::Local<v8::Function> ctor = s_ct->GetFunction();
  v8::Local<v8::Object> javaObjectObj = ctor->NewInstance();
  JavaObject *self = new JavaObject(java, obj);
  self->Wrap(javaObjectObj);

  return scope.Close(javaObjectObj);
}

JavaObject::JavaObject(Java *java, jobject obj) {
  m_java = java;
  m_obj = obj;
}

JavaObject::~JavaObject() {

}
