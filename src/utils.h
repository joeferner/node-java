
#ifndef _utils_h_
#define _utils_h_

#include <v8.h>
#include <jni.h>
#include <list>
#include <string>

typedef enum _jvalueType {
  TYPE_VOID,
  TYPE_INT,
  TYPE_OBJECT,
  TYPE_STRING,
  TYPE_BOOLEAN
} jvalueType;

std::list<jobject> javaReflectionGetMethods(JNIEnv *env, jclass clazz);
std::list<jobject> javaReflectionGetConstructors(JNIEnv *env, jclass clazz);
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
jarray v8ToJava(JNIEnv* env, const v8::Arguments& args, int start, int end, std::list<int> *methodArgTypes);
jobject v8ToJava(JNIEnv* env, v8::Local<v8::Value> arg, int *methodArgType);

#endif
