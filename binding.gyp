{
  "variables": {
    "arch%": "amd64", # linux JVM architecture. See $(JAVA_HOME)jre/lib/<@(arch)/server/
    'conditions': [
      ['target_arch=="ia32"', {
        'arch%': "i386"
      }]
    ]
  },
  "targets": [
    {
      "target_name": "nodejavabridge_bindings",
      "sources": [
        "src/java.cpp",
        "src/javaObject.cpp",
        "src/javaScope.cpp",
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
            'actions': [
              {
                'action_name': 'verifyDeps',
                'inputs': [
                  '$(JAVA_HOME)/lib/jvm.lib',
                  '$(JAVA_HOME)/include/jni.h',
                  '$(JAVA_HOME)/include/win32/jni_md.h'
                ],
                'outputs': ['./build/depsVerified'],
                'action': ['python', 'touch.py'],
                'message': 'Verify Deps'
              }
            ],
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
            'actions': [
              {
                'action_name': 'verifyDeps',
                'inputs': [
                  '$(JAVA_HOME)jre/lib/<(arch)/server/libjvm.so',
                  '$(JAVA_HOME)/include/jni.h',
                  '$(JAVA_HOME)/include/linux/jni_md.h'
                ],
                'outputs': ['./build/depsVerified'],
                'action': [],
                'message': 'Verify Deps'
              }
            ],
            "include_dirs": [
              "$(JAVA_HOME)/include/linux",
            ],
            "libraries": [
              "-L$(JAVA_HOME)jre/lib/<(arch)/server/",
              "-Wl,-rpath,$(JAVA_HOME)jre/lib/<(arch)/server/",
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
