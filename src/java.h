
#ifndef _node_java_h_
#define _node_java_h_

#include <v8.h>
#include <node.h>
#include <jni.h>
#include <string>

class Java : public node::ObjectWrap {
public:
  static void Init(v8::Handle<v8::Object> target);
  JavaVM* getJvm() { return m_jvm; }
  JNIEnv* getJavaEnv() { return m_env; }
  jobject getClassLoader() { return m_classLoader; }

private:
  Java();
  ~Java();
  v8::Handle<v8::Value> createJVM(JavaVM** jvm, JNIEnv** env);
  void destroyJVM(JavaVM** jvm, JNIEnv** env);

  static v8::Handle<v8::Value> New(const v8::Arguments& args);
  static v8::Handle<v8::Value> getClassLoader(const v8::Arguments& args);
  static v8::Handle<v8::Value> newInstance(const v8::Arguments& args);
  static v8::Handle<v8::Value> newInstanceSync(const v8::Arguments& args);
  static v8::Handle<v8::Value> newProxy(const v8::Arguments& args);
  static v8::Handle<v8::Value> callStaticMethod(const v8::Arguments& args);
  static v8::Handle<v8::Value> callStaticMethodSync(const v8::Arguments& args);
  static v8::Handle<v8::Value> findClassSync(const v8::Arguments& args);
  static v8::Handle<v8::Value> newArray(const v8::Arguments& args);
  static v8::Handle<v8::Value> newByte(const v8::Arguments& args);
  static v8::Handle<v8::Value> newChar(const v8::Arguments& args);
  static v8::Handle<v8::Value> newShort(const v8::Arguments& args);
  static v8::Handle<v8::Value> newFloat(const v8::Arguments& args);
  static v8::Handle<v8::Value> getStaticFieldValue(const v8::Arguments& args);
  static v8::Handle<v8::Value> setStaticFieldValue(const v8::Arguments& args);
  static v8::Handle<v8::Value> instanceof(const v8::Arguments& args);
  static v8::Handle<v8::Value> AccessorProhibitsOverwritingGetter(v8::Local<v8::String> name, const v8::AccessorInfo &info);
  static void AccessorProhibitsOverwritingSetter(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::AccessorInfo &info);
  v8::Handle<v8::Value> ensureJvm();

  static v8::Persistent<v8::FunctionTemplate> s_ct;
  JavaVM* m_jvm;
  JNIEnv* m_env;
  jobject m_classLoader;
  std::string m_classPath;
  static std::string s_nativeBindingLocation;
  v8::Persistent<v8::Array> m_classPathArray;
  v8::Persistent<v8::Array> m_optionsArray;
};

#endif
