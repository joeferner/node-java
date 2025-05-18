const java = require("../../");
java.classpath.push("poi-3.9-20121203.jar");
java.classpath.push("poi-scratchpad-3.9-20121203.jar");

const stream = java.newInstanceSync("java.io.FileInputStream", "presentation.ppt");
const ppt = java.newInstanceSync("org.apache.poi.hslf.usermodel.SlideShow", stream);
stream.close();

const pgsize = ppt.getPageSizeSync();
console.log(`found page size ${pgsize.width}x${pgsize.height}`);

const slides = ppt.getSlidesSync();
console.log(`found ${slides.length} slides`);

const TYPE_INT_RGB = java.getStaticFieldValue("java.awt.image.BufferedImage", "TYPE_INT_RGB");

for (let i = 0; i < slides.length; i++) {
  console.log(`creating image: ${i}`);
  const img = java.newInstanceSync("java.awt.image.BufferedImage", pgsize.width, pgsize.height, TYPE_INT_RGB);
  img.createGraphicsSync();
}
console.log("done");
process.exit(0);
