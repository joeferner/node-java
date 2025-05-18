const glob = require("glob");
const fs = require("fs");
const path = require("path");
const os = require("os");

require("find-java-home")((err, home) => {
  if (home) {
    const dll = glob.sync("**/jvm.dll", { cwd: home })[0];
    const dylib = glob.sync("**/libjli.dylib", { cwd: home })[0];
    const soFiles = glob.sync("**/libjvm.so", { cwd: home });

    let so;
    if (soFiles.length > 0) {
      so = getCorrectSoForPlatform(soFiles);
    }

    const binary = dll ?? dylib ?? so;

    fs.writeFileSync(
      path.resolve(__dirname, "./build/jvm_dll_path.json"),
      binary ? JSON.stringify(path.delimiter + path.dirname(path.resolve(home, binary))) : '""'
    );
  }
});

function getCorrectSoForPlatform(soFiles) {
  let so = _getCorrectSoForPlatform(soFiles);
  if (so) {
    so = removeDuplicateJre(so);
  }
  return so;
}

function removeDuplicateJre(filePath) {
  while (filePath.indexOf("jre/jre") >= 0) {
    filePath = filePath.replace("jre/jre", "jre");
  }
  return filePath;
}

function _getCorrectSoForPlatform(soFiles) {
  const architectureFolderNames = {
    ia32: "i386",
    x64: "amd64",
  };

  if (os.platform() != "sunos") {
    return soFiles[0];
  }

  const requiredFolderName = architectureFolderNames[os.arch()];

  for (let i = 0; i < soFiles.length; i++) {
    const so = soFiles[i];
    if (so.indexOf("server") > 0 && so.indexOf(requiredFolderName) > 0) {
      return so;
    }
  }

  return soFiles[0];
}
