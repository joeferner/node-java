{
  "variables": {
    #"arch%": "amd64"
    "arch%": "i386" # linux JVM architecture. See $(JAVA_HOME)jre/lib/<@(arch)/server/
  },
  "targets": [
    {
      "target_name": "nodejavabridge_bindings",
      "sources": [
        "src/java.cpp",
        "src/javaObject.cpp",
        "src/methodCallBaton.cpp",
        "src/nodeJavaBridge.cpp",
        "src/utils.cpp"
      ],
      "include_dirs": [
        "$(JAVA_HOME)/include",
      ],
      'conditions': [
        ['OS=="win"',
          {
            "include_dirs": [
              "$(JAVA_HOME)/include/win32",
            ],
            "libraries": [
              "-l$(JAVA_HOME)/lib/jvm.lib"
            ]
          }
        ],
        ['OS=="linux"',
          {
            "include_dirs": [
              "$(JAVA_HOME)/include/linux",
            ],
            "libraries": [
              "-L$(JAVA_HOME)jre/lib/<@(arch)/server/",
              "-Wl,-rpath,$(JAVA_HOME)jre/lib/<@(arch)/server/",
              "-ljvm"
            ]
          }
        ],
        ['OS=="mac"',
          {
            "include_dirs": [
              "/System/Library/Frameworks/JavaVM.framework/Headers",
            ],
            "libraries": [
              "-framework JavaVM"
            ]
          }
        ]
      ]
    }
  ]
}
