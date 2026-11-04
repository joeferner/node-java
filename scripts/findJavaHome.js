require("find-java-home")(function (err, home) {
  if (err || !home) {
    process.exit(1);
  }
  process.stdout.write(home);
});
