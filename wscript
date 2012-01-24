import os
import Options, Utils
from os import unlink, symlink, chdir, environ
from os.path import exists

def set_options(opt):
  opt.tool_options("compiler_cxx")

def configure(conf):
  conf.check_tool("compiler_cxx")
  conf.check_tool("node_addon")

  # Enables all the warnings that are easy to avoid
  conf.env.append_unique('CXXFLAGS', ["-Wall"])
  conf.env.append_unique('CXXFLAGS', ['-Isrc/'])
  conf.env.append_unique('CXXFLAGS', ['-g'])
  conf.env.append_unique('CXXFLAGS', ['-D_FILE_OFFSET_BITS=64'])
  conf.env.append_unique('CXXFLAGS', ['-D_LARGEFILE_SOURCE'])
  conf.env.append_unique('CXXFLAGS', ['-DHAVE_CONFIG_H'])

  if os.path.exists("/System/Library/Frameworks/JavaVM.framework/"):
    jdk_include_dir = environ.get("JDK_INCLUDE_DIR", "/System/Library/Frameworks/JavaVM.framework/Headers")
    if jdk_include_dir:
      conf.env.append_unique('CXXFLAGS', [ '-I' + jdk_include_dir ])

    conf.env.append_unique('LINKFLAGS', [ '-framework', 'JavaVM' ])
  else:
    java_home = environ.get("JAVA_HOME")

    jdk_include_dir = environ.get("JDK_INCLUDE_DIR", java_home + "/include")
    if jdk_include_dir:
      conf.env.append_unique('CXXFLAGS', [ '-I' + jdk_include_dir ])

    jdk_additional_include_dir = environ.get("JDK_AUX_INCLUDE_DIR", jdk_include_dir + "/linux")
    if jdk_additional_include_dir:
      conf.env.append_unique('CXXFLAGS', [ '-I' + jdk_additional_include_dir ])

    jdk_lib_dir_guess = ""
    if os.path.exists(java_home + "/jre/lib/i386/client/"):
      jdk_lib_dir_guess = java_home + "/jre/lib/i386/server/"
    else:
      jdk_lib_dir_guess = java_home + "/jre/lib/amd64/server/"

    jdk_lib_dir = environ.get("JDK_LIB_DIR", jdk_lib_dir_guess)
    if jdk_lib_dir:
      conf.env.append_unique('LINKFLAGS', [ '-L' + jdk_lib_dir ])

    conf.env.append_unique('LINKFLAGS', ['-Wl,-rpath,' + jdk_lib_dir])
    conf.env.append_unique('LINKFLAGS', ['-ljvm'])

def build(bld):
  obj = bld.new_task_gen("cxx", "shlib", "node_addon")
  obj.target = "nodejavabridge_bindings"
  obj.source = " ".join([
    "src/nodeJavaBridge.cpp",
    "src/java.cpp",
    "src/javaObject.cpp",
    "src/methodCallBaton.cpp",
    "src/utils.cpp"])
  obj.includes = "src/"
