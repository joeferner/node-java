
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

  jclass intClazz = env->FindClass("java/lang/Integer");
  //jclass clazzClazz = env->FindClass("java/lang/Class");
  jclass methodClazz = env->FindClass("java/lang/reflect/Method");
  jmethodID method_invoke = env->GetMethodID(methodClazz, "invoke", "(Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;");
  jmethodID method_getReturnType = env->GetMethodID(methodClazz, "getReturnType", "()Ljava/lang/Class;");
  //printf("a %d\n", (int)clazzClazz);
  //jmethodID clazz_isInstance = env->GetMethodID(clazzClazz, "isInstance", "(Ljava/lang/Object;)Z");

  printf("1\n");
  jclass returnType = (jclass)env->CallObjectMethod(self->m_method, method_getReturnType);

  jclass objectClazz = env->FindClass("java/lang/Object");
  jobjectArray parameters = env->NewObjectArray(0, objectClazz, NULL); // TODO: init parameters
  printf("2 %d %d %s %s\n", (int)intClazz, (int)returnType, javaObjectToString(env, intClazz).c_str(), javaObjectToString(env, intClazz).c_str());
  //if(env->CallBooleanMethod(intClazz, clazz_isInstance, returnType)) {
  if(returnType == intClazz) {
    printf("4\n");
    self->m_result.i = env->CallIntMethod(self->m_method, method_invoke, self->m_javaObject->m_obj, parameters);
    self->m_resultType = TYPE_INT;
  } else {
    printf("5\n");
    self->m_result.l = env->CallObjectMethod(self->m_method, method_invoke, self->m_javaObject->m_obj, parameters);
    self->m_resultType = TYPE_OBJECT;
  }
  printf("3\n");
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

