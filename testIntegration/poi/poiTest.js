var java = require('../../');
java.classpath.push("poi-3.9-20121203.jar");
java.classpath.push("poi-scratchpad-3.9-20121203.jar");

var stream = java.newInstanceSync("java.io.FileInputStream", 'presentation.ppt');
var ppt = java.newInstanceSync('org.apache.poi.hslf.usermodel.SlideShow', stream);
stream.close();

var pgsize = ppt.getPageSizeSync();

var slides = ppt.getSlidesSync();

var TYPE_INT_RGB = java.getStaticFieldValue("java.awt.image.BufferedImage", "TYPE_INT_RGB");

var img, graphics;
for (i = 0; i < slides.length; i++) {
  img = java.newInstanceSync('java.awt.image.BufferedImage', pgsize.width, pgsize.height, TYPE_INT_RGB);
  graphics = img.createGraphicsSync();
}
console.log('done');