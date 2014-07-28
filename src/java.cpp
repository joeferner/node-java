
#include "java.h"
#include <string.h>
#ifdef WIN32
#else
  #include <unistd.h>
#endif
#include "javaObject.h"
#include "javaScope.h"
#include "methodCallBaton.h"
#include "node_NodeDynamicProxyClass.h"
#include <node_version.h>
#include <sstream>
#include <nan.h>

long v8ThreadId;

/*static*/ v8::Persistent<v8::FunctionTemplate> Java::s_ct;
/*static*/ std::string Java::s_nativeBindingLocation;

void my_sleep(int dur) {
#ifdef WIN32
  Sleep(dur);
#else
  usleep(dur);
#endif
}

long my_getThreadId() {
#ifdef WIN32
  return (long)GetCurrentThreadId();
#else
  return (long)pthread_self();
#endif
}

/*static*/ void Java::Init(v8::Handle<v8::Object> target) {
  NanScope();

  v8ThreadId = my_getThreadId();

  v8::Local<v8::FunctionTemplate> t = NanNew<v8::FunctionTemplate>(New);
  NanAssignPersistent(s_ct, t);
  t->InstanceTemplate()->SetInternalFieldCount(1);
  t->SetClassName(NanNew<v8::String>("Java"));

  NODE_SET_PROTOTYPE_METHOD(t, "getClassLoader", getClassLoader);
  NODE_SET_PROTOTYPE_METHOD(t, "newInstance", newInstance);
  NODE_SET_PROTOTYPE_METHOD(t, "newInstanceSync", newInstanceSync);
  NODE_SET_PROTOTYPE_METHOD(t, "newProxy", newProxy);
  NODE_SET_PROTOTYPE_METHOD(t, "callStaticMethod", callStaticMethod);
  NODE_SET_PROTOTYPE_METHOD(t, "callStaticMethodSync", callStaticMethodSync);
  NODE_SET_PROTOTYPE_METHOD(t, "callMethod", callMethod);
  NODE_SET_PROTOTYPE_METHOD(t, "callMethodSync", callMethodSync);
  NODE_SET_PROTOTYPE_METHOD(t, "findClassSync", findClassSync);
  NODE_SET_PROTOTYPE_METHOD(t, "newArray", newArray);
  NODE_SET_PROTOTYPE_METHOD(t, "newByte", newByte);
  NODE_SET_PROTOTYPE_METHOD(t, "newShort", newShort);
  NODE_SET_PROTOTYPE_METHOD(t, "newLong", newLong);
  NODE_SET_PROTOTYPE_METHOD(t, "newChar", newChar);
  NODE_SET_PROTOTYPE_METHOD(t, "newFloat", newFloat);
  NODE_SET_PROTOTYPE_METHOD(t, "newDouble", newDouble);
  NODE_SET_PROTOTYPE_METHOD(t, "getStaticFieldValue", getStaticFieldValue);
  NODE_SET_PROTOTYPE_METHOD(t, "setStaticFieldValue", setStaticFieldValue);
  NODE_SET_PROTOTYPE_METHOD(t, "instanceOf", instanceOf);

  target->Set(NanNew<v8::String>("Java"), t->GetFunction());
}

NAN_METHOD(Java::New) {
  NanScope();

  Java *self = new Java();
  self->Wrap(args.This());

  NanObjectWrapHandle(self)->Set(NanNew<v8::String>("classpath"), NanNew<v8::Array>());
  NanObjectWrapHandle(self)->Set(NanNew<v8::String>("options"), NanNew<v8::Array>());
  NanObjectWrapHandle(self)->Set(NanNew<v8::String>("nativeBindingLocation"), NanNew<v8::String>("Not Set"));

  NanReturnValue(args.This());
}

Java::Java() {
  this->m_jvm = NULL;
  this->m_env = NULL;
}

Java::~Java() {
  this->destroyJVM(&this->m_jvm, &this->m_env);
}

v8::Local<v8::Value> Java::ensureJvm() {
  if(!m_jvm) {
    return createJVM(&this->m_jvm, &this->m_env);
  }

  return NanNull();
}

v8::Local<v8::Value> Java::createJVM(JavaVM** jvm, JNIEnv** env) {
  JavaVM* jvmTemp;
  JavaVMInitArgs args;

  // setup classpath
  std::ostringstream classPath;
  classPath << "-Djava.class.path=";

  v8::Local<v8::Value> classPathValue = NanObjectWrapHandle(this)->Get(NanNew<v8::String>("classpath"));
  if(!classPathValue->IsArray()) {
    return NanTypeError("Classpath must be an array");
  }
  v8::Handle<v8::Array> classPathArrayTemp = v8::Handle<v8::Array>::Cast(classPathValue);
  NanAssignPersistent(m_classPathArray, classPathArrayTemp);
  for(uint32_t i=0; i<classPathArrayTemp->Length(); i++) {
    if(i != 0) {
      #ifdef WIN32
        classPath << ";";
      #else
        classPath << ":";
      #endif
    }
    v8::Local<v8::Value> arrayItemValue = classPathArrayTemp->Get(i);
    if(!arrayItemValue->IsString()) {
      return NanTypeError("Classpath must only contain strings");
    }
    v8::Local<v8::String> arrayItem = arrayItemValue->ToString();
    v8::String::Utf8Value arrayItemStr(arrayItem);
    classPath << *arrayItemStr;
  }

  // set the native binding location
  v8::Local<v8::Value> v8NativeBindingLocation = NanObjectWrapHandle(this)->Get(NanNew<v8::String>("nativeBindingLocation"));
  v8::String::Utf8Value nativeBindingLocationStr(v8NativeBindingLocation);
  s_nativeBindingLocation = *nativeBindingLocationStr;

  // get other options
  v8::Local<v8::Value> optionsValue = NanObjectWrapHandle(this)->Get(NanNew<v8::String>("options"));
  if(!optionsValue->IsArray()) {
    return NanTypeError("options must be an array");
  }
  v8::Handle<v8::Array> optionsArrayTemp = v8::Handle<v8::Array>::Cast(optionsValue);
  NanAssignPersistent(m_optionsArray, optionsArrayTemp);

  // create vm options
  int vmOptionsCount = optionsArrayTemp->Length() + 1;
  JavaVMOption* vmOptions = new JavaVMOption[vmOptionsCount];
  //printf("classPath: %s\n", classPath.str().c_str());
  vmOptions[0].optionString = strdup(classPath.str().c_str());
  for(uint32_t i=0; i<optionsArrayTemp->Length(); i++) {
    v8::Local<v8::Value> arrayItemValue = optionsArrayTemp->Get(i);
    if(!arrayItemValue->IsString()) {
      delete[] vmOptions;
      return NanTypeError("options must only contain strings");
    }
    v8::Local<v8::String> arrayItem = arrayItemValue->ToString();
    v8::String::Utf8Value arrayItemStr(arrayItem);
    vmOptions[i+1].optionString = strdup(*arrayItemStr);
  }

  JNI_GetDefaultJavaVMInitArgs(&args);
  args.version = JNI_VERSION_1_6;
  args.ignoreUnrecognized = false;
  args.options = vmOptions;
  args.nOptions = vmOptionsCount;
  JNI_CreateJavaVM(&jvmTemp, (void **)env, &args);
  *jvm = jvmTemp;

  m_classLoader = getSystemClassLoader(*env);

  // TODO: this handles sets put doesn't prevent modifing the underlying data. So java.classpath.push will still work which is invalid.
  NanObjectWrapHandle(this)->SetAccessor(NanNew<v8::String>("classpath"), AccessorProhibitsOverwritingGetter, AccessorProhibitsOverwritingSetter);
  NanObjectWrapHandle(this)->SetAccessor(NanNew<v8::String>("options"), AccessorProhibitsOverwritingGetter, AccessorProhibitsOverwritingSetter);
  NanObjectWrapHandle(this)->SetAccessor(NanNew<v8::String>("nativeBindingLocation"), AccessorProhibitsOverwritingGetter, AccessorProhibitsOverwritingSetter);

  return NanNull();
}

NAN_GETTER(Java::AccessorProhibitsOverwritingGetter) {
  Java* self = node::ObjectWrap::Unwrap<Java>(args.This());
  NanScope();
  v8::String::Utf8Value nameStr(property);
  if(!strcmp("classpath", *nameStr)) {
    NanReturnValue(self->m_classPathArray);
  } else if(!strcmp("options", *nameStr)) {
    NanReturnValue(self->m_optionsArray);
  } else if(!strcmp("nativeBindingLocation", *nameStr)) {
    NanReturnValue(NanNew<v8::String>(Java::s_nativeBindingLocation.c_str()));
  }

  std::ostringstream errStr;
  errStr << "Invalid call to accessor " << *nameStr;
  NanReturnValue(v8::Exception::Error(NanNew<v8::String>(errStr.str().c_str())));
}

NAN_SETTER(Java::AccessorProhibitsOverwritingSetter) {
  v8::String::Utf8Value nameStr(property);
  std::ostringstream errStr;
  errStr << "Cannot set " << *nameStr << " after calling any other java function.";
  NanThrowError(errStr.str().c_str());
}

void Java::destroyJVM(JavaVM** jvm, JNIEnv** env) {
  (*jvm)->DestroyJavaVM();
  *jvm = NULL;
  *env = NULL;
}

NAN_METHOD(Java::getClassLoader) {
  NanScope();
  Java* self = node::ObjectWrap::Unwrap<Java>(args.This());
  v8::Local<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsNull()) {
    NanReturnValue(ensureJvmResults);
  }
  JNIEnv* env = self->getJavaEnv();
  JavaScope javaScope(env);

  jclass classClazz = env->FindClass("java/lang/ClassLoader");
  jmethodID class_getClassLoader = env->GetStaticMethodID(classClazz, "getSystemClassLoader", "()Ljava/lang/ClassLoader;");
  jobject classLoader = env->CallStaticObjectMethod(classClazz, class_getClassLoader);
  checkJavaException(env);

  jobject result = env->NewGlobalRef(classLoader);
  NanReturnValue(javaToV8(self, env, result));
}

NAN_METHOD(Java::newInstance) {
  NanScope();
  Java* self = node::ObjectWrap::Unwrap<Java>(args.This());
  v8::Handle<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsNull()) {
    NanReturnValue(ensureJvmResults);
  }
  JNIEnv* env = self->getJavaEnv();
  JavaScope javaScope(env);

  int argsStart = 0;
  int argsEnd = args.Length();

  // arguments
  ARGS_FRONT_CLASSNAME();
  ARGS_BACK_CALLBACK();

  // find class
  jclass clazz = javaFindClass(env, className);
  if(clazz == NULL) {
    EXCEPTION_CALL_CALLBACK(self, "Could not find class " << className.c_str());
    NanReturnUndefined();
  }

  // get method
  jobjectArray methodArgs = v8ToJava(env, args, argsStart, argsEnd);
  jobject method = javaFindConstructor(env, clazz, methodArgs);
  if(method == NULL) {
    std::string msg = methodNotFoundToString(env, clazz, className, true, args, argsStart, argsEnd);
    EXCEPTION_CALL_CALLBACK(self, msg);
    NanReturnUndefined();
  }

  // run
  NewInstanceBaton* baton = new NewInstanceBaton(self, clazz, method, methodArgs, callback);
  baton->run();

  END_CALLBACK_FUNCTION("\"Constructor for class '" << className << "' called without a callback did you mean to use the Sync version?\"");
}

NAN_METHOD(Java::newInstanceSync) {
  NanScope();
  Java* self = node::ObjectWrap::Unwrap<Java>(args.This());
  v8::Handle<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsNull()) {
    NanReturnValue(ensureJvmResults);
  }
  JNIEnv* env = self->getJavaEnv();
  JavaScope javaScope(env);

  int argsStart = 0;
  int argsEnd = args.Length();

  // arguments
  ARGS_FRONT_CLASSNAME();

  // find class
  jclass clazz = javaFindClass(env, className);
  if(clazz == NULL) {
    std::ostringstream errStr;
    errStr << "Could not create class " << className.c_str();
    return NanThrowError(javaExceptionToV8(self, env, errStr.str()));
  }

  // find method
  jobjectArray methodArgs = v8ToJava(env, args, argsStart, argsEnd);
  jobject method = javaFindConstructor(env, clazz, methodArgs);
  if(method == NULL) {
    std::string msg = methodNotFoundToString(env, clazz, className, true, args, argsStart, argsEnd);
    return NanThrowError(javaExceptionToV8(self, env, msg));
  }

  // run
  v8::Handle<v8::Value> callback = NanNull();
  NewInstanceBaton* baton = new NewInstanceBaton(self, clazz, method, methodArgs, callback);
  v8::Handle<v8::Value> result = baton->runSync();
  delete baton;
  if(result->IsNativeError()) {
    return NanThrowError(result);
  }
  NanReturnValue(result);
}

NAN_METHOD(Java::newProxy) {
  NanScope();
  Java* self = node::ObjectWrap::Unwrap<Java>(args.This());
  v8::Handle<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsNull()) {
    NanReturnValue(ensureJvmResults);
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
  NanAssignPersistent(dynamicProxyData->functions, functions);

  // find NodeDynamicProxyClass
  std::string className = "node.NodeDynamicProxyClass";
  jclass clazz = javaFindClass(env, className);
  if(clazz == NULL) {
    std::ostringstream errStr;
    errStr << "Could not create class node/NodeDynamicProxyClass";
    delete dynamicProxyData;
    return NanThrowError(javaExceptionToV8(self, env, errStr.str()));
  }

  // find constructor
  jclass objectClazz = env->FindClass("java/lang/Object");
  jobjectArray methodArgs = env->NewObjectArray(2, objectClazz, NULL);
  env->SetObjectArrayElement(methodArgs, 0, v8ToJava(env, NanNew<v8::String>(s_nativeBindingLocation.c_str())));
  env->SetObjectArrayElement(methodArgs, 1, longToJavaLongObj(env, (long)dynamicProxyData));
  jobject method = javaFindConstructor(env, clazz, methodArgs);
  if(method == NULL) {
    std::ostringstream errStr;
    errStr << "Could not find constructor for class node/NodeDynamicProxyClass";
    return NanThrowError(javaExceptionToV8(self, env, errStr.str()));
  }

  // run constructor
  v8::Handle<v8::Value> callback = NanNull();
  NewInstanceBaton* baton = new NewInstanceBaton(self, clazz, method, methodArgs, callback);
  v8::Handle<v8::Value> result = baton->runSync();
  delete baton;
  if(result->IsNativeError()) {
    return NanThrowError(result);
  }
  NanAssignPersistent(dynamicProxyData->jsObject, result);
  NanReturnValue(result);
}

NAN_METHOD(Java::callStaticMethod) {
  NanScope();
  Java* self = node::ObjectWrap::Unwrap<Java>(args.This());
  v8::Handle<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsNull()) {
    NanReturnValue(ensureJvmResults);
  }
  JNIEnv* env = self->getJavaEnv();
  JavaScope javaScope(env);

  int argsStart = 0;
  int argsEnd = args.Length();

  // arguments
  ARGS_FRONT_CLASSNAME();
  ARGS_FRONT_STRING(methodName);
  ARGS_BACK_CALLBACK();

  // find class
  jclass clazz = javaFindClass(env, className);
  if(clazz == NULL) {
    EXCEPTION_CALL_CALLBACK(self, "Could not create class " << className.c_str());
    NanReturnUndefined();
  }

  // find method
  jobjectArray methodArgs = v8ToJava(env, args, argsStart, argsEnd);
  jobject method = javaFindMethod(env, clazz, methodName, methodArgs);
  if(method == NULL) {
    std::string msg = methodNotFoundToString(env, clazz, methodName, false, args, argsStart, argsEnd);
    EXCEPTION_CALL_CALLBACK(self, msg);
    NanReturnUndefined();
  }

  // run
  StaticMethodCallBaton* baton = new StaticMethodCallBaton(self, clazz, method, methodArgs, callback);
  baton->run();

  END_CALLBACK_FUNCTION("\"Static method '" << methodName << "' called without a callback did you mean to use the Sync version?\"");
}

NAN_METHOD(Java::callStaticMethodSync) {
  NanScope();
  Java* self = node::ObjectWrap::Unwrap<Java>(args.This());
  v8::Handle<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsNull()) {
    NanReturnValue(ensureJvmResults);
  }
  JNIEnv* env = self->getJavaEnv();
  JavaScope javaScope(env);

  int argsStart = 0;
  int argsEnd = args.Length();

  // arguments
  ARGS_FRONT_CLASSNAME();
  ARGS_FRONT_STRING(methodName);

  // find class
  jclass clazz = javaFindClass(env, className);
  if(clazz == NULL) {
    std::ostringstream errStr;
    errStr << "Could not create class " << className.c_str();
    return NanThrowError(javaExceptionToV8(self, env, errStr.str()));
  }

  // find method
  jobjectArray methodArgs = v8ToJava(env, args, argsStart, argsEnd);
  jobject method = javaFindMethod(env, clazz, methodName, methodArgs);
  if(method == NULL) {
    std::string msg = methodNotFoundToString(env, clazz, methodName, false, args, argsStart, argsEnd);
    return NanThrowError(javaExceptionToV8(self, env, msg));
  }

  // run
  v8::Handle<v8::Value> callback = NanNull();
  StaticMethodCallBaton* baton = new StaticMethodCallBaton(self, clazz, method, methodArgs, callback);
  v8::Handle<v8::Value> result = baton->runSync();
  delete baton;
  if(result->IsNativeError()) {
    return NanThrowError(result);
  }
  NanReturnValue(result);
}

NAN_METHOD(Java::callMethodSync) {
  NanScope();
  Java* self = node::ObjectWrap::Unwrap<Java>(args.This());
  v8::Handle<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsNull()) {
    NanReturnValue(ensureJvmResults);
  }
  JNIEnv* env = self->getJavaEnv();
  JavaScope javaScope(env);

  int argsStart = 0;
  int argsEnd = args.Length();

  // arguments
  ARGS_FRONT_OBJECT(instanceObj);
  ARGS_FRONT_STRING(methodName);

  JavaObject* javaObj = node::ObjectWrap::Unwrap<JavaObject>(instanceObj);

  // find method
  jclass clazz = javaObj->getClass();
  jobjectArray methodArgs = v8ToJava(env, args, argsStart, argsEnd);
  jobject method = javaFindMethod(env, clazz, methodName, methodArgs);
  if(method == NULL) {
    std::string msg = methodNotFoundToString(env, clazz, methodName, false, args, argsStart, argsEnd);
    return NanThrowError(javaExceptionToV8(self, env, msg));
  }

  // run
  v8::Handle<v8::Value> callback = NanNull();
  InstanceMethodCallBaton* baton = new InstanceMethodCallBaton(self, javaObj, method, methodArgs, callback);
  v8::Handle<v8::Value> result = baton->runSync();
  delete baton;
  if(result->IsNativeError()) {
    return NanThrowError(result);
  }
  NanReturnValue(result);
}

NAN_METHOD(Java::callMethod) {
  NanScope();
  Java* self = node::ObjectWrap::Unwrap<Java>(args.This());
  v8::Handle<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsNull()) {
    NanReturnValue(ensureJvmResults);
  }
  JNIEnv* env = self->getJavaEnv();
  JavaScope javaScope(env);

  int argsStart = 0;
  int argsEnd = args.Length();

  // arguments
  ARGS_FRONT_OBJECT(instanceObj);
  ARGS_FRONT_STRING(methodName);
  ARGS_BACK_CALLBACK();

  JavaObject* javaObj = node::ObjectWrap::Unwrap<JavaObject>(instanceObj);

  // find method
  jclass clazz = javaObj->getClass();
  jobjectArray methodArgs = v8ToJava(env, args, argsStart, argsEnd);
  jobject method = javaFindMethod(env, clazz, methodName, methodArgs);
  if(method == NULL) {
    std::string msg = methodNotFoundToString(env, clazz, methodName, false, args, argsStart, argsEnd);
    EXCEPTION_CALL_CALLBACK(self, msg);
    NanReturnUndefined();
  }

  // run
  InstanceMethodCallBaton* baton = new InstanceMethodCallBaton(self, javaObj, method, methodArgs, callback);
  baton->run();

  END_CALLBACK_FUNCTION("\"method '" << methodName << "' called without a callback did you mean to use the Sync version?\"");
}

NAN_METHOD(Java::findClassSync) {
  NanScope();
  Java* self = node::ObjectWrap::Unwrap<Java>(args.This());
  v8::Handle<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsNull()) {
    NanReturnValue(ensureJvmResults);
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
    return NanThrowError(javaExceptionToV8(self, env, errStr.str()));
  }

  // run
  v8::Handle<v8::Value> result = javaToV8(self, env, clazz);
  NanReturnValue(result);
}

NAN_METHOD(Java::newArray) {
  NanScope();
  Java* self = node::ObjectWrap::Unwrap<Java>(args.This());
  v8::Handle<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsNull()) {
    NanReturnValue(ensureJvmResults);
  }
  JNIEnv* env = self->getJavaEnv();
  JavaScope javaScope(env);

  int argsStart = 0;

  // arguments
  ARGS_FRONT_CLASSNAME();

  // argument - array
  if(args.Length() < argsStart+1 || !args[argsStart]->IsArray()) {
    std::ostringstream errStr;
    errStr << "Argument " << (argsStart+1) << " must be an array";
    return NanThrowError(v8::Exception::TypeError(NanNew<v8::String>(errStr.str().c_str())));
  }
  v8::Local<v8::Array> arrayObj = v8::Local<v8::Array>::Cast(args[argsStart]);

  // find class and method
  jarray results;
  if(strcmp(className.c_str(), "byte") == 0) {
    results = env->NewByteArray(arrayObj->Length());
    for(uint32_t i=0; i<arrayObj->Length(); i++) {
      v8::Local<v8::Value> item = arrayObj->Get(i);
      jobject val = v8ToJava(env, item);
      jclass byteClazz = env->FindClass("java/lang/Byte");
      jmethodID byte_byteValue = env->GetMethodID(byteClazz, "byteValue", "()B");
      jbyte byteValues[1];
      byteValues[0] = env->CallByteMethod(val, byte_byteValue);
      assert(!env->ExceptionCheck());
      env->SetByteArrayRegion((jbyteArray)results, i, 1, byteValues);
    }
  }

  else if(strcmp(className.c_str(), "char") == 0) {
    results = env->NewCharArray(arrayObj->Length());
    for(uint32_t i=0; i<arrayObj->Length(); i++) {
      v8::Local<v8::Value> item = arrayObj->Get(i);
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
      v8::Local<v8::Value> item = arrayObj->Get(i);
      jobject val = v8ToJava(env, item);
      jclass shortClazz = env->FindClass("java/lang/Short");
      jmethodID short_shortValue = env->GetMethodID(shortClazz, "shortValue", "()S");
      jshort shortValues[1];
      shortValues[0] = env->CallShortMethod(val, short_shortValue);
      assert(!env->ExceptionCheck());
      env->SetShortArrayRegion((jshortArray)results, i, 1, shortValues);
    }
  }

  else if(strcmp(className.c_str(), "boolean") == 0) {
    results = env->NewBooleanArray(arrayObj->Length());
    for(uint32_t i=0; i<arrayObj->Length(); i++) {
      v8::Local<v8::Value> item = arrayObj->Get(i);
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
      return NanThrowError(javaExceptionToV8(self, env, errStr.str()));
    }

    // create array
    results = env->NewObjectArray(arrayObj->Length(), clazz, NULL);

    for(uint32_t i=0; i<arrayObj->Length(); i++) {
      v8::Local<v8::Value> item = arrayObj->Get(i);
      jobject val = v8ToJava(env, item);
      env->SetObjectArrayElement((jobjectArray)results, i, val);
      if(env->ExceptionOccurred()) {
        std::ostringstream errStr;
        v8::String::Utf8Value valStr(item);
        errStr << "Could not add item \"" << *valStr << "\" to array.";
        return NanThrowError(javaExceptionToV8(self, env, errStr.str()));
      }
    }
  }

  NanReturnValue(JavaObject::New(self, results));
}

NAN_METHOD(Java::newByte) {
  NanScope();
  Java* self = node::ObjectWrap::Unwrap<Java>(args.This());
  v8::Handle<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsNull()) {
    NanReturnValue(ensureJvmResults);
  }
  JNIEnv* env = self->getJavaEnv();
  JavaScope javaScope(env);

  if(args.Length() != 1) {
    return NanThrowError(v8::Exception::TypeError(NanNew<v8::String>("newByte only takes 1 argument")));
  }

  // argument - value
  if(!args[0]->IsNumber()) {
    return NanThrowError(v8::Exception::TypeError(NanNew<v8::String>("Argument 1 must be a number")));
  }

  v8::Local<v8::Number> val = args[0]->ToNumber();

  jclass clazz = env->FindClass("java/lang/Byte");
  jmethodID constructor = env->GetMethodID(clazz, "<init>", "(B)V");
  jobject newObj = env->NewObject(clazz, constructor, (jbyte)val->Value());

  NanReturnValue(JavaObject::New(self, newObj));
}

NAN_METHOD(Java::newShort) {
  NanScope();
  Java* self = node::ObjectWrap::Unwrap<Java>(args.This());
  v8::Handle<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsNull()) {
    NanReturnValue(ensureJvmResults);
  }
  JNIEnv* env = self->getJavaEnv();
  JavaScope javaScope(env);

  if(args.Length() != 1) {
    return NanThrowError(v8::Exception::TypeError(NanNew<v8::String>("newShort only takes 1 argument")));
  }

  // argument - value
  if(!args[0]->IsNumber()) {
    return NanThrowError(v8::Exception::TypeError(NanNew<v8::String>("Argument 1 must be a number")));
  }

  v8::Local<v8::Number> val = args[0]->ToNumber();

  jclass clazz = env->FindClass("java/lang/Short");
  jmethodID constructor = env->GetMethodID(clazz, "<init>", "(S)V");
  jobject newObj = env->NewObject(clazz, constructor, (jshort)val->Value());

  NanReturnValue(JavaObject::New(self, newObj));
}

NAN_METHOD(Java::newLong) {
  NanScope();
  Java* self = node::ObjectWrap::Unwrap<Java>(args.This());
  v8::Handle<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsNull()) {
    NanReturnValue(ensureJvmResults);
  }
  JNIEnv* env = self->getJavaEnv();
  JavaScope javaScope(env);

  if(args.Length() != 1) {
    return NanThrowError(v8::Exception::TypeError(NanNew<v8::String>("newLong only takes 1 argument")));
  }

  // argument - value
  if(!args[0]->IsNumber()) {
    return NanThrowError(v8::Exception::TypeError(NanNew<v8::String>("Argument 1 must be a number")));
  }

  v8::Local<v8::Number> val = args[0]->ToNumber();

  jclass clazz = env->FindClass("java/lang/Long");
  jmethodID constructor = env->GetMethodID(clazz, "<init>", "(J)V");
  jobject newObj = env->NewObject(clazz, constructor, (jlong)val->Value());

  NanReturnValue(JavaObject::New(self, newObj));
}

NAN_METHOD(Java::newChar) {
  NanScope();
  Java* self = node::ObjectWrap::Unwrap<Java>(args.This());
  v8::Handle<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsNull()) {
    NanReturnValue(ensureJvmResults);
  }
  JNIEnv* env = self->getJavaEnv();
  JavaScope javaScope(env);

  if(args.Length() != 1) {
    return NanThrowError(v8::Exception::TypeError(NanNew<v8::String>("newChar only takes 1 argument")));
  }

  // argument - value
  jchar charVal;
  if(args[0]->IsNumber()) {
    v8::Local<v8::Number> val = args[0]->ToNumber();
    charVal = (jchar)val->Value();
  } else if(args[0]->IsString()) {
    v8::Local<v8::String> val = args[0]->ToString();
    if(val->Length() != 1) {
      return NanThrowError(v8::Exception::TypeError(NanNew<v8::String>("Argument 1 must be a string of 1 character.")));
    }
    std::string strVal = std::string(*v8::String::Utf8Value(val));
    charVal = (jchar)strVal[0];
  } else {
    return NanThrowError(v8::Exception::TypeError(NanNew<v8::String>("Argument 1 must be a number or string")));
  }

  jclass clazz = env->FindClass("java/lang/Character");
  jmethodID constructor = env->GetMethodID(clazz, "<init>", "(C)V");
  jobject newObj = env->NewObject(clazz, constructor, charVal);

  NanReturnValue(JavaObject::New(self, newObj));
}

NAN_METHOD(Java::newFloat) {
  NanScope();
  Java* self = node::ObjectWrap::Unwrap<Java>(args.This());
  v8::Handle<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsNull()) {
    NanReturnValue(ensureJvmResults);
  }
  JNIEnv* env = self->getJavaEnv();
  JavaScope javaScope(env);

  if(args.Length() != 1) {
    return NanThrowError(v8::Exception::TypeError(NanNew<v8::String>("newFloat only takes 1 argument")));
  } else if(!args[0]->IsNumber()) {
    return NanThrowError(v8::Exception::TypeError(NanNew<v8::String>("Argument 1 must be a number")));
  }
  v8::Local<v8::Number> val = args[0]->ToNumber();

  jclass clazz = env->FindClass("java/lang/Float");
  jmethodID constructor = env->GetMethodID(clazz, "<init>", "(F)V");
  jobject newObj = env->NewObject(clazz, constructor, (jfloat)val->Value());

  NanReturnValue(JavaObject::New(self, newObj));
}

NAN_METHOD(Java::newDouble) {
  NanScope();
  Java* self = node::ObjectWrap::Unwrap<Java>(args.This());
  v8::Handle<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsNull()) {
    NanReturnValue(ensureJvmResults);
  }
  JNIEnv* env = self->getJavaEnv();
  JavaScope javaScope(env);

  if(args.Length() != 1) {
    return NanThrowError(v8::Exception::TypeError(NanNew<v8::String>("newDouble only takes 1 argument")));
  } else if(!args[0]->IsNumber()) {
    return NanThrowError(v8::Exception::TypeError(NanNew<v8::String>("Argument 1 must be a number")));
  }
  v8::Local<v8::Number> val = args[0]->ToNumber();

  jclass clazz = env->FindClass("java/lang/Double");
  jmethodID constructor = env->GetMethodID(clazz, "<init>", "(D)V");
  jobject newObj = env->NewObject(clazz, constructor, (jdouble)val->Value());

  NanReturnValue(JavaObject::New(self, newObj));
}

NAN_METHOD(Java::getStaticFieldValue) {
  NanScope();
  Java* self = node::ObjectWrap::Unwrap<Java>(args.This());
  v8::Handle<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsNull()) {
    NanReturnValue(ensureJvmResults);
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
    return NanThrowError(javaExceptionToV8(self, env, errStr.str()));
  }

  // get the field
  jobject field = javaFindField(env, clazz, fieldName);
  if(field == NULL) {
    std::ostringstream errStr;
    errStr << "Could not find field " << fieldName.c_str() << " on class " << className.c_str();
    return NanThrowError(javaExceptionToV8(self, env, errStr.str()));
  }

  jclass fieldClazz = env->FindClass("java/lang/reflect/Field");
  jmethodID field_get = env->GetMethodID(fieldClazz, "get", "(Ljava/lang/Object;)Ljava/lang/Object;");

  // get field value
  jobject val = env->CallObjectMethod(field, field_get, NULL);
  if(env->ExceptionOccurred()) {
    std::ostringstream errStr;
    errStr << "Could not get field " << fieldName.c_str() << " on class " << className.c_str();
    return NanThrowError(javaExceptionToV8(self, env, errStr.str()));
  }

  NanReturnValue(javaToV8(self, env, val));
}

NAN_METHOD(Java::setStaticFieldValue) {
  NanScope();
  Java* self = node::ObjectWrap::Unwrap<Java>(args.This());
  v8::Handle<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsNull()) {
    NanReturnValue(ensureJvmResults);
  }
  JNIEnv* env = self->getJavaEnv();
  JavaScope javaScope(env);

  int argsStart = 0;

  // arguments
  ARGS_FRONT_CLASSNAME();
  ARGS_FRONT_STRING(fieldName);

  // argument - new value
  if(args.Length() < argsStart+1) {
    std::ostringstream errStr;
    errStr << "setStaticFieldValue requires " << (argsStart+1) << " arguments";
    return NanThrowError(v8::Exception::TypeError(NanNew<v8::String>(errStr.str().c_str())));
  }
  jobject newValue = v8ToJava(env, args[argsStart]);
  argsStart++;

  // find the class
  jclass clazz = javaFindClass(env, className);
  if(clazz == NULL) {
    std::ostringstream errStr;
    errStr << "Could not create class " << className.c_str();
    return NanThrowError(javaExceptionToV8(self, env, errStr.str()));
  }

  // get the field
  jobject field = javaFindField(env, clazz, fieldName);
  if(field == NULL) {
    std::ostringstream errStr;
    errStr << "Could not find field " << fieldName.c_str() << " on class " << className.c_str();
    return NanThrowError(javaExceptionToV8(self, env, errStr.str()));
  }

  jclass fieldClazz = env->FindClass("java/lang/reflect/Field");
  jmethodID field_set = env->GetMethodID(fieldClazz, "set", "(Ljava/lang/Object;Ljava/lang/Object;)V");

  //printf("newValue: %s\n", javaObjectToString(env, newValue).c_str());

  // set field value
  env->CallObjectMethod(field, field_set, NULL, newValue);
  if(env->ExceptionOccurred()) {
    std::ostringstream errStr;
    errStr << "Could not set field " << fieldName.c_str() << " on class " << className.c_str();
    return NanThrowError(javaExceptionToV8(self, env, errStr.str()));
  }

  NanReturnUndefined();
}

NAN_METHOD(Java::instanceOf) {
  NanScope();
  Java* self = node::ObjectWrap::Unwrap<Java>(args.This());
  v8::Handle<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsNull()) {
    NanReturnValue(ensureJvmResults);
  }
  JNIEnv* env = self->getJavaEnv();
  JavaScope javaScope(env);

  int argsStart = 0;
  ARGS_FRONT_OBJECT(obj);
  ARGS_FRONT_STRING(className);

  jobject instance = v8ToJava(env, obj);
  if (!instance) {
    // not even a Java object
    NanReturnValue(NanNew<v8::Boolean>(false));
  }

  jclass clazz = javaFindClass(env, className);
  if(!clazz) {
    std::ostringstream errStr;
    errStr << "Could not find class " << className.c_str();
    return NanThrowError(javaExceptionToV8(self, env, errStr.str()));
  }

  jboolean res = env->IsInstanceOf(instance, clazz);
  NanReturnValue(NanNew<v8::Boolean>(res));
}

void EIO_CallJs(uv_work_t* req) {
}

#if NODE_MINOR_VERSION >= 10
void EIO_AfterCallJs(uv_work_t* req, int status) {
#else
void EIO_AfterCallJs(uv_work_t* req) {
#endif
  DynamicProxyData* dynamicProxyData = static_cast<DynamicProxyData*>(req->data);
  if(!dynamicProxyDataVerify(dynamicProxyData)) {
    return;
  }
  dynamicProxyData->result = NULL;

  JNIEnv* env = dynamicProxyData->env;

  NanScope();
  v8::Array* v8Args;
  v8::Function* fn;
  v8::Handle<v8::Value>* argv;
  int argc;
  int i;
  v8::Local<v8::Value> v8Result;
  jobject javaResult;

  v8::Local<v8::Object> dynamicProxyDataFunctions = NanNew(dynamicProxyData->functions);
  v8::Local<v8::Value> fnObj = dynamicProxyDataFunctions->Get(NanNew<v8::String>(dynamicProxyData->methodName.c_str()));
  if(fnObj->IsUndefined() || fnObj->IsNull()) {
    printf("ERROR: Could not find method %s\n", dynamicProxyData->methodName.c_str());
    goto CleanUp;
  }
  if(!fnObj->IsFunction()) {
    printf("ERROR: %s is not a function.\n", dynamicProxyData->methodName.c_str());
    goto CleanUp;
  }

  fn = v8::Function::Cast(*fnObj);

  if(dynamicProxyData->args) {
    v8Args = v8::Array::Cast(*javaArrayToV8(dynamicProxyData->java, env, dynamicProxyData->args));
    argc = v8Args->Length();
  } else {
    argc = 0;
  }
  argv = new v8::Handle<v8::Value>[argc];
  for(i=0; i<argc; i++) {
    argv[i] = v8Args->Get(i);
  }
  v8Result = fn->Call(dynamicProxyDataFunctions, argc, argv);
  delete[] argv;
  if(!dynamicProxyDataVerify(dynamicProxyData)) {
    return;
  }

  javaResult = v8ToJava(env, v8Result);
  if(javaResult == NULL) {
    dynamicProxyData->result = NULL;
    dynamicProxyData->resultGlobalRef = NULL;
  } else {
    dynamicProxyData->result = javaResult;
    dynamicProxyData->resultGlobalRef = env->NewGlobalRef(javaResult);
  }

CleanUp:
  dynamicProxyData->done = true;
}

JNIEXPORT jobject JNICALL Java_node_NodeDynamicProxyClass_callJs(JNIEnv *env, jobject src, jlong ptr, jobject method, jobjectArray args) {
  long myThreadId = my_getThreadId();

  DynamicProxyData* dynamicProxyData = (DynamicProxyData*)ptr;
  dynamicProxyData->env = env;
  dynamicProxyData->args = args;
  dynamicProxyData->done = false;
  dynamicProxyData->result = NULL;
  dynamicProxyData->resultGlobalRef = NULL;

  jclass methodClazz = env->FindClass("java/lang/reflect/Method");
  jmethodID method_getName = env->GetMethodID(methodClazz, "getName", "()Ljava/lang/String;");
  dynamicProxyData->methodName = javaObjectToString(env, env->CallObjectMethod(method, method_getName));
  assert(!env->ExceptionCheck());

  uv_work_t* req = new uv_work_t();
  req->data = dynamicProxyData;
  if(myThreadId == v8ThreadId) {
#if NODE_MINOR_VERSION >= 10
    EIO_AfterCallJs(req, 0);
#else
    EIO_AfterCallJs(req);
#endif
  } else {
    uv_queue_work(uv_default_loop(), req, EIO_CallJs, (uv_after_work_cb)EIO_AfterCallJs);

    while(!dynamicProxyData->done) {
      my_sleep(100);
    }
  }

  if(!dynamicProxyDataVerify(dynamicProxyData)) {
    return NULL;
  }
  if(dynamicProxyData->result) {
    env->DeleteGlobalRef(dynamicProxyData->resultGlobalRef);
  }
  return dynamicProxyData->result;
}

JNIEXPORT void JNICALL Java_node_NodeDynamicProxyClass_unref(JNIEnv *env, jobject src, jlong ptr) {
  DynamicProxyData* dynamicProxyData = (DynamicProxyData*)ptr;
  if(!dynamicProxyDataVerify(dynamicProxyData)) {
    return;
  }
  NanDisposePersistent(dynamicProxyData->jsObject);
}
