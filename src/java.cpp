
#include "java.h"
#include <string.h>
#include <algorithm>
#include "javaObject.h"

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

  // argument - className
  if(args.Length() < 1 || !args[0]->IsString()) {
    return ThrowException(v8::Exception::TypeError(v8::String::New("Argument 0 must be a string")));
  }
  v8::Local<v8::String> classNameObj = v8::Local<v8::String>::Cast(args[0]);
  v8::String::AsciiValue className(classNameObj);

  // argument - callback
  v8::Handle<v8::Value> callback;
  if(args[args.Length()-1]->IsFunction()) {
    callback = args[args.Length()-1];
  } else {
    callback = v8::Null();
  }

  // run
  NewInstanceBaton* baton = new NewInstanceBaton(self, *className, callback);
  eio_custom(EIO_NewInstance, EIO_PRI_DEFAULT, EIO_AfterNewInstance, baton);
  ev_ref(EV_DEFAULT_UC);

  return v8::Undefined();
}

/*static*/ void Java::EIO_NewInstance(eio_req* req) {
  NewInstanceBaton* baton = static_cast<NewInstanceBaton*>(req->data);
  JNIEnv *env = javaAttachCurrentThread(baton->m_java->getJvm());
  baton->run(env);
  javaDetachCurrentThread(baton->m_java->getJvm());
}

/*static*/ int Java::EIO_AfterNewInstance(eio_req* req) {
  NewInstanceBaton* baton = static_cast<NewInstanceBaton*>(req->data);
  ev_unref(EV_DEFAULT_UC);
  baton->doCallback();
  delete baton;
  return 0;
}

NewInstanceBaton::NewInstanceBaton(Java* java, const char *className, v8::Handle<v8::Value> &callback) {
  m_java = java;
  m_className = className;
  std::replace(m_className.begin(), m_className.end(), '.', '/');
  m_callback = v8::Persistent<v8::Value>::New(callback);
  m_java->Ref();
}

NewInstanceBaton::~NewInstanceBaton() {
  m_callback.Dispose();
  m_java->Unref();
}

void NewInstanceBaton::run(JNIEnv *env) {
  jclass clazz = env->FindClass(m_className.c_str());
  if(env->ExceptionCheck()) {
    env->ExceptionDescribe(); // TODO: handle error
    return;
  }

  jmethodID method = env->GetMethodID(clazz, "<init>", "()V"); // TODO: add arguments
  if(env->ExceptionCheck()) {
    env->ExceptionDescribe(); // TODO: handle error
    return;
  }

  jobject result = env->NewObject(clazz, method);
  if(env->ExceptionCheck()) {
    env->ExceptionDescribe(); // TODO: handle error
    return;
  }
  
  m_result = env->NewGlobalRef(result);
}

void NewInstanceBaton::doCallback() {
  if(m_callback->IsFunction()) {
    v8::Handle<v8::Value> argv[2];
    argv[0] = v8::Undefined();
    argv[1] = JavaObject::New(m_java, m_result);
    v8::Function::Cast(*this->m_callback)->Call(v8::Context::GetCurrent()->Global(), 2, argv);
  }
  m_java->getJavaEnv()->DeleteGlobalRef(m_result);
}
