
#ifndef _utils_h_
#define _utils_h_

#define BUILDING_NODE_EXTENSION 1
#include <v8.h>
#include <jni.h>
#include <list>
#include <vector>
#include <string>
#include <uv.h>
#include <nan.h>

class Java;

#define V8_HIDDEN_MARKER_JAVA_LONG   "__isJavaLong"
#define V8_HIDDEN_MARKER_JAVA_OBJECT "__isJavaObject"

typedef enum _jvalueType {
  TYPE_VOID    = 1,
  TYPE_INT     = 2,
  TYPE_LONG    = 3,
  TYPE_OBJECT  = 4,
  TYPE_STRING  = 5,
  TYPE_BOOLEAN = 6,
  TYPE_SHORT   = 7,
  TYPE_BYTE    = 8,
  TYPE_DOUBLE  = 9,
  TYPE_FLOAT   = 10,
  TYPE_ARRAY   = 11
} jvalueType;

struct DynamicProxyData {
  unsigned int markerStart;
  Java* java;
  std::string interfaceName;
  v8::Persistent<v8::Object> functions;
  v8::Persistent<v8::Value> jsObject;
  std::string methodName;
  jobjectArray args;
  jobject result;
  std::string throwableClass;
  std::string throwableMessage;
  int done;
  unsigned int markerEnd;
};

#define DYNAMIC_PROXY_DATA_MARKER_START 0x12345678
#define DYNAMIC_PROXY_DATA_MARKER_END   0x87654321

int dynamicProxyDataVerify(DynamicProxyData* data);

void javaReflectionGetMethods(JNIEnv *env, jclass clazz, std::list<jobject>* methods, bool includeStatic);
void javaReflectionGetConstructors(JNIEnv *env, jclass clazz, std::list<jobject>* methods);
void javaReflectionGetFields(JNIEnv *env, jclass clazz, std::list<jobject>* fields);
std::string javaToString(JNIEnv *env, jstring str);
std::string javaObjectToString(JNIEnv *env, jobject obj);
std::string javaArrayToString(JNIEnv *env, jobjectArray arr);
std::string javaMethodCallToString(JNIEnv *env, jobject obj, jmethodID methodId, jarray args);
JNIEnv* javaGetEnv(JavaVM* jvm, jobject classLoader);
jobject getSystemClassLoader(JNIEnv *env);
jvalueType javaGetArrayComponentType(JNIEnv *env, jobjectArray array);
jvalueType javaGetType(JNIEnv *env, jclass type);
jobjectArray v8ToJava(JNIEnv* env, _NAN_METHOD_ARGS_TYPE args, int start, int end);
jobject v8ToJava(JNIEnv* env, v8::Local<v8::Value> arg);
v8::Handle<v8::Value> javaExceptionToV8(Java* java, JNIEnv* env, const std::string& alternateMessage);
v8::Handle<v8::Value> javaExceptionToV8(Java* java, JNIEnv* env, jthrowable ex, const std::string& alternateMessage);
std::string javaExceptionToString(JNIEnv* env, jthrowable ex);
void checkJavaException(JNIEnv* env);
v8::Handle<v8::Value> javaArrayToV8(Java* java, JNIEnv* env, jobjectArray objArray);
v8::Handle<v8::Value> javaToV8(Java* java, JNIEnv* env, jobject obj);
v8::Handle<v8::Value> javaToV8(Java* java, JNIEnv* env, jobject obj, DynamicProxyData* dynamicProxyData);
jobjectArray javaObjectArrayToClasses(JNIEnv *env, jobjectArray objs);
jobject longToJavaLongObj(JNIEnv *env, jlong l);

jclass javaFindClass(JNIEnv* env, std::string& className);
jobject javaFindField(JNIEnv* env, jclass clazz, std::string& fieldName);
jobject javaFindMethod(JNIEnv *env, jclass clazz, std::string& methodName, jobjectArray methodArgs);
jobject javaFindConstructor(JNIEnv *env, jclass clazz, jobjectArray methodArgs);
void javaCastArguments(JNIEnv *env, jobjectArray methodArgs, jobject method);

std::string methodNotFoundToString(JNIEnv *env, jclass clazz, std::string methodName, bool constructor, _NAN_METHOD_ARGS_TYPE args, int argStart, int argEnd);

void unref(DynamicProxyData* dynamicProxyData);

#define UNUSED_VARIABLE(var) var = var;

#define ARGS_FRONT_OBJECT(ARGNAME) \
  if(args.Length() < argsStart+1 || !args[argsStart]->IsObject()) {                          \
    std::ostringstream errStr;                                                               \
    errStr << "Argument " << (argsStart+1) << " must be an object";                          \
    return NanThrowError(v8::Exception::TypeError(NanNew<v8::String>(errStr.str().c_str())));   \
  }                                                                                          \
  v8::Local<v8::Object> ARGNAME = v8::Local<v8::Object>::Cast(args[argsStart]);              \
  argsStart++;

#define ARGS_FRONT_STRING(ARGNAME) \
  if(args.Length() < argsStart+1 || !args[argsStart]->IsString()) {                          \
    std::ostringstream errStr;                                                               \
    errStr << "Argument " << (argsStart+1) << " must be a string";                           \
    return NanThrowError(v8::Exception::TypeError(NanNew<v8::String>(errStr.str().c_str())));   \
  }                                                                                          \
  v8::Local<v8::String> _##ARGNAME##_obj = v8::Local<v8::String>::Cast(args[argsStart]);     \
  v8::String::Utf8Value _##ARGNAME##_val(_##ARGNAME##_obj);                                 \
  std::string ARGNAME = *_##ARGNAME##_val;                                                   \
  argsStart++;

#define ARGS_FRONT_CLASSNAME() ARGS_FRONT_STRING(className)

#define ARGS_BACK_CALLBACK()                              \
  bool callbackProvided;                                  \
  v8::Handle<v8::Value> callback;                         \
  if(args[args.Length()-1]->IsFunction()) {               \
    callback = args[argsEnd-1];                           \
    argsEnd--;                                            \
    callbackProvided = true;                              \
  } else {                                                \
    callback = NanNull();                                 \
    callbackProvided = false;                             \
  }

#define EXCEPTION_CALL_CALLBACK(JAVA, STRBUILDER) \
  std::ostringstream errStr;                                                            \
  errStr << STRBUILDER;                                                                 \
  v8::Handle<v8::Value> error = javaExceptionToV8(JAVA, env, errStr.str());             \
  v8::Handle<v8::Value> argv[2];                                                        \
  argv[0] = error;                                                                      \
  argv[1] = NanUndefined();                                                            \
  v8::Function::Cast(*callback)->Call(NanGetCurrentContext()->Global(), 2, argv);

#define END_CALLBACK_FUNCTION(MSG) \
  if(callbackProvided) {                                     \
    NanReturnUndefined();                                    \
  } else {                                                   \
    std::ostringstream str;                                  \
    str << MSG;                                              \
    NanReturnValue(NanNew<v8::String>(str.str().c_str()));      \
  }

#ifndef UNUSED_VARIABLE
  #define UNUSED_VARIABLE(a) a = a;
#endif

#endif
