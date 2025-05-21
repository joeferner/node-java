const fs = require("node:fs");
const path = require("node:path");

const buildDir = path.join(__dirname, "..", "build", "esm");
function createEsmModulePackageJson() {
    const packageJsonFile = path.join(buildDir, "/package.json");
    fs.writeFileSync(
        packageJsonFile,
        new Uint8Array(Buffer.from('{"type": "module"}')),
        function (err) {
            if (err) {
                throw err;
            }
        }
    );
}

createEsmModulePackageJson();