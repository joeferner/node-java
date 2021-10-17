require('find-java-home')(function(err, home){
  if (err || !home) {
    if (!err) err = Error('Unable to determine Java home location');
    process.exit(1);
  }
  process.stdout.write(home);
});
