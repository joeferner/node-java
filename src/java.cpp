#include "java.h"
#include <string>
#ifdef WIN32
#else
  #include <unistd.h>
#endif
#include "javaObject.h"
#include "javaScope.h"
#include "methodCallBaton.h"
#include "node_NodeDynamicProxyClass.h"
#include <node_version.h>
#include <queue>
#include <sstream>
#include <nan.h>

#define DYNAMIC_PROXY_JS_ERROR -4

#ifdef WIN32
  typedef long threadId;
#else
  typedef pthread_t threadId;
#endif

threadId v8ThreadId;

std::queue<DynamicProxyJsCallData *> queue_dynamicProxyJsCallData;
uv_mutex_t uvMutex_dynamicProxyJsCall;
uv_async_t uvAsync_dynamicProxyJsCall;

/*static*/ Nan::Persistent<v8::FunctionTemplate> Java::s_ct;
/*static*/ std::string Java::s_nativeBindingLocation;

void my_sleep(int dur) {
#ifdef WIN32
  Sleep(dur);
#else
  usleep(dur);
#endif
}

threadId my_getThreadId() {
#ifdef WIN32
  return (long)GetCurrentThreadId();
#else
  return pthread_self();
#endif
}

bool v8ThreadIdEquals(threadId a, threadId b) {
#ifdef WIN32
  return a == b;
#else
  return pthread_equal(a, b);
#endif
}

void EIO_CallJs(DynamicProxyJsCallData *callData);

void uvAsyncCb_dynamicProxyJsCall(uv_async_t *handle) {
  DynamicProxyJsCallData *callData;
  do {
    uv_mutex_lock(&uvMutex_dynamicProxyJsCall);
    if(!queue_dynamicProxyJsCallData.empty()) {
      callData = queue_dynamicProxyJsCallData.front();
      queue_dynamicProxyJsCallData.pop();
    } else {
      callData = NULL;
    }
    uv_mutex_unlock(&uvMutex_dynamicProxyJsCall);

    if(callData) {
      EIO_CallJs(callData);
    }
  } while(callData);
}

/*static*/ void Java::Init(v8::Local<v8::Object> target) {
  Nan::HandleScope scope;

  v8ThreadId = my_getThreadId();

  uv_mutex_init(&uvMutex_dynamicProxyJsCall);
  uv_async_init(uv_default_loop(), &uvAsync_dynamicProxyJsCall, uvAsyncCb_dynamicProxyJsCall);

  v8::Local<v8::FunctionTemplate> t = Nan::New<v8::FunctionTemplate>(New);
  s_ct.Reset(t);
  t->InstanceTemplate()->SetInternalFieldCount(1);
  t->SetClassName(Nan::New<v8::String>("Java").ToLocalChecked());

  Nan::SetPrototypeMethod(t, "getClassLoader", getClassLoader);
  Nan::SetPrototypeMethod(t, "newInstance", newInstance);
  Nan::SetPrototypeMethod(t, "newInstanceSync", newInstanceSync);
  Nan::SetPrototypeMethod(t, "newProxy", newProxy);
  Nan::SetPrototypeMethod(t, "callStaticMethod", callStaticMethod);
  Nan::SetPrototypeMethod(t, "callStaticMethodSync", callStaticMethodSync);
  Nan::SetPrototypeMethod(t, "callMethod", callMethod);
  Nan::SetPrototypeMethod(t, "callMethodSync", callMethodSync);
  Nan::SetPrototypeMethod(t, "findClassSync", findClassSync);
  Nan::SetPrototypeMethod(t, "newArray", newArray);
  Nan::SetPrototypeMethod(t, "newByte", newByte);
  Nan::SetPrototypeMethod(t, "newShort", newShort);
  Nan::SetPrototypeMethod(t, "newLong", newLong);
  Nan::SetPrototypeMethod(t, "newChar", newChar);
  Nan::SetPrototypeMethod(t, "newFloat", newFloat);
  Nan::SetPrototypeMethod(t, "newDouble", newDouble);
  Nan::SetPrototypeMethod(t, "getStaticFieldValue", getStaticFieldValue);
  Nan::SetPrototypeMethod(t, "setStaticFieldValue", setStaticFieldValue);
  Nan::SetPrototypeMethod(t, "instanceOf", instanceOf);

  Nan::Set(target, Nan::New<v8::String>("Java").ToLocalChecked(), Nan::GetFunction(t).ToLocalChecked());

  JavaProxyObject::init();
}

NAN_METHOD(Java::New) {
  Nan::HandleScope scope;

  Java *self = new Java();
  self->Wrap(info.This());

  Nan::Set(self->handle(), Nan::New<v8::String>("classpath").ToLocalChecked(), Nan::New<v8::Array>());
  Nan::Set(self->handle(), Nan::New<v8::String>("options").ToLocalChecked(), Nan::New<v8::Array>());
  Nan::Set(self->handle(), Nan::New<v8::String>("nativeBindingLocation").ToLocalChecked(), Nan::New<v8::String>("Not Set").ToLocalChecked());
  Nan::Set(self->handle(), Nan::New<v8::String>("asyncOptions").ToLocalChecked(), Nan::Null());

  info.GetReturnValue().Set(info.This());
}

Java::Java() {
  this->m_jvm = NULL;
  this->m_env = NULL;

  m_SyncSuffix = "Sync";
  m_AsyncSuffix = "";
  doSync = true;
  doAsync = true;
  doPromise = false;
}

Java::~Java() {
  this->destroyJVM(&this->m_jvm, &this->m_env);
}

v8::Local<v8::Value> Java::ensureJvm() {
  if(!m_jvm) {
    v8::Local<v8::Value> result = createJVM(&this->m_jvm, &this->m_env);
    assert(result->IsNull());
    return result;
  }

  return Nan::Null();
}

void Java::configureAsync(v8::Local<v8::Value>& asyncOptions) {
  v8::Local<v8::Object> asyncOptionsObj = asyncOptions.As<v8::Object>();

  m_SyncSuffix = "invalid";
  m_AsyncSuffix = "invalid";
  m_PromiseSuffix = "invalid";
  doSync = false;
  doAsync = false;
  doPromise = false;

  v8::MaybeLocal<v8::Value> maybeSuffixValue = Nan::Get(asyncOptionsObj, Nan::New<v8::String>("syncSuffix").ToLocalChecked());
  v8::Local<v8::Value> suffixValue;
  if (maybeSuffixValue.ToLocal(&suffixValue) && suffixValue->IsString()) {
    v8::Local<v8::String> suffix = suffixValue->ToString(Nan::GetCurrentContext()).ToLocalChecked();
    Nan::Utf8String utf8(suffix);
    m_SyncSuffix.assign(*utf8);
    doSync = true;
  }

  maybeSuffixValue = Nan::Get(asyncOptionsObj, Nan::New<v8::String>("asyncSuffix").ToLocalChecked());
  if (maybeSuffixValue.ToLocal(&suffixValue) && suffixValue->IsString()) {
    v8::Local<v8::String> suffix = suffixValue->ToString(Nan::GetCurrentContext()).ToLocalChecked();
    Nan::Utf8String utf8(suffix);
    m_AsyncSuffix.assign(*utf8);
    doAsync = true;
  }

  maybeSuffixValue = Nan::Get(asyncOptionsObj, Nan::New<v8::String>("promiseSuffix").ToLocalChecked());
  if (maybeSuffixValue.ToLocal(&suffixValue) && suffixValue->IsString()) {
    v8::Local<v8::String> suffix = suffixValue->ToString(Nan::GetCurrentContext()).ToLocalChecked();
    Nan::Utf8String utf8(suffix);
    m_PromiseSuffix.assign(*utf8);
    v8::MaybeLocal<v8::Value> maybePromisify = Nan::Get(asyncOptionsObj, Nan::New<v8::String>("promisify").ToLocalChecked());
    v8::Local<v8::Value> promisify;
    if (maybePromisify.ToLocal(&promisify) && !promisify->IsFunction()) {
      fprintf(stderr, "asyncOptions.promisify must be a function");
      assert(promisify->IsFunction());
    }
    doPromise = true;
  }

  if (doSync && doAsync) {
    assert(m_SyncSuffix != m_AsyncSuffix);
  }
  if (doSync && doPromise) {
    assert(m_SyncSuffix != m_PromiseSuffix);
  }
  if (doAsync && doPromise) {
    assert(m_AsyncSuffix != m_PromiseSuffix);
  }

  m_asyncOptions.Reset(asyncOptionsObj);
}

v8::Local<v8::Value> Java::createJVM(JavaVM** jvm, JNIEnv** env) {
  v8::MaybeLocal<v8::Value> maybeAsyncOptions = Nan::Get(this->handle(), Nan::New<v8::String>("asyncOptions").ToLocalChecked());
  v8::Local<v8::Value> asyncOptions;
  if (maybeAsyncOptions.ToLocal(&asyncOptions) && asyncOptions->IsObject()) {
    configureAsync(asyncOptions);
  }

  // setup classpath
  std::ostringstream classPath;
  classPath << "-Djava.class.path=";

  v8::MaybeLocal<v8::Value> maybeClassPathValue = Nan::Get(this->handle(), Nan::New<v8::String>("classpath").ToLocalChecked());
  v8::Local<v8::Value> classPathValue;
  if(!maybeClassPathValue.ToLocal(&classPathValue) || !classPathValue->IsArray()) {
    return Nan::TypeError("Classpath must be an array");
  }
  v8::Local<v8::Array> classPathArrayTemp = v8::Local<v8::Array>::Cast(classPathValue);
  m_classPathArray.Reset(classPathArrayTemp);
  for(uint32_t i=0; i<classPathArrayTemp->Length(); i++) {
    if(i != 0) {
      #ifdef WIN32
        classPath << ";";
      #else
        classPath << ":";
      #endif
    }
    v8::Local<v8::Value> arrayItemValue = classPathArrayTemp->Get(Nan::GetCurrentContext(), i).ToLocalChecked();
    if(!arrayItemValue->IsString()) {
      return Nan::TypeError("Classpath must only contain strings");
    }
    v8::Local<v8::String> arrayItem = arrayItemValue->ToString(Nan::GetCurrentContext()).ToLocalChecked();
    Nan::Utf8String arrayItemStr(arrayItem);
    classPath << *arrayItemStr;
  }

  // set the native binding location
  v8::Local<v8::Value> v8NativeBindingLocation = Nan::Get(this->handle(), Nan::New<v8::String>("nativeBindingLocation").ToLocalChecked()).FromMaybe(v8::Local<v8::Value>());
  Nan::Utf8String nativeBindingLocationStr(v8NativeBindingLocation);
  s_nativeBindingLocation = *nativeBindingLocationStr;

  // get other options
  v8::Local<v8::Value> optionsValue = Nan::Get(this->handle(), Nan::New<v8::String>("options").ToLocalChecked()).FromMaybe(v8::Local<v8::Value>());
  if(!optionsValue->IsArray()) {
    return Nan::TypeError("options must be an array");
  }
  v8::Local<v8::Array> optionsArrayTemp = v8::Local<v8::Array>::Cast(optionsValue);
  m_optionsArray.Reset(optionsArrayTemp);

  // create vm options
  int vmOptionsCount = optionsArrayTemp->Length() + 1;
  JavaVMOption* vmOptions = new JavaVMOption[vmOptionsCount];
  //printf("classPath: %s\n", classPath.str().c_str());
  vmOptions[0].optionString = strdup(classPath.str().c_str());
  for(uint32_t i=0; i<optionsArrayTemp->Length(); i++) {
    v8::Local<v8::Value> arrayItemValue = optionsArrayTemp->Get(Nan::GetCurrentContext(), i).ToLocalChecked();
    if(!arrayItemValue->IsString()) {
      delete[] vmOptions;
      return Nan::TypeError("options must only contain strings");
    }
    v8::Local<v8::String> arrayItem = arrayItemValue->ToString(Nan::GetCurrentContext()).ToLocalChecked();
    Nan::Utf8String arrayItemStr(arrayItem);
    vmOptions[i+1].optionString = strdup(*arrayItemStr);
  }

  JavaVMInitArgs args;
  // The JNI invocation is documented to include a function JNI_GetDefaultJavaVMInitArgs that
  // was formerly called here. But the documentation from Oracle is confusing/contradictory.
  // 1) It claims that the caller must set args.version before calling JNI_GetDefaultJavaVMInitArgs, which
  // we did not do.
  // 2) The sample code provide at the top of the doc doesn't even call JNI_GetDefaultJavaVMInitArgs.
  // 3) The Oracle documentation for Java 6 through Java 8 all contain a comment "Note that in the JDK/JRE, there is no
  // longer any need to call JNI_GetDefaultJavaVMInitArgs."
  // 4) It seems that some platforms don't implement JNI_GetDefaultJavaVMInitArgs, or have
  // marked it deprecated.
  // Omitting the call to JNI_GetDefaultJavaVMInitArgs works fine on Mac and Linux with Java 7 and Java 8.
  // The Oracle documentation is here:
  //     http://docs.oracle.com/javase/6/docs/technotes/guides/jni/spec/invocation.html
  //     http://docs.oracle.com/javase/7/docs/technotes/guides/jni/spec/invocation.html
  //     http://docs.oracle.com/javase/8/docs/technotes/guides/jni/spec/invocation.html
  args.version = JNI_BEST_VERSION;
  // JNI_GetDefaultJavaVMInitArgs(&args);  // If this turns out to be necessary, it should be called here.
  args.ignoreUnrecognized = false;
  args.options = vmOptions;
  args.nOptions = vmOptionsCount;

  JavaVM* jvmTemp;
  JNI_CreateJavaVM(&jvmTemp, (void **)env, &args);
  *jvm = jvmTemp;

  delete [] vmOptions;

  m_classLoader = getSystemClassLoader(*env);

  v8::Local<v8::Value> onJvmCreated = Nan::Get(this->handle(), Nan::New<v8::String>("onJvmCreated").ToLocalChecked()).FromMaybe(v8::Local<v8::Value>());

  // TODO: this handles sets put doesn't prevent modifing the underlying data. So java.classpath.push will still work which is invalid.
  Nan::SetAccessor(this->handle(), Nan::New<v8::String>("classpath").ToLocalChecked(), AccessorProhibitsOverwritingGetter, AccessorProhibitsOverwritingSetter);
  Nan::SetAccessor(this->handle(), Nan::New<v8::String>("options").ToLocalChecked(), AccessorProhibitsOverwritingGetter, AccessorProhibitsOverwritingSetter);
  Nan::SetAccessor(this->handle(), Nan::New<v8::String>("nativeBindingLocation").ToLocalChecked(), AccessorProhibitsOverwritingGetter, AccessorProhibitsOverwritingSetter);
  Nan::SetAccessor(this->handle(), Nan::New<v8::String>("asyncOptions").ToLocalChecked(), AccessorProhibitsOverwritingGetter, AccessorProhibitsOverwritingSetter);
  Nan::SetAccessor(this->handle(), Nan::New<v8::String>("onJvmCreated").ToLocalChecked(), AccessorProhibitsOverwritingGetter, AccessorProhibitsOverwritingSetter);

  if (onJvmCreated->IsFunction()) {
    v8::Local<v8::Function> onJvmCreatedFunc = onJvmCreated.As<v8::Function>();
    v8::Local<v8::Object> context = Nan::New<v8::Object>();
    Nan::Call(onJvmCreatedFunc, context, 0, NULL);
  }

  return Nan::Null();
}

NAN_GETTER(Java::AccessorProhibitsOverwritingGetter) {
  Java* self = Nan::ObjectWrap::Unwrap<Java>(info.This());
  Nan::HandleScope scope;
  Nan::Utf8String nameStr(property);
  if(!strcmp("classpath", *nameStr)) {
    info.GetReturnValue().Set(Nan::New(self->m_classPathArray));
    return;
  } else if(!strcmp("options", *nameStr)) {
    info.GetReturnValue().Set(Nan::New(self->m_optionsArray));
    return;
  } else if(!strcmp("nativeBindingLocation", *nameStr)) {
    info.GetReturnValue().Set(Nan::New(Java::s_nativeBindingLocation.c_str()).ToLocalChecked());
    return;
  } else if(!strcmp("asyncOptions", *nameStr)) {
    info.GetReturnValue().Set(Nan::New(self->m_asyncOptions));
    return;
  } else if(!strcmp("onJvmCreated", *nameStr)) {
    // There is no good reason to get onJvmCreated, so just fall through to error below.
  }

  std::ostringstream errStr;
  errStr << "Invalid call to accessor " << *nameStr;
  info.GetReturnValue().Set(Nan::Error(errStr.str().c_str()));
}

NAN_SETTER(Java::AccessorProhibitsOverwritingSetter) {
  Nan::Utf8String nameStr(property);
  std::ostringstream errStr;
  errStr << "Cannot set " << *nameStr << " after calling any other java function.";
  Nan::ThrowError(errStr.str().c_str());
}

void Java::destroyJVM(JavaVM** jvm, JNIEnv** env) {
  (*jvm)->DestroyJavaVM();
  *jvm = NULL;
  *env = NULL;
}

NAN_METHOD(Java::getClassLoader) {
  Nan::HandleScope scope;
  Java* self = Nan::ObjectWrap::Unwrap<Java>(info.This());
  v8::Local<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsNull()) {
    info.GetReturnValue().Set(ensureJvmResults);
    return;
  }
  JNIEnv* env = self->getJavaEnv();
  JavaScope javaScope(env);

  jclass classClazz = env->FindClass("java/lang/ClassLoader");
  jmethodID class_getClassLoader = env->GetStaticMethodID(classClazz, "getSystemClassLoader", "()Ljava/lang/ClassLoader;");
  jobject classLoader = env->CallStaticObjectMethod(classClazz, class_getClassLoader);
  checkJavaException(env);

  jobject result = env->NewGlobalRef(classLoader);
  info.GetReturnValue().Set(javaToV8(self, env, result));
}

NAN_METHOD(Java::newInstance) {
  Nan::HandleScope scope;
  Java* self = Nan::ObjectWrap::Unwrap<Java>(info.This());
  v8::Local<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsNull()) {
    info.GetReturnValue().Set(ensureJvmResults);
    return;
  }
  JNIEnv* env = self->getJavaEnv();
  JavaScope javaScope(env);

  int argsStart = 0;
  int argsEnd = info.Length();

  // arguments
  ARGS_FRONT_CLASSNAME();
  ARGS_BACK_CALLBACK();

  // find class
  jclass clazz = javaFindClass(env, className);
  if(clazz == NULL) {
    EXCEPTION_CALL_CALLBACK(self, "Could not find class " << className.c_str());
    info.GetReturnValue().SetUndefined();
    return;
  }

  // get method
  jobjectArray methodArgs = v8ToJava(env, info, argsStart, argsEnd);
  jobject method = javaFindConstructor(env, clazz, methodArgs);
  if(method == NULL) {
    std::string msg = methodNotFoundToString(env, clazz, className, true, info, argsStart, argsEnd);
    EXCEPTION_CALL_CALLBACK(self, msg);
    info.GetReturnValue().SetUndefined();
    return;
  }

  // run
  NewInstanceBaton* baton = new NewInstanceBaton(self, clazz, method, methodArgs, callback);
  baton->run();

  END_CALLBACK_FUNCTION("\"Constructor for class '" << className << "' called without a callback did you mean to use the Sync version?\"");
}

NAN_METHOD(Java::newInstanceSync) {
  Nan::HandleScope scope;
  Java* self = Nan::ObjectWrap::Unwrap<Java>(info.This());
  v8::Local<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsNull()) {
    info.GetReturnValue().Set(ensureJvmResults);
    return;
  }
  JNIEnv* env = self->getJavaEnv();
  JavaScope javaScope(env);

  int argsStart = 0;
  int argsEnd = info.Length();

  // arguments
  ARGS_FRONT_CLASSNAME();

  // find class
  jclass clazz = javaFindClass(env, className);
  if(clazz == NULL) {
    std::ostringstream errStr;
    errStr << "Could not create class " << className.c_str();
    return Nan::ThrowError(javaExceptionToV8(self, env, errStr.str()));
  }

  // find method
  jobjectArray methodArgs = v8ToJava(env, info, argsStart, argsEnd);
  jobject method = javaFindConstructor(env, clazz, methodArgs);
  if(method == NULL) {
    std::string msg = methodNotFoundToString(env, clazz, className, true, info, argsStart, argsEnd);
    return Nan::ThrowError(javaExceptionToV8(self, env, msg));
  }

  // run
  v8::Local<v8::Value> callback = Nan::Null();
  NewInstanceBaton* baton = new NewInstanceBaton(self, clazz, method, methodArgs, callback);
  v8::Local<v8::Value> result = baton->runSync();
  delete baton;
  if(result->IsNativeError()) {
    return Nan::ThrowError(result);
  }
  info.GetReturnValue().Set(result);
}

NAN_METHOD(Java::newProxy) {
  Nan::HandleScope scope;
  Java* self = Nan::ObjectWrap::Unwrap<Java>(info.This());
  v8::Local<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsNull()) {
    info.GetReturnValue().Set(ensureJvmResults);
    return;
  }
  JNIEnv* env = self->getJavaEnv();
  JavaScope javaScope(env);

  int argsStart = 0;

  ARGS_FRONT_STRING(interfaceName);
  ARGS_FRONT_OBJECT(functions);

  DynamicProxyData* dynamicProxyData = new DynamicProxyData();
  dynamicProxyData->markerStart = DYNAMIC_PROXY_DATA_MARKER_START;
  dynamicProxyData->markerEnd = DYNAMIC_PROXY_DATA_MARKER_END;
  dynamicProxyData->java = self;
  dynamicProxyData->interfaceName = interfaceName;
  dynamicProxyData->functions.Reset(functions);

  // find NodeDynamicProxyClass
  std::string className = "node.NodeDynamicProxyClass";
  jclass clazz = javaFindClass(env, className);
  if(clazz == NULL) {
    std::ostringstream errStr;
    errStr << "Could not create class node/NodeDynamicProxyClass";
    delete dynamicProxyData;
    return Nan::ThrowError(javaExceptionToV8(self, env, errStr.str()));
  }

  // find constructor
  jclass objectClazz = env->FindClass("java/lang/Object");
  jobjectArray methodArgs = env->NewObjectArray(2, objectClazz, NULL);
  env->SetObjectArrayElement(methodArgs, 0, v8ToJava(env, Nan::New<v8::String>(s_nativeBindingLocation.c_str()).ToLocalChecked()));
  env->SetObjectArrayElement(methodArgs, 1, longToJavaLongObj(env, (jlong)dynamicProxyData));
  jobject method = javaFindConstructor(env, clazz, methodArgs);
  if(method == NULL) {
    std::ostringstream errStr;
    errStr << "Could not find constructor for class node/NodeDynamicProxyClass";
    return Nan::ThrowError(javaExceptionToV8(self, env, errStr.str()));
  }

  // create the NodeDynamicProxyClass
  jclass constructorClazz = env->FindClass("java/lang/reflect/Constructor");
  jmethodID constructor_newInstance = env->GetMethodID(constructorClazz, "newInstance", "([Ljava/lang/Object;)Ljava/lang/Object;");

  //printf("invoke: %s\n", javaMethodCallToString(env, m_method, constructor_newInstance, m_args).c_str());

  // run constructor
  jobject dynamicProxy = env->CallObjectMethod(method, constructor_newInstance, methodArgs);
  if(env->ExceptionCheck()) {
    std::ostringstream errStr;
    errStr << "Error creating class";
    return Nan::ThrowError(javaExceptionToV8(self, env, errStr.str()));
  }

  jclass dynamicInterface = javaFindClass(env, interfaceName);
  if(dynamicInterface == NULL) {
    std::ostringstream errStr;
    errStr << "Could not find interface ";
    errStr << interfaceName;
    return Nan::ThrowError(javaExceptionToV8(self, env, errStr.str()));
  }
  jclass classClazz = env->FindClass("java/lang/Class");
  jobjectArray classArray = env->NewObjectArray(1, classClazz, NULL);
  if(classArray == NULL) {
    std::ostringstream errStr;
    errStr << "Could not create class array for Proxy";
    return Nan::ThrowError(javaExceptionToV8(self, env, errStr.str()));
  }
  env->SetObjectArrayElement(classArray, 0, dynamicInterface);

  jmethodID class_getClassLoader = env->GetMethodID(classClazz, "getClassLoader", "()Ljava/lang/ClassLoader;");
  jobject classLoader = env->CallObjectMethod(dynamicInterface, class_getClassLoader);
  assertNoException(env);

  if(classLoader == NULL) {
    jclass objectClazz = env->FindClass("java/lang/Object");
    jmethodID object_getClass = env->GetMethodID(objectClazz, "getClass", "()Ljava/lang/Class;");
    jobject jobjClass = env->CallObjectMethod(dynamicProxy, object_getClass);
    checkJavaException(env);
    classLoader = env->CallObjectMethod(jobjClass, class_getClassLoader);
    checkJavaException(env);
  }
  if(classLoader == NULL) {
    std::ostringstream errStr;
    errStr << "Could not get classloader for Proxy";
    return Nan::ThrowError(javaExceptionToV8(self, env, errStr.str()));
  }

  // create proxy instance
  jclass proxyClass = env->FindClass("java/lang/reflect/Proxy");
  jmethodID proxy_newProxyInstance = env->GetStaticMethodID(proxyClass, "newProxyInstance", "(Ljava/lang/ClassLoader;[Ljava/lang/Class;Ljava/lang/reflect/InvocationHandler;)Ljava/lang/Object;");
  jobject proxyInstance = env->CallStaticObjectMethod(proxyClass, proxy_newProxyInstance, classLoader, classArray, dynamicProxy);
  if(env->ExceptionCheck()) {
    std::ostringstream errStr;
    errStr << "Error creating java.lang.reflect.Proxy";
    return Nan::ThrowError(javaExceptionToV8(self, env, errStr.str()));
  }

  v8::Local<v8::Value> result = javaToV8(self, env, proxyInstance, dynamicProxyData);

  dynamicProxyData->jsObject.Reset(result);
  info.GetReturnValue().Set(result);
}

NAN_METHOD(Java::callStaticMethod) {
  Nan::HandleScope scope;
  Java* self = Nan::ObjectWrap::Unwrap<Java>(info.This());
  v8::Local<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsNull()) {
    info.GetReturnValue().Set(ensureJvmResults);
    return;
  }
  JNIEnv* env = self->getJavaEnv();
  JavaScope javaScope(env);

  int argsStart = 0;
  int argsEnd = info.Length();

  // arguments
  ARGS_FRONT_CLASSNAME();
  ARGS_FRONT_STRING(methodName);
  ARGS_BACK_CALLBACK();

  // find class
  jclass clazz = javaFindClass(env, className);
  if(clazz == NULL) {
    EXCEPTION_CALL_CALLBACK(self, "Could not create class " << className.c_str());
    info.GetReturnValue().SetUndefined();
    return;
  }

  // find method
  jobjectArray methodArgs = v8ToJava(env, info, argsStart, argsEnd);
  jobject method = javaFindMethod(env, clazz, methodName, methodArgs);
  if(method == NULL) {
    std::string msg = methodNotFoundToString(env, clazz, methodName, false, info, argsStart, argsEnd);
    EXCEPTION_CALL_CALLBACK(self, msg);
    info.GetReturnValue().SetUndefined();
    return;
  }

  // run
  StaticMethodCallBaton* baton = new StaticMethodCallBaton(self, clazz, method, methodArgs, callback);
  baton->run();

  END_CALLBACK_FUNCTION("\"Static method '" << methodName << "' called without a callback did you mean to use the Sync version?\"");
}

NAN_METHOD(Java::callStaticMethodSync) {
  Nan::HandleScope scope;
  Java* self = Nan::ObjectWrap::Unwrap<Java>(info.This());
  v8::Local<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsNull()) {
    info.GetReturnValue().Set(ensureJvmResults);
    return;
  }
  JNIEnv* env = self->getJavaEnv();
  JavaScope javaScope(env);

  int argsStart = 0;
  int argsEnd = info.Length();

  // arguments
  ARGS_FRONT_CLASSNAME();
  ARGS_FRONT_STRING(methodName);

  // find class
  jclass clazz = javaFindClass(env, className);
  if(clazz == NULL) {
    std::ostringstream errStr;
    errStr << "Could not create class " << className.c_str();
    return Nan::ThrowError(javaExceptionToV8(self, env, errStr.str()));
  }

  // find method
  jobjectArray methodArgs = v8ToJava(env, info, argsStart, argsEnd);
  jobject method = javaFindMethod(env, clazz, methodName, methodArgs);
  if(method == NULL) {
    std::string msg = methodNotFoundToString(env, clazz, methodName, false, info, argsStart, argsEnd);
    return Nan::ThrowError(javaExceptionToV8(self, env, msg));
  }

  // run
  v8::Local<v8::Value> callback = Nan::Null();
  StaticMethodCallBaton* baton = new StaticMethodCallBaton(self, clazz, method, methodArgs, callback);
  v8::Local<v8::Value> result = baton->runSync();
  delete baton;
  if(result->IsNativeError()) {
    Nan::ThrowError(result);
    return;
  }
  info.GetReturnValue().Set(result);
}

NAN_METHOD(Java::callMethodSync) {
  Nan::HandleScope scope;
  Java* self = Nan::ObjectWrap::Unwrap<Java>(info.This());
  v8::Local<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsNull()) {
    info.GetReturnValue().Set(ensureJvmResults);
    return;
  }
  JNIEnv* env = self->getJavaEnv();
  JavaScope javaScope(env);

  int argsStart = 0;
  int argsEnd = info.Length();

  // arguments
  ARGS_FRONT_OBJECT(instanceObj);
  ARGS_FRONT_STRING(methodName);

  JavaObject* javaObj = Nan::ObjectWrap::Unwrap<JavaObject>(instanceObj);

  // find method
  jclass clazz = javaObj->getClass();
  jobjectArray methodArgs = v8ToJava(env, info, argsStart, argsEnd);
  jobject method = javaFindMethod(env, clazz, methodName, methodArgs);
  if(method == NULL) {
    std::string msg = methodNotFoundToString(env, clazz, methodName, false, info, argsStart, argsEnd);
    return Nan::ThrowError(javaExceptionToV8(self, env, msg));
  }

  // run
  v8::Local<v8::Value> callback = Nan::Null();
  InstanceMethodCallBaton* baton = new InstanceMethodCallBaton(self, javaObj, method, methodArgs, callback);
  v8::Local<v8::Value> result = baton->runSync();
  delete baton;
  if(result->IsNativeError()) {
    return Nan::ThrowError(result);
  }
  info.GetReturnValue().Set(result);
}

NAN_METHOD(Java::callMethod) {
  Nan::HandleScope scope;
  Java* self = Nan::ObjectWrap::Unwrap<Java>(info.This());
  v8::Local<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsNull()) {
    info.GetReturnValue().Set(ensureJvmResults);
    return;
  }
  JNIEnv* env = self->getJavaEnv();
  JavaScope javaScope(env);

  int argsStart = 0;
  int argsEnd = info.Length();

  // arguments
  ARGS_FRONT_OBJECT(instanceObj);
  ARGS_FRONT_STRING(methodName);
  ARGS_BACK_CALLBACK();

  JavaObject* javaObj = Nan::ObjectWrap::Unwrap<JavaObject>(instanceObj);

  // find method
  jclass clazz = javaObj->getClass();
  jobjectArray methodArgs = v8ToJava(env, info, argsStart, argsEnd);
  jobject method = javaFindMethod(env, clazz, methodName, methodArgs);
  if(method == NULL) {
    std::string msg = methodNotFoundToString(env, clazz, methodName, false, info, argsStart, argsEnd);
    EXCEPTION_CALL_CALLBACK(self, msg);
    info.GetReturnValue().SetUndefined();
    return;
  }

  // run
  InstanceMethodCallBaton* baton = new InstanceMethodCallBaton(self, javaObj, method, methodArgs, callback);
  baton->run();

  END_CALLBACK_FUNCTION("\"method '" << methodName << "' called without a callback did you mean to use the Sync version?\"");
}

NAN_METHOD(Java::findClassSync) {
  Nan::HandleScope scope;
  Java* self = Nan::ObjectWrap::Unwrap<Java>(info.This());
  v8::Local<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsNull()) {
    info.GetReturnValue().Set(ensureJvmResults);
    return;
  }
  JNIEnv* env = self->getJavaEnv();
  JavaScope javaScope(env);

  int argsStart = 0;

  // arguments
  ARGS_FRONT_CLASSNAME();

  // find class
  jclass clazz = javaFindClass(env, className);
  if(clazz == NULL) {
    std::ostringstream errStr;
    errStr << "Could not create class " << className.c_str();
    return Nan::ThrowError(javaExceptionToV8(self, env, errStr.str()));
  }

  // run
  v8::Local<v8::Value> result = javaToV8(self, env, clazz);
  info.GetReturnValue().Set(result);
}

NAN_METHOD(Java::newArray) {
  Nan::HandleScope scope;
  Java* self = Nan::ObjectWrap::Unwrap<Java>(info.This());
  v8::Local<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsNull()) {
    info.GetReturnValue().Set(ensureJvmResults);
    return;
  }
  JNIEnv* env = self->getJavaEnv();
  JavaScope javaScope(env);

  int argsStart = 0;

  // arguments
  ARGS_FRONT_CLASSNAME();

  // argument - array
  if(info.Length() < argsStart+1 || !info[argsStart]->IsArray()) {
    std::ostringstream errStr;
    errStr << "Argument " << (argsStart+1) << " must be an array";
    return Nan::ThrowError(Nan::TypeError(errStr.str().c_str()));
  }
  v8::Local<v8::Array> arrayObj = v8::Local<v8::Array>::Cast(info[argsStart]);

  // find class and method
  jarray results;
  if(strcmp(className.c_str(), "byte") == 0) {
    results = env->NewByteArray(arrayObj->Length());
    for(uint32_t i=0; i<arrayObj->Length(); i++) {
      v8::Local<v8::Value> item = arrayObj->Get(Nan::GetCurrentContext(), i).ToLocalChecked();
      jobject val = v8ToJava(env, item);
      jclass byteClazz = env->FindClass("java/lang/Byte");
      jmethodID byte_byteValue = env->GetMethodID(byteClazz, "byteValue", "()B");
      jbyte byteValues[1];
      byteValues[0] = env->CallByteMethod(val, byte_byteValue);
      assertNoException(env);
      env->SetByteArrayRegion((jbyteArray)results, i, 1, byteValues);
    }
  }

  else if(strcmp(className.c_str(), "char") == 0) {
    results = env->NewCharArray(arrayObj->Length());
    for(uint32_t i=0; i<arrayObj->Length(); i++) {
      v8::Local<v8::Value> item = arrayObj->Get(Nan::GetCurrentContext(), i).ToLocalChecked();
      jobject val = v8ToJava(env, item);
      jclass stringClazz = env->FindClass("java/lang/String");
      jmethodID string_charAt = env->GetMethodID(stringClazz, "charAt", "(I)C");
      jchar itemValues[1];
      itemValues[0] = env->CallCharMethod(val, string_charAt, 0);
      checkJavaException(env);
      env->SetCharArrayRegion((jcharArray)results, i, 1, itemValues);
    }
  }

  else if(strcmp(className.c_str(), "short") == 0) {
    results = env->NewShortArray(arrayObj->Length());
    for(uint32_t i=0; i<arrayObj->Length(); i++) {
      v8::Local<v8::Value> item = arrayObj->Get(Nan::GetCurrentContext(), i).ToLocalChecked();
      jobject val = v8ToJava(env, item);
      jclass shortClazz = env->FindClass("java/lang/Short");
      jmethodID short_shortValue = env->GetMethodID(shortClazz, "shortValue", "()S");
      jshort shortValues[1];
      shortValues[0] = env->CallShortMethod(val, short_shortValue);
      assertNoException(env);
      env->SetShortArrayRegion((jshortArray)results, i, 1, shortValues);
    }
  }

  else if(strcmp(className.c_str(), "double") == 0) {
    results = env->NewDoubleArray(arrayObj->Length());
    for(uint32_t i=0; i<arrayObj->Length(); i++) {
      v8::Local<v8::Value> item = arrayObj->Get(Nan::GetCurrentContext(), i).ToLocalChecked();
      jobject val = v8ToJava(env, item);
      jclass doubleClazz = env->FindClass("java/lang/Double");
      jmethodID double_doubleValue = env->GetMethodID(doubleClazz, "doubleValue", "()D");
      jdouble doubleValues[1];
      doubleValues[0] = env->CallDoubleMethod(val, double_doubleValue);
      assertNoException(env);
      env->SetDoubleArrayRegion((jdoubleArray)results, i, 1, doubleValues);
    }
  }

  else if(strcmp(className.c_str(), "int") == 0) {
    results = env->NewIntArray(arrayObj->Length());
    for(uint32_t i=0; i<arrayObj->Length(); i++) {
      v8::Local<v8::Value> item = arrayObj->Get(Nan::GetCurrentContext(), i).ToLocalChecked();
      jobject val = v8ToJava(env, item);
      jclass integerClazz = env->FindClass("java/lang/Integer");
      jmethodID integer_intValue = env->GetMethodID(integerClazz, "intValue", "()I");
      jint intValues[1];
      intValues[0] = env->CallIntMethod(val, integer_intValue);
      assertNoException(env);
      env->SetIntArrayRegion((jintArray)results, i, 1, intValues);
    }
  }

  else if(strcmp(className.c_str(), "float") == 0) {
    results = env->NewFloatArray(arrayObj->Length());
    for(uint32_t i=0; i<arrayObj->Length(); i++) {
      v8::Local<v8::Value> item = arrayObj->Get(Nan::GetCurrentContext(), i).ToLocalChecked();
      jobject val = v8ToJava(env, item);
      jclass floatClazz = env->FindClass("java/lang/Float");
      jmethodID float_floatValue = env->GetMethodID(floatClazz, "floatValue", "()F");
      jfloat floatValues[1];
      floatValues[0] = env->CallFloatMethod(val, float_floatValue);
      checkJavaException(env);
      env->SetFloatArrayRegion((jfloatArray)results, i, 1, floatValues);
    }
  }

  else if(strcmp(className.c_str(), "boolean") == 0) {
    results = env->NewBooleanArray(arrayObj->Length());
    for(uint32_t i=0; i<arrayObj->Length(); i++) {
      v8::Local<v8::Value> item = arrayObj->Get(Nan::GetCurrentContext(), i).ToLocalChecked();
      jobject val = v8ToJava(env, item);
      jclass booleanClazz = env->FindClass("java/lang/Boolean");
      jmethodID boolean_booleanValue = env->GetMethodID(booleanClazz, "booleanValue", "()Z");
      jboolean booleanValues[1];
      booleanValues[0] = env->CallBooleanMethod(val, boolean_booleanValue);
      checkJavaException(env);
      env->SetBooleanArrayRegion((jbooleanArray)results, i, 1, booleanValues);
    }
  }

  else
  {
    jclass clazz = javaFindClass(env, className);
    if(clazz == NULL) {
      std::ostringstream errStr;
      errStr << "Could not create class " << className.c_str();
      return Nan::ThrowError(javaExceptionToV8(self, env, errStr.str()));
    }

    // create array
    results = env->NewObjectArray(arrayObj->Length(), clazz, NULL);

    for(uint32_t i=0; i<arrayObj->Length(); i++) {
      v8::Local<v8::Value> item = arrayObj->Get(Nan::GetCurrentContext(), i).ToLocalChecked();
      jobject val = v8ToJava(env, item);
      env->SetObjectArrayElement((jobjectArray)results, i, val);
      if(env->ExceptionOccurred()) {
        std::ostringstream errStr;
        Nan::Utf8String valStr(item);
        errStr << "Could not add item \"" << *valStr << "\" to array.";
        return Nan::ThrowError(javaExceptionToV8(self, env, errStr.str()));
      }
    }
  }

  info.GetReturnValue().Set(JavaObject::New(self, results));
}

NAN_METHOD(Java::newByte) {
  Nan::HandleScope scope;
  Java* self = Nan::ObjectWrap::Unwrap<Java>(info.This());
  v8::Local<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsNull()) {
    info.GetReturnValue().Set(ensureJvmResults);
    return;
  }
  JNIEnv* env = self->getJavaEnv();
  JavaScope javaScope(env);

  if(info.Length() != 1) {
    return Nan::ThrowError(Nan::TypeError("newByte only takes 1 argument"));
  }

  // argument - value
  if(!info[0]->IsNumber()) {
    return Nan::ThrowError(Nan::TypeError("Argument 1 must be a number"));
  }

  jbyte val = Nan::To<int32_t>(info[0]).FromJust();

  jclass clazz = env->FindClass("java/lang/Byte");
  jmethodID constructor = env->GetMethodID(clazz, "<init>", "(B)V");
  jobject newObj = env->NewObject(clazz, constructor, val);

  info.GetReturnValue().Set(JavaObject::New(self, newObj));
  return;
}

NAN_METHOD(Java::newShort) {
  Nan::HandleScope scope;
  Java* self = Nan::ObjectWrap::Unwrap<Java>(info.This());
  v8::Local<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsNull()) {
    info.GetReturnValue().Set(ensureJvmResults);
    return;
  }
  JNIEnv* env = self->getJavaEnv();
  JavaScope javaScope(env);

  if(info.Length() != 1) {
    return Nan::ThrowError(Nan::TypeError("newShort only takes 1 argument"));
  }

  // argument - value
  if(!info[0]->IsNumber()) {
    return Nan::ThrowError(Nan::TypeError("Argument 1 must be a number"));
  }

  jshort val = Nan::To<int32_t>(info[0]).FromJust();

  jclass clazz = env->FindClass("java/lang/Short");
  jmethodID constructor = env->GetMethodID(clazz, "<init>", "(S)V");
  jobject newObj = env->NewObject(clazz, constructor, val);

  info.GetReturnValue().Set(JavaObject::New(self, newObj));
}

NAN_METHOD(Java::newLong) {
  Nan::HandleScope scope;
  Java* self = Nan::ObjectWrap::Unwrap<Java>(info.This());
  v8::Local<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsNull()) {
    info.GetReturnValue().Set(ensureJvmResults);
    return;
  }
  JNIEnv* env = self->getJavaEnv();
  JavaScope javaScope(env);

  if(info.Length() != 1) {
    return Nan::ThrowError(Nan::TypeError("newLong only takes 1 argument"));
  }

  // argument - value
  if(!info[0]->IsNumber()) {
    return Nan::ThrowError(Nan::TypeError("Argument 1 must be a number"));
  }

  jlong val = Nan::To<int64_t>(info[0]).FromJust();

  jclass clazz = env->FindClass("java/lang/Long");
  jmethodID constructor = env->GetMethodID(clazz, "<init>", "(J)V");
  jobject newObj = env->NewObject(clazz, constructor, val);

  info.GetReturnValue().Set(JavaObject::New(self, newObj));
}

NAN_METHOD(Java::newChar) {
  Nan::HandleScope scope;
  Java* self = Nan::ObjectWrap::Unwrap<Java>(info.This());
  v8::Local<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsNull()) {
    info.GetReturnValue().Set(ensureJvmResults);
    return;
  }
  JNIEnv* env = self->getJavaEnv();
  JavaScope javaScope(env);

  if(info.Length() != 1) {
    return Nan::ThrowError(Nan::TypeError("newChar only takes 1 argument"));
  }

  // argument - value
  jchar charVal;
  if(info[0]->IsNumber()) {
    charVal = (jchar)Nan::To<int32_t>(info[0]).FromJust();
  } else if(info[0]->IsString()) {
    v8::Local<v8::String> val = info[0]->ToString(Nan::GetCurrentContext()).ToLocalChecked();
    if(val->Length() != 1) {
      return Nan::ThrowError(Nan::TypeError("Argument 1 must be a string of 1 character."));
    }
    std::string strVal = std::string(*Nan::Utf8String(val));
    charVal = (jchar)strVal[0];
  } else {
    return Nan::ThrowError(Nan::TypeError("Argument 1 must be a number or string"));
  }

  jclass clazz = env->FindClass("java/lang/Character");
  jmethodID constructor = env->GetMethodID(clazz, "<init>", "(C)V");
  jobject newObj = env->NewObject(clazz, constructor, charVal);

  info.GetReturnValue().Set(JavaObject::New(self, newObj));
}

NAN_METHOD(Java::newFloat) {
  Nan::HandleScope scope;
  Java* self = Nan::ObjectWrap::Unwrap<Java>(info.This());
  v8::Local<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsNull()) {
    info.GetReturnValue().Set(ensureJvmResults);
    return;
  }
  JNIEnv* env = self->getJavaEnv();
  JavaScope javaScope(env);

  if(info.Length() != 1) {
    return Nan::ThrowError(Nan::TypeError("newFloat only takes 1 argument"));
  } else if(!info[0]->IsNumber()) {
    return Nan::ThrowError(Nan::TypeError("Argument 1 must be a number"));
  }
  jfloat val = (jfloat)Nan::To<double>(info[0]).FromJust();

  jclass clazz = env->FindClass("java/lang/Float");
  jmethodID constructor = env->GetMethodID(clazz, "<init>", "(F)V");
  jobject newObj = env->NewObject(clazz, constructor, val);

  info.GetReturnValue().Set(JavaObject::New(self, newObj));
}

NAN_METHOD(Java::newDouble) {
  Nan::HandleScope scope;
  Java* self = Nan::ObjectWrap::Unwrap<Java>(info.This());
  v8::Local<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsNull()) {
    info.GetReturnValue().Set(ensureJvmResults);
    return;
  }
  JNIEnv* env = self->getJavaEnv();
  JavaScope javaScope(env);

  if(info.Length() != 1) {
    return Nan::ThrowError(Nan::TypeError("newDouble only takes 1 argument"));
  } else if(!info[0]->IsNumber()) {
    return Nan::ThrowError(Nan::TypeError("Argument 1 must be a number"));
  }

  jdouble val = (jdouble)Nan::To<double>(info[0]).FromJust();

  jclass clazz = env->FindClass("java/lang/Double");
  jmethodID constructor = env->GetMethodID(clazz, "<init>", "(D)V");
  jobject newObj = env->NewObject(clazz, constructor, val);

  info.GetReturnValue().Set(JavaObject::New(self, newObj));
}

NAN_METHOD(Java::getStaticFieldValue) {
  Nan::HandleScope scope;
  Java* self = Nan::ObjectWrap::Unwrap<Java>(info.This());
  v8::Local<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsNull()) {
    info.GetReturnValue().Set(ensureJvmResults);
    return;
  }
  JNIEnv* env = self->getJavaEnv();
  JavaScope javaScope(env);

  int argsStart = 0;

  // arguments
  ARGS_FRONT_CLASSNAME();
  ARGS_FRONT_STRING(fieldName);

  // find the class
  jclass clazz = javaFindClass(env, className);
  if(clazz == NULL) {
    std::ostringstream errStr;
    errStr << "Could not create class " << className.c_str();
    return Nan::ThrowError(javaExceptionToV8(self, env, errStr.str()));
  }

  // get the field
  jobject field = javaFindField(env, clazz, fieldName);
  if(field == NULL) {
    std::ostringstream errStr;
    errStr << "Could not find field \"" << fieldName.c_str() << "\" on class \"" << className.c_str() << "\"";
    return Nan::ThrowError(javaExceptionToV8(self, env, errStr.str()));
  }

  jclass fieldClazz = env->FindClass("java/lang/reflect/Field");
  jmethodID field_get = env->GetMethodID(fieldClazz, "get", "(Ljava/lang/Object;)Ljava/lang/Object;");

  // get field value
  jobject val = env->CallObjectMethod(field, field_get, NULL);
  if(env->ExceptionOccurred()) {
    std::ostringstream errStr;
    errStr << "Could not get field " << fieldName.c_str() << " on class " << className.c_str();
    return Nan::ThrowError(javaExceptionToV8(self, env, errStr.str()));
  }

  info.GetReturnValue().Set(javaToV8(self, env, val));
}

NAN_METHOD(Java::setStaticFieldValue) {
  Nan::HandleScope scope;
  Java* self = Nan::ObjectWrap::Unwrap<Java>(info.This());
  v8::Local<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsNull()) {
    info.GetReturnValue().Set(ensureJvmResults);
    return;
  }
  JNIEnv* env = self->getJavaEnv();
  JavaScope javaScope(env);

  int argsStart = 0;

  // arguments
  ARGS_FRONT_CLASSNAME();
  ARGS_FRONT_STRING(fieldName);

  // argument - new value
  if(info.Length() < argsStart+1) {
    std::ostringstream errStr;
    errStr << "setStaticFieldValue requires " << (argsStart+1) << " arguments";
    Nan::ThrowError(Nan::TypeError(errStr.str().c_str()));
    return;
  }
  jobject newValue = v8ToJava(env, info[argsStart]);
  argsStart++;

  // find the class
  jclass clazz = javaFindClass(env, className);
  if(clazz == NULL) {
    std::ostringstream errStr;
    errStr << "Could not create class " << className.c_str();
    Nan::ThrowError(javaExceptionToV8(self, env, errStr.str()));
    return;
  }

  // get the field
  jobject field = javaFindField(env, clazz, fieldName);
  if(field == NULL) {
    std::ostringstream errStr;
    errStr << "Could not find field \"" << fieldName.c_str() << "\" on class \"" << className.c_str() << "\"";
    Nan::ThrowError(javaExceptionToV8(self, env, errStr.str()));
    return;
  }

  jclass fieldClazz = env->FindClass("java/lang/reflect/Field");
  jmethodID field_set = env->GetMethodID(fieldClazz, "set", "(Ljava/lang/Object;Ljava/lang/Object;)V");

  //printf("newValue: %s\n", javaObjectToString(env, newValue).c_str());

  // set field value
  env->CallObjectMethod(field, field_set, NULL, newValue);
  if(env->ExceptionOccurred()) {
    std::ostringstream errStr;
    errStr << "Could not set field " << fieldName.c_str() << " on class " << className.c_str();
    Nan::ThrowError(javaExceptionToV8(self, env, errStr.str()));
    return;
  }

  info.GetReturnValue().SetUndefined();
}

NAN_METHOD(Java::instanceOf) {
  Nan::HandleScope scope;
  Java* self = Nan::ObjectWrap::Unwrap<Java>(info.This());
  v8::Local<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsNull()) {
    info.GetReturnValue().Set(ensureJvmResults);
    return;
  }
  JNIEnv* env = self->getJavaEnv();
  JavaScope javaScope(env);

  int argsStart = 0;
  ARGS_FRONT_OBJECT(obj);
  ARGS_FRONT_STRING(className);

  jobject instance = v8ToJava(env, obj);
  if (!instance) {
    // not even a Java object
    info.GetReturnValue().Set(Nan::New<v8::Boolean>(false));
    return;
  }

  jclass clazz = javaFindClass(env, className);
  if(!clazz) {
    std::ostringstream errStr;
    errStr << "Could not find class " << className.c_str();
    Nan::ThrowError(javaExceptionToV8(self, env, errStr.str()));
    return;
  }

  jboolean res = env->IsInstanceOf(instance, clazz);
  info.GetReturnValue().Set(Nan::New<v8::Boolean>(res));
}

template <typename T>
std::string to_string(T value) {
  std::ostringstream os;
  os << value;
  return os.str();
}

void EIO_CallJs(DynamicProxyJsCallData *callData) {
  DynamicProxyData* dynamicProxyData = callData->dynamicProxyData;

  assert(callData->done == 0);

  if(!dynamicProxyDataVerify(dynamicProxyData)) {
    return;
  }
  callData->result = NULL;

  JNIEnv* env;
  int ret = dynamicProxyData->java->getJvm()->GetEnv((void**)&env, JNI_BEST_VERSION);
  if (ret != JNI_OK) {
    callData->throwableClass = "java/lang/IllegalStateException";
    callData->throwableMessage = "Could not retrieve JNIEnv: jvm->GetEnv returned " + to_string<int>(ret);
    callData->done = DYNAMIC_PROXY_JS_ERROR;
    return;
  }

  Nan::HandleScope scope;
  v8::Array* v8Args;
  v8::Local<v8::Function> fn;
  v8::Local<v8::Value>* argv;
  int argc;
  int i;
  v8::Local<v8::Value> v8Result;
  jobject javaResult;

  v8::Local<v8::Object> dynamicProxyDataFunctions = Nan::New(dynamicProxyData->functions);
  v8::Local<v8::Value> fnObj = dynamicProxyDataFunctions->Get(Nan::GetCurrentContext(), Nan::New<v8::String>(callData->methodName.c_str()).ToLocalChecked()).ToLocalChecked();
  if(fnObj->IsUndefined() || fnObj->IsNull()) {
    callData->throwableClass = "java/lang/NoSuchMethodError";
    callData->throwableMessage = "Could not find js function " + callData->methodName;
    callData->done = DYNAMIC_PROXY_JS_ERROR;
    return;
  }
  if(!fnObj->IsFunction()) {
    callData->throwableClass = "java/lang/IllegalStateException";
    callData->throwableMessage = callData->methodName + " is not a function";
    callData->done = DYNAMIC_PROXY_JS_ERROR;
    return;
  }

  fn = fnObj.As<v8::Function>();

  if(callData->args) {
    v8Args = v8::Array::Cast(*javaArrayToV8(dynamicProxyData->java, env, callData->args));
    argc = v8Args->Length();
  } else {
    argc = 0;
  }
  argv = new v8::Local<v8::Value>[argc];
  for(i=0; i<argc; i++) {
    argv[i] = v8Args->Get(Nan::GetCurrentContext(), i).ToLocalChecked();
  }

  Nan::TryCatch tryCatch;
  tryCatch.SetCaptureMessage(true);
  v8Result = Nan::Call(fn, dynamicProxyDataFunctions, argc, argv).FromMaybe(v8::Local<v8::Value>());
  delete[] argv;
  if (tryCatch.HasCaught()) {
    callData->throwableClass = "node/NodeJsException";
    Nan::Utf8String message(tryCatch.Message()->Get());
    callData->throwableMessage = std::string(*message);
    tryCatch.Reset();
    callData->done = DYNAMIC_PROXY_JS_ERROR;
    return;
  }

  if(!dynamicProxyDataVerify(dynamicProxyData)) {
    return;
  }

  javaResult = v8ToJava(env, v8Result);
  if(javaResult == NULL) {
    callData->result = NULL;
  } else {
    callData->result = env->NewGlobalRef(javaResult);
  }

  callData->done = true;
}

void throwNewThrowable(JNIEnv* env, const char * excClassName, std::string msg) {
  jclass newExcCls = env->FindClass(excClassName);
  jthrowable throwable = env->ExceptionOccurred();
  if (throwable != NULL) {
    env->Throw(throwable); // this should only be Errors, according to the docs
  }
  env->ThrowNew(newExcCls, msg.c_str());
}

JNIEXPORT jobject JNICALL Java_node_NodeDynamicProxyClass_callJs(JNIEnv *env, jobject src, jlong ptr, jobject method, jobjectArray args) {
  threadId myThreadId = my_getThreadId();

  bool hasArgsGlobalRef = false;

  DynamicProxyData* dynamicProxyData = (DynamicProxyData*)ptr;

  // args needs to be global, you can't send env across thread boundaries
  DynamicProxyJsCallData callData;
  callData.dynamicProxyData = dynamicProxyData;
  callData.args = args;
  callData.done = false;
  callData.result = NULL;
  callData.throwableClass = "";
  callData.throwableMessage = "";

  jclass methodClazz = env->FindClass("java/lang/reflect/Method");
  jmethodID method_getName = env->GetMethodID(methodClazz, "getName", "()Ljava/lang/String;");
  callData.methodName = javaObjectToString(env, env->CallObjectMethod(method, method_getName));
  assertNoException(env);

  if(v8ThreadIdEquals(myThreadId, v8ThreadId)) {
    EIO_CallJs(&callData);
  } else {
    if (args) {
      // if args is not null and we have to kick this across the thread boundary, make it a global ref
      callData.args = (jobjectArray) env->NewGlobalRef(args);
      hasArgsGlobalRef = true;
    }

    uv_mutex_lock(&uvMutex_dynamicProxyJsCall);
    queue_dynamicProxyJsCallData.push(&callData); // we wait for work to finish, so ok to pass ref to local var
    uv_mutex_unlock(&uvMutex_dynamicProxyJsCall);
    uv_async_send(&uvAsync_dynamicProxyJsCall);

    while(!callData.done) {
      my_sleep(100);
    }
  }

  if(!dynamicProxyDataVerify(dynamicProxyData)) {
    throwNewThrowable(env, "java/lang/IllegalStateException", "dynamicProxyData was corrupted");
  }
  if(hasArgsGlobalRef) {
    env->DeleteGlobalRef(callData.args);
  }

  if (callData.done == DYNAMIC_PROXY_JS_ERROR) {
    throwNewThrowable(env, callData.throwableClass.c_str(), callData.throwableMessage);
  }

  jobject result = NULL;
  if(callData.result) {
    // need to retain a local ref so that we can return it, otherwise the returned object gets corrupted
    result = env->NewLocalRef(callData.result);
    env->DeleteGlobalRef(callData.result);
  }
  return result;
}

JNIEXPORT void JNICALL Java_node_NodeDynamicProxyClass_unref(JNIEnv *env, jobject src, jlong ptr) {
  DynamicProxyData* dynamicProxyData = (DynamicProxyData*)ptr;
  unref(dynamicProxyData);
}
