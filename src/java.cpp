
#include "java.h"
#include <string.h>
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
  NODE_SET_PROTOTYPE_METHOD(s_ct, "newInstanceSync", newInstanceSync);

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

  std::list<jobject> methodArgs; // TODO: build args
  jclass clazz = javaFindClass(env, className);
  std::list<jobject> constructors = javaReflectionGetConstructors(env, clazz);  
  jobject method = javaFindBestMatchingConstructor(env, constructors, methodArgs);
  
  // run
  NewInstanceBaton* baton = new NewInstanceBaton(self, clazz, method, methodArgs, callback);
  baton->run();

  return v8::Undefined();
}

/*static*/ v8::Handle<v8::Value> Java::newInstanceSync(const v8::Arguments& args) {
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

  std::list<jobject> methodArgs; // TODO: build args
  jclass clazz = javaFindClass(env, className);
  std::list<jobject> constructors = javaReflectionGetConstructors(env, clazz);  
  jobject method = javaFindBestMatchingConstructor(env, constructors, methodArgs);
  
  // run
  v8::Handle<v8::Value> callback = v8::Object::New();
  NewInstanceBaton* baton = new NewInstanceBaton(self, clazz, method, methodArgs, callback);
  v8::Handle<v8::Value> result = baton->runSync();
  delete baton;
  return scope.Close(result);
}
