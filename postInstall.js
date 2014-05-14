var glob = require('glob');
var fs = require('fs');
var path = require('path');

require('find-java-home')(function(err, home){
  var dll;
  var so;
  var binary;

  if(home){
    dll = glob.sync('**/jvm.dll', {cwd: home})[0];
    so = glob.sync('**/libjvm.so', {cwd: home})[0];
    binary = dll || so;

    fs.writeFileSync(
      path.resolve(__dirname, './build/jvm_dll_path.json'),
      binary
      ? JSON.stringify(
          path.delimiter
          + path.dirname(path.resolve(home, binary))
        )
      : '""'
    );
  }
});
