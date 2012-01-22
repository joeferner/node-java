
#ifndef _utils_h_
#define _utils_h_

#include <v8.h>
#include <jni.h>
#include <list>
#include <string>

typedef enum _jvalueType {
  TYPE_VOID    = 1,
  TYPE_INT     = 2,
  TYPE_LONG    = 3,
  TYPE_OBJECT  = 4,
  TYPE_STRING  = 5,
  TYPE_BOOLEAN = 6,
  TYPE_BYTE    = 7
} jvalueType;

std::list<jobject> javaReflectionGetMethods(JNIEnv *env, jclass clazz);
std::list<jobject> javaReflectionGetConstructors(JNIEnv *env, jclass clazz);
std::list<jobject> javaReflectionGetStaticMethods(JNIEnv *env, jclass clazz);
std::string javaToString(JNIEnv *env, jstring str);
std::string javaObjectToString(JNIEnv *env, jobject obj);
jobject javaFindBestMatchingMethod(
  JNIEnv *env,
  std::list<jobject>& methods,
  const char *methodName,
  std::list<int>& argTypes);
jobject javaFindBestMatchingConstructor(
  JNIEnv *env,
  std::list<jobject>& constructors,
  std::list<int>& argTypes);
JNIEnv* javaAttachCurrentThread(JavaVM* jvm);
void javaDetachCurrentThread(JavaVM* jvm);
jvalueType javaGetType(JNIEnv *env, jclass type);
jclass javaFindClass(JNIEnv* env, std::string className);
jobject javaFindField(JNIEnv* env, jclass clazz, std::string fieldName);
jarray v8ToJava(JNIEnv* env, const v8::Arguments& args, int start, int end, std::list<int> *methodArgTypes);
jobject v8ToJava(JNIEnv* env, v8::Local<v8::Value> arg, int *methodArgType);
v8::Handle<v8::Value> javaExceptionToV8(JNIEnv* env, const std::string& alternateMessage);
v8::Handle<v8::Value> javaExceptionToV8(JNIEnv* env, jthrowable ex, const std::string& alternateMessage);

#endif
