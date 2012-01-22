
#include "methodCallBaton.h"
#include "java.h"
#include "javaObject.h"

MethodCallBaton::MethodCallBaton(Java* java, jobject method, jarray args, v8::Handle<v8::Value>& callback) {
  JNIEnv *env = java->getJavaEnv();

  m_java = java;
  m_args = (jarray)env->NewGlobalRef(args);
  m_callback = v8::Persistent<v8::Value>::New(callback);
  m_method = env->NewGlobalRef(method);
  m_error = NULL;
  m_result = NULL;
}

MethodCallBaton::~MethodCallBaton() {
  JNIEnv *env = m_java->getJavaEnv();
  env->DeleteGlobalRef(m_args);
  env->DeleteGlobalRef(m_method);
  m_callback.Dispose();
}

void MethodCallBaton::run() {
  eio_custom(MethodCallBaton::EIO_MethodCall, EIO_PRI_DEFAULT, MethodCallBaton::EIO_AfterMethodCall, this);
  ev_ref(EV_DEFAULT_UC);
}

v8::Handle<v8::Value> MethodCallBaton::runSync() {
  JNIEnv *env = m_java->getJavaEnv();
  execute(env);
  return resultsToV8(env);
}

/*static*/ void MethodCallBaton::EIO_MethodCall(eio_req* req) {
  MethodCallBaton* self = static_cast<MethodCallBaton*>(req->data);
  JNIEnv *env = javaAttachCurrentThread(self->m_java->getJvm());
  self->execute(env);
  javaDetachCurrentThread(self->m_java->getJvm());
}

/*static*/ int MethodCallBaton::EIO_AfterMethodCall(eio_req* req) {
  MethodCallBaton* self = static_cast<MethodCallBaton*>(req->data);
  JNIEnv *env = self->m_java->getJavaEnv();
  self->after(env);
  ev_unref(EV_DEFAULT_UC);
  delete self;
  return 0;
}

void MethodCallBaton::after(JNIEnv *env) {
  if(m_callback->IsFunction()) {
    v8::Handle<v8::Value> result = resultsToV8(env);
    v8::Handle<v8::Value> argv[2];
    if(result->IsNativeError()) {
      argv[0] = result;
      argv[1] = v8::Undefined();
    } else {
      argv[0] = v8::Undefined();
      argv[1] = result;
    }
    v8::Function::Cast(*m_callback)->Call(v8::Context::GetCurrent()->Global(), 2, argv);
  }

  if(m_result) {
    env->DeleteGlobalRef(m_result);
  }
}

v8::Handle<v8::Value> MethodCallBaton::resultsToV8(JNIEnv *env) {
  v8::HandleScope scope;

  if(m_error) {
    v8::Handle<v8::Value> err = javaExceptionToV8(env, m_error, m_errorString);
    env->DeleteGlobalRef(m_error);
    return scope.Close(err);
  }

  return scope.Close(javaToV8(m_java, env, m_resultType, m_result));
}

void NewInstanceBaton::execute(JNIEnv *env) {
  jclass constructorClazz = env->FindClass("java/lang/reflect/Constructor");
  jmethodID constructor_newInstance = env->GetMethodID(constructorClazz, "newInstance", "([Ljava/lang/Object;)Ljava/lang/Object;");

  //printf("invoke: %s\n", javaObjectToString(env, m_method).c_str());

  jobject result = env->CallObjectMethod(m_method, constructor_newInstance, m_args);
  jthrowable err = env->ExceptionOccurred();
  if(err) {
    m_error = (jthrowable)env->NewGlobalRef(err);
    m_errorString = "Error creating class";
    env->ExceptionClear();
    return;
  }

  m_resultType = TYPE_OBJECT;
  m_result = env->NewGlobalRef(result);
}

void StaticMethodCallBaton::execute(JNIEnv *env) {
  jclass methodClazz = env->FindClass("java/lang/reflect/Method");
  jmethodID method_invoke = env->GetMethodID(methodClazz, "invoke", "(Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;");
  jmethodID method_getReturnType = env->GetMethodID(methodClazz, "getReturnType", "()Ljava/lang/Class;");

  jclass returnType = (jclass)env->CallObjectMethod(m_method, method_getReturnType);

  /*
  printf("calling %s\n", javaObjectToString(env, m_method).c_str());
  printf("arguments\n");
  for(int i=0; i<env->GetArrayLength(m_args); i++) {
    printf("  %s\n", javaObjectToString(env, env->GetObjectArrayElement((jobjectArray)m_args, i)).c_str());
  }
  */

  m_resultType = javaGetType(env, returnType);
  jobject result = env->CallObjectMethod(m_method, method_invoke, NULL, m_args);

  jthrowable err = env->ExceptionOccurred();
  if(err) {
    m_error = (jthrowable)env->NewGlobalRef(err);
    m_errorString = "Error running static method";
    env->ExceptionClear();
    return;
  }

  m_result = env->NewGlobalRef(result);
}

void InstanceMethodCallBaton::execute(JNIEnv *env) {
  jclass methodClazz = env->FindClass("java/lang/reflect/Method");
  jmethodID method_invoke = env->GetMethodID(methodClazz, "invoke", "(Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;");
  jmethodID method_getReturnType = env->GetMethodID(methodClazz, "getReturnType", "()Ljava/lang/Class;");

  jclass returnType = (jclass)env->CallObjectMethod(m_method, method_getReturnType);

  /*
  printf("calling %s\n", javaObjectToString(env, m_method).c_str());
  printf("arguments\n");
  for(int i=0; i<env->GetArrayLength(m_args); i++) {
    printf("  %s\n", javaObjectToString(env, env->GetObjectArrayElement((jobjectArray)m_args, i)).c_str());
  }
  */

  m_resultType = javaGetType(env, returnType);
  jobject result = env->CallObjectMethod(m_method, method_invoke, m_javaObject->getObject(), m_args);

  jthrowable err = env->ExceptionOccurred();
  if(err) {
    m_error = (jthrowable)env->NewGlobalRef(err);
    m_errorString = "Error running instance method";
    env->ExceptionClear();
    return;
  }

  m_result = env->NewGlobalRef(result);
}

NewInstanceBaton::NewInstanceBaton(
  Java* java,
  jclass clazz,
  jobject method,
  jarray args,
  v8::Handle<v8::Value>& callback) : MethodCallBaton(java, method, args, callback) {
  JNIEnv *env = m_java->getJavaEnv();
  m_clazz = (jclass)env->NewGlobalRef(clazz);
}

NewInstanceBaton::~NewInstanceBaton() {
  JNIEnv *env = m_java->getJavaEnv();
  env->DeleteGlobalRef(m_clazz);
}

StaticMethodCallBaton::StaticMethodCallBaton(
  Java* java,
  jclass clazz,
  jobject method,
  jarray args,
  v8::Handle<v8::Value>& callback) : MethodCallBaton(java, method, args, callback) {
  JNIEnv *env = m_java->getJavaEnv();
  m_clazz = (jclass)env->NewGlobalRef(clazz);
}

StaticMethodCallBaton::~StaticMethodCallBaton() {
  JNIEnv *env = m_java->getJavaEnv();
  env->DeleteGlobalRef(m_clazz);
}

InstanceMethodCallBaton::InstanceMethodCallBaton(
  Java* java,
  JavaObject* obj,
  jobject method,
  jarray args,
  v8::Handle<v8::Value>& callback) : MethodCallBaton(java, method, args, callback) {
  m_javaObject = obj;
  m_javaObject->Ref();
}

InstanceMethodCallBaton::~InstanceMethodCallBaton() {
  m_javaObject->Unref();
}
