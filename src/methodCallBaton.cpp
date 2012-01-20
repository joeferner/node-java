
#include "methodCallBaton.h"
#include "java.h"
#include "javaObject.h"

MethodCallBaton::MethodCallBaton(Java* java, JavaObject* obj, jobject method, std::list<jobject> args, v8::Handle<v8::Value> &callback) {
  JNIEnv *env = java->getJavaEnv();
  
  m_java = java;
  m_javaObject = obj;
  m_method = env->NewGlobalRef(method);
  for(std::list<jobject>::iterator it = args.begin(); it != args.end(); it++) {
    m_args.push_back(env->NewGlobalRef(*it));
  }
  m_callback = v8::Persistent<v8::Value>::New(callback);
  m_javaObject->Ref();
}

MethodCallBaton::~MethodCallBaton() {
  JNIEnv *env = m_java->getJavaEnv();
  env->DeleteGlobalRef(m_method);
  for(std::list<jobject>::iterator it = m_args.begin(); it != m_args.end(); it++) {
    env->DeleteGlobalRef(*it);
  }
  
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
  jobject result = env->CallObjectMethod(self->m_method, method_invoke, self->m_javaObject->m_obj, parameters);
  self->m_result = env->NewGlobalRef(result);
  if(env->ExceptionCheck()) {
    env->ExceptionDescribe(); // TODO: handle error
    return;
  }

  javaDetachCurrentThread(self->m_java->getJvm());
}

/*static*/ int MethodCallBaton::EIO_AfterMethodCall(eio_req* req) {
  MethodCallBaton* self = static_cast<MethodCallBaton*>(req->data);
  JNIEnv *env = self->m_java->getJavaEnv();
  
  if(self->m_callback->IsFunction()) {
    v8::Handle<v8::Value> argv[2];
    argv[0] = v8::Undefined();
    switch(self->m_resultType) {
      case TYPE_INT:
        {
          jclass integerClazz = env->FindClass("java/lang/Integer");
          jmethodID integer_intValue = env->GetMethodID(integerClazz, "intValue", "()I");
          int result = env->CallIntMethod(self->m_result, integer_intValue);
          argv[1] = v8::Integer::New(result);
        }
        break;
      case TYPE_OBJECT:
        argv[1] = JavaObject::New(self->m_java, self->m_result);
        break;
      case TYPE_STRING:
        argv[1] = v8::String::New(javaObjectToString(env, self->m_result).c_str());
        break;
    }
    v8::Function::Cast(*self->m_callback)->Call(v8::Context::GetCurrent()->Global(), 2, argv);
  }

  env->DeleteGlobalRef(self->m_result);
  ev_unref(EV_DEFAULT_UC);
  delete self;
  return 0;
}

