
var java = require("../testHelpers").java;

var nodeunit = require("nodeunit");
var util = require("util");
var path = require('path');

exports['AWT'] = nodeunit.testCase({
  "calling AWT method (see issue 1)": function(test) {
    var headlessVal = java.callStaticMethodSync("java.lang.System", "getProperty", "java.awt.headless");
    console.log("java.awt.headless =", headlessVal);
    test.equal(headlessVal, 'true');
    var filename = path.join(path.dirname(__filename), "nodejs.png");
    var file = java.newInstanceSync("java.io.File", filename);
    var image = java.callStaticMethodSync("javax.imageio.ImageIO", "read", file);
    test.done();
  }
});
