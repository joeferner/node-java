
#ifndef _javaScope_h_
#define _javaScope_h_

#include <jni.h>

#define LOCAL_FRAME_SIZE 500

class JavaScope {
public:
  JavaScope(JNIEnv *env);
  ~JavaScope();
  jobject Close(jobject result);

private:
  JNIEnv *m_env;
  jobject m_result;
};

#endif
