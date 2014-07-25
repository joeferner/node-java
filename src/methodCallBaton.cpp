
#include "methodCallBaton.h"
#include "java.h"
#include "javaObject.h"
#include "javaScope.h"

NanCallback* toNanCallback(v8::Handle<v8::Value>& callback) {
  if(callback->IsFunction()) {
    return new NanCallback(callback.As<v8::Function>());
  }
  return NULL;
}

MethodCallBaton::MethodCallBaton(Java* java, jobject method, jarray args, v8::Handle<v8::Value>& callback) :
  NanAsyncWorker(toNanCallback(callback)),
  m_methodInvokeMethodId(0) {
  JNIEnv *env = java->getJavaEnv();
  m_java = java;
  m_args = (jarray)env->NewGlobalRef(args);
  m_method = env->NewGlobalRef(method);
  m_error = NULL;
  m_result = NULL;
}

MethodCallBaton::~MethodCallBaton() {
  JNIEnv *env = m_java->getJavaEnv();

  if(m_result) {
    env->DeleteGlobalRef(m_result);
  }
  if(m_error) {
    env->DeleteGlobalRef(m_error);
  }
  env->DeleteGlobalRef(m_args);
  env->DeleteGlobalRef(m_method);
}

jmethodID MethodCallBaton::getMethodInvokeMethodId() {
  if(m_methodInvokeMethodId == 0) {
    jclass methodClazz = m_env->FindClass("java/lang/reflect/Method");
    m_methodInvokeMethodId = m_env->GetMethodID(methodClazz, "invoke", "(Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;");
  }
  return m_methodInvokeMethodId;
}

void MethodCallBaton::run() {
  NanAsyncQueueWorker(this);
}

v8::Handle<v8::Value> MethodCallBaton::runSync() {
  m_env = m_java->getJavaEnv();
  ExecuteInternal();
  return resultsToV8(m_env);
}

void MethodCallBaton::Execute() {
  m_env = javaGetEnv(this->m_java->getJvm(), this->m_java->getClassLoader());
  ExecuteInternal();
}

void MethodCallBaton::WorkComplete() {
  NanScope();

  v8::Handle<v8::Value> result = resultsToV8(m_env);
  if (result->IsNativeError()) {
    v8::Handle<v8::Value> argv[] = {
      result
    };
    callback->Call(1, argv);
  } else {
    v8::Handle<v8::Value> argv[] = {
      NanUndefined(),
      result
    };
    callback->Call(2, argv);
  }
  delete callback;
  callback = NULL;
}

v8::Handle<v8::Value> MethodCallBaton::resultsToV8(JNIEnv *env) {
  NanEscapableScope();

  if(m_error) {
    jthrowable cause = m_error;

    // if we've caught an InvocationTargetException exception,
    // let's grab the cause. users don't necessarily know that
    // we're invoking the methods through reflection
    jclass invocationExceptionClazz = env->FindClass("java/lang/reflect/InvocationTargetException");
    if (env->IsInstanceOf(m_error, invocationExceptionClazz)) {
      jclass throwableClazz = env->FindClass("java/lang/Throwable");
      jmethodID throwable_getCause = env->GetMethodID(throwableClazz, "getCause", "()Ljava/lang/Throwable;");
      cause = (jthrowable)env->CallObjectMethod(m_error, throwable_getCause);
      checkJavaException(env);
    }

    v8::Handle<v8::Value> err = javaExceptionToV8(m_java, env, cause, m_errorString);
    return NanEscapeScope(err);
  }

  return NanEscapeScope(javaToV8(m_java, env, m_result));
}

void NewInstanceBaton::ExecuteInternal() {
  jclass constructorClazz = m_env->FindClass("java/lang/reflect/Constructor");
  jmethodID constructor_newInstance = m_env->GetMethodID(constructorClazz, "newInstance", "([Ljava/lang/Object;)Ljava/lang/Object;");

  //printf("invoke: %s\n", javaMethodCallToString(m_env, m_method, constructor_newInstance, m_args).c_str());

  jobject result = m_env->CallObjectMethod(m_method, constructor_newInstance, m_args);
  if(m_env->ExceptionCheck()) {
    jthrowable ex = m_env->ExceptionOccurred();
    m_env->ExceptionClear();
    m_error = (jthrowable)m_env->NewGlobalRef(ex);
    m_errorString = "Error creating class";
    return;
  }

  m_result = m_env->NewGlobalRef(result);
}

void StaticMethodCallBaton::ExecuteInternal() {
  jmethodID method_invoke = getMethodInvokeMethodId();

  /*
  printf("calling %s\n", javaObjectToString(m_env, m_method).c_str());
  printf("arguments\n");
  for(int i=0; i<m_env->GetArrayLength(m_args); i++) {
    printf("  %s\n", javaObjectToString(m_env, m_env->GetObjectArrayElement((jobjectArray)m_args, i)).c_str());
  }
  */

  jobject result = m_env->CallObjectMethod(m_method, method_invoke, NULL, m_args);

  if(m_env->ExceptionCheck()) {
    jthrowable ex = m_env->ExceptionOccurred();
    m_env->ExceptionClear();
    m_error = (jthrowable)m_env->NewGlobalRef(ex);
    m_errorString = "Error running static method";
    return;
  }

  m_result = m_env->NewGlobalRef(result);
}

void InstanceMethodCallBaton::ExecuteInternal() {
  jmethodID method_invoke = getMethodInvokeMethodId();

  /*
  printf("calling %s\n", javaObjectToString(m_env, m_method).c_str());
  printf("arguments\n");
  for(int i=0; i<m_env->GetArrayLength(m_args); i++) {
    printf("  %s\n", javaObjectToString(m_env, m_env->GetObjectArrayElement((jobjectArray)m_args, i)).c_str());
  }
  */

  jobject result = m_env->CallObjectMethod(m_method, method_invoke, m_javaObject->getObject(), m_args);

  if(m_env->ExceptionCheck()) {
    jthrowable ex = m_env->ExceptionOccurred();
    m_env->ExceptionClear();
    m_error = (jthrowable)m_env->NewGlobalRef(ex);
    m_errorString = "Error running instance method";
    return;
  }

  if(result == NULL) {
    m_result = NULL;
  } else {
    m_result = m_env->NewGlobalRef(result);
  }
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
