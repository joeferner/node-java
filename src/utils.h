
#ifndef _utils_h_
#define _utils_h_

#define BUILDING_NODE_EXTENSION 1
#include <v8.h>
#include <jni.h>
#include <list>
#include <vector>
#include <string>

class Java;

typedef enum _jvalueType {
  TYPE_VOID    = 1,
  TYPE_INT     = 2,
  TYPE_LONG    = 3,
  TYPE_OBJECT  = 4,
  TYPE_STRING  = 5,
  TYPE_BOOLEAN = 6,
  TYPE_BYTE    = 7,
  TYPE_DOUBLE  = 8,
  TYPE_ARRAY   = 9
} jvalueType;

std::list<jobject> javaReflectionGetMethods(JNIEnv *env, jclass clazz);
std::list<jobject> javaReflectionGetFields(JNIEnv *env, jclass clazz);
std::string javaToString(JNIEnv *env, jstring str);
std::string javaObjectToString(JNIEnv *env, jobject obj);
JNIEnv* javaAttachCurrentThread(JavaVM* jvm);
void javaDetachCurrentThread(JavaVM* jvm);
jvalueType javaGetType(JNIEnv *env, jclass type);
jobjectArray v8ToJava(JNIEnv* env, const v8::Arguments& args, int start, int end);
jobject v8ToJava(JNIEnv* env, v8::Local<v8::Value> arg);
v8::Handle<v8::Value> javaExceptionToV8(JNIEnv* env, const std::string& alternateMessage);
v8::Handle<v8::Value> javaExceptionToV8(JNIEnv* env, jthrowable ex, const std::string& alternateMessage);
v8::Handle<v8::Value> javaArrayToV8(Java* java, JNIEnv* env, jobjectArray objArray);
v8::Handle<v8::Value> javaToV8(Java* java, JNIEnv* env, jobject obj);
jobjectArray javaObjectArrayToClasses(JNIEnv *env, jobjectArray objs);

jclass javaFindClass(JNIEnv* env, std::string& className);
jobject javaFindField(JNIEnv* env, jclass clazz, std::string& fieldName);
jobject javaFindMethod(JNIEnv *env, jclass clazz, std::string& methodName, jobjectArray methodArgs);
jobject javaFindConstructor(JNIEnv *env, jclass clazz, jobjectArray methodArgs);

#define UNUSED_VARIABLE(var) var = var;

#define ARGS_FRONT_STRING(ARGNAME) \
  if(args.Length() < argsStart+1 || !args[argsStart]->IsString()) {                          \
    std::ostringstream errStr;                                                               \
    errStr << "Argument " << (argsStart+1) << " must be a string";                           \
    return ThrowException(v8::Exception::TypeError(v8::String::New(errStr.str().c_str())));  \
  }                                                                                          \
  v8::Local<v8::String> _##ARGNAME##_obj = v8::Local<v8::String>::Cast(args[argsStart]);     \
  v8::String::AsciiValue _##ARGNAME##_val(_##ARGNAME##_obj);                                 \
  std::string ARGNAME = *_##ARGNAME##_val;                                              \
  argsStart++;

#define ARGS_FRONT_CLASSNAME() ARGS_FRONT_STRING(className)

#define ARGS_BACK_CALLBACK() \
  bool callbackProvided;                      \
  v8::Handle<v8::Value> callback;             \
  if(args[args.Length()-1]->IsFunction()) {   \
    callback = args[argsEnd-1];               \
    argsEnd--;                                \
    callbackProvided = true;                  \
  } else {                                    \
    callback = v8::Null();                    \
    callbackProvided = false;                 \
  }

#endif
