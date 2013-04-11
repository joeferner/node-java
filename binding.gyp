{
  "variables": {
    "arch%": "amd64", # linux JVM architecture. See $(JAVA_HOME)/jre/lib/<@(arch)/server/
    'conditions': [
      ['target_arch=="ia32"', {
        'arch%': "i386"
      }],
      ['OS=="win"', {
        'javahome%': "<!(echo %JAVA_HOME%)"
      }],
      ['OS=="linux" or OS=="mac"', {
        'javahome%': "<!(echo $JAVA_HOME)"
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
        "<(javahome)/include",
      ],
      'conditions': [
        ['OS=="win"',
          {
            'actions': [
              {
                'action_name': 'verifyDeps',
                'inputs': [
                  '<(javahome)/lib/jvm.lib',
                  '<(javahome)/include/jni.h',
                  '<(javahome)/include/win32/jni_md.h'
                ],
                'outputs': ['./build/depsVerified'],
                'action': ['python', 'touch.py'],
                'message': 'Verify Deps'
              }
            ],
            "include_dirs": [
              "<(javahome)/include/win32",
            ],
            "libraries": [
              "-l<(javahome)/lib/jvm.lib"
            ]
          }
        ],
        ['OS=="linux"',
          {
            'actions': [
              {
                'action_name': 'verifyDeps',
                'inputs': [
                  '<(javahome)/jre/lib/<(arch)/server/libjvm.so',
                  '<(javahome)/include/jni.h',
                  '<(javahome)/include/linux/jni_md.h'
                ],
                'outputs': ['./build/depsVerified'],
                'action': [],
                'message': 'Verify Deps'
              }
            ],
            "include_dirs": [
              "<(javahome)/include/linux",
            ],
            "libraries": [
              "-L<(javahome)/jre/lib/<(arch)/server/",
              "-Wl,-rpath,<(javahome)/jre/lib/<(arch)/server/",
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
