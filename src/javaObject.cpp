
#include "javaObject.h"
#include "java.h"
#include "javaScope.h"
#include "utils.h"
#include <sstream>
#include <algorithm>

/*static*/ std::map<std::string, v8::Persistent<v8::FunctionTemplate> > JavaObject::sFunctionTemplates;

/*static*/ void JavaObject::Init(v8::Handle<v8::Object> target) {
}

/*static*/ v8::Local<v8::Object> JavaObject::New(Java *java, jobject obj) {
  v8::HandleScope scope;

  JNIEnv *env = java->getJavaEnv();
  obj = env->NewGlobalRef(obj);
  JavaScope javaScope(env);

  jclass objClazz = env->GetObjectClass(obj);
  jclass classClazz = env->FindClass("java/lang/Class");
  jmethodID class_getName = env->GetMethodID(classClazz, "getName", "()Ljava/lang/String;");
  jobject classNameJava = env->CallObjectMethod(objClazz, class_getName);
  std::string className = javaObjectToString(env, classNameJava);
  std::replace(className.begin(), className.end(), '.', '_');
  std::replace(className.begin(), className.end(), '$', '_');
  std::replace(className.begin(), className.end(), '[', 'a');
  className = "nodeJava_" + className;

  v8::Persistent<v8::FunctionTemplate> persistentFuncTemplate;
  if(sFunctionTemplates.find(className) != sFunctionTemplates.end()) {
    //printf("existing className: %s\n", className.c_str());
    persistentFuncTemplate = sFunctionTemplates[className];
  } else {
    //printf("create className: %s\n", className.c_str());

    v8::Local<v8::FunctionTemplate> funcTemplate = v8::FunctionTemplate::New();
    funcTemplate->InstanceTemplate()->SetInternalFieldCount(1);
    funcTemplate->SetClassName(v8::String::NewSymbol(className.c_str()));

    std::list<jobject> methods;
    javaReflectionGetMethods(env, objClazz, &methods, false);
    jclass methodClazz = env->FindClass("java/lang/reflect/Method");
    jmethodID method_getName = env->GetMethodID(methodClazz, "getName", "()Ljava/lang/String;");
    for(std::list<jobject>::iterator it = methods.begin(); it != methods.end(); it++) {
      jstring methodNameJava = (jstring)env->CallObjectMethod(*it, method_getName);
      std::string methodNameStr = javaToString(env, methodNameJava);

      v8::Handle<v8::String> methodName = v8::String::New(methodNameStr.c_str());
      v8::Local<v8::FunctionTemplate> methodCallTemplate = v8::FunctionTemplate::New(methodCall, methodName);
      funcTemplate->PrototypeTemplate()->Set(methodName, methodCallTemplate->GetFunction());

      v8::Handle<v8::String> methodNameSync = v8::String::New((methodNameStr + "Sync").c_str());
      v8::Local<v8::FunctionTemplate> methodCallSyncTemplate = v8::FunctionTemplate::New(methodCallSync, methodName);
      funcTemplate->PrototypeTemplate()->Set(methodNameSync, methodCallSyncTemplate->GetFunction());
    }

    std::list<jobject> fields;
    javaReflectionGetFields(env, objClazz, &fields);
    jclass fieldClazz = env->FindClass("java/lang/reflect/Field");
    jmethodID field_getName = env->GetMethodID(fieldClazz, "getName", "()Ljava/lang/String;");
    for(std::list<jobject>::iterator it = fields.begin(); it != fields.end(); it++) {
      jstring fieldNameJava = (jstring)env->CallObjectMethod(*it, field_getName);
      std::string fieldNameStr = javaToString(env, fieldNameJava);

      v8::Handle<v8::String> fieldName = v8::String::New(fieldNameStr.c_str());
      funcTemplate->InstanceTemplate()->SetAccessor(fieldName, fieldGetter, fieldSetter);
    }

    sFunctionTemplates[className] = persistentFuncTemplate = v8::Persistent<v8::FunctionTemplate>::New(funcTemplate);
  }

  v8::Local<v8::Function> ctor = persistentFuncTemplate->GetFunction();
  v8::Local<v8::Object> javaObjectObj = ctor->NewInstance();
  javaObjectObj->SetHiddenValue(v8::String::New(V8_HIDDEN_MARKER_JAVA_OBJECT), v8::Boolean::New(true));
  JavaObject *self = new JavaObject(java, obj);
  self->Wrap(javaObjectObj);

  return scope.Close(javaObjectObj);
}

JavaObject::JavaObject(Java *java, jobject obj) {
  m_java = java;
  JNIEnv *env = m_java->getJavaEnv();
  m_obj = obj;
  m_class = (jclass)env->NewGlobalRef(env->GetObjectClass(obj));
}

JavaObject::~JavaObject() {
  JNIEnv *env = m_java->getJavaEnv();

  jclass nodeDynamicProxyClass = env->FindClass("node/NodeDynamicProxyClass");
  if(env->IsInstanceOf(m_obj, nodeDynamicProxyClass)) {
    jfieldID ptrField = env->GetFieldID(nodeDynamicProxyClass, "ptr", "J");
    DynamicProxyData* proxyData = (DynamicProxyData*)(long)env->GetLongField(m_obj, ptrField);
    if(dynamicProxyDataVerify(proxyData)) {
      delete proxyData;
    }
  }

  env->DeleteGlobalRef(m_obj);
  env->DeleteGlobalRef(m_class);
}

/*static*/ v8::Handle<v8::Value> JavaObject::methodCall(const v8::Arguments& args) {
  v8::HandleScope scope;
  JavaObject* self = node::ObjectWrap::Unwrap<JavaObject>(args.This());
  JNIEnv *env = self->m_java->getJavaEnv();
  JavaScope javaScope(env);

  v8::String::AsciiValue methodName(args.Data());
  std::string methodNameStr = *methodName;

  int argsStart = 0;
  int argsEnd = args.Length();

  // arguments
  ARGS_BACK_CALLBACK();

  if(!callbackProvided && methodNameStr == "toString") {
    return methodCallSync(args);
  }

  jobjectArray methodArgs = v8ToJava(env, args, argsStart, argsEnd);

  jobject method = javaFindMethod(env, self->m_class, methodNameStr, methodArgs);
  if(method == NULL) {
    std::string msg = methodNotFoundToString(env, self->m_class, methodNameStr, false, args, argsStart, argsEnd);
    EXCEPTION_CALL_CALLBACK(msg);
    return v8::Undefined();
  }

  // run
  InstanceMethodCallBaton* baton = new InstanceMethodCallBaton(self->m_java, self, method, methodArgs, callback);
  baton->run();

  END_CALLBACK_FUNCTION("\"Method '" << methodNameStr << "' called without a callback did you mean to use the Sync version?\"");
}

/*static*/ v8::Handle<v8::Value> JavaObject::methodCallSync(const v8::Arguments& args) {
  v8::HandleScope scope;
  JavaObject* self = node::ObjectWrap::Unwrap<JavaObject>(args.This());
  JNIEnv *env = self->m_java->getJavaEnv();
  JavaScope javaScope(env);

  v8::String::AsciiValue methodName(args.Data());
  std::string methodNameStr = *methodName;

  int argsStart = 0;
  int argsEnd = args.Length();

  jobjectArray methodArgs = v8ToJava(env, args, argsStart, argsEnd);

  jobject method = javaFindMethod(env, self->m_class, methodNameStr, methodArgs);
  if(method == NULL) {
    std::string msg = methodNotFoundToString(env, self->m_class, methodNameStr, false, args, argsStart, argsEnd);
    v8::Handle<v8::Value> ex = javaExceptionToV8(env, msg);
    return ThrowException(ex);
  }

  // run
  v8::Handle<v8::Value> callback = v8::Undefined();
  InstanceMethodCallBaton* baton = new InstanceMethodCallBaton(self->m_java, self, method, methodArgs, callback);
  v8::Handle<v8::Value> result = baton->runSync();
  delete baton;

  if(result->IsNativeError()) {
    return ThrowException(result);
  }

  return scope.Close(result);
}

/*static*/ v8::Handle<v8::Value> JavaObject::fieldGetter(v8::Local<v8::String> property, const v8::AccessorInfo& info) {
  v8::HandleScope scope;
  JavaObject* self = node::ObjectWrap::Unwrap<JavaObject>(info.This());
  JNIEnv *env = self->m_java->getJavaEnv();
  JavaScope javaScope(env);

  v8::String::AsciiValue propertyCStr(property);
  std::string propertyStr = *propertyCStr;
  jobject field = javaFindField(env, self->m_class, propertyStr);
  if(field == NULL) {
    std::ostringstream errStr;
    errStr << "Could not find field " << propertyStr;
    v8::Handle<v8::Value> ex = javaExceptionToV8(env, errStr.str());
    return ThrowException(ex);
  }

  jclass fieldClazz = env->FindClass("java/lang/reflect/Field");
  jmethodID field_get = env->GetMethodID(fieldClazz, "get", "(Ljava/lang/Object;)Ljava/lang/Object;");

  // get field value
  jobject val = env->CallObjectMethod(field, field_get, self->m_obj);
  if(env->ExceptionOccurred()) {
    std::ostringstream errStr;
    errStr << "Could not get field " << propertyStr;
    v8::Handle<v8::Value> ex = javaExceptionToV8(env, errStr.str());
    return ThrowException(ex);
  }

  v8::Handle<v8::Value> result = javaToV8(self->m_java, env, val);

  return scope.Close(result);
}

/*static*/ void JavaObject::fieldSetter(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::AccessorInfo& info) {
  v8::HandleScope scope;
  JavaObject* self = node::ObjectWrap::Unwrap<JavaObject>(info.This());
  JNIEnv *env = self->m_java->getJavaEnv();
  JavaScope javaScope(env);

  jobject newValue = v8ToJava(env, value);

  v8::String::AsciiValue propertyCStr(property);
  std::string propertyStr = *propertyCStr;
  jobject field = javaFindField(env, self->m_class, propertyStr);
  if(field == NULL) {
    std::ostringstream errStr;
    errStr << "Could not find field " << propertyStr;
    v8::Handle<v8::Value> ex = javaExceptionToV8(env, errStr.str());
    ThrowException(ex);
    return;
  }

  jclass fieldClazz = env->FindClass("java/lang/reflect/Field");
  jmethodID field_set = env->GetMethodID(fieldClazz, "set", "(Ljava/lang/Object;Ljava/lang/Object;)V");

  //printf("newValue: %s\n", javaObjectToString(env, newValue).c_str());

  // set field value
  env->CallObjectMethod(field, field_set, self->m_obj, newValue);
  if(env->ExceptionOccurred()) {
    std::ostringstream errStr;
    errStr << "Could not set field " << propertyStr;
    v8::Handle<v8::Value> ex = javaExceptionToV8(env, errStr.str());
    ThrowException(ex);
    return;
  }
}
