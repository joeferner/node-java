# java

Bridge API to connect with existing Java APIs.

## Installation Linux

```bash
$ sudo ln -s /usr/local/share/jdk1.6.0_30/jre/lib/i386/client/libjvm.so /usr/local/lib/libjvm.so
$ export JDK_INCLUDE_DIR=/usr/local/share/jdk1.6.0_30/include/
$ export JDK_AUX_INCLUDE_DIR=/usr/local/share/jdk1.6.0_30/include/linux/
$ export JDK_LIB_DIR=/usr/local/share/jdk1.6.0_30/jre/lib/i386/client/
$ npm install java
```

## Installation Windows

* Install Python
* Download node.js source
* Run a Visual Studios command prompt
* Build node.js source by running "vcbuild.bat release"
* The directory where jvm.dll exists must be in the PATH.

```bash
$ set NODE_ROOT=C:\Program Files (x86)\nodejs
$ vcbuild.bat
```

## Installation Mac

```bash
$ npm install java
```

## Usage

```javascript
var java = require("java");
java.classpath.push("commons-lang3-3.1.jar");
java.classpath.push("commons-io.jar");

var list = java.newInstanceSync("java.util.ArrayList");

java.newInstance("java.util.ArrayList", function(err, list) {
  // you have a list
});

```

## License

(The MIT License)

Copyright (c) 2012 Near Infinity Corporation

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
