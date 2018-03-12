
#include "methodCallBaton.h"
#include "java.h"
#include "javaObject.h"
#include "javaScope.h"

jmethodID MethodCallBaton::m_methodInvokeMethodId = 0;

Nan::Callback* toNanCallback(v8::Local<v8::Value>& callback) {
  if(callback->IsFunction()) {
    return new Nan::Callback(callback.As<v8::Function>());
  }
  return NULL;
}

MethodCallBaton::MethodCallBaton(Java* java, jobject method, jarray args, v8::Local<v8::Value>& callback) :
  Nan::AsyncWorker(toNanCallback(callback)) {
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
    m_result = NULL;
  }

  if(m_error) {
    env->DeleteGlobalRef(m_error);
    m_error = NULL;
  }

  env->DeleteGlobalRef(m_args);
  m_args = NULL;

  env->DeleteGlobalRef(m_method);
  m_method = NULL;
}

jmethodID MethodCallBaton::getMethodInvokeMethodId(JNIEnv *env) {
  if(m_methodInvokeMethodId == 0) {
    jclass methodClazz = env->FindClass("java/lang/reflect/Method");
    m_methodInvokeMethodId = env->GetMethodID(methodClazz, "invoke", "(Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;");
  }
  return m_methodInvokeMethodId;
}

void MethodCallBaton::run() {
  Nan::AsyncQueueWorker(this);
}

v8::Local<v8::Value> MethodCallBaton::runSync() {
  JNIEnv* env = m_java->getJavaEnv();
  ExecuteInternal(env);
  return resultsToV8(env);
}

// called by NanAsyncWorker. This will be on a worker thread
void MethodCallBaton::Execute() {
  JNIEnv* env = javaGetEnv(this->m_java->getJvm(), this->m_java->getClassLoader());
  JavaScope javaScope(env);
  ExecuteInternal(env);
}

// callback from NanAsyncWorker. This will be on the v8 main thread
void MethodCallBaton::WorkComplete() {
  Nan::HandleScope scope;

  if(callback) {
    JNIEnv* env = javaGetEnv(this->m_java->getJvm(), this->m_java->getClassLoader());
    JavaScope javaScope(env);
    v8::Local<v8::Value> result = resultsToV8(env);
    if (result->IsNativeError()) {
      v8::Local<v8::Value> argv[] = {
        result
      };
      callback->Call(1, argv, async_resource);
    } else {
      v8::Local<v8::Value> argv[] = {
        Nan::Undefined(),
        result
      };
      callback->Call(2, argv, async_resource);
    }

    delete callback;
    callback = NULL;
  }
}

v8::Local<v8::Value> MethodCallBaton::resultsToV8(JNIEnv *env) {
  Nan::EscapableHandleScope scope;

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

    v8::Local<v8::Value> err = javaExceptionToV8(m_java, env, cause, m_errorString);
    return scope.Escape(err);
  }

  return scope.Escape(javaToV8(m_java, 	env, m_result));
}

void NewInstanceBaton::ExecuteInternal(JNIEnv* env) {
  jclass constructorClazz = env->FindClass("java/lang/reflect/Constructor");
  jmethodID constructor_newInstance = env->GetMethodID(constructorClazz, "newInstance", "([Ljava/lang/Object;)Ljava/lang/Object;");

  //printf("invoke: %s\n", javaMethodCallToString(env, m_method, constructor_newInstance, m_args).c_str());

  jarray args = javaGetArgsForConstructor(env, m_method, m_args);
  jobject result = env->CallObjectMethod(m_method, constructor_newInstance, args);
  if(env->ExceptionCheck()) {
    jthrowable ex = env->ExceptionOccurred();
    env->ExceptionClear();
    m_error = (jthrowable)env->NewGlobalRef(ex);
    m_errorString = "Error creating class";
    return;
  }

  m_result = env->NewGlobalRef(result);
}

void StaticMethodCallBaton::ExecuteInternal(JNIEnv* env) {
  jmethodID method_invoke = getMethodInvokeMethodId(env);

  /*
  printf("calling %s\n", javaObjectToString(env, m_method).c_str());
  printf("arguments\n");
  for(int i=0; i<env->GetArrayLength(m_args); i++) {
    jobject o = env->GetObjectArrayElement((jobjectArray)m_args, i);
    jclass c = env->GetObjectClass(o);
    printf("  %s (%s)\n", javaObjectToString(env, o).c_str(), javaObjectToString(env, c).c_str());
  }
  */

  jarray args = javaGetArgsForMethod(env, m_method, m_args);
  jobject result = env->CallObjectMethod(m_method, method_invoke, NULL, args);

  if(env->ExceptionCheck()) {
    jthrowable ex = env->ExceptionOccurred();
    env->ExceptionClear();
    m_error = (jthrowable)env->NewGlobalRef(ex);
    m_errorString = "Error running static method";
    return;
  }

  m_result = env->NewGlobalRef(result);
}

void InstanceMethodCallBaton::ExecuteInternal(JNIEnv* env) {
  jmethodID method_invoke = getMethodInvokeMethodId(env);

  /*
  printf("calling %s\n", javaObjectToString(env, m_method).c_str());
  printf("arguments\n");
  for(int i=0; i<env->GetArrayLength(m_args); i++) {
    printf("  %s\n", javaObjectToString(env, env->GetObjectArrayElement((jobjectArray)m_args, i)).c_str());
  }
  */
  
  jarray args = javaGetArgsForMethod(env, m_method, m_args);
  jobject result = env->CallObjectMethod(m_method, method_invoke, m_javaObject->getObject(), args);

  if(env->ExceptionCheck()) {
    jthrowable ex = env->ExceptionOccurred();
    env->ExceptionClear();
    m_error = (jthrowable)env->NewGlobalRef(ex);
    m_errorString = "Error running instance method";
    return;
  }

  if(result == NULL) {
    m_result = NULL;
  } else {
    m_result = env->NewGlobalRef(result);
  }
}

NewInstanceBaton::NewInstanceBaton(
  Java* java,
  jclass clazz,
  jobject method,
  jarray args,
  v8::Local<v8::Value>& callback) : MethodCallBaton(java, method, args, callback) {
  JNIEnv *env = m_java->getJavaEnv();
  m_clazz = (jclass)env->NewGlobalRef(clazz);
}

NewInstanceBaton::~NewInstanceBaton() {
  JNIEnv *env = m_java->getJavaEnv();
  env->DeleteGlobalRef(m_clazz);
  m_clazz = NULL;
}

StaticMethodCallBaton::StaticMethodCallBaton(
  Java* java,
  jclass clazz,
  jobject method,
  jarray args,
  v8::Local<v8::Value>& callback) : MethodCallBaton(java, method, args, callback) {
  JNIEnv *env = m_java->getJavaEnv();
  m_clazz = (jclass)env->NewGlobalRef(clazz);
}

StaticMethodCallBaton::~StaticMethodCallBaton() {
  JNIEnv *env = m_java->getJavaEnv();
  env->DeleteGlobalRef(m_clazz);
  m_clazz = NULL;
}

InstanceMethodCallBaton::InstanceMethodCallBaton(
  Java* java,
  JavaObject* obj,
  jobject method,
  jarray args,
  v8::Local<v8::Value>& callback) : MethodCallBaton(java, method, args, callback) {
  m_javaObject = obj;
  m_javaObject->Ref();
}

InstanceMethodCallBaton::~InstanceMethodCallBaton() {
  m_javaObject->Unref();
}
