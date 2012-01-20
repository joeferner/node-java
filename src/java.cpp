
#include "java.h"
#include <string.h>
#include <algorithm>
#include "javaObject.h"
#include "methodCallBaton.h"

/*static*/ v8::Persistent<v8::FunctionTemplate> Java::s_ct;

/*static*/ void Java::Init(v8::Handle<v8::Object> target) {
  v8::HandleScope scope;

  v8::Local<v8::FunctionTemplate> t = v8::FunctionTemplate::New(New);
  s_ct = v8::Persistent<v8::FunctionTemplate>::New(t);
  s_ct->InstanceTemplate()->SetInternalFieldCount(1);
  s_ct->SetClassName(v8::String::NewSymbol("Java"));

  NODE_SET_PROTOTYPE_METHOD(s_ct, "newInstance", newInstance);

  target->Set(v8::String::NewSymbol("Java"), s_ct->GetFunction());
}

/*static*/ v8::Handle<v8::Value> Java::New(const v8::Arguments& args) {
  v8::HandleScope scope;

  Java *self = new Java();
  self->Wrap(args.This());
  return args.This();
}

Java::Java() {
  this->m_jvm = NULL;
  this->m_env = NULL;
  createJVM(&this->m_jvm, &this->m_env);
}

Java::~Java() {

}

/*static*/ void Java::createJVM(JavaVM** jvm, JNIEnv** env) {
  JavaVM* jvmTemp;
  JavaVMInitArgs args;

  args.version = JNI_VERSION_1_4;
  JNI_GetDefaultJavaVMInitArgs(&args);
  JNI_CreateJavaVM(&jvmTemp, (void **)env, &args);
  *jvm = jvmTemp;
}

/*static*/ v8::Handle<v8::Value> Java::newInstance(const v8::Arguments& args) {
  v8::HandleScope scope;
  Java* self = node::ObjectWrap::Unwrap<Java>(args.This());
  JNIEnv* env = self->getJavaEnv();

  // argument - className
  if(args.Length() < 1 || !args[0]->IsString()) {
    return ThrowException(v8::Exception::TypeError(v8::String::New("Argument 0 must be a string")));
  }
  v8::Local<v8::String> classNameObj = v8::Local<v8::String>::Cast(args[0]);
  v8::String::AsciiValue classNameVal(classNameObj);
  std::string className = *classNameVal;

  // argument - callback
  v8::Handle<v8::Value> callback;
  if(args[args.Length()-1]->IsFunction()) {
    callback = args[args.Length()-1];
  } else {
    callback = v8::Null();
  }

  std::replace(className.begin(), className.end(), '.', '/');

  jclass clazz = env->FindClass(className.c_str());
  if(env->ExceptionCheck()) {
    env->ExceptionDescribe(); // TODO: handle error
    return v8::Undefined();
  }

  jmethodID method = env->GetMethodID(clazz, "<init>", "()V"); // TODO: add arguments
  if(env->ExceptionCheck()) {
    env->ExceptionDescribe(); // TODO: handle error
    return v8::Undefined();
  }

  std::list<jobject> methodArgs; // TODO: build args
  
  // run
  NewInstanceBaton* baton = new NewInstanceBaton(self, clazz, method, methodArgs, callback);
  eio_custom(MethodCallBaton::EIO_MethodCall, EIO_PRI_DEFAULT, MethodCallBaton::EIO_AfterMethodCall, baton);
  ev_ref(EV_DEFAULT_UC);

  return v8::Undefined();
}

