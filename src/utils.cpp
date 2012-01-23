
#include "utils.h"
#include <string.h>
#include <algorithm>
#include <sstream>
#include "javaObject.h"
#include "java.h"

#define MODIFIER_STATIC 9

std::list<jobject> javaReflectionGetMethods(JNIEnv *env, jclass clazz) {
  std::list<jobject> results;

  jclass clazzclazz = env->GetObjectClass(clazz);
  jmethodID clazz_getMethods = env->GetMethodID(clazzclazz, "getMethods", "()[Ljava/lang/reflect/Method;");
  jclass methodClazz = env->FindClass("java/lang/reflect/Method");
  jmethodID method_getModifiers = env->GetMethodID(methodClazz, "getModifiers", "()I");

  jobjectArray methodObjects = (jobjectArray)env->CallObjectMethod(clazz, clazz_getMethods);
  jsize methodCount = env->GetArrayLength(methodObjects);
  for(jsize i=0; i<methodCount; i++) {
    jobject method = env->GetObjectArrayElement(methodObjects, i);
    jint methodModifiers = env->CallIntMethod(method, method_getModifiers);
    if((methodModifiers & MODIFIER_STATIC) == MODIFIER_STATIC) {
      continue;
    }
    results.push_back(method);
  }

  return results;
}

std::list<jobject> javaReflectionGetFields(JNIEnv *env, jclass clazz) {
  std::list<jobject> results;

  jclass clazzclazz = env->GetObjectClass(clazz);
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
    results.push_back(field);
  }

  return results;
}

std::string javaToString(JNIEnv *env, jstring str) {
  const char* chars = env->GetStringUTFChars(str, NULL);
  std::string results = chars;
  env->ReleaseStringUTFChars(str, chars);
  return results;
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

JNIEnv* javaAttachCurrentThread(JavaVM* jvm) {
  JNIEnv* env;
  JavaVMAttachArgs attachArgs;
  attachArgs.version = JNI_VERSION_1_4;
  attachArgs.name = NULL;
  attachArgs.group = NULL;
  jvm->AttachCurrentThread((void**)&env, &attachArgs);
  return env;
}

void javaDetachCurrentThread(JavaVM* jvm) {
  jvm->DetachCurrentThread();
}

jvalueType javaGetType(JNIEnv *env, jclass type) {
  jclass clazzClazz = env->FindClass("java/lang/Class");
  jmethodID class_isArray = env->GetMethodID(clazzClazz, "isArray", "()Z");
  jmethodID class_getComponentType = env->GetMethodID(clazzClazz, "getComponentType", "()Ljava/lang/Class;");

  jboolean isArray = env->CallBooleanMethod(type, class_isArray);
  if(isArray) {
    jclass componentTypeClass = (jclass)env->CallObjectMethod(type, class_getComponentType);
    jvalueType componentType = javaGetType(env, componentTypeClass);
    switch(componentType) {
      case TYPE_INT: return TYPE_ARRAY_INT;
      case TYPE_LONG: return TYPE_ARRAY_LONG;
      case TYPE_OBJECT: return TYPE_ARRAY_OBJECT;
      case TYPE_STRING: return TYPE_ARRAY_STRING;
      case TYPE_BOOLEAN: return TYPE_ARRAY_BOOLEAN;
      case TYPE_BYTE: return TYPE_ARRAY_BYTE;
      default:
        return TYPE_ARRAY_OBJECT;
    }
  } else {
    // TODO: has to be a better way
    std::string str = javaObjectToString(env, type);
    const char *typeStr = str.c_str();
    //printf("javaGetType: %s\n", typeStr);
    if(strcmp(typeStr, "int") == 0) {
      return TYPE_INT;
    } else if(strcmp(typeStr, "double") == 0) {
      return TYPE_DOUBLE;
    } else if(strcmp(typeStr, "long") == 0) {
      return TYPE_LONG;
    } else if(strcmp(typeStr, "void") == 0) {
      return TYPE_VOID;
    } else if(strcmp(typeStr, "boolean") == 0) {
      return TYPE_BOOLEAN;
    } else if(strcmp(typeStr, "byte") == 0) {
      return TYPE_BYTE;
    } else if(strcmp(typeStr, "class java.lang.String") == 0) {
      return TYPE_STRING;
    }
    return TYPE_OBJECT;
  }
}

jclass javaFindClass(JNIEnv* env, std::string className) {
  std::replace(className.begin(), className.end(), '.', '/');
  jclass clazz = env->FindClass(className.c_str());
  return clazz;
}

jobject javaFindField(JNIEnv* env, jclass clazz, std::string fieldName) {
  jclass clazzclazz = env->GetObjectClass(clazz);
  jclass fieldClazz = env->FindClass("java/lang/reflect/Field");
  jmethodID field_getName = env->GetMethodID(fieldClazz, "getName", "()Ljava/lang/String;");
  jmethodID class_getFields = env->GetMethodID(clazzclazz, "getFields", "()[Ljava/lang/reflect/Field;");
  jobjectArray fieldObjects = (jobjectArray)env->CallObjectMethod(clazz, class_getFields);

  jsize fieldCount = env->GetArrayLength(fieldObjects);
  for(jsize i=0; i<fieldCount; i++) {
    jobject field = env->GetObjectArrayElement(fieldObjects, i);
    std::string itFieldName = javaToString(env, (jstring)env->CallObjectMethod(field, field_getName));
    if(strcmp(itFieldName.c_str(), fieldName.c_str()) == 0) {
      return field;
    }
  }

  return NULL;
}

jobject v8ToJava(JNIEnv* env, v8::Local<v8::Value> arg) {
  if(arg->IsNull() || arg->IsUndefined()) {
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
    v8::String::AsciiValue val(arg->ToString());
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
    v8::String::AsciiValue constructorName(obj->GetConstructorName());
    if(strcmp(*constructorName, "JavaObject") == 0) {
      JavaObject* javaObject = node::ObjectWrap::Unwrap<JavaObject>(obj);
      return javaObject->getObject();
    }
  }

  // TODO: handle other arg types
  v8::String::AsciiValue typeStr(arg);
  printf("Unhandled type: %s\n", *typeStr);
  return NULL;
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

v8::Handle<v8::Value> javaArrayToV8(Java* java, JNIEnv* env, jvalueType itemType, jobjectArray objArray) {
  v8::HandleScope scope;

  if(objArray == NULL) {
    return v8::Null();
  }

  //printf("javaArrayToV8: %d %s\n", itemType, javaObjectToString(env, objArray).c_str());

  jsize arraySize = env->GetArrayLength(objArray);
  //printf("array size: %d\n", arraySize);

  v8::Handle<v8::Array> result = v8::Array::New(arraySize);
  for(jsize i=0; i<arraySize; i++) {
    jobject obj = env->GetObjectArrayElement(objArray, i);
    v8::Handle<v8::Value> item = javaToV8(java, env, itemType, obj);
    result->Set(i, item);
  }

  return scope.Close(result);
}

v8::Handle<v8::Value> javaToV8(Java* java, JNIEnv* env, jvalueType resultType, jobject obj) {
  v8::HandleScope scope;

  //printf("javaToV8: %d %s\n", resultType, javaObjectToString(env, obj).c_str());

  if((resultType & VALUE_TYPE_ARRAY) == VALUE_TYPE_ARRAY) {
    v8::Handle<v8::Value> result = javaArrayToV8(java, env, (jvalueType)(resultType & ~VALUE_TYPE_ARRAY), (jobjectArray)obj);
    return scope.Close(result);
  } else {
    switch(resultType) {
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
          return scope.Close(v8::Number::New(result));
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
      case TYPE_STRING:
        return scope.Close(v8::String::New(javaObjectToString(env, obj).c_str()));
      case TYPE_OBJECT:
        return scope.Close(JavaObject::New(java, obj));
      default:
        printf("unhandled type: 0x%03x\n", resultType);
        return scope.Close(JavaObject::New(java, obj));
    }
  }
  return v8::Undefined();
}

jobjectArray javaObjectArrayToClasses(JNIEnv *env, jobjectArray objs) {
  jclass clazzClazz = env->FindClass("java/lang/Class");
  jsize objsLength = env->GetArrayLength(objs);
  jobjectArray results = env->NewObjectArray(objsLength, clazzClazz, NULL);
  for(jsize i=0; i<objsLength; i++) {
    jclass objClazz = env->GetObjectClass(env->GetObjectArrayElement(objs, i));
    env->SetObjectArrayElement(results, i, objClazz);
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
