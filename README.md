# nodeshark

Wrapper around libwireshark providing network packet dissection for [node.js](http://nodejs.org/).

## Installation

    $ npm install nodeshark

### Build

```bash
$ svn co http://anonsvn.wireshark.org/wireshark/trunk/ wireshark
$ cd wireshark
$ ./autogen.sh
$ ./configure
$ make
$ sudo make install
$ export WIRESHARK_INCLUDE_DIR=[wireshark source directory]
$ node-waf configure build
```

## Usage

```javascript
var nodeshark = require("nodeshark");

var dissector = new nodeshark.Dissector(nodeshark.LINK_LAYER_TYPE_ETHERNET);

var rawPacket = {
  header: {
    timestampSeconds: 10,
    timestampMicroseconds: 20,
    capturedLength: buffer.length,
    originalLength: buffer.length
  },
  data: new Buffer([
    0x58, 0x6d, 0x8f, 0x67, 0x8a, 0x4d, 0x00, 0x1b, 0x21, 0xcf, 0xa1, 0x00, 0x08, 0x00, 0x45, 0x00,
    0x00, 0x3b, 0xd1, 0xb0, 0x40, 0x00, 0x40, 0x11, 0xc5, 0xde, 0x0a, 0x14, 0x08, 0x65, 0xc0, 0xa8,
    0xd0, 0x01, 0xc5, 0x32, 0x00, 0x35, 0x00, 0x27, 0xa3, 0x5b, 0x65, 0x89, 0x01, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x6d, 0x61, 0x69, 0x6c, 0x04, 0x6c, 0x69, 0x76, 0x65,
    0x03, 0x63, 0x6f, 0x6d, 0x00, 0x00, 0x01, 0x00, 0x01
  ]);
};
var packet = dissector.dissect(rawPacket);

// OR

var buffer = new Buffer([
  0x58, 0x6d, 0x8f, 0x67, 0x8a, 0x4d, 0x00, 0x1b, 0x21, 0xcf, 0xa1, 0x00, 0x08, 0x00, 0x45, 0x00,
  0x00, 0x3b, 0xd1, 0xb0, 0x40, 0x00, 0x40, 0x11, 0xc5, 0xde, 0x0a, 0x14, 0x08, 0x65, 0xc0, 0xa8,
  0xd0, 0x01, 0xc5, 0x32, 0x00, 0x35, 0x00, 0x27, 0xa3, 0x5b, 0x65, 0x89, 0x01, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x6d, 0x61, 0x69, 0x6c, 0x04, 0x6c, 0x69, 0x76, 0x65,
  0x03, 0x63, 0x6f, 0x6d, 0x00, 0x00, 0x01, 0x00, 0x01
]);
var packet = dissector.dissect(buffer);
```

You can also use it it conjunction with [pcap-parser](https://github.com/nearinfinity/node-pcap-parser).

```javascript
var pcapp = require('pcap-parser');

var parser = new pcapp.Parser('/path/to/file.pcap');
var dissector;
parser.on('globalHeader', function(globalHeader) {
  dissector = new nodeshark.Dissector(globalHeader.linkLayerType);
});
parser.on('packet', function(rawPacket) {
  var packet = dissector.dissect(rawPacket);
});
parser.parse();
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
