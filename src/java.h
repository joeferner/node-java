
#ifndef _node_java_h_
#define _node_java_h_

#include <v8.h>
#include <node.h>
#include <jni.h>
#include <string>
#include <nan.h>

#ifdef JNI_VERSION_1_8
#define JNI_BEST_VERSION JNI_VERSION_1_8
#else
#define JNI_BEST_VERSION JNI_VERSION_1_6
#endif

class Java : public node::ObjectWrap {
public:
  static void Init(v8::Handle<v8::Object> target);
  JavaVM* getJvm() { return m_jvm; }
  JNIEnv* getJavaEnv() { return m_env; } // can only be used safely by the main thread as this is the thread it belongs to
  jobject getClassLoader() { return m_classLoader; }

public:
  bool DoSync() const { return doSync; }
  bool DoAsync() const { return doAsync; }
  bool DoPromise() const { return doPromise; }
  std::string SyncSuffix() const { return m_SyncSuffix; }
  std::string AsyncSuffix() const { return m_AsyncSuffix; }
  std::string PromiseSuffix() const { return m_PromiseSuffix; }

private:
  Java();
  ~Java();
  v8::Local<v8::Value> createJVM(JavaVM** jvm, JNIEnv** env);
  void destroyJVM(JavaVM** jvm, JNIEnv** env);
  void configureAsync(v8::Local<v8::Value>& asyncOptions);

  static NAN_METHOD(New);
  static NAN_METHOD(getClassLoader);
  static NAN_METHOD(newInstance);
  static NAN_METHOD(newInstanceSync);
  static NAN_METHOD(newProxy);
  static NAN_METHOD(callStaticMethod);
  static NAN_METHOD(callStaticMethodSync);
  static NAN_METHOD(callMethod);
  static NAN_METHOD(callMethodSync);
  static NAN_METHOD(findClassSync);
  static NAN_METHOD(newArray);
  static NAN_METHOD(newByte);
  static NAN_METHOD(newChar);
  static NAN_METHOD(newShort);
  static NAN_METHOD(newLong);
  static NAN_METHOD(newFloat);
  static NAN_METHOD(newDouble);
  static NAN_METHOD(getStaticFieldValue);
  static NAN_METHOD(setStaticFieldValue);
  static NAN_METHOD(instanceOf);
  static NAN_GETTER(AccessorProhibitsOverwritingGetter);
  static NAN_SETTER(AccessorProhibitsOverwritingSetter);
  v8::Local<v8::Value> ensureJvm();

  static v8::Persistent<v8::FunctionTemplate> s_ct;
  JavaVM* m_jvm;
  JNIEnv* m_env; // can only be used safely by the main thread as this is the thread it belongs to
  jobject m_classLoader;
  std::string m_classPath;
  static std::string s_nativeBindingLocation;
  v8::Persistent<v8::Array> m_classPathArray;
  v8::Persistent<v8::Array> m_optionsArray;
  v8::Persistent<v8::Object> m_asyncOptions;

  std::string m_SyncSuffix;
  std::string m_AsyncSuffix;
  std::string m_PromiseSuffix;

  bool doSync;
  bool doAsync;
  bool doPromise;
};

#endif
