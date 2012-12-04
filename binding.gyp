{
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
        ['OS!="win"',
          {

          }
        ]
      ]
    }
  ]
}
