#include "javaObject.h"
#include "java.h"
#include "javaScope.h"
#include "utils.h"
#include <sstream>
#include <algorithm>

/*static*/ std::map<std::string, v8::Persistent<v8::FunctionTemplate>*> JavaObject::sFunctionTemplates;

/*static*/ void JavaObject::Init(v8::Handle<v8::Object> target) {
}

/*static*/ v8::Local<v8::Object> JavaObject::New(Java *java, jobject obj) {
  NanEscapableScope();

  JNIEnv *env = java->getJavaEnv();
  JavaScope javaScope(env);

  jclass objClazz = env->GetObjectClass(obj);
  jclass classClazz = env->FindClass("java/lang/Class");
  jmethodID class_getName = env->GetMethodID(classClazz, "getName", "()Ljava/lang/String;");
  jobject classNameJava = env->CallObjectMethod(objClazz, class_getName);
  checkJavaException(env);
  std::string className = javaObjectToString(env, classNameJava);
  std::replace(className.begin(), className.end(), '.', '_');
  std::replace(className.begin(), className.end(), '$', '_');
  std::replace(className.begin(), className.end(), '[', 'a');
  className = "nodeJava_" + className;

  // Set up promisification
  v8::Local<v8::Object> asyncOptions = NanObjectWrapHandle(java)->Get(NanNew<v8::String>("asyncOptions")).As<v8::Object>();
  v8::Local<v8::Function> promisify;
  std::string promiseSuffix;
  bool promisifying = asyncOptions->IsObject();
  if(promisifying) {
    v8::Local<v8::Value> promisifyValue = asyncOptions->Get(NanNew<v8::String>("promisify"));
    promisify = promisifyValue.As<v8::Function>();
    v8::Local<v8::String> suffix = asyncOptions->Get(NanNew<v8::String>("promiseSuffix"))->ToString();
    v8::String::Utf8Value utf8(suffix);
    promiseSuffix.assign(*utf8);
  }

  v8::Local<v8::FunctionTemplate> funcTemplate;
  if(sFunctionTemplates.find(className) != sFunctionTemplates.end()) {
    //printf("existing className: %s\n", className.c_str());
    funcTemplate = NanNew(*sFunctionTemplates[className]);
  } else {
    //printf("create className: %s\n", className.c_str());

    funcTemplate = NanNew<v8::FunctionTemplate>();
    funcTemplate->InstanceTemplate()->SetInternalFieldCount(1);
    funcTemplate->SetClassName(NanNew<v8::String>(className.c_str()));

    std::list<jobject> methods;
    javaReflectionGetMethods(env, objClazz, &methods, false);
    jclass methodClazz = env->FindClass("java/lang/reflect/Method");
    jmethodID method_getName = env->GetMethodID(methodClazz, "getName", "()Ljava/lang/String;");
    for(std::list<jobject>::iterator it = methods.begin(); it != methods.end(); ++it) {
      jstring methodNameJava = (jstring)env->CallObjectMethod(*it, method_getName);
      assert(!env->ExceptionCheck());
      std::string methodNameStr = javaToString(env, methodNameJava);

      v8::Handle<v8::String> methodName = NanNew<v8::String>(methodNameStr.c_str());
      v8::Local<v8::FunctionTemplate> methodCallTemplate = NanNew<v8::FunctionTemplate>(methodCall, methodName);
      funcTemplate->PrototypeTemplate()->Set(methodName, methodCallTemplate->GetFunction());

      v8::Handle<v8::String> methodNameSync = NanNew<v8::String>((methodNameStr + "Sync").c_str());
      v8::Local<v8::FunctionTemplate> methodCallSyncTemplate = NanNew<v8::FunctionTemplate>(methodCallSync, methodName);
      funcTemplate->PrototypeTemplate()->Set(methodNameSync, methodCallSyncTemplate->GetFunction());

      if (promisifying) {
        v8::Local<v8::Object> recv = NanNew<v8::Object>();
        v8::Local<v8::Value> argv[] = { methodCallTemplate->GetFunction() };
        v8::Local<v8::Value> result = promisify->Call(recv, 1, argv);
        if (!result->IsFunction()) {
          fprintf(stderr, "Promisified result is not a function -- asyncOptions.promisify must return a function.\n");
          assert(result->IsFunction());
        }
        v8::Local<v8::Function> promFunction = result.As<v8::Function>();
        v8::Handle<v8::String> methodNamePromise = NanNew<v8::String>((methodNameStr + promiseSuffix).c_str());
        funcTemplate->PrototypeTemplate()->Set(methodNamePromise, promFunction);
      }
    }

    std::list<jobject> fields;
    javaReflectionGetFields(env, objClazz, &fields);
    jclass fieldClazz = env->FindClass("java/lang/reflect/Field");
    jmethodID field_getName = env->GetMethodID(fieldClazz, "getName", "()Ljava/lang/String;");
    for(std::list<jobject>::iterator it = fields.begin(); it != fields.end(); ++it) {
      jstring fieldNameJava = (jstring)env->CallObjectMethod(*it, field_getName);
      checkJavaException(env);
      std::string fieldNameStr = javaToString(env, fieldNameJava);

      v8::Handle<v8::String> fieldName = NanNew<v8::String>(fieldNameStr.c_str());
      funcTemplate->InstanceTemplate()->SetAccessor(fieldName, fieldGetter, fieldSetter);
    }

    v8::Persistent<v8::FunctionTemplate>* persistentFuncTemplate = new v8::Persistent<v8::FunctionTemplate>();
    NanAssignPersistent(*persistentFuncTemplate, funcTemplate);
    sFunctionTemplates[className] = persistentFuncTemplate;
  }

  v8::Local<v8::Function> ctor = funcTemplate->GetFunction();
  v8::Local<v8::Object> javaObjectObj = ctor->NewInstance();
  javaObjectObj->SetHiddenValue(NanNew<v8::String>(V8_HIDDEN_MARKER_JAVA_OBJECT), NanNew<v8::Boolean>(true));
  JavaObject *self = new JavaObject(java, obj);
  self->Wrap(javaObjectObj);

  return NanEscapeScope(javaObjectObj);
}

JavaObject::JavaObject(Java *java, jobject obj) {
  m_java = java;
  JNIEnv *env = m_java->getJavaEnv();
  m_obj = env->NewGlobalRef(obj);
  m_class = (jclass)env->NewGlobalRef(env->GetObjectClass(obj));
}

JavaObject::~JavaObject() {
  JNIEnv *env = m_java->getJavaEnv();
  env->DeleteGlobalRef(m_obj);
  env->DeleteGlobalRef(m_class);
}

NAN_METHOD(JavaObject::methodCall) {
  NanScope();
  JavaObject* self = node::ObjectWrap::Unwrap<JavaObject>(args.This());
  JNIEnv *env = self->m_java->getJavaEnv();
  JavaScope javaScope(env);

  v8::String::Utf8Value methodName(args.Data());
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
    EXCEPTION_CALL_CALLBACK(self->m_java, msg);
    NanReturnUndefined();
  }

  // run
  InstanceMethodCallBaton* baton = new InstanceMethodCallBaton(self->m_java, self, method, methodArgs, callback);
  baton->run();

  END_CALLBACK_FUNCTION("\"Method '" << methodNameStr << "' called without a callback did you mean to use the Sync version?\"");
}

NAN_METHOD(JavaObject::methodCallSync) {
  NanScope();
  JavaObject* self = node::ObjectWrap::Unwrap<JavaObject>(args.This());
  JNIEnv *env = self->m_java->getJavaEnv();
  JavaScope javaScope(env);

  v8::String::Utf8Value methodName(args.Data());
  std::string methodNameStr = *methodName;

  int argsStart = 0;
  int argsEnd = args.Length();

  jobjectArray methodArgs = v8ToJava(env, args, argsStart, argsEnd);

  jobject method = javaFindMethod(env, self->m_class, methodNameStr, methodArgs);
  if(method == NULL) {
    std::string msg = methodNotFoundToString(env, self->m_class, methodNameStr, false, args, argsStart, argsEnd);
    v8::Handle<v8::Value> ex = javaExceptionToV8(self->m_java, env, msg);
    return NanThrowError(ex);
  }

  // run
  v8::Handle<v8::Value> callback = NanUndefined();
  InstanceMethodCallBaton* baton = new InstanceMethodCallBaton(self->m_java, self, method, methodArgs, callback);
  v8::Handle<v8::Value> result = baton->runSync();
  delete baton;

  if(result->IsNativeError()) {
    return NanThrowError(result);
  }

  NanReturnValue(result);
}

NAN_GETTER(JavaObject::fieldGetter) {
  NanScope();
  JavaObject* self = node::ObjectWrap::Unwrap<JavaObject>(args.This());
  JNIEnv *env = self->m_java->getJavaEnv();
  JavaScope javaScope(env);

  v8::String::Utf8Value propertyCStr(property);
  std::string propertyStr = *propertyCStr;
  jobject field = javaFindField(env, self->m_class, propertyStr);
  if(field == NULL) {
    std::ostringstream errStr;
    errStr << "Could not find field " << propertyStr;
    v8::Handle<v8::Value> ex = javaExceptionToV8(self->m_java, env, errStr.str());
    return NanThrowError(ex);
  }

  jclass fieldClazz = env->FindClass("java/lang/reflect/Field");
  jmethodID field_get = env->GetMethodID(fieldClazz, "get", "(Ljava/lang/Object;)Ljava/lang/Object;");

  // get field value
  jobject val = env->CallObjectMethod(field, field_get, self->m_obj);
  if(env->ExceptionOccurred()) {
    std::ostringstream errStr;
    errStr << "Could not get field " << propertyStr;
    v8::Handle<v8::Value> ex = javaExceptionToV8(self->m_java, env, errStr.str());
    return NanThrowError(ex);
  }

  v8::Handle<v8::Value> result = javaToV8(self->m_java, env, val);

  NanReturnValue(result);
}

NAN_SETTER(JavaObject::fieldSetter) {
  NanScope();
  JavaObject* self = node::ObjectWrap::Unwrap<JavaObject>(args.This());
  JNIEnv *env = self->m_java->getJavaEnv();
  JavaScope javaScope(env);

  jobject newValue = v8ToJava(env, value);

  v8::String::Utf8Value propertyCStr(property);
  std::string propertyStr = *propertyCStr;
  jobject field = javaFindField(env, self->m_class, propertyStr);
  if(field == NULL) {
    std::ostringstream errStr;
    errStr << "Could not find field " << propertyStr;
    v8::Handle<v8::Value> error = javaExceptionToV8(self->m_java, env, errStr.str());
    NanThrowError(error);
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
    v8::Handle<v8::Value> error = javaExceptionToV8(self->m_java, env, errStr.str());
    NanThrowError(error);
    return;
  }
}

/*static*/ v8::Persistent<v8::FunctionTemplate> JavaProxyObject::s_proxyCt;

/*static*/ void JavaProxyObject::init() {
  v8::Local<v8::FunctionTemplate> t = NanNew<v8::FunctionTemplate>();
  NanAssignPersistent(s_proxyCt, t);
  t->InstanceTemplate()->SetInternalFieldCount(1);
  t->SetClassName(NanNew<v8::String>("NodeDynamicProxy"));

  v8::Handle<v8::String> methodName = NanNew<v8::String>("unref");
  v8::Local<v8::FunctionTemplate> methodCallTemplate = NanNew<v8::FunctionTemplate>(doUnref);
  t->PrototypeTemplate()->Set(methodName, methodCallTemplate->GetFunction());

  v8::Handle<v8::String> fieldName = NanNew<v8::String>("invocationHandler");
  t->InstanceTemplate()->SetAccessor(fieldName, invocationHandlerGetter);
}

v8::Local<v8::Object> JavaProxyObject::New(Java *java, jobject obj, DynamicProxyData* dynamicProxyData) {
  NanEscapableScope();

  v8::Local<v8::Function> ctor = NanNew(s_proxyCt)->GetFunction();
  v8::Local<v8::Object> javaObjectObj = ctor->NewInstance();
  javaObjectObj->SetHiddenValue(NanNew<v8::String>(V8_HIDDEN_MARKER_JAVA_OBJECT), NanNew<v8::Boolean>(true));
  JavaProxyObject *self = new JavaProxyObject(java, obj, dynamicProxyData);
  self->Wrap(javaObjectObj);

  return NanEscapeScope(javaObjectObj);
}

JavaProxyObject::JavaProxyObject(Java *java, jobject obj, DynamicProxyData* dynamicProxyData) : JavaObject(java, obj) {
  m_dynamicProxyData = dynamicProxyData;
}

JavaProxyObject::~JavaProxyObject() {
  if(dynamicProxyDataVerify(m_dynamicProxyData)) {
    unref(m_dynamicProxyData);
  }
}

NAN_METHOD(JavaProxyObject::doUnref) {
  JavaProxyObject* self = node::ObjectWrap::Unwrap<JavaProxyObject>(args.This());
  if (dynamicProxyDataVerify(self->m_dynamicProxyData)) {
    unref(self->m_dynamicProxyData);
  }
  NanReturnUndefined();
}

NAN_GETTER(JavaProxyObject::invocationHandlerGetter) {
 NanScope();

 JavaProxyObject* self = node::ObjectWrap::Unwrap<JavaProxyObject>(args.This());
 if (!dynamicProxyDataVerify(self->m_dynamicProxyData)) {
   return NanThrowError("dynamicProxyData has been destroyed or corrupted");
 }
 NanReturnValue(self->m_dynamicProxyData->functions);
}
