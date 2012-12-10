
#include "javaScope.h"

JavaScope::JavaScope(JNIEnv *env) {
  m_env = env;
  m_result = NULL;
  m_env->PushLocalFrame(LOCAL_FRAME_SIZE);
}

JavaScope::~JavaScope() {
  m_env->PopLocalFrame(m_result);
}

jobject JavaScope::Close(jobject result) {
  m_result = result;
  return m_result;
}