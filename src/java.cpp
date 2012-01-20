
#include "java.h"
#include <string.h>
#include "javaObject.h"
#include "methodCallBaton.h"
#include <sstream>

/*static*/ v8::Persistent<v8::FunctionTemplate> Java::s_ct;

/*static*/ void Java::Init(v8::Handle<v8::Object> target) {
  v8::HandleScope scope;

  v8::Local<v8::FunctionTemplate> t = v8::FunctionTemplate::New(New);
  s_ct = v8::Persistent<v8::FunctionTemplate>::New(t);
  s_ct->InstanceTemplate()->SetInternalFieldCount(1);
  s_ct->SetClassName(v8::String::NewSymbol("Java"));

  NODE_SET_PROTOTYPE_METHOD(s_ct, "newInstance", newInstance);
  NODE_SET_PROTOTYPE_METHOD(s_ct, "newInstanceSync", newInstanceSync);
  NODE_SET_PROTOTYPE_METHOD(s_ct, "callStaticMethod", callStaticMethod);
  NODE_SET_PROTOTYPE_METHOD(s_ct, "callStaticMethodSync", callStaticMethodSync);
  NODE_SET_PROTOTYPE_METHOD(s_ct, "newArray", newArray);
  NODE_SET_PROTOTYPE_METHOD(s_ct, "newByte", newByte);

  target->Set(v8::String::NewSymbol("Java"), s_ct->GetFunction());
}

/*static*/ v8::Handle<v8::Value> Java::New(const v8::Arguments& args) {
  v8::HandleScope scope;

  Java *self = new Java();
  self->Wrap(args.This());

  self->handle_->Set(v8::String::New("classpath"), v8::Array::New());

  return args.This();
}

Java::Java() {
  this->m_jvm = NULL;
  this->m_env = NULL;
}

Java::~Java() {

}

v8::Handle<v8::Value> Java::ensureJvm() {
  if(!m_jvm) {
    return createJVM(&this->m_jvm, &this->m_env);
  }

  return v8::Undefined();
}

v8::Handle<v8::Value> Java::createJVM(JavaVM** jvm, JNIEnv** env) {
  JavaVM* jvmTemp;
  JavaVMInitArgs args;

  std::ostringstream classPath;
  classPath << "-Djava.class.path=";

  v8::Local<v8::Value> classPathValue = handle_->Get(v8::String::New("classpath"));
  if(!classPathValue->IsArray()) {
    return ThrowException(v8::Exception::TypeError(v8::String::New("Classpath must be an array")));
  }
  v8::Local<v8::Array> classPathArray = v8::Array::Cast(*classPathValue);
  for(uint32_t i=0; i<classPathArray->Length(); i++) {
    if(i != 0) {
      classPath << ":"; // TODO: figure out path seperator
    }
    v8::Local<v8::Value> arrayItemValue = classPathArray->Get(i);
    if(!arrayItemValue->IsString()) {
      return ThrowException(v8::Exception::TypeError(v8::String::New("Classpath must only contain strings")));
    }
    v8::Local<v8::String> arrayItem = arrayItemValue->ToString();
    v8::String::AsciiValue arrayItemStr(arrayItem);
    classPath << *arrayItemStr;
  }

  JavaVMOption options[1];
  options[0].optionString = strdup(classPath.str().c_str());

  JNI_GetDefaultJavaVMInitArgs(&args);
  args.version = JNI_VERSION_1_6;
  args.ignoreUnrecognized = false;
  args.options = options;
  args.nOptions = 1;
  JNI_CreateJavaVM(&jvmTemp, (void **)env, &args);
  *jvm = jvmTemp;

  return v8::Undefined();
}

/*static*/ v8::Handle<v8::Value> Java::newInstance(const v8::Arguments& args) {
  v8::HandleScope scope;
  Java* self = node::ObjectWrap::Unwrap<Java>(args.This());
  v8::Handle<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsUndefined()) {
    return ensureJvmResults;
  }
  JNIEnv* env = self->getJavaEnv();

  int argsEnd = args.Length();

  // argument - className
  if(args.Length() < 1 || !args[0]->IsString()) {
    return ThrowException(v8::Exception::TypeError(v8::String::New("Argument 0 must be a string")));
  }
  v8::Local<v8::String> classNameObj = v8::Local<v8::String>::Cast(args[0]);
  v8::String::AsciiValue classNameVal(classNameObj);
  std::string className = *classNameVal;

  // argument - callback
  v8::Handle<v8::Value> callback;
  if(args[args.Length()-1]->IsFunction()) {
    callback = args[argsEnd-1];
    argsEnd--;
  } else {
    callback = v8::Null();
  }

	std::list<int> methodArgTypes;
  jarray methodArgs = v8ToJava(env, args, 1, argsEnd, &methodArgTypes);

  jclass clazz = javaFindClass(env, className);
  if(clazz == NULL) {
    std::ostringstream errStr;
    errStr << "Could not create class " << className.c_str();
    v8::Handle<v8::Value> error = javaExceptionToV8(env, errStr.str());

    v8::Handle<v8::Value> argv[2];
    argv[0] = error;
    argv[1] = v8::Undefined();
    v8::Function::Cast(*callback)->Call(v8::Context::GetCurrent()->Global(), 2, argv);
    return v8::Undefined();
  }
  std::list<jobject> constructors = javaReflectionGetConstructors(env, clazz);
  jobject method = javaFindBestMatchingConstructor(env, constructors, methodArgTypes);

  // run
  NewInstanceBaton* baton = new NewInstanceBaton(self, clazz, method, methodArgs, callback);
  baton->run();

  return v8::Undefined();
}

/*static*/ v8::Handle<v8::Value> Java::newInstanceSync(const v8::Arguments& args) {
  v8::HandleScope scope;
  Java* self = node::ObjectWrap::Unwrap<Java>(args.This());
  v8::Handle<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsUndefined()) {
    return ensureJvmResults;
  }
  JNIEnv* env = self->getJavaEnv();

  int argsEnd = args.Length();

  // argument - className
  if(args.Length() < 1 || !args[0]->IsString()) {
    return ThrowException(v8::Exception::TypeError(v8::String::New("Argument 0 must be a string")));
  }
  v8::Local<v8::String> classNameObj = v8::Local<v8::String>::Cast(args[0]);
  v8::String::AsciiValue classNameVal(classNameObj);
  std::string className = *classNameVal;

	std::list<int> methodArgTypes;
  jarray methodArgs = v8ToJava(env, args, 1, argsEnd, &methodArgTypes);

  jclass clazz = javaFindClass(env, className);
  if(clazz == NULL) {
    std::ostringstream errStr;
    errStr << "Could not create class " << className.c_str();
    return ThrowException(javaExceptionToV8(env, errStr.str()));
  }
  std::list<jobject> constructors = javaReflectionGetConstructors(env, clazz);
  jobject method = javaFindBestMatchingConstructor(env, constructors, methodArgTypes);

  // run
  v8::Handle<v8::Value> callback = v8::Object::New();
  NewInstanceBaton* baton = new NewInstanceBaton(self, clazz, method, methodArgs, callback);
  v8::Handle<v8::Value> result = baton->runSync();
  delete baton;
  if(result->IsNativeError()) {
    return ThrowException(result);
  }
  return scope.Close(result);
}

/*static*/ v8::Handle<v8::Value> Java::callStaticMethod(const v8::Arguments& args) {
  v8::HandleScope scope;
  Java* self = node::ObjectWrap::Unwrap<Java>(args.This());
  v8::Handle<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsUndefined()) {
    return ensureJvmResults;
  }
  JNIEnv* env = self->getJavaEnv();

  int argsEnd = args.Length();

  // argument - className
  if(args.Length() < 1 || !args[0]->IsString()) {
    return ThrowException(v8::Exception::TypeError(v8::String::New("Argument 0 must be a string")));
  }
  v8::Local<v8::String> classNameObj = v8::Local<v8::String>::Cast(args[0]);
  v8::String::AsciiValue classNameVal(classNameObj);
  std::string className = *classNameVal;

  // argument - method name
  if(args.Length() < 2 || !args[1]->IsString()) {
    return ThrowException(v8::Exception::TypeError(v8::String::New("Argument 1 must be a string")));
  }
  v8::Local<v8::String> methodNameObj = v8::Local<v8::String>::Cast(args[1]);
  v8::String::AsciiValue methodNameVal(methodNameObj);
  std::string methodName = *methodNameVal;

  // argument - callback
  v8::Handle<v8::Value> callback;
  if(args[args.Length()-1]->IsFunction()) {
    callback = args[argsEnd-1];
    argsEnd--;
  } else {
    callback = v8::Null();
  }

  // build args
	std::list<int> methodArgTypes;
  jarray methodArgs = v8ToJava(env, args, 2, argsEnd, &methodArgTypes);

  // find class and method
  jclass clazz = javaFindClass(env, className);
  if(clazz == NULL) {
    std::ostringstream errStr;
    errStr << "Could not create class " << className.c_str();
    v8::Handle<v8::Value> error = javaExceptionToV8(env, errStr.str());

    v8::Handle<v8::Value> argv[2];
    argv[0] = error;
    argv[1] = v8::Undefined();
    v8::Function::Cast(*callback)->Call(v8::Context::GetCurrent()->Global(), 2, argv);
    return v8::Undefined();
  }
  std::list<jobject> staticMethods = javaReflectionGetStaticMethods(env, clazz);
  jobject method = javaFindBestMatchingMethod(env, staticMethods, methodName.c_str(), methodArgTypes);
  if(method == NULL) {
    std::ostringstream errStr;
    errStr << "Could not find method \"" << methodName.c_str() << "\"";
    v8::Handle<v8::Value> error = javaExceptionToV8(env, errStr.str());

    v8::Handle<v8::Value> argv[2];
    argv[0] = error;
    argv[1] = v8::Undefined();
    v8::Function::Cast(*callback)->Call(v8::Context::GetCurrent()->Global(), 2, argv);
    return v8::Undefined();
  }

  // run
  StaticMethodCallBaton* baton = new StaticMethodCallBaton(self, clazz, method, methodArgs, callback);
  baton->run();

  return v8::Undefined();
}

/*static*/ v8::Handle<v8::Value> Java::callStaticMethodSync(const v8::Arguments& args) {
  v8::HandleScope scope;
  Java* self = node::ObjectWrap::Unwrap<Java>(args.This());
  v8::Handle<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsUndefined()) {
    return ensureJvmResults;
  }
  JNIEnv* env = self->getJavaEnv();

  int argsEnd = args.Length();

  // argument - className
  if(args.Length() < 1 || !args[0]->IsString()) {
    return ThrowException(v8::Exception::TypeError(v8::String::New("Argument 0 must be a string")));
  }
  v8::Local<v8::String> classNameObj = v8::Local<v8::String>::Cast(args[0]);
  v8::String::AsciiValue classNameVal(classNameObj);
  std::string className = *classNameVal;

  // argument - method name
  if(args.Length() < 2 || !args[1]->IsString()) {
    return ThrowException(v8::Exception::TypeError(v8::String::New("Argument 1 must be a string")));
  }
  v8::Local<v8::String> methodNameObj = v8::Local<v8::String>::Cast(args[1]);
  v8::String::AsciiValue methodNameVal(methodNameObj);
  std::string methodName = *methodNameVal;

  // build args
	std::list<int> methodArgTypes;
  jarray methodArgs = v8ToJava(env, args, 2, argsEnd, &methodArgTypes);

  // find class and method
  jclass clazz = javaFindClass(env, className);
  if(clazz == NULL) {
    std::ostringstream errStr;
    errStr << "Could not create class " << className.c_str();
    return ThrowException(javaExceptionToV8(env, errStr.str()));
  }
  std::list<jobject> staticMethods = javaReflectionGetStaticMethods(env, clazz);
  jobject method = javaFindBestMatchingMethod(env, staticMethods, methodName.c_str(), methodArgTypes);
  if(method == NULL) {
    std::ostringstream errStr;
    errStr << "Could not find method \"" << methodName.c_str() << "\"";
    return ThrowException(javaExceptionToV8(env, errStr.str()));
  }

  // run
  v8::Handle<v8::Value> callback = v8::Object::New();
  StaticMethodCallBaton* baton = new StaticMethodCallBaton(self, clazz, method, methodArgs, callback);
  v8::Handle<v8::Value> result = baton->runSync();
  delete baton;
  if(result->IsNativeError()) {
    return ThrowException(result);
  }
  return scope.Close(result);
}

/*static*/ v8::Handle<v8::Value> Java::newArray(const v8::Arguments& args) {
  v8::HandleScope scope;
  Java* self = node::ObjectWrap::Unwrap<Java>(args.This());
  v8::Handle<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsUndefined()) {
    return ensureJvmResults;
  }
  JNIEnv* env = self->getJavaEnv();

  // argument - className
  if(args.Length() < 1 || !args[0]->IsString()) {
    return ThrowException(v8::Exception::TypeError(v8::String::New("Argument 0 must be a string")));
  }
  v8::Local<v8::String> classNameObj = v8::Local<v8::String>::Cast(args[0]);
  v8::String::AsciiValue classNameVal(classNameObj);
  std::string className = *classNameVal;

  // argument - array
  if(args.Length() < 2 || !args[1]->IsArray()) {
    return ThrowException(v8::Exception::TypeError(v8::String::New("Argument 1 must be an array")));
  }
  v8::Local<v8::Array> arrayObj = v8::Local<v8::Array>::Cast(args[1]);

  // find class and method
  jarray results;
  if(strcmp(className.c_str(), "byte") == 0) {
    results = env->NewByteArray(arrayObj->Length());
    for(uint32_t i=0; i<arrayObj->Length(); i++) {
      int methodArgType;
      v8::Local<v8::Value> item = arrayObj->Get(i);
      jobject val = v8ToJava(env, item, &methodArgType);
      jclass byteClazz = env->FindClass("java/lang/Byte");
      jmethodID byte_byteValue = env->GetMethodID(byteClazz, "byteValue", "()B");
      jbyte byteValues[1];
      byteValues[0] = env->CallByteMethod(val, byte_byteValue);
      env->SetByteArrayRegion((jbyteArray)results, i, 1, byteValues);
    }
  }

  else
  {
    jclass clazz = javaFindClass(env, className);
    if(clazz == NULL) {
      std::ostringstream errStr;
      errStr << "Could not create class " << className.c_str();
      return ThrowException(javaExceptionToV8(env, errStr.str()));
    }

    // create array
    results = env->NewObjectArray(arrayObj->Length(), clazz, NULL);

    for(uint32_t i=0; i<arrayObj->Length(); i++) {
      int methodArgType;
      v8::Local<v8::Value> item = arrayObj->Get(i);
      jobject val = v8ToJava(env, item, &methodArgType);
      env->SetObjectArrayElement((jobjectArray)results, i, val);
      if(env->ExceptionOccurred()) {
        std::ostringstream errStr;
        v8::String::AsciiValue valStr(item);
        errStr << "Could not add item \"" << *valStr << "\" to array.";
        return ThrowException(javaExceptionToV8(env, errStr.str()));
      }
    }
  }

  return scope.Close(JavaObject::New(self, results));
}

/*static*/ v8::Handle<v8::Value> Java::newByte(const v8::Arguments& args) {
  v8::HandleScope scope;
  Java* self = node::ObjectWrap::Unwrap<Java>(args.This());
  v8::Handle<v8::Value> ensureJvmResults = self->ensureJvm();
  if(!ensureJvmResults->IsUndefined()) {
    return ensureJvmResults;
  }
  JNIEnv* env = self->getJavaEnv();

  if(args.Length() != 1) {
    return ThrowException(v8::Exception::TypeError(v8::String::New("newByte only takes 1 argument")));
  }

  // argument - value
  if(!args[0]->IsNumber()) {
    return ThrowException(v8::Exception::TypeError(v8::String::New("Argument 0 must be a number")));
  }

  v8::Local<v8::Number> val = args[0]->ToNumber();

  jclass clazz = env->FindClass("java/lang/Byte");
  jmethodID constructor = env->GetMethodID(clazz, "<init>", "(B)V");
  jobject newObj = env->NewObject(clazz, constructor, (jbyte)val->Value());

  return scope.Close(JavaObject::New(self, newObj));
}
