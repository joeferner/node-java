#include "javaObject.h"
#include "java.h"
#include "javaScope.h"
#include "utils.h"
#include <sstream>
#include <algorithm>

/*static*/ std::map<std::string, Nan::Persistent<v8::FunctionTemplate>*> JavaObject::sFunctionTemplates;

/*static*/ void JavaObject::Init(v8::Handle<v8::Object> target) {
}

/*static*/ v8::Local<v8::Object> JavaObject::New(Java *java, jobject obj) {
  Nan::EscapableHandleScope scope;

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

  v8::Local<v8::Function> promisify;
  if(java->DoPromise()) {
    v8::Local<v8::Object> asyncOptions = java->handle()->Get(Nan::New<v8::String>("asyncOptions").ToLocalChecked()).As<v8::Object>();
    v8::Local<v8::Value> promisifyValue = asyncOptions->Get(Nan::New<v8::String>("promisify").ToLocalChecked());
    promisify = promisifyValue.As<v8::Function>();
  }

  v8::Local<v8::FunctionTemplate> funcTemplate;
  if(sFunctionTemplates.find(className) != sFunctionTemplates.end()) {
    //printf("existing className: %s\n", className.c_str());
    funcTemplate = Nan::New(*sFunctionTemplates[className]);
  } else {
    //printf("create className: %s\n", className.c_str());

    funcTemplate = Nan::New<v8::FunctionTemplate>();
    funcTemplate->InstanceTemplate()->SetInternalFieldCount(1);
    funcTemplate->SetClassName(Nan::New<v8::String>(className.c_str()).ToLocalChecked());

    std::list<jobject> methods;
    javaReflectionGetMethods(env, objClazz, &methods, false);
    jclass methodClazz = env->FindClass("java/lang/reflect/Method");
    jmethodID method_getName = env->GetMethodID(methodClazz, "getName", "()Ljava/lang/String;");
    for(std::list<jobject>::iterator it = methods.begin(); it != methods.end(); ++it) {
      jstring methodNameJava = (jstring)env->CallObjectMethod(*it, method_getName);
      assert(!env->ExceptionCheck());
      std::string methodNameStr = javaToString(env, methodNameJava);

      v8::Local<v8::String> baseMethodName = Nan::New<v8::String>(methodNameStr.c_str()).ToLocalChecked();

      v8::Local<v8::String> methodNameAsync = Nan::New<v8::String>((methodNameStr + java->AsyncSuffix()).c_str()).ToLocalChecked();
      v8::Local<v8::FunctionTemplate> methodCallTemplate = Nan::New<v8::FunctionTemplate>(methodCall, baseMethodName);
      funcTemplate->PrototypeTemplate()->Set(methodNameAsync, methodCallTemplate->GetFunction());

      v8::Local<v8::String> methodNameSync = Nan::New<v8::String>((methodNameStr + java->SyncSuffix()).c_str()).ToLocalChecked();
      v8::Local<v8::FunctionTemplate> methodCallSyncTemplate = Nan::New<v8::FunctionTemplate>(methodCallSync, baseMethodName);
      funcTemplate->PrototypeTemplate()->Set(methodNameSync, methodCallSyncTemplate->GetFunction());

      if (java->DoPromise()) {
        v8::Local<v8::Object> recv = Nan::New<v8::Object>();
        v8::Local<v8::Value> argv[] = { methodCallTemplate->GetFunction() };
        v8::Local<v8::Value> result = promisify->Call(recv, 1, argv);
        if (!result->IsFunction()) {
          fprintf(stderr, "Promisified result is not a function -- asyncOptions.promisify must return a function.\n");
          assert(result->IsFunction());
        }
        v8::Local<v8::Function> promFunction = result.As<v8::Function>();
        v8::Local<v8::String> methodNamePromise = Nan::New<v8::String>((methodNameStr + java->PromiseSuffix()).c_str()).ToLocalChecked();
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

      v8::Local<v8::String> fieldName = Nan::New<v8::String>(fieldNameStr.c_str()).ToLocalChecked();
      Nan::SetAccessor(funcTemplate->InstanceTemplate(), fieldName, fieldGetter, fieldSetter);
    }

    Nan::Persistent<v8::FunctionTemplate>* persistentFuncTemplate = new Nan::Persistent<v8::FunctionTemplate>();
    persistentFuncTemplate->Reset(funcTemplate);
    sFunctionTemplates[className] = persistentFuncTemplate;
  }

  v8::Local<v8::Function> ctor = funcTemplate->GetFunction();
  v8::Local<v8::Object> javaObjectObj = ctor->NewInstance();
  javaObjectObj->SetHiddenValue(Nan::New<v8::String>(V8_HIDDEN_MARKER_JAVA_OBJECT).ToLocalChecked(), Nan::New<v8::Boolean>(true));
  JavaObject *self = new JavaObject(java, obj);
  self->Wrap(javaObjectObj);

  return scope.Escape(javaObjectObj);
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
  Nan::HandleScope scope;
  JavaObject* self = Nan::ObjectWrap::Unwrap<JavaObject>(info.This());
  JNIEnv *env = self->m_java->getJavaEnv();
  JavaScope javaScope(env);

  v8::String::Utf8Value methodName(info.Data());
  std::string methodNameStr = *methodName;

  int argsStart = 0;
  int argsEnd = info.Length();

  // arguments
  ARGS_BACK_CALLBACK();

  if(!callbackProvided && methodNameStr == "toString") {
    return methodCallSync(info);
  }

  jobjectArray methodArgs = v8ToJava(env, info, argsStart, argsEnd);

  jobject method = javaFindMethod(env, self->m_class, methodNameStr, methodArgs);
  if(method == NULL) {
    std::string msg = methodNotFoundToString(env, self->m_class, methodNameStr, false, info, argsStart, argsEnd);
    EXCEPTION_CALL_CALLBACK(self->m_java, msg);
    info.GetReturnValue().SetUndefined();
    return;
  }

  // run
  InstanceMethodCallBaton* baton = new InstanceMethodCallBaton(self->m_java, self, method, methodArgs, callback);
  baton->run();

  END_CALLBACK_FUNCTION("\"Method '" << methodNameStr << "' called without a callback did you mean to use the Sync version?\"");
}

NAN_METHOD(JavaObject::methodCallSync) {
  Nan::HandleScope scope;
  JavaObject* self = Nan::ObjectWrap::Unwrap<JavaObject>(info.This());
  JNIEnv *env = self->m_java->getJavaEnv();
  JavaScope javaScope(env);

  v8::String::Utf8Value methodName(info.Data());
  std::string methodNameStr = *methodName;

  int argsStart = 0;
  int argsEnd = info.Length();

  jobjectArray methodArgs = v8ToJava(env, info, argsStart, argsEnd);

  jobject method = javaFindMethod(env, self->m_class, methodNameStr, methodArgs);
  if(method == NULL) {
    std::string msg = methodNotFoundToString(env, self->m_class, methodNameStr, false, info, argsStart, argsEnd);
    v8::Local<v8::Value> ex = javaExceptionToV8(self->m_java, env, msg);
    Nan::ThrowError(ex);
    return;
  }

  // run
  v8::Local<v8::Value> callback = Nan::Undefined();
  InstanceMethodCallBaton* baton = new InstanceMethodCallBaton(self->m_java, self, method, methodArgs, callback);
  v8::Local<v8::Value> result = baton->runSync();
  delete baton;

  if(result->IsNativeError()) {
    Nan::ThrowError(result);
    return;
  }

  info.GetReturnValue().Set(result);
}

NAN_GETTER(JavaObject::fieldGetter) {
  Nan::HandleScope scope;
  JavaObject* self = Nan::ObjectWrap::Unwrap<JavaObject>(info.This());
  JNIEnv *env = self->m_java->getJavaEnv();
  JavaScope javaScope(env);

  v8::String::Utf8Value propertyCStr(property);
  std::string propertyStr = *propertyCStr;
  jobject field = javaFindField(env, self->m_class, propertyStr);
  if(field == NULL) {
    std::ostringstream errStr;
    errStr << "Could not find field " << propertyStr;
    v8::Local<v8::Value> ex = javaExceptionToV8(self->m_java, env, errStr.str());
    Nan::ThrowError(ex);
    return;
  }

  jclass fieldClazz = env->FindClass("java/lang/reflect/Field");
  jmethodID field_get = env->GetMethodID(fieldClazz, "get", "(Ljava/lang/Object;)Ljava/lang/Object;");

  // get field value
  jobject val = env->CallObjectMethod(field, field_get, self->m_obj);
  if(env->ExceptionOccurred()) {
    std::ostringstream errStr;
    errStr << "Could not get field " << propertyStr;
    v8::Local<v8::Value> ex = javaExceptionToV8(self->m_java, env, errStr.str());
    Nan::ThrowError(ex);
    return;
  }

  v8::Local<v8::Value> result = javaToV8(self->m_java, env, val);

  info.GetReturnValue().Set(result);
}

NAN_SETTER(JavaObject::fieldSetter) {
  Nan::HandleScope scope;
  JavaObject* self = Nan::ObjectWrap::Unwrap<JavaObject>(info.This());
  JNIEnv *env = self->m_java->getJavaEnv();
  JavaScope javaScope(env);

  jobject newValue = v8ToJava(env, value);

  v8::String::Utf8Value propertyCStr(property);
  std::string propertyStr = *propertyCStr;
  jobject field = javaFindField(env, self->m_class, propertyStr);
  if(field == NULL) {
    std::ostringstream errStr;
    errStr << "Could not find field " << propertyStr;
    v8::Local<v8::Value> error = javaExceptionToV8(self->m_java, env, errStr.str());
    Nan::ThrowError(error);
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
    v8::Local<v8::Value> error = javaExceptionToV8(self->m_java, env, errStr.str());
    Nan::ThrowError(error);
    return;
  }
}

/*static*/ Nan::Persistent<v8::FunctionTemplate> JavaProxyObject::s_proxyCt;

/*static*/ void JavaProxyObject::init() {
  v8::Local<v8::FunctionTemplate> t = Nan::New<v8::FunctionTemplate>();
  s_proxyCt.Reset(t);
  t->InstanceTemplate()->SetInternalFieldCount(1);
  t->SetClassName(Nan::New<v8::String>("NodeDynamicProxy").ToLocalChecked());

  v8::Local<v8::String> methodName = Nan::New<v8::String>("unref").ToLocalChecked();
  v8::Local<v8::FunctionTemplate> methodCallTemplate = Nan::New<v8::FunctionTemplate>(doUnref);
  t->PrototypeTemplate()->Set(methodName, methodCallTemplate->GetFunction());

  v8::Local<v8::String> fieldName = Nan::New<v8::String>("invocationHandler").ToLocalChecked();
  Nan::SetAccessor(t->InstanceTemplate(), fieldName, invocationHandlerGetter);
}

v8::Local<v8::Object> JavaProxyObject::New(Java *java, jobject obj, DynamicProxyData* dynamicProxyData) {
  Nan::EscapableHandleScope scope;

  v8::Local<v8::Function> ctor = Nan::New(s_proxyCt)->GetFunction();
  v8::Local<v8::Object> javaObjectObj = ctor->NewInstance();
  javaObjectObj->SetHiddenValue(Nan::New<v8::String>(V8_HIDDEN_MARKER_JAVA_OBJECT).ToLocalChecked(), Nan::New<v8::Boolean>(true));
  JavaProxyObject *self = new JavaProxyObject(java, obj, dynamicProxyData);
  self->Wrap(javaObjectObj);

  return scope.Escape(javaObjectObj);
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
  JavaProxyObject* self = Nan::ObjectWrap::Unwrap<JavaProxyObject>(info.This());
  if (dynamicProxyDataVerify(self->m_dynamicProxyData)) {
    unref(self->m_dynamicProxyData);
  }
  info.GetReturnValue().SetUndefined();
}

NAN_GETTER(JavaProxyObject::invocationHandlerGetter) {
 Nan::HandleScope scope;

 JavaProxyObject* self = Nan::ObjectWrap::Unwrap<JavaProxyObject>(info.This());
 if (!dynamicProxyDataVerify(self->m_dynamicProxyData)) {
   Nan::ThrowError("dynamicProxyData has been destroyed or corrupted");
   return;
 }
 info.GetReturnValue().Set(Nan::New(self->m_dynamicProxyData->functions));
}
