{
  'variables': {
    'arch%': 'amd64', # linux JVM architecture. See $(JAVA_HOME)/jre/lib/<@(arch)/server/
    'uname_m': '',
    'conditions': [
      ['target_arch=="ia32"', {
        'arch%': 'i386'
      }],
      ['OS!="win"', {
        'uname_m': '<!(uname -m)'
      }],
      ['uname_m=="s390" or uname_m=="s390x"', {
        'target_arch': 's390'
      }],
      ['OS=="win"', {
        'javahome%': '<!(node findJavaHome.js)'
      }],
      ['OS=="linux" or OS=="mac" or OS=="freebsd" or OS=="openbsd" or OS=="solaris"'  , {
        'javahome%': '<!(node findJavaHome.js)'
      }],
      ['OS=="mac"', {
      	'javaver%' : "<!(awk -F/ -v h=`node findJavaHome.js` 'BEGIN {n=split(h, a); print a[2]; exit}')"
      }],
      ['OS=="linux" or OS=="mac" or OS=="freebsd" or OS=="openbsd" or OS=="solaris"', {
        'javalibdir%': "<!(./find_java_libdir.sh <(target_arch) <(OS))"
      }],
    ]
  },
  'targets': [
    {
      'target_name': 'nodejavabridge_bindings',
      'sources': [
        'src/java.cpp',
        'src/javaObject.cpp',
        'src/javaScope.cpp',
        'src/methodCallBaton.cpp',
        'src/nodeJavaBridge.cpp',
        'src/utils.cpp'
      ],
      'include_dirs': [
        '<(javahome)/include',
        "<!(node -e \"require('nan')\")",
      ],
      'cflags': ['-O3'],
      'conditions': [
        ['OS=="win"',
          {
            'include_dirs': [
              '<(javahome)/include/win32',
            ],
            'libraries': [
              '-l<(javahome)/lib/jvm.lib'
            ]
          }
        ],
        ['OS=="linux"',
          {
            'include_dirs': [
              '<(javahome)/include/linux',
            ],
            'libraries': [
              '-L<(javalibdir)',
              '-Wl,-rpath,<(javalibdir)',
              '-ljvm'
            ]
          }
        ],
        ['OS=="solaris"',
          {
            'include_dirs': [
              '<(javahome)/include/solaris',
            ],
            'libraries': [
               '-L<(javalibdir)',
              '-Wl,-rpath,<(javalibdir)',
              '-ljvm'
            ]
          }
        ],
        ['OS=="freebsd"',
          {
            'include_dirs': [
              '<(javahome)/include/freebsd',
            ],
            'libraries': [
              '-L<(javalibdir)',
              '-Wl,-rpath,<(javalibdir)',
              '-ljvm'
            ]
          }
        ],
        ['OS=="openbsd"',
          {
            'include_dirs': [
              '<(javahome)/include/openbsd',
            ],
            'libraries': [
              '-L<(javalibdir)',
              '-Wl,-rpath,<(javalibdir)',
              '-ljvm'
            ]
          }
        ],
        ['OS=="mac"',
          {
            'xcode_settings': {
              'OTHER_CFLAGS': ['-O3'],
            },
            'conditions': [
              ['javaver=="Library"',
                {
                  'include_dirs': [
                    '<(javahome)/include',
                    '<(javahome)/include/darwin'
                  ],
                  'libraries': [
                    '-L<(javalibdir)',
                    '-Wl,-rpath,<(javalibdir)',
                    '-ljvm'
                  ],
                },
              ],
              ['javaver=="System"',
                {
                  'include_dirs': [
                    '/System/Library/Frameworks/JavaVM.framework/Headers'
                  ],
                  'libraries': [
                    '-framework JavaVM'
                  ],
                },
              ],
              ['javaver==""',
                {
                  'include_dirs': [
                    '/System/Library/Frameworks/JavaVM.framework/Headers'
                  ],
                  'libraries': [
                    '-framework JavaVM'
                  ],
                },
              ],
            ],
          },
        ],
      ]
    }
  ]
}
