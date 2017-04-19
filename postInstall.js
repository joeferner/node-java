var glob = require('glob');
var fs = require('fs');
var path = require('path');
var os = require('os');
var _ = require('lodash');
require('locate-java-home')(function (err, homes) {
  var homePath = _.get(homes, '[0].path');
  // console.log(JSON.stringify(homes, null, 2))
  // console.log('\n\n\n\n')
  // console.log(home)
  // console.log(homePath);

  var dll;
  var dylib;
  var so, soFiles;
  var binary;


  if (homePath) {
    dll = glob.sync('**/jvm.dll', { cwd: homePath })[0];
    dylib = glob.sync('**/libjvm.dylib', { cwd: homePath })[0];
    soFiles = glob.sync('**/libjvm.so', { cwd: homePath });

    if (soFiles.length > 0)
      so = getCorrectSoForPlatform(soFiles);

    binary = dll || dylib || so;

    fs.writeFileSync(
      path.resolve(__dirname, './build/jvm_dll_path.json'),
      binary
        ? JSON.stringify(
          path.delimiter
          + path.dirname(path.resolve(homePath, binary))
        )
        : '""'
    );
  }
});

function getCorrectSoForPlatform(soFiles) {
  var so = _getCorrectSoForPlatform(soFiles);
  if (so) {
    so = removeDuplicateJre(so);
  }
  return so;
}

function removeDuplicateJre(filePath) {
  while (filePath.indexOf('jre/jre') >= 0) {
    filePath = filePath.replace('jre/jre', 'jre');
  }
  return filePath;
}

function _getCorrectSoForPlatform(soFiles) {

  var architectureFolderNames = {
    'ia32': 'i386',
    'x64': 'amd64'
  };

  if (os.platform() != 'sunos')
    return soFiles[0];

  var requiredFolderName = architectureFolderNames[os.arch()];

  for (var i = 0; i < soFiles.length; i++) {
    var so = soFiles[i];

    if (so.indexOf('server') > 0)
      if (so.indexOf(requiredFolderName) > 0)
        return so;
  }

  return soFiles[0];
}
