
#include "java.h"
#include <string.h>

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
  this->m_env = createJVM();
}

Java::~Java() {

}

/*static*/ JNIEnv* Java::createJVM() {
  JavaVM* jvm;
  JNIEnv* env;
  JavaVMInitArgs args;
  JavaVMOption options[0];

  /* There is a new JNI_VERSION_1_4, but it doesn't add anything for the purposes of our example. */
  args.version = JNI_VERSION_1_2;
  args.nOptions = 0;
  args.options = options;
  args.ignoreUnrecognized = JNI_FALSE;

  JNI_CreateJavaVM(&jvm, (void **)&env, &args);

  return env;
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
}

/*static*/ int Java::EIO_AfterNewInstance(eio_req* req) {
  NewInstanceBaton* baton = static_cast<NewInstanceBaton*>(req->data);
  ev_unref(EV_DEFAULT_UC);

  baton->doCallback();

  delete baton;
  return 0;
}

NewInstanceBaton::NewInstanceBaton(Java* java, const char *className, v8::Handle<v8::Value> &callback) {
  this->m_java = java;
  this->m_className = strdup(className);
  this->m_callback = v8::Persistent<v8::Value>::New(callback);
  this->m_java->Ref();
}

NewInstanceBaton::~NewInstanceBaton() {
  this->m_callback.Dispose();
  this->m_java->Unref();
}

void NewInstanceBaton::doCallback() {
  v8::Handle<v8::Value> argv[2];
  argv[0] = v8::Undefined();
  argv[1] = v8::Undefined();

  if(this->m_callback->IsFunction()) {
    v8::Function::Cast(*this->m_callback)->Call(v8::Context::GetCurrent()->Global(), 2, argv);
  }
}
