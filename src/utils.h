
#ifndef _utils_h_
#define _utils_h_

#include <jni.h>
#include <list>
#include <string>

typedef enum _jvalueType {
  TYPE_INT,
  TYPE_OBJECT
} jvalueType;

std::list<jobject> javaReflectionGetDeclaredMethods(JNIEnv *env, jclass clazz);
std::string javaToString(JNIEnv *env, jstring str);
std::string javaObjectToString(JNIEnv *env, jobject obj);
jobject javaFindBestMatchingMethod(
  JNIEnv *env,
  std::list<jobject>& methods,
  const char *methodName,
  std::list<jobject>& args);
JNIEnv* javaAttachCurrentThread(JavaVM* jvm);
void javaDetachCurrentThread(JavaVM* jvm);
jvalueType javaGetType(JNIEnv *env, jclass type);

#endif
