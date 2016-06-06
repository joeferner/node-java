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
      ['uname_m=="ppc64" or uname_m=="ppc64le"', {
        'target_arch': 'ppc64'
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
      ['OS=="linux" and target_arch=="arm"', {
        'javalibdir%': "<!(h=\"`node findJavaHome.js`\" sh -c 'if [ -d \"$h/jre/lib/arm/classic\" ]; then echo $h/jre/arm/i386/classic; else echo $h/jre/lib/arm/server; fi')"
      }],
      ['OS=="linux" and target_arch=="ia32"', {
        'javalibdir%': "<!(h=\"`node findJavaHome.js`\" sh -c 'if [ -d \"$h/jre/lib/i386/classic\" ]; then echo $h/jre/lib/i386/classic; else echo $h/jre/lib/i386/server; fi')"
      }],
      ['OS=="linux" and target_arch=="x64"', {
        'javalibdir%': "<!(h=\"`node findJavaHome.js`\" sh -c 'if [ -d \"$h/jre/lib/amd64/classic\" ]; then echo $h/jre/lib/amd64/classic; else echo $h/jre/lib/amd64/server; fi')"
      }],
      ['OS=="linux" and (target_arch=="s390x" or target_arch=="s390")', {
        'javalibdir%': "<!(h=\"`node findJavaHome.js`\" sh -c 'if [ -d \"$h/jre/lib/s390x/classic\" ]; then echo $h/jre/lib/s390x/classic; else echo $h/jre/lib/s390/classic; fi')"
      }],
      ['OS=="linux" and (target_arch=="ppc64" or target_arch=="ppc")', {
        'javalibdir%': "<!(h=\"`node findJavaHome.js`\" sh -c 'if [ -d \"$h/jre/lib/ppc64/classic\" ]; then echo $h/jre/lib/ppc64/classic; fi')"
      }],
      ['OS=="solaris" and target_arch=="ia32"', {
        'javalibdir%': "<!(h=\"`node findJavaHome.js`\" sh -c 'if [ -d \"$h/jre/lib/i386/classic\" ]; then echo $h/jre/lib/i386/classic; else echo $h/jre/lib/i386/server; fi')"
      }],
      ['OS=="solaris" and target_arch=="x64"', {
        'javalibdir%': "<!(h=\"`node findJavaHome.js`\" sh -c 'if [ -d \"$h/jre/lib/amd64/classic\" ]; then echo $h/jre/lib/amd64/classic; else echo $h/jre/lib/amd64/server; fi')"
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
               '-L<(javahome)/jre/lib/<(arch)/server/',
              '-Wl,-rpath,<(javahome)/jre/lib/<(arch)/server/',
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
              '-L<(javahome)/jre/lib/<(arch)/server/',
              '-Wl,-rpath,<(javahome)/jre/lib/<(arch)/server/',
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
              '-L<(javahome)/jre/lib/<(arch)/server/',
              '-Wl,-rpath,<(javahome)/jre/lib/<(arch)/server/',
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
                    '-L<(javahome)/jre/lib/server',
                    '-Wl,-rpath,<(javahome)/jre/lib/server',
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
