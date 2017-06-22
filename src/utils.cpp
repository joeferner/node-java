#include "utils.h"
#include <string.h>
#include <algorithm>
#include <sstream>
#include <set>
#include "javaObject.h"
#include "java.h"

#define MODIFIER_STATIC 9

jobject v8ToJava_javaObject(JNIEnv* env, v8::Local<v8::Object> obj);
jobject v8ToJava_javaLong(JNIEnv* env, v8::Local<v8::Object> obj);

void javaReflectionGetMethods(JNIEnv *env, jclass clazz, std::list<jobject>* methods, bool includeStatic) {
  jclass clazzclazz = env->FindClass("java/lang/Class");
  jmethodID clazz_getMethods = env->GetMethodID(clazzclazz, "getMethods", "()[Ljava/lang/reflect/Method;");
  jclass methodClazz = env->FindClass("java/lang/reflect/Method");
  jmethodID method_getModifiers = env->GetMethodID(methodClazz, "getModifiers", "()I");

  jobjectArray methodObjects = (jobjectArray)env->CallObjectMethod(clazz, clazz_getMethods);
  checkJavaException(env);
  jsize methodCount = env->GetArrayLength(methodObjects);
  for(jsize i=0; i<methodCount; i++) {
    jobject method = env->GetObjectArrayElement(methodObjects, i);
    jint methodModifiers = env->CallIntMethod(method, method_getModifiers);
    assertNoException(env);
    if(!includeStatic && (methodModifiers & MODIFIER_STATIC) == MODIFIER_STATIC) {
      continue;
    }
    methods->push_back(method);
  }
}

void javaReflectionGetConstructors(JNIEnv *env, jclass clazz, std::list<jobject>* methods) {
  jclass clazzclazz = env->FindClass("java/lang/Class");
  jmethodID clazz_getConstructors = env->GetMethodID(clazzclazz, "getConstructors", "()[Ljava/lang/reflect/Constructor;");

  jobjectArray constructorObjects = (jobjectArray)env->CallObjectMethod(clazz, clazz_getConstructors);
  checkJavaException(env);
  jsize constructorCount = env->GetArrayLength(constructorObjects);
  for(jsize i=0; i<constructorCount; i++) {
    jobject constructor = env->GetObjectArrayElement(constructorObjects, i);
    methods->push_back(constructor);
  }
}

void javaReflectionGetFields(JNIEnv *env, jclass clazz, std::list<jobject>* fields) {
  jclass clazzclazz = env->FindClass("java/lang/Class");
  jmethodID clazz_getFields = env->GetMethodID(clazzclazz, "getFields", "()[Ljava/lang/reflect/Field;");
  jclass fieldClazz = env->FindClass("java/lang/reflect/Field");
  jmethodID field_getModifiers = env->GetMethodID(fieldClazz, "getModifiers", "()I");

  jobjectArray fieldObjects = (jobjectArray)env->CallObjectMethod(clazz, clazz_getFields);
  assertNoException(env);
  jsize fieldCount = env->GetArrayLength(fieldObjects);
  for(jsize i=0; i<fieldCount; i++) {
    jobject field = env->GetObjectArrayElement(fieldObjects, i);
    jint fieldModifiers = env->CallIntMethod(field, field_getModifiers);
    checkJavaException(env);
    if((fieldModifiers & MODIFIER_STATIC) == MODIFIER_STATIC) {
      continue;
    }
    fields->push_back(field);
  }
}

std::string javaToString(JNIEnv *env, jstring str) {
  jclass objClazz = env->GetObjectClass(str);
  jmethodID methodId = env->GetMethodID(objClazz, "getBytes", "(Ljava/lang/String;)[B");

  jstring charsetName = env->NewStringUTF("UTF-8");
  jbyteArray stringJbytes = (jbyteArray)env->CallObjectMethod(str, methodId, charsetName);
  env->DeleteLocalRef(charsetName);

  jbyte* pBytes = env->GetByteArrayElements(stringJbytes, NULL);

  const jsize length = env->GetArrayLength(stringJbytes);
  std::string results((const char*)pBytes, length);

  env->ReleaseByteArrayElements(stringJbytes, pBytes, JNI_ABORT);
  env->DeleteLocalRef(stringJbytes);

  return results;
}

std::string javaArrayToString(JNIEnv *env, jobjectArray arr) {
  if(arr == NULL) {
    return "(null)";
  }

  std::ostringstream result;
  result << "[";
  jsize count = env->GetArrayLength(arr);
  for(jsize i=0; i<count; i++) {
    if(i != 0) {
      result << ", ";
    }
    jobject obj = env->GetObjectArrayElement(arr, i);
    result << javaObjectToString(env, obj);
  }
  result << "]";
  return result.str();
}

std::string javaObjectToString(JNIEnv *env, jobject obj) {
  if(obj == NULL) {
    return "(null)";
  }
  jclass objClazz = env->GetObjectClass(obj);
  jmethodID methodId = env->GetMethodID(objClazz, "toString", "()Ljava/lang/String;");
  jstring result = (jstring)env->CallObjectMethod(obj, methodId);
  assertNoException(env);
  return javaToString(env, result);
}

std::string javaMethodCallToString(JNIEnv *env, jobject obj, jmethodID methodId, jarray args) {
  char temp[100];

  std::ostringstream result;
  sprintf(temp, "%p", env);
  result << temp;
  result << ": ";
  result << javaObjectToString(env, obj);
  result << ": ";
  sprintf(temp, "%p", methodId);
  result << temp;
  result << ": (";
  jsize arraySize = env->GetArrayLength(args);
  for(int i=0; i<arraySize; i++) {
    if(i != 0) {
      result << ", ";
    }
    jobject arg = env->GetObjectArrayElement((jobjectArray)args, i);
    result << javaObjectToString(env, arg);
  }
  result << ")";

  return result.str();
}

JNIEnv* javaGetEnv(JavaVM* jvm, jobject classLoader) {
  JNIEnv *env = NULL;
  int ret = jvm->GetEnv((void**)&env, JNI_BEST_VERSION);

  if (ret == JNI_EDETACHED) {
    JavaVMAttachArgs attachArgs;
    attachArgs.version = JNI_BEST_VERSION;
    attachArgs.name = NULL;
    attachArgs.group = NULL;
    jvm->AttachCurrentThread((void**)&env, &attachArgs);

    jclass threadClazz = env->FindClass("java/lang/Thread");
    jmethodID thread_currentThread = env->GetStaticMethodID(threadClazz, "currentThread", "()Ljava/lang/Thread;");
    jmethodID thread_setContextClassLoader = env->GetMethodID(threadClazz, "setContextClassLoader", "(Ljava/lang/ClassLoader;)V");
    jobject currentThread = env->CallStaticObjectMethod(threadClazz, thread_currentThread);
    checkJavaException(env);
    env->CallObjectMethod(currentThread, thread_setContextClassLoader, classLoader);
    assertNoException(env);

    env->DeleteLocalRef(threadClazz);
    env->DeleteLocalRef(currentThread);
  }

  return env;
}

jobject getSystemClassLoader(JNIEnv *env) {
  jclass threadClazz = env->FindClass("java/lang/Thread");
  jmethodID thread_currentThread = env->GetStaticMethodID(threadClazz, "currentThread", "()Ljava/lang/Thread;");
  jmethodID thread_getContextClassLoader = env->GetMethodID(threadClazz, "getContextClassLoader", "()Ljava/lang/ClassLoader;");
  jobject currentThread = env->CallStaticObjectMethod(threadClazz, thread_currentThread);
  checkJavaException(env);
  jobject result = env->CallObjectMethod(currentThread, thread_getContextClassLoader);
  checkJavaException(env);
  return result;
}

jvalueType javaGetType(JNIEnv *env, jclass type) {
  jclass clazzClazz = env->FindClass("java/lang/Class");
  jmethodID class_isArray = env->GetMethodID(clazzClazz, "isArray", "()Z");

  jboolean isArray = env->CallBooleanMethod(type, class_isArray);
  assertNoException(env);
  if(isArray) {
    return TYPE_ARRAY;
  } else {
    // TODO: has to be a better way
    std::string str = javaObjectToString(env, type);
    const char *typeStr = str.c_str();
    //printf("javaGetType: %s\n", typeStr);
    if(strcmp(typeStr, "void") == 0) {
      return TYPE_VOID;
    } else if(strcmp(typeStr, "char") == 0 || strcmp(typeStr, "class java.lang.Character") == 0) {
      return TYPE_CHAR;
    } else if(strcmp(typeStr, "int") == 0 || strcmp(typeStr, "class java.lang.Integer") == 0) {
      return TYPE_INT;
    } else if(strcmp(typeStr, "double") == 0 || strcmp(typeStr, "class java.lang.Double") == 0) {
      return TYPE_DOUBLE;
    } else if(strcmp(typeStr, "float") == 0 || strcmp(typeStr, "class java.lang.Float") == 0) {
      return TYPE_FLOAT;
    } else if(strcmp(typeStr, "long") == 0 || strcmp(typeStr, "class java.lang.Long") == 0) {
      return TYPE_LONG;
    } else if(strcmp(typeStr, "boolean") == 0 || strcmp(typeStr, "class java.lang.Boolean") == 0) {
      return TYPE_BOOLEAN;
    } else if(strcmp(typeStr, "short") == 0 || strcmp(typeStr, "class java.lang.Short") == 0) {
      return TYPE_SHORT;
    } else if(strcmp(typeStr, "byte") == 0 || strcmp(typeStr, "class java.lang.Byte") == 0) {
      return TYPE_BYTE;
    } else if(strcmp(typeStr, "class java.lang.String") == 0) {
      return TYPE_STRING;
    }
    return TYPE_OBJECT;
  }
}

jclass javaFindClass(JNIEnv* env, std::string& className) {
  std::string searchClassName = className;
  std::replace(searchClassName.begin(), searchClassName.end(), '.', '/');

// Alternate find class trying to fix Class.forName
//  jclass threadClazz = env->FindClass("java/lang/Thread");
//  jmethodID thread_getCurrentThread = env->GetStaticMethodID(threadClazz, "currentThread", "()Ljava/lang/Thread;");
//  jmethodID thread_getContextClassLoader = env->GetMethodID(threadClazz, "getContextClassLoader", "()Ljava/lang/ClassLoader;");
//
//  jclass classLoaderClazz = env->FindClass("java/lang/ClassLoader");
//  jmethodID classLoader_loadClass = env->GetMethodID(classLoaderClazz, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
//
//  jobject currentThread = env->CallObjectMethod(threadClazz, thread_getCurrentThread);
//  jobject classLoader = env->CallObjectMethod(currentThread, thread_getContextClassLoader);
//  jstring searchClassNameJava = env->NewStringUTF(className.c_str());
//  jclass clazz = (jclass)env->CallObjectMethod(classLoader, classLoader_loadClass, searchClassNameJava);

  jclass clazz = env->FindClass(searchClassName.c_str());
  return clazz;
}

jobject javaFindField(JNIEnv* env, jclass clazz, std::string& fieldName) {
  jobject result = NULL;
  jclass clazzclazz = env->GetObjectClass(clazz);
  jclass fieldClazz = env->FindClass("java/lang/reflect/Field");
  jmethodID field_getName = env->GetMethodID(fieldClazz, "getName", "()Ljava/lang/String;");
  jmethodID class_getFields = env->GetMethodID(clazzclazz, "getFields", "()[Ljava/lang/reflect/Field;");
  jobjectArray fieldObjects = (jobjectArray)env->CallObjectMethod(clazz, class_getFields);
  checkJavaException(env);

  jsize fieldCount = env->GetArrayLength(fieldObjects);
  for(jsize i=0; i<fieldCount; i++) {
    jobject field = env->GetObjectArrayElement(fieldObjects, i);
    jstring fieldNameJava = (jstring)env->CallObjectMethod(field, field_getName);
    checkJavaException(env);

    std::string itFieldName = javaToString(env, fieldNameJava);
    if(strcmp(itFieldName.c_str(), fieldName.c_str()) == 0) {
      result = field;
      break;
    }
  }

  return result;
}

const std::string kObject("java/lang/Object");
const std::string kString("java/lang/String");
const std::string kInteger("java/lang/Integer");
const std::string kNumber("java/lang/Number");
const std::string kLong("java/lang/Long");
const std::string kDouble("java/lang/Double");
const std::string kBoolean("java/lang/Boolean");

static std::string getArrayElementType(v8::Local<v8::Array> array, uint32_t arraySize) {
  std::set<std::string> types;

  if (arraySize == 0) {
    return kObject;
  }

  for(uint32_t i=0; i<arraySize; i++) {
    v8::Local<v8::Value> arg = array->Get(i);
    if (arg->IsArray()) {
      return kObject; // We can exit as soon as we know java/lang/Object is required.
    }
    else if(arg->IsString()) {
      types.insert(kString);
    }
    else if(arg->IsInt32() || arg->IsUint32()) {
      types.insert(kInteger);
    }
    else if(arg->IsNumber()) {
      types.insert(kDouble);
    }
    else if(arg->IsBoolean()) {
      types.insert(kBoolean);
    }
    else if(arg->IsObject()) {
      v8::Local<v8::Object> obj = v8::Local<v8::Object>::Cast(arg);
      v8::Local<v8::Value> isJavaLong = GetHiddenValue(obj, Nan::New<v8::String>(V8_HIDDEN_MARKER_JAVA_LONG).ToLocalChecked());
      if(!isJavaLong.IsEmpty() && isJavaLong->IsBoolean()) {
        types.insert(kLong);
      }
      else {
        return kObject; // We can exit as soon as we know java/lang/Object is required.
      }
    }
  }

  if(types.size() == 1) {
    return *(types.begin());
  }

  assert(types.size() >= 1);
  assert(types.find(kObject)==types.end());

  // We have an array with two or more types. All types can be converted to Object, but there is one other
  // case we support, which is when all the types are numeric types.
  // We currently have only two non-numeric types. If neither is present in the set, the rest must be numeric.
  if (types.find(kString)==types.end() && types.find(kBoolean)==types.end()) {
    return kNumber;
  }

  return kObject;
}

jobject v8ToJava(JNIEnv* env, v8::Local<v8::Value> arg) {
  if(arg.IsEmpty() || arg->IsNull() || arg->IsUndefined()) {
    return NULL;
  }

  if(arg->IsArray()) {
    v8::Local<v8::Array> array = v8::Local<v8::Array>::Cast(arg);
    uint32_t arraySize = array->Length();
    std::string arrayType = getArrayElementType(array, arraySize);
    jclass objectClazz = env->FindClass(arrayType.c_str());
    jobjectArray result = env->NewObjectArray(arraySize, objectClazz, NULL);
    for(uint32_t i=0; i<arraySize; i++) {
      jobject val = v8ToJava(env, array->Get(i));
      env->SetObjectArrayElement(result, i, val);
    }
    return result;
  }

  if(arg->IsString()) {
    v8::String::Value val(arg->ToString());
    return env->NewString(*val, val.length());
  }

  if(arg->IsInt32() || arg->IsUint32()) {
    jint val = Nan::To<int32_t>(arg).FromJust();
    jclass clazz = env->FindClass("java/lang/Integer");
    jmethodID constructor = env->GetMethodID(clazz, "<init>", "(I)V");
    return env->NewObject(clazz, constructor, val);
  }

  if(arg->IsNumber()) {
    jdouble val = Nan::To<double>(arg).FromJust();
    jclass clazz = env->FindClass("java/lang/Double");
    jmethodID constructor = env->GetMethodID(clazz, "<init>", "(D)V");
    return env->NewObject(clazz, constructor, val);
  }

  if(arg->IsBoolean()) {
    jboolean val = Nan::To<bool>(arg).FromJust();
    jclass clazz = env->FindClass("java/lang/Boolean");
    jmethodID constructor = env->GetMethodID(clazz, "<init>", "(Z)V");
    return env->NewObject(clazz, constructor, val);
  }

  if(arg->IsObject()) {
    v8::Local<v8::Object> obj = v8::Local<v8::Object>::Cast(arg);

    v8::Local<v8::Value> isJavaObject = GetHiddenValue(obj, Nan::New<v8::String>(V8_HIDDEN_MARKER_JAVA_OBJECT).ToLocalChecked());
    if(!isJavaObject.IsEmpty() && isJavaObject->IsBoolean()) {
      return v8ToJava_javaObject(env, obj);
    }

    v8::Local<v8::Value> isJavaLong = GetHiddenValue(obj, Nan::New<v8::String>(V8_HIDDEN_MARKER_JAVA_LONG).ToLocalChecked());
    if(!isJavaLong.IsEmpty() && isJavaLong->IsBoolean()) {
      return v8ToJava_javaLong(env, obj);
    }
  }

  // TODO: handle other arg types. Don't print here, see instanceof-test#non-java object
  // v8::String::AsciiValue typeStr(arg);
  // printf("v8ToJava: Unhandled type: %s\n", *typeStr);
  return NULL;
}

jobject v8ToJava_javaObject(JNIEnv* env, v8::Local<v8::Object> obj) {
  JavaObject* javaObject = Nan::ObjectWrap::Unwrap<JavaObject>(obj);
  return javaObject->getObject();
}

void checkJavaException(JNIEnv* env) {
  if(env->ExceptionCheck()) {
    jthrowable ex = env->ExceptionOccurred();
    env->ExceptionClear();

    std::string exString = javaExceptionToString(env, ex);
    printf("%s\n", exString.c_str());
    assert(false);
  }
}

jobject v8ToJava_javaLong(JNIEnv* env, v8::Local<v8::Object> obj) {
  jobject longValue = v8ToJava(env, obj->Get(Nan::New<v8::String>("longValue").ToLocalChecked()));
  jclass longClazz = env->FindClass("java/lang/Long");
  jmethodID long_constructor = env->GetMethodID(longClazz, "<init>", "(Ljava/lang/String;)V");
  jobject jobj = env->NewObject(longClazz, long_constructor, longValue);
  return jobj;
}

jobjectArray v8ToJava(JNIEnv* env, Nan::NAN_METHOD_ARGS_TYPE args, int start, int end) {
  jclass clazz = env->FindClass("java/lang/Object");
  jobjectArray results = env->NewObjectArray(end-start, clazz, NULL);

  for(int i=start; i<end; i++) {
    jobject val = v8ToJava(env, args[i]);
    env->SetObjectArrayElement(results, i - start, val);
  }

  return results;
}

std::string javaExceptionToString(JNIEnv* env, jthrowable ex) {
  jclass stringWriterClazz = env->FindClass("java/io/StringWriter");
  jmethodID stringWriter_constructor = env->GetMethodID(stringWriterClazz, "<init>", "()V");
  jmethodID stringWriter_toString = env->GetMethodID(stringWriterClazz, "toString", "()Ljava/lang/String;");
  jobject stringWriter = env->NewObject(stringWriterClazz, stringWriter_constructor);

  jclass printWriterClazz = env->FindClass("java/io/PrintWriter");
  jmethodID printWriter_constructor = env->GetMethodID(printWriterClazz, "<init>", "(Ljava/io/Writer;)V");
  jobject printWriter = env->NewObject(printWriterClazz, printWriter_constructor, stringWriter);

  jclass throwableClazz = env->FindClass("java/lang/Throwable");
  jmethodID throwable_printStackTrace = env->GetMethodID(throwableClazz, "printStackTrace", "(Ljava/io/PrintWriter;)V");
  env->CallObjectMethod(ex, throwable_printStackTrace, printWriter);
  checkJavaException(env);
  jstring strObj = (jstring)env->CallObjectMethod(stringWriter, stringWriter_toString);
  checkJavaException(env);

  return javaToString(env, strObj);
}

v8::Local<v8::Value> javaExceptionToV8(Java* java, JNIEnv* env, jthrowable ex, const std::string& alternateMessage) {
  std::ostringstream msg;
  msg << alternateMessage;

  if(ex) {
    msg << "\n" << javaExceptionToString(env, ex);

    v8::Local<v8::Value> v8ex = v8::Exception::Error(Nan::New<v8::String>(msg.str().c_str()).ToLocalChecked());
    ((v8::Object*)*v8ex)->Set(Nan::New<v8::String>("cause").ToLocalChecked(), javaToV8(java, env, ex));
    return v8ex;
  }

  return v8::Exception::Error(Nan::New<v8::String>(msg.str().c_str()).ToLocalChecked());
}

v8::Local<v8::Value> javaExceptionToV8(Java* java, JNIEnv* env, const std::string& alternateMessage) {
  jthrowable ex = env->ExceptionOccurred();
  env->ExceptionClear();
  return javaExceptionToV8(java, env, ex, alternateMessage);
}

jvalueType javaGetArrayComponentType(JNIEnv *env, jobjectArray array) {
  jclass objectClazz = env->FindClass("java/lang/Object");
  jclass clazzclazz = env->FindClass("java/lang/Class");

  jmethodID object_getClass = env->GetMethodID(objectClazz, "getClass", "()Ljava/lang/Class;");
  jobject arrayClass = env->CallObjectMethod(array, object_getClass);
  assertNoException(env);

  jmethodID class_getComponentType = env->GetMethodID(clazzclazz, "getComponentType", "()Ljava/lang/Class;");
  jobject arrayComponentTypeClass = env->CallObjectMethod(arrayClass, class_getComponentType);
  checkJavaException(env);

  jvalueType arrayComponentType = javaGetType(env, (jclass)arrayComponentTypeClass);
  return arrayComponentType;
}

#if (NODE_VERSION_AT_LEAST(4, 0, 0))
v8::Local<v8::ArrayBuffer> newArrayBuffer(void* elems, size_t length) {
  v8::Local<v8::ArrayBuffer> ab = v8::ArrayBuffer::New(v8::Isolate::GetCurrent(), length);
  memcpy(ab->GetContents().Data(), elems, length);
  return ab;
}
#endif

v8::Local<v8::String> javaCharToV8String(jchar c) {
#if ((NODE_MAJOR_VERSION == 0) && (NODE_MINOR_VERSION <= 10))
  return v8::String::New(&c, 1);
#elif ((NODE_MAJOR_VERSION == 0) && (NODE_MINOR_VERSION <= 12))
  return v8::String::NewFromTwoByte(v8::Isolate::GetCurrent(), &c, v8::String::kNormalString, 1);
#else
  return v8::String::NewFromTwoByte(v8::Isolate::GetCurrent(), &c, v8::NewStringType::kNormal, 1).ToLocalChecked();
#endif
}

v8::Local<v8::Value> javaArrayToV8(Java* java, JNIEnv* env, jobjectArray objArray) {
  if(objArray == NULL) {
    return Nan::Null();
  }

  jvalueType arrayComponentType = javaGetArrayComponentType(env, objArray);
  //printf("javaArrayToV8: %d %s\n", arrayComponentType, javaObjectToString(env, objArray).c_str());

  jsize arraySize = env->GetArrayLength(objArray);
  //printf("array size: %d\n", arraySize);

  v8::Local<v8::Array> result = Nan::New<v8::Array>(arraySize);
  // http://docs.oracle.com/javase/7/docs/technotes/guides/jni/spec/types.html
  switch(arrayComponentType) {
  case TYPE_CHAR:
    {
      jchar* elems = env->GetCharArrayElements((jcharArray)objArray, 0);
#if (NODE_VERSION_AT_LEAST(4, 0, 0))
      size_t byteLength = arraySize * 2;
      v8::Local<v8::ArrayBuffer> ab = newArrayBuffer(elems, byteLength);
      env->ReleaseCharArrayElements((jcharArray)objArray, elems, 0);
      return v8::Uint16Array::New(ab, 0, arraySize);
#else
      jchar str;
      for(jsize i=0; i<arraySize; i++) {
        str = elems[i];
        result->Set(i, javaCharToV8String(str));
      }
      env->ReleaseCharArrayElements((jcharArray)objArray, elems, 0);
#endif
    }
    break;

  case TYPE_INT:
    {
      jint* elems = env->GetIntArrayElements((jintArray)objArray, 0);
#if (NODE_VERSION_AT_LEAST(4, 0, 0))
      size_t byteLength = arraySize * 4;
      v8::Local<v8::ArrayBuffer> ab = newArrayBuffer(elems, byteLength);
      env->ReleaseIntArrayElements((jintArray)objArray, elems, 0);
      return v8::Int32Array::New(ab, 0, arraySize);
#else
      for(jsize i=0; i<arraySize; i++) {
        result->Set(i, Nan::New<v8::Integer>(elems[i]));
      }
      env->ReleaseIntArrayElements((jintArray)objArray, elems, 0);
#endif
    }
    break;

  case TYPE_BYTE:
    {
      jbyte* elems = env->GetByteArrayElements((jbyteArray)objArray, 0);
#if (NODE_VERSION_AT_LEAST(4, 0, 0))
      size_t byteLength = arraySize;
      v8::Local<v8::ArrayBuffer> ab = newArrayBuffer(elems, byteLength);
      env->ReleaseByteArrayElements((jbyteArray)objArray, elems, 0);
      return v8::Int8Array::New(ab, 0, arraySize);
#else
      for(jsize i=0; i<arraySize; i++) {
        result->Set(i, Nan::New<v8::Number>(elems[i]));
      }
      env->ReleaseByteArrayElements((jbyteArray)objArray, elems, 0);
#endif
    }
    break;

  case TYPE_BOOLEAN:
    {
      jboolean* elems = env->GetBooleanArrayElements((jbooleanArray)objArray, 0);
#if (NODE_VERSION_AT_LEAST(4, 0, 0))
      size_t byteLength = arraySize;
      v8::Local<v8::ArrayBuffer> ab = newArrayBuffer(elems, byteLength);
      env->ReleaseBooleanArrayElements((jbooleanArray)objArray, elems, 0);
      return v8::Uint8Array::New(ab, 0, arraySize);
#else
      for(jsize i=0; i<arraySize; i++) {
        result->Set(i, Nan::New<v8::Boolean>(elems[i]));
      }
      env->ReleaseBooleanArrayElements((jbooleanArray)objArray, elems, 0);
#endif
    }
    break;

  case TYPE_SHORT:
    {
      jshort* elems = env->GetShortArrayElements((jshortArray)objArray, 0);
#if (NODE_VERSION_AT_LEAST(4, 0, 0))
      size_t byteLength = arraySize * 2;
      v8::Local<v8::ArrayBuffer> ab = newArrayBuffer(elems, byteLength);
      env->ReleaseShortArrayElements((jshortArray)objArray, elems, 0);
      return v8::Int16Array::New(ab, 0, arraySize);
#else
      for(jsize i=0; i<arraySize; i++) {
        result->Set(i, Nan::New<v8::Number>(elems[i]));
      }
      env->ReleaseShortArrayElements((jshortArray)objArray, elems, 0);
#endif
    }
    break;

  case TYPE_DOUBLE:
    {
      jdouble* elems = env->GetDoubleArrayElements((jdoubleArray)objArray, 0);
#if (NODE_VERSION_AT_LEAST(4, 0, 0))
      size_t byteLength = arraySize * 8;
      v8::Local<v8::ArrayBuffer> ab = newArrayBuffer(elems, byteLength);
      env->ReleaseDoubleArrayElements((jdoubleArray)objArray, elems, 0);
      return v8::Float64Array::New(ab, 0, arraySize);
#else
      for(jsize i=0; i<arraySize; i++) {
        result->Set(i, Nan::New<v8::Number>(elems[i]));
      }
      env->ReleaseDoubleArrayElements((jdoubleArray)objArray, elems, 0);
#endif
    }
    break;

  case TYPE_FLOAT:
    {
      jfloat* elems = env->GetFloatArrayElements((jfloatArray)objArray, 0);
#if (NODE_VERSION_AT_LEAST(4, 0, 0))
      size_t byteLength = arraySize * 4;
      v8::Local<v8::ArrayBuffer> ab = newArrayBuffer(elems, byteLength);
      env->ReleaseFloatArrayElements((jfloatArray)objArray, elems, 0);
      return v8::Float32Array::New(ab, 0, arraySize);
#else
      for(jsize i=0; i<arraySize; i++) {
        result->Set(i, Nan::New<v8::Number>(elems[i]));
      }
      env->ReleaseFloatArrayElements((jfloatArray)objArray, elems, 0);
#endif
    }
    break;

  case TYPE_LONG:
    {
      jlong* elems = env->GetLongArrayElements((jlongArray)objArray, 0);
      for(jsize i=0; i<arraySize; i++) {
        jobject obj = longToJavaLongObj(env, elems[i]);
        result->Set(i, JavaObject::New(java, obj));
      }
      env->ReleaseLongArrayElements((jlongArray)objArray, elems, 0);
    }
    break;

  default:
    for(jsize i=0; i<arraySize; i++) {
        jobject obj = env->GetObjectArrayElement(objArray, i);
        v8::Handle<v8::Value> item = javaToV8(java, env, obj);
        result->Set(i, item);
    }
    break;
  }

  return result;
}

v8::Local<v8::Value> javaToV8(Java* java, JNIEnv* env, jobject obj) {
  return javaToV8(java, env, obj, NULL);
}

v8::Local<v8::Value> javaToV8(Java* java, JNIEnv* env, jobject obj, DynamicProxyData* dynamicProxyData) {
  if(obj == NULL) {
    return Nan::Null();
  }

  jclass objClazz = env->GetObjectClass(obj);
  jvalueType resultType = javaGetType(env, objClazz);

  //printf("javaToV8: %d %s\n", resultType, javaObjectToString(env, obj).c_str());

  switch(resultType) {
    case TYPE_ARRAY:
      {
        v8::Local<v8::Value> result = javaArrayToV8(java, env, (jobjectArray)obj);
        return result;
      }
    case TYPE_VOID:
      return Nan::Undefined();
    case TYPE_CHAR:
      {
        jclass charClazz = env->FindClass("java/lang/Character");
        jmethodID char_charValue = env->GetMethodID(charClazz, "charValue", "()C");
        jchar c = env->CallCharMethod(obj, char_charValue);
        checkJavaException(env);
        return javaCharToV8String(c);
      }
    case TYPE_BOOLEAN:
      {
        jclass booleanClazz = env->FindClass("java/lang/Boolean");
        jmethodID boolean_booleanValue = env->GetMethodID(booleanClazz, "booleanValue", "()Z");
        bool result = env->CallBooleanMethod(obj, boolean_booleanValue);
        assertNoException(env);
        return Nan::New<v8::Boolean>(result);
      }
    case TYPE_BYTE:
      {
        jclass byteClazz = env->FindClass("java/lang/Byte");
        jmethodID byte_byteValue = env->GetMethodID(byteClazz, "byteValue", "()B");
        jbyte result = env->CallByteMethod(obj, byte_byteValue);
        checkJavaException(env);
        return Nan::New<v8::Number>(result);
      }
    case TYPE_LONG:
      {
        jclass longClazz = env->FindClass("java/lang/Long");
        jmethodID long_longValue = env->GetMethodID(longClazz, "longValue", "()J");
        jlong result = env->CallLongMethod(obj, long_longValue);
        checkJavaException(env);
        std::string strValue = javaObjectToString(env, obj);
        v8::Local<v8::Value> v8Result = Nan::New<v8::NumberObject>((double)result);
        v8::NumberObject* v8ResultNumberObject = v8::NumberObject::Cast(*v8Result);
        v8ResultNumberObject->Set(Nan::New<v8::String>("longValue").ToLocalChecked(), Nan::New<v8::String>(strValue.c_str()).ToLocalChecked());
        SetHiddenValue(v8ResultNumberObject, Nan::New<v8::String>(V8_HIDDEN_MARKER_JAVA_LONG).ToLocalChecked(), Nan::New<v8::Boolean>(true));
        return v8Result;
      }
    case TYPE_INT:
      {
        jclass integerClazz = env->FindClass("java/lang/Integer");
        jmethodID integer_intValue = env->GetMethodID(integerClazz, "intValue", "()I");
        jint result = env->CallIntMethod(obj, integer_intValue);
        checkJavaException(env);
        return Nan::New<v8::Integer>(result);
      }
    case TYPE_SHORT:
      {
        jclass shortClazz = env->FindClass("java/lang/Short");
        jmethodID short_shortValue = env->GetMethodID(shortClazz, "shortValue", "()S");
        jshort result = env->CallShortMethod(obj, short_shortValue);
        assertNoException(env);
        return Nan::New<v8::Integer>(result);
      }
    case TYPE_DOUBLE:
      {
        jclass doubleClazz = env->FindClass("java/lang/Double");
        jmethodID double_doubleValue = env->GetMethodID(doubleClazz, "doubleValue", "()D");
        jdouble result = env->CallDoubleMethod(obj, double_doubleValue);
        checkJavaException(env);
        return Nan::New<v8::Number>(result);
      }
    case TYPE_FLOAT:
      {
        jclass floatClazz = env->FindClass("java/lang/Float");
        jmethodID float_floatValue = env->GetMethodID(floatClazz, "floatValue", "()F");
        jfloat result = env->CallFloatMethod(obj, float_floatValue);
        assertNoException(env);
        return Nan::New<v8::Number>(result);
      }
    case TYPE_STRING:
      {
        std::string str = javaObjectToString(env, obj);
        return Nan::New<v8::String>(str.c_str(), str.length()).ToLocalChecked();
      }
    case TYPE_OBJECT:
      if (dynamicProxyData != NULL) {
        return JavaProxyObject::New(java, obj, dynamicProxyData);
      }
      return JavaObject::New(java, obj);
    default:
      printf("javaToV8: unhandled type: 0x%03x\n", resultType);
      return JavaObject::New(java, obj);
  }

  return Nan::Undefined();
}

jobjectArray javaObjectArrayToClasses(JNIEnv *env, jobjectArray objs) {
  jclass clazzClazz = env->FindClass("java/lang/Class");
  jsize objsLength = env->GetArrayLength(objs);
  jobjectArray results = env->NewObjectArray(objsLength, clazzClazz, NULL);
  for(jsize i=0; i<objsLength; i++) {
    jobject elem = env->GetObjectArrayElement(objs, i);
    if(elem == NULL) {
      env->SetObjectArrayElement(results, i, NULL);
    } else {
      jclass objClazz = env->GetObjectClass(elem);
      env->SetObjectArrayElement(results, i, objClazz);
    }
  }

  return results;
}

jobject javaFindMethod(JNIEnv *env, jclass clazz, std::string& methodName, jobjectArray methodArgs) {
  std::string::size_type parenLoc = methodName.find("(");
  if(parenLoc != std::string::npos) {
    jobject method = NULL;

    std::string methodSig = methodName.substr(parenLoc);
    methodName = methodName.substr(0, parenLoc);
    jmethodID methodID = env->GetStaticMethodID(clazz, methodName.c_str(), methodSig.c_str());
    if(methodID != 0) {
      method = env->ToReflectedMethod(clazz, methodID, true);
    } else {
      methodID = env->GetMethodID(clazz, methodName.c_str(), methodSig.c_str());
      if(methodID != 0) {
        method = env->ToReflectedMethod(clazz, methodID, true);
      }
    }
    env->ExceptionClear(); // If GetStaticMethodID can't find the method it throws an exception and we need to just return NULL

    // cast arguments
    if(method != NULL) {
      javaCastArguments(env, methodArgs, method);
    }

    return method;
  } else {
    jclass methodUtilsClazz = env->FindClass("nodejava/org/apache/commons/lang3/reflect/MethodUtils");
    jmethodID methodUtils_getMatchingAccessibleMethod = env->GetStaticMethodID(methodUtilsClazz, "getMatchingAccessibleMethod", "(Ljava/lang/Class;Ljava/lang/String;[Ljava/lang/Class;)Ljava/lang/reflect/Method;");
    const char *methodNameCStr = methodName.c_str();
    jstring methodNameJavaStr = env->NewStringUTF(methodNameCStr);
    jobjectArray methodArgClasses = javaObjectArrayToClasses(env, methodArgs);
    jobject method = env->CallStaticObjectMethod(methodUtilsClazz, methodUtils_getMatchingAccessibleMethod, clazz, methodNameJavaStr, methodArgClasses);
    checkJavaException(env);
    return method;
  }
}

void javaCastArguments(JNIEnv *env, jobjectArray methodArgs, jobject method) {
  jclass castingUtilsClazz = env->FindClass("node/CastingUtils");
  jmethodID castingUtilsClazz_cast = env->GetStaticMethodID(castingUtilsClazz, "cast", "(Ljava/lang/reflect/Method;[Ljava/lang/Object;)V");
  env->CallStaticObjectMethod(castingUtilsClazz, castingUtilsClazz_cast, method, methodArgs);
}

jobject javaFindConstructor(JNIEnv *env, jclass clazz, jobjectArray methodArgs) {
  jclass constructorUtilsClazz = env->FindClass("nodejava/org/apache/commons/lang3/reflect/ConstructorUtils");
  jmethodID constructorUtils_getMatchingAccessibleConstructor = env->GetStaticMethodID(constructorUtilsClazz, "getMatchingAccessibleConstructor", "(Ljava/lang/Class;[Ljava/lang/Class;)Ljava/lang/reflect/Constructor;");
  jobjectArray methodArgClasses = javaObjectArrayToClasses(env, methodArgs);
  jobject method = env->CallStaticObjectMethod(constructorUtilsClazz, constructorUtils_getMatchingAccessibleConstructor, clazz, methodArgClasses);
  assertNoException(env);
  return method;
}

jobject longToJavaLongObj(JNIEnv *env, jlong val) {
  jclass longClass = env->FindClass("java/lang/Long");
  jmethodID constructor = env->GetMethodID(longClass, "<init>", "(J)V");
  jobject result = env->NewObject(longClass, constructor, val);
  return result;
}

int dynamicProxyDataVerify(DynamicProxyData* data) {
  if(data->markerStart == DYNAMIC_PROXY_DATA_MARKER_START && data->markerEnd == DYNAMIC_PROXY_DATA_MARKER_END) {
    return 1;
  }

  printf("*** ERROR: Lost reference to the dynamic proxy. You must maintain a reference in javascript land using ref() and unref(). (%p) ***\n", data);
  return 0;
}

std::string methodNotFoundToString(JNIEnv *env, jclass clazz, std::string methodName, bool constructor, Nan::NAN_METHOD_ARGS_TYPE args, int argStart, int argEnd) {
  std::ostringstream startOfMessage;
  std::ostringstream msg;

  jclass classClazz = env->FindClass("java/lang/Class");
  jmethodID class_getName = env->GetMethodID(classClazz, "getName", "()Ljava/lang/String;");

  startOfMessage << "Could not find method \"" << methodName.c_str() << "(";
  for(int i=argStart; i<argEnd; i++) {
    jobject val = v8ToJava(env, args[i]);
    if(i != argStart) {
      startOfMessage << ", ";
    }
    if(val == NULL) {
      startOfMessage << "(null)";
    } else {
      jclass argClass = env->GetObjectClass(val);
      jstring argClassNameJava = (jstring)env->CallObjectMethod(argClass, class_getName);
      checkJavaException(env);
      std::string argClassName = javaToString(env, argClassNameJava);
      startOfMessage << argClassName;
    }
  }

  startOfMessage << ")\" on class \""<< javaObjectToString(env, clazz).c_str() << "\".";

  msg << startOfMessage.str() << " Possible matches:\n";

  jclass memberClazz = env->FindClass("java/lang/reflect/Member");
  jmethodID member_getName = env->GetMethodID(memberClazz, "getName", "()Ljava/lang/String;");

  std::list<jobject> methods;
  if(constructor) {
    javaReflectionGetConstructors(env, clazz, &methods);
  } else {
    javaReflectionGetMethods(env, clazz, &methods, true);
  }

  int count = 0;
  for(std::list<jobject>::iterator it = methods.begin(); it != methods.end(); ++it) {
    jstring methodNameTestJava = (jstring)env->CallObjectMethod(*it, member_getName);
    assertNoException(env);
    std::string methodNameTest = javaToString(env, methodNameTestJava);
    if(methodNameTest == methodName) {
      msg << "  " << javaObjectToString(env, *it).c_str() << "\n";
      count++;
    }
  }

  if(count == 0) {
    std::ostringstream noMethodsMsg;
    noMethodsMsg << startOfMessage.str() << " No methods with that name.";
    return noMethodsMsg.str();
  }

  return msg.str();
}

void unref(DynamicProxyData* dynamicProxyData) {
  if(!dynamicProxyDataVerify(dynamicProxyData)) {
    return;
  }
  dynamicProxyData->jsObject.Reset();
  dynamicProxyData->functions.Reset();
  dynamicProxyData->markerStart = 0;
  dynamicProxyData->markerEnd = 0;
  delete dynamicProxyData;
}

jarray javaGetArgsForMethod(JNIEnv *env, jobject method, jarray args) {
  jclass varArgsClazz = env->FindClass("node/VarArgs");
  jmethodID method_getVarArgs = env->GetStaticMethodID(varArgsClazz, "getVarArgs", "(Ljava/lang/reflect/Method;[Ljava/lang/Object;)[Ljava/lang/Object;");
  jarray result = (jarray)env->CallStaticObjectMethod(varArgsClazz, method_getVarArgs, method, args);
  checkJavaException(env);
  return result;
}

jarray javaGetArgsForConstructor(JNIEnv *env, jobject method, jarray args) {
  jclass varArgsClazz = env->FindClass("node/VarArgs");
  jmethodID method_getVarArgs = env->GetStaticMethodID(varArgsClazz, "getVarArgs", "(Ljava/lang/reflect/Constructor;[Ljava/lang/Object;)[Ljava/lang/Object;");
  jarray result = (jarray)env->CallStaticObjectMethod(varArgsClazz, method_getVarArgs, method, args);
  checkJavaException(env);
  return result;
}

#if (NODE_MODULE_VERSION > 48)
  // The two methods below were copied from
  // https://github.com/electron/electron?branch=master&filepath=atom/common/api/atom_api_v8_util.cc
  // Copyright (c) 2013 GitHub, Inc.
  // Use of this source code is governed by the MIT license.

  v8::Local<v8::Value> GetHiddenValue(v8::Local<v8::Object> object, v8::Local<v8::String> key) {
    v8::Local<v8::Context> context = v8::Isolate::GetCurrent()->GetCurrentContext();
    v8::Local<v8::Private> privateKey = v8::Private::ForApi(v8::Isolate::GetCurrent(), key);
    v8::Local<v8::Value> value;
    v8::Maybe<bool> result = object->HasPrivate(context, privateKey);
    if (!(result.IsJust() && result.FromJust()))
      return v8::Local<v8::Value>();
    if (object->GetPrivate(context, privateKey).ToLocal(&value))
      return value;
    return v8::Local<v8::Value>();
  }

  void SetHiddenValue(v8::NumberObject* object, v8::Local<v8::String> key, v8::Local<v8::Value> value) {
    if (value.IsEmpty())
      return;
    v8::Local<v8::Context> context = v8::Isolate::GetCurrent()->GetCurrentContext();
    v8::Local<v8::Private> privateKey = v8::Private::ForApi(v8::Isolate::GetCurrent(), key);
    object->SetPrivate(context, privateKey, value);
  }

  void SetHiddenValue(v8::Local<v8::Object> object, v8::Local<v8::String> key, v8::Local<v8::Value> value) {
    if (value.IsEmpty())
      return;
    v8::Local<v8::Context> context = v8::Isolate::GetCurrent()->GetCurrentContext();
    v8::Local<v8::Private> privateKey = v8::Private::ForApi(v8::Isolate::GetCurrent(), key);
    object->SetPrivate(context, privateKey, value);
  }
#else
  v8::Local<v8::Value> GetHiddenValue(v8::Local<v8::Object> object, v8::Local<v8::String> key) {
    return object->GetHiddenValue(key);
  }

  void SetHiddenValue(v8::NumberObject* object, v8::Local<v8::String> key, v8::Local<v8::Value> value) {
    object->SetHiddenValue(key, value);
  }

  void SetHiddenValue(v8::Local<v8::Object> object, v8::Local<v8::String> key, v8::Local<v8::Value> value) {
    object->SetHiddenValue(key, value);
  }
#endif
