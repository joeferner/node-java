
#include "methodCallBaton.h"
#include "java.h"
#include "javaObject.h"

MethodCallBaton::MethodCallBaton(Java* java, JavaObject* obj, jobject method, std::list<jobject> args, v8::Handle<v8::Value> &callback) {
  m_java = java;
  m_javaObject = obj;
  m_method = method;
  m_args = args;
  m_callback = v8::Persistent<v8::Value>::New(callback);
  m_javaObject->Ref();
}

MethodCallBaton::~MethodCallBaton() {
  m_callback.Dispose();
  m_javaObject->Unref();
}

/*static*/ void MethodCallBaton::EIO_MethodCall(eio_req* req) {
  MethodCallBaton* self = static_cast<MethodCallBaton*>(req->data);
  JNIEnv *env = javaAttachCurrentThread(self->m_java->getJvm());

  jclass methodClazz = env->FindClass("java/lang/reflect/Method");
  jmethodID method_invoke = env->GetMethodID(methodClazz, "invoke", "(Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;");
  jmethodID method_getReturnType = env->GetMethodID(methodClazz, "getReturnType", "()Ljava/lang/Class;");

  jclass returnType = (jclass)env->CallObjectMethod(self->m_method, method_getReturnType);

  jclass objectClazz = env->FindClass("java/lang/Object");
  jobjectArray parameters = env->NewObjectArray(0, objectClazz, NULL); // TODO: init parameters
  self->m_resultType = javaGetType(env, returnType);
  switch(self->m_resultType) {
    case TYPE_INT:
      printf("m_method: %s\n", javaObjectToString(env, self->m_method).c_str());
      printf("obj: %s\n", javaObjectToString(env, self->m_javaObject->m_obj).c_str());
      printf("parameters: %s\n", javaObjectToString(env, parameters).c_str());
      self->m_result.i = env->CallIntMethod(self->m_method, method_invoke, self->m_javaObject->m_obj, parameters);
      printf("%d\n", self->m_result.i);
      break;
    case TYPE_OBJECT:
      self->m_result.l = env->CallObjectMethod(self->m_method, method_invoke, self->m_javaObject->m_obj, parameters);
      break;
  }
  if(env->ExceptionCheck()) {
    env->ExceptionDescribe(); // TODO: handle error
    return;
  }

  javaDetachCurrentThread(self->m_java->getJvm());
}

/*static*/ int MethodCallBaton::EIO_AfterMethodCall(eio_req* req) {
  MethodCallBaton* self = static_cast<MethodCallBaton*>(req->data);

  if(self->m_callback->IsFunction()) {
    v8::Handle<v8::Value> argv[2];
    argv[0] = v8::Undefined();
    switch(self->m_resultType) {
      case TYPE_INT:
        argv[1] = v8::Integer::New(self->m_result.i);
        break;
      case TYPE_OBJECT:
        argv[1] = JavaObject::New(self->m_java, self->m_result.l);
        break;
    }
    v8::Function::Cast(*self->m_callback)->Call(v8::Context::GetCurrent()->Global(), 2, argv);
  }

  ev_unref(EV_DEFAULT_UC);
  delete self;
  return 0;
}

