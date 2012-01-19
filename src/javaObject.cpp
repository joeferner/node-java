
#include "javaObject.h"
#include "java.h"
#include "utils.h"

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

  JNIEnv *env = self->m_java->getJavaEnv();

  self->m_methods = javaReflectionGetDeclaredMethods(env, self->m_class);
  jclass methodClazz = env->FindClass("java/lang/reflect/Method");
  jmethodID method_getNameMethod = env->GetMethodID(methodClazz, "getName", "()Ljava/lang/String;");
  for(std::list<jobject>::iterator it = self->m_methods.begin(); it != self->m_methods.end(); it++) {
    v8::Handle<v8::String> methodName = v8::String::New(javaToString(env, (jstring)env->CallObjectMethod(*it, method_getNameMethod)).c_str());
    v8::Local<v8::FunctionTemplate> methodCallTemplate = v8::FunctionTemplate::New(methodCall, methodName);
    javaObjectObj->Set(methodName, methodCallTemplate->GetFunction());
  }

  return scope.Close(javaObjectObj);
}

JavaObject::JavaObject(Java *java, jobject obj) {
  m_java = java;
  m_obj = obj;
  m_class = java->getJavaEnv()->GetObjectClass(obj);
}

JavaObject::~JavaObject() {

}

/*static*/ v8::Handle<v8::Value> JavaObject::methodCall(const v8::Arguments& args) {
  v8::HandleScope scope;
  JavaObject* self = node::ObjectWrap::Unwrap<JavaObject>(args.This());
  JNIEnv *env = self->m_java->getJavaEnv();

  v8::String::AsciiValue methodName(args.Data());

  int argsLength = args.Length();

  // argument - callback
  v8::Handle<v8::Value> callback;
  if(args[args.Length()-1]->IsFunction()) {
    callback = args[argsLength-1];
    argsLength--;
  } else {
    callback = v8::Null();
  }

  std::list<jobject> methodArgs; // TODO: build args

  jobject method = javaFindBestMatchingMethod(env, self->m_methods, *methodName, methodArgs);
  if(method == NULL) {
    return v8::Undefined();
  }

  // run
  MethodCallBaton* baton = new MethodCallBaton(self->m_java, self, method, methodArgs, callback);
  eio_custom(MethodCallBaton::EIO_MethodCall, EIO_PRI_DEFAULT, MethodCallBaton::EIO_AfterMethodCall, baton);
  ev_ref(EV_DEFAULT_UC);

  return v8::Undefined();
}
