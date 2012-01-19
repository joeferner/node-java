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

  jdk_include_dir = environ.get("JDK_INCLUDE_DIR", "/usr/local/share/jdk1.6.0_30/include/")
  if jdk_include_dir:
      conf.env.append_unique('CXXFLAGS', [ '-I' + jdk_include_dir ])

  jdk_additional_include_dir = environ.get("JDK_INCLUDE_DIR", "/usr/local/share/jdk1.6.0_30/include/linux/")
  if jdk_additional_include_dir:
      conf.env.append_unique('CXXFLAGS', [ '-I' + jdk_additional_include_dir ])

  jdk_lib_dir = environ.get("JDK_LIB_DIR", "/usr/local/share/jdk1.6.0_30/jre/lib/i386/client/")
  if jdk_lib_dir:
      conf.env.append_unique('LINKFLAGS', [ '-L' + jdk_lib_dir ])

  conf.env.append_unique('LINKFLAGS', ['-ljvm'])

def build(bld):
  obj = bld.new_task_gen("cxx", "shlib", "node_addon")
  obj.target = "nodejavabridge_bindings"
  obj.source = " ".join([
    "src/nodeJavaBridge.cpp",
    "src/java.cpp"])
  obj.includes = "src/"
