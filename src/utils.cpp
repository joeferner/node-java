
#include "utils.h"
#include <string.h>
#include <algorithm>
#include <sstream>
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
  jsize methodCount = env->GetArrayLength(methodObjects);
  for(jsize i=0; i<methodCount; i++) {
    jobject method = env->GetObjectArrayElement(methodObjects, i);
    jint methodModifiers = env->CallIntMethod(method, method_getModifiers);
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
  jsize fieldCount = env->GetArrayLength(fieldObjects);
  for(jsize i=0; i<fieldCount; i++) {
    jobject field = env->GetObjectArrayElement(fieldObjects, i);
    jint fieldModifiers = env->CallIntMethod(field, field_getModifiers);
    if((fieldModifiers & MODIFIER_STATIC) == MODIFIER_STATIC) {
      continue;
    }
    fields->push_back(field);
  }
}

std::string javaToString(JNIEnv *env, jstring str) {
  const char* chars = env->GetStringUTFChars(str, NULL);
  std::string results = chars;
  env->ReleaseStringUTFChars(str, chars);
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

JNIEnv* javaAttachCurrentThread(JavaVM* jvm, jobject classLoader) {
  JNIEnv* env;
  JavaVMAttachArgs attachArgs;
  attachArgs.version = JNI_VERSION_1_4;
  attachArgs.name = NULL;
  attachArgs.group = NULL;
  jvm->AttachCurrentThread((void**)&env, &attachArgs);

  jclass threadClazz = env->FindClass("java/lang/Thread");
  jmethodID thread_currentThread = env->GetStaticMethodID(threadClazz, "currentThread", "()Ljava/lang/Thread;");
  jmethodID thread_setContextClassLoader = env->GetMethodID(threadClazz, "setContextClassLoader", "(Ljava/lang/ClassLoader;)V");
  jobject currentThread = env->CallStaticObjectMethod(threadClazz, thread_currentThread);
  env->CallObjectMethod(currentThread, thread_setContextClassLoader, classLoader);

  return env;
}

jobject getSystemClassLoader(JNIEnv *env) {
  jclass threadClazz = env->FindClass("java/lang/Thread");
  jmethodID thread_currentThread = env->GetStaticMethodID(threadClazz, "currentThread", "()Ljava/lang/Thread;");
  jmethodID thread_getContextClassLoader = env->GetMethodID(threadClazz, "getContextClassLoader", "()Ljava/lang/ClassLoader;");
  jobject currentThread = env->CallStaticObjectMethod(threadClazz, thread_currentThread);
  return env->CallObjectMethod(currentThread, thread_getContextClassLoader);
}

void javaDetachCurrentThread(JavaVM* jvm) {
  jvm->DetachCurrentThread();
}

jvalueType javaGetType(JNIEnv *env, jclass type) {
  jclass clazzClazz = env->FindClass("java/lang/Class");
  jmethodID class_isArray = env->GetMethodID(clazzClazz, "isArray", "()Z");

  jboolean isArray = env->CallBooleanMethod(type, class_isArray);
  if(isArray) {
    return TYPE_ARRAY;
  } else {
    // TODO: has to be a better way
    std::string str = javaObjectToString(env, type);
    const char *typeStr = str.c_str();
    //printf("javaGetType: %s\n", typeStr);
    if(strcmp(typeStr, "void") == 0) {
      return TYPE_VOID;
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

  jsize fieldCount = env->GetArrayLength(fieldObjects);
  for(jsize i=0; i<fieldCount; i++) {
    jobject field = env->GetObjectArrayElement(fieldObjects, i);
    jstring fieldNameJava = (jstring)env->CallObjectMethod(field, field_getName);
    std::string itFieldName = javaToString(env, fieldNameJava);
    if(strcmp(itFieldName.c_str(), fieldName.c_str()) == 0) {
      result = field;
      break;
    }
  }

  return result;
}

jobject v8ToJava(JNIEnv* env, v8::Local<v8::Value> arg) {
  if(arg.IsEmpty() || arg->IsNull() || arg->IsUndefined()) {
    return NULL;
  }

  if(arg->IsArray()) {
    v8::Local<v8::Array> array = v8::Array::Cast(*arg);
    uint32_t arraySize = array->Length();
    jclass objectClazz = env->FindClass("java/lang/Object");
    jobjectArray result = env->NewObjectArray(arraySize, objectClazz, NULL);
    for(uint32_t i=0; i<arraySize; i++) {
      jobject val = v8ToJava(env, array->Get(i));
      env->SetObjectArrayElement(result, i, val);
    }
    return result;
  }

  if(arg->IsString()) {
    v8::String::Utf8Value val(arg->ToString());
    return env->NewStringUTF(*val);
  }

  if(arg->IsInt32() || arg->IsUint32()) {
    jint val = arg->ToInt32()->Value();
    jclass clazz = env->FindClass("java/lang/Integer");
    jmethodID constructor = env->GetMethodID(clazz, "<init>", "(I)V");
    return env->NewObject(clazz, constructor, val);
  }

  if(arg->IsNumber()) {
    jdouble val = arg->ToNumber()->Value();
    jclass clazz = env->FindClass("java/lang/Double");
    jmethodID constructor = env->GetMethodID(clazz, "<init>", "(D)V");
    return env->NewObject(clazz, constructor, val);
  }

  if(arg->IsBoolean()) {
    jboolean val = arg->ToBoolean()->Value();
    jclass clazz = env->FindClass("java/lang/Boolean");
    jmethodID constructor = env->GetMethodID(clazz, "<init>", "(Z)V");
    return env->NewObject(clazz, constructor, val);
  }

  if(arg->IsObject()) {
    v8::Local<v8::Object> obj = v8::Object::Cast(*arg);

    v8::Local<v8::Value> isJavaObject = obj->GetHiddenValue(v8::String::New(V8_HIDDEN_MARKER_JAVA_OBJECT));
    if(!isJavaObject.IsEmpty() && isJavaObject->IsBoolean()) {
      return v8ToJava_javaObject(env, obj);
    }

    v8::Local<v8::Value> isJavaLong = obj->GetHiddenValue(v8::String::New(V8_HIDDEN_MARKER_JAVA_LONG));
    if(!isJavaLong.IsEmpty() && isJavaLong->IsBoolean()) {
      return v8ToJava_javaLong(env, obj);
    }
  }

  // TODO: handle other arg types
  v8::String::AsciiValue typeStr(arg);
  printf("v8ToJava: Unhandled type: %s\n", *typeStr);
  return NULL;
}

jobject v8ToJava_javaObject(JNIEnv* env, v8::Local<v8::Object> obj) {
  JavaObject* javaObject = node::ObjectWrap::Unwrap<JavaObject>(obj);
  jobject jobj = javaObject->getObject();

  jclass nodeDynamicProxyClass = env->FindClass("node/NodeDynamicProxyClass");
  if(env->IsInstanceOf(jobj, nodeDynamicProxyClass)) {
    jfieldID ptrField = env->GetFieldID(nodeDynamicProxyClass, "ptr", "J");
    DynamicProxyData* proxyData = (DynamicProxyData*)(long)env->GetLongField(jobj, ptrField);
    if(!dynamicProxyDataVerify(proxyData)) {
      return NULL;
    }

    jclass dynamicInterface = javaFindClass(env, proxyData->interfaceName);
    if(dynamicInterface == NULL) {
      printf("Could not find interface %s\n", proxyData->interfaceName.c_str());
      return NULL;
    }
    jclass classClazz = env->FindClass("java/lang/Class");
    jobjectArray classArray = env->NewObjectArray(1, classClazz, NULL);
    env->SetObjectArrayElement(classArray, 0, dynamicInterface);

    jmethodID class_getClassLoader = env->GetMethodID(classClazz, "getClassLoader", "()Ljava/lang/ClassLoader;");
    jobject classLoader = env->CallObjectMethod(dynamicInterface, class_getClassLoader);
    if(classLoader == NULL) {
      jclass objectClazz = env->FindClass("java/lang/Object");
      jmethodID object_getClass = env->GetMethodID(objectClazz, "getClass", "()Ljava/lang/Class;");
      jobject jobjClass = env->CallObjectMethod(jobj, object_getClass);
      classLoader = env->CallObjectMethod(jobjClass, class_getClassLoader);
    }

    jclass proxyClass = env->FindClass("java/lang/reflect/Proxy");
    jmethodID proxy_newProxyInstance = env->GetStaticMethodID(proxyClass, "newProxyInstance", "(Ljava/lang/ClassLoader;[Ljava/lang/Class;Ljava/lang/reflect/InvocationHandler;)Ljava/lang/Object;");
    if(classLoader == NULL) {
      printf("Could not get classloader for Proxy\n");
      return NULL;
    }
    if(classArray == NULL) {
      printf("Could not create class array for Proxy\n");
      return NULL;
    }
    if(jobj == NULL) {
      printf("Not a valid object to wrap\n");
      return NULL;
    }
    jobj = env->CallStaticObjectMethod(proxyClass, proxy_newProxyInstance, classLoader, classArray, jobj);
  }

  return jobj;
}

jobject v8ToJava_javaLong(JNIEnv* env, v8::Local<v8::Object> obj) {
  jobject longValue = v8ToJava(env, obj->Get(v8::String::New("longValue")));
  jclass longClazz = env->FindClass("java/lang/Long");
  jmethodID long_constructor = env->GetMethodID(longClazz, "<init>", "(Ljava/lang/String;)V");
  jobject jobj = env->NewObject(longClazz, long_constructor, longValue);
  return jobj;
}

jobjectArray v8ToJava(JNIEnv* env, const v8::Arguments& args, int start, int end) {
  jclass clazz = env->FindClass("java/lang/Object");
  jobjectArray results = env->NewObjectArray(end-start, clazz, NULL);

  for(int i=start; i<end; i++) {
    jobject val = v8ToJava(env, args[i]);
    env->SetObjectArrayElement(results, i - start, val);
  }

  return results;
}

v8::Handle<v8::Value> javaExceptionToV8(JNIEnv* env, jthrowable ex, const std::string& alternateMessage) {
  v8::HandleScope scope;

  std::ostringstream msg;
  msg << alternateMessage;

  if(ex) {
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

    jstring strObj = (jstring)env->CallObjectMethod(stringWriter, stringWriter_toString);
    std::string stackTrace = javaToString(env, strObj);

    msg << "\n" << stackTrace;
  }

  return scope.Close(v8::Exception::Error(v8::String::New(msg.str().c_str())));
}

v8::Handle<v8::Value> javaExceptionToV8(JNIEnv* env, const std::string& alternateMessage) {
  v8::HandleScope scope;
  jthrowable ex = env->ExceptionOccurred();
  env->ExceptionClear();
  return scope.Close(javaExceptionToV8(env, ex, alternateMessage));
}

jvalueType javaGetArrayComponentType(JNIEnv *env, jobjectArray array) {
  jclass objectClazz = env->FindClass("java/lang/Object");
  jclass clazzclazz = env->FindClass("java/lang/Class");

  jmethodID object_getClass = env->GetMethodID(objectClazz, "getClass", "()Ljava/lang/Class;");
  jobject arrayClass = env->CallObjectMethod(array, object_getClass);

  jmethodID class_getComponentType = env->GetMethodID(clazzclazz, "getComponentType", "()Ljava/lang/Class;");
  jobject arrayComponentTypeClass = env->CallObjectMethod(arrayClass, class_getComponentType);

  jvalueType arrayComponentType = javaGetType(env, (jclass)arrayComponentTypeClass);
  return arrayComponentType;
}

v8::Handle<v8::Value> javaArrayToV8(Java* java, JNIEnv* env, jobjectArray objArray) {
  v8::HandleScope scope;

  if(objArray == NULL) {
    return v8::Null();
  }

  jvalueType arrayComponentType = javaGetArrayComponentType(env, objArray);
  //printf("javaArrayToV8: %d %s\n", arrayComponentType, javaObjectToString(env, objArray).c_str());

  jsize arraySize = env->GetArrayLength(objArray);
  //printf("array size: %d\n", arraySize);

  v8::Handle<v8::Array> result = v8::Array::New(arraySize);
  switch(arrayComponentType) {
  case TYPE_INT:
    {
      jint* elems = env->GetIntArrayElements((jintArray)objArray, 0);
      for(jsize i=0; i<arraySize; i++) {
        result->Set(i, v8::Integer::New(elems[i]));
      }
      env->ReleaseIntArrayElements((jintArray)objArray, elems, 0);
    }
    break;

  case TYPE_BYTE:
    {
      jbyte* elems = env->GetByteArrayElements((jbyteArray)objArray, 0);
      for(jsize i=0; i<arraySize; i++) {
        result->Set(i, v8::Number::New(elems[i]));
      }
      env->ReleaseByteArrayElements((jbyteArray)objArray, elems, 0);
    }
    break;

  case TYPE_BOOLEAN:
    {
      jboolean* elems = env->GetBooleanArrayElements((jbooleanArray)objArray, 0);
      for(jsize i=0; i<arraySize; i++) {
        result->Set(i, v8::Boolean::New(elems[i]));
      }
      env->ReleaseBooleanArrayElements((jbooleanArray)objArray, elems, 0);
    }
    break;

  case TYPE_DOUBLE:
    {
      jdouble* elems = env->GetDoubleArrayElements((jdoubleArray)objArray, 0);
      for(jsize i=0; i<arraySize; i++) {
        result->Set(i, v8::Number::New(elems[i]));
      }
      env->ReleaseDoubleArrayElements((jdoubleArray)objArray, elems, 0);
    }
    break;

  case TYPE_FLOAT:
    {
      jfloat* elems = env->GetFloatArrayElements((jfloatArray)objArray, 0);
      for(jsize i=0; i<arraySize; i++) {
        result->Set(i, v8::Number::New(elems[i]));
      }
      env->ReleaseFloatArrayElements((jfloatArray)objArray, elems, 0);
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

  return scope.Close(result);
}

v8::Handle<v8::Value> javaToV8(Java* java, JNIEnv* env, jobject obj) {
  v8::HandleScope scope;

  if(obj == NULL) {
    return v8::Null();
  }

  jclass objClazz = env->GetObjectClass(obj);
  jvalueType resultType = javaGetType(env, objClazz);

  //printf("javaToV8: %d %s\n", resultType, javaObjectToString(env, obj).c_str());

  switch(resultType) {
    case TYPE_ARRAY:
      {
        v8::Handle<v8::Value> result = javaArrayToV8(java, env, (jobjectArray)obj);
        return scope.Close(result);
      }
    case TYPE_VOID:
      return v8::Undefined();
    case TYPE_BOOLEAN:
      {
        jclass booleanClazz = env->FindClass("java/lang/Boolean");
        jmethodID boolean_booleanValue = env->GetMethodID(booleanClazz, "booleanValue", "()Z");
        bool result = env->CallBooleanMethod(obj, boolean_booleanValue);
        return scope.Close(v8::Boolean::New(result));
      }
    case TYPE_BYTE:
      {
        jclass byteClazz = env->FindClass("java/lang/Byte");
        jmethodID byte_byteValue = env->GetMethodID(byteClazz, "byteValue", "()B");
        jbyte result = env->CallByteMethod(obj, byte_byteValue);
        return scope.Close(v8::Number::New(result));
      }
    case TYPE_LONG:
      {
        jclass longClazz = env->FindClass("java/lang/Long");
        jmethodID long_longValue = env->GetMethodID(longClazz, "longValue", "()J");
        jlong result = env->CallLongMethod(obj, long_longValue);
        std::string strValue = javaObjectToString(env, obj);
        v8::Local<v8::Value> v8Result = v8::NumberObject::New(result);
        v8::NumberObject* v8ResultNumberObject = v8::NumberObject::Cast(*v8Result);
        v8ResultNumberObject->Set(v8::String::New("longValue"), v8::String::New(strValue.c_str()));
        v8ResultNumberObject->SetHiddenValue(v8::String::New(V8_HIDDEN_MARKER_JAVA_LONG), v8::Boolean::New(true));
        return scope.Close(v8Result);
      }
    case TYPE_INT:
      {
        jclass integerClazz = env->FindClass("java/lang/Integer");
        jmethodID integer_intValue = env->GetMethodID(integerClazz, "intValue", "()I");
        jint result = env->CallIntMethod(obj, integer_intValue);
        return scope.Close(v8::Integer::New(result));
      }
    case TYPE_DOUBLE:
      {
        jclass doubleClazz = env->FindClass("java/lang/Double");
        jmethodID double_doubleValue = env->GetMethodID(doubleClazz, "doubleValue", "()D");
        jdouble result = env->CallDoubleMethod(obj, double_doubleValue);
        return scope.Close(v8::Number::New(result));
      }
    case TYPE_FLOAT:
      {
        jclass floatClazz = env->FindClass("java/lang/Float");
        jmethodID float_floatValue = env->GetMethodID(floatClazz, "floatValue", "()F");
        jfloat result = env->CallFloatMethod(obj, float_floatValue);
        return scope.Close(v8::Number::New(result));
      }
    case TYPE_STRING:
      return scope.Close(v8::String::New(javaObjectToString(env, obj).c_str()));
    case TYPE_OBJECT:
      return scope.Close(JavaObject::New(java, obj));
    default:
      printf("javaToV8: unhandled type: 0x%03x\n", resultType);
      return scope.Close(JavaObject::New(java, obj));
  }

  return v8::Undefined();
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
  jclass methodUtilsClazz = env->FindClass("com/nearinfinity/org/apache/commons/lang3/reflect/MethodUtils");
  jmethodID methodUtils_getMatchingAccessibleMethod = env->GetStaticMethodID(methodUtilsClazz, "getMatchingAccessibleMethod", "(Ljava/lang/Class;Ljava/lang/String;[Ljava/lang/Class;)Ljava/lang/reflect/Method;");
  const char *methodNameCStr = methodName.c_str();
  jstring methodNameJavaStr = env->NewStringUTF(methodNameCStr);
  jobjectArray methodArgClasses = javaObjectArrayToClasses(env, methodArgs);
  jobject method = env->CallStaticObjectMethod(methodUtilsClazz, methodUtils_getMatchingAccessibleMethod, clazz, methodNameJavaStr, methodArgClasses);

  return method;
}

jobject javaFindConstructor(JNIEnv *env, jclass clazz, jobjectArray methodArgs) {
  jclass constructorUtilsClazz = env->FindClass("com/nearinfinity/org/apache/commons/lang3/reflect/ConstructorUtils");
  jmethodID constructorUtils_getMatchingAccessibleConstructor = env->GetStaticMethodID(constructorUtilsClazz, "getMatchingAccessibleConstructor", "(Ljava/lang/Class;[Ljava/lang/Class;)Ljava/lang/reflect/Constructor;");
  jobjectArray methodArgClasses = javaObjectArrayToClasses(env, methodArgs);
  jobject method = env->CallStaticObjectMethod(constructorUtilsClazz, constructorUtils_getMatchingAccessibleConstructor, clazz, methodArgClasses);
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

  printf("*** ERROR: Lost reference to the dynamic proxy. You must maintain a reference in javascript land using ref() and unref(). ***\n");
  return 0;
}

std::string methodNotFoundToString(JNIEnv *env, jclass clazz, std::string methodName, bool constructor, const v8::Arguments& args, int argStart, int argEnd) {
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
  for(std::list<jobject>::iterator it = methods.begin(); it != methods.end(); it++) {
    jstring methodNameTestJava = (jstring)env->CallObjectMethod(*it, member_getName);
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